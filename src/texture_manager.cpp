#include "texture_manager.hpp"
#include <utils_exception.hpp>

#include <png.h>
#include <fstream>
#include <iostream>

texture_manager::texture_manager()
{
}

utils::wptr<texture> texture_manager::get_texture(const std::string& sFile)
{
    std::map<std::string, utils::refptr<texture>>::iterator iter = lTextureList_.find(sFile);
    if (iter != lTextureList_.end())
        return iter->second;

    utils::refptr<texture> pTex = load_texture(sFile);
    if (!pTex)
        return nullptr;

    lTextureList_.insert(std::make_pair(sFile, pTex));

    return pTex;
}

void raise_error(png_struct* png, char const* message)
{
    throw utils::exception("# Error # : libpng : "+std::string(message));
}

void read_data(png_structp pReadStruct, png_bytep pData, png_size_t uiLength)
{
    png_voidp p = png_get_io_ptr(pReadStruct);
    ((std::ifstream*)p)->read((char*)pData, uiLength);
}

utils::refptr<texture> texture_manager::load_texture(const std::string& sFile)
{
    // Loading a PNG file, code inspired from :
    // http://www.piko3d.com/tutorials/libpng-tutorial-loading-png-files-from-streams

    std::ifstream mFile(sFile, std::ios::binary);
    if (!mFile.is_open())
    {
        std::cout << "# Warning # : texture_manager : cannot find file '" << sFile << "'." << std::endl;
        return nullptr;
    }

    const uint PNGSIGSIZE = 8;
    png_byte lSignature[PNGSIGSIZE];
    mFile.read((char*)lSignature, PNGSIGSIZE);
    if (!mFile.good() || png_sig_cmp(lSignature, 0, PNGSIGSIZE) != 0)
    {
        std::cout << "# Warning # : texture_manager : '" << sFile << "' is not a valid PNG image : '" << lSignature << "'." << std::endl;
        return nullptr;
    }

    png_structp pReadStruct = nullptr;
    png_infop pInfoStruct = nullptr;

    try
    {
        pReadStruct = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, raise_error, nullptr);
        if (!pReadStruct)
        {
            std::cout << "# Error # : texture_manager : 'png_create_read_struct' failed." << std::endl;
            return nullptr;
        }

        pInfoStruct = png_create_info_struct(pReadStruct);
        if (!pInfoStruct)
            throw utils::exception("# Error # : texture_manager : 'png_create_info_struct' failed.");

        png_set_read_fn(pReadStruct, (png_voidp)(&mFile), read_data);

        png_set_sig_bytes(pReadStruct, PNGSIGSIZE);
        png_read_info(pReadStruct, pInfoStruct);

        png_uint_32 uiDepth = png_get_bit_depth(pReadStruct, pInfoStruct);

        if (uiDepth != 8)
            throw utils::exception("# Error # : texture_manager : only 8 bit color chanels are supported for PNG images.");

        png_uint_32 uiChannels = png_get_channels(pReadStruct, pInfoStruct);

        if (uiChannels != 4 && uiChannels != 3)
            throw utils::exception("# Error # : texture_manager : only RGB or RGBA is supported for PNG images.");

        png_uint_32 uiColorType = png_get_color_type(pReadStruct, pInfoStruct);

        if (uiColorType == PNG_COLOR_TYPE_RGB)
        {
            png_set_filler(pReadStruct, 0xff, PNG_FILLER_AFTER);
            uiChannels = 4;
        }
        else if (uiColorType != PNG_COLOR_TYPE_RGBA)
            throw utils::exception("# Error # : texture_manager : only RGB or RGBA is supported for PNG images.");

        png_uint_32 uiWidth  = png_get_image_width(pReadStruct, pInfoStruct);
        png_uint_32 uiHeight = png_get_image_height(pReadStruct, pInfoStruct);

        utils::refptr<png_bytep> pRows(new png_bytep[uiHeight]);
        utils::refptr<texture>   pTex(new texture(uiWidth, uiHeight));

        png_bytep*      pTempRows = pRows.get();
        texture::color* pTempData = pTex->get_data().data();
        for (uint i = 0; i < uiHeight; ++i)
            pTempRows[i] = (png_bytep)(pTempData + i*uiWidth);

        png_read_image(pReadStruct, pTempRows);

        png_destroy_read_struct(&pReadStruct, &pInfoStruct, nullptr);

        pTex->update_texture();

        return pTex;
    }
    catch (utils::exception& e)
    {
        std::cout << e.get_description() << std::endl;

        if (pReadStruct && pInfoStruct)
            png_destroy_read_struct(&pReadStruct, &pInfoStruct, nullptr);
        else if (pReadStruct)
            png_destroy_read_struct(&pReadStruct, nullptr, nullptr);

        return nullptr;
    }
    catch (...)
    {
        if (pReadStruct && pInfoStruct)
            png_destroy_read_struct(&pReadStruct, &pInfoStruct, nullptr);
        else if (pReadStruct)
            png_destroy_read_struct(&pReadStruct, nullptr, nullptr);

        throw;
    }
}

void write_data(png_structp pWriteStruct, png_bytep pData, png_size_t uiLength)
{
    png_voidp p = png_get_io_ptr(pWriteStruct);
    ((std::ofstream*)p)->write((char*)pData, uiLength);
}

void flush_data(png_structp pWriteStruct)
{
    png_voidp p = png_get_io_ptr(pWriteStruct);
    ((std::ofstream*)p)->flush();
}

void texture_manager::save_texture(const texture& mTex, const std::string& sFile)
{
    std::ofstream mFile(sFile, std::ios::binary);
    if (!mFile.is_open())
    {
        std::cout << "# Warning # : texture_manager : cannot open file '" << sFile << "'." << std::endl;
        return;
    }

    png_structp pWriteStruct = nullptr;
    png_infop   pInfoStruct  = nullptr;

    try
    {
        pWriteStruct = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, raise_error, nullptr);
        if (!pWriteStruct)
        {
            std::cout << "# Error # : texture_manager : 'png_create_write_struct' failed." << std::endl;
            return;
        }

        pInfoStruct = png_create_info_struct(pWriteStruct);
        if (!pInfoStruct)
            throw utils::exception("# Error # : texture_manager : 'png_create_info_struct' failed.");

        png_set_write_fn(pWriteStruct, (png_voidp)(&mFile), write_data, flush_data);

        png_set_IHDR(pWriteStruct, pInfoStruct,
            mTex.get_width(), mTex.get_height(), 8,
            PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT,
            PNG_FILTER_TYPE_DEFAULT
        );

        png_write_info(pWriteStruct, pInfoStruct);

        for (uint i = 0; i < mTex.get_height(); ++i)
            png_write_row(pWriteStruct, (png_bytep)(mTex.get_data().data() + i*mTex.get_width()));

        png_write_end(pWriteStruct, pInfoStruct);

        png_destroy_write_struct(&pWriteStruct, &pInfoStruct);
    }
    catch (utils::exception& e)
    {
        std::cout << e.get_description() << std::endl;

        if (pWriteStruct && pInfoStruct)
            png_destroy_write_struct(&pWriteStruct, &pInfoStruct);
        else if (pWriteStruct)
            png_destroy_write_struct(&pWriteStruct, nullptr);

        return;
    }
    catch (...)
    {
        if (pWriteStruct && pInfoStruct)
            png_destroy_write_struct(&pWriteStruct, &pInfoStruct);
        else if (pWriteStruct)
            png_destroy_write_struct(&pWriteStruct, nullptr);

        throw;
    }
}
