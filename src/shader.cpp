#include "shader.hpp"
#include "texture.hpp"
#include "glew.hpp"

#include <lxgui/utils_filesystem.hpp>
#include <lxgui/utils_exception.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

shader::shader(const std::string& sVertexFile)
{
    if (!utils::file_exists(sVertexFile))
        throw utils::exception("shader", "Cannot find file : '"+sVertexFile+"'.");

    glew::initialize();

    uint uiVertexHandle = compile_vertex_shader_(sVertexFile);

    link_program_(uiVertexHandle, 0);
}

shader::shader(const std::string& sVertexFile, const std::string& sFragmentFile)
{
    if (!utils::file_exists(sVertexFile))
        throw utils::exception("shader", "cannot find file : '"+sVertexFile+"'.");

    if (!utils::file_exists(sFragmentFile))
        throw utils::exception("shader", "cannot find file : '"+sFragmentFile+"'.");

    glew::initialize();

    bUseFragment_ = true;
    uint uiVertexHandle = compile_vertex_shader_(sVertexFile);
    uint uiFragmentHandle = 0;
    try
    {
        uiFragmentHandle = compile_fragment_shader_(sFragmentFile);
    }
    catch (utils::exception& e)
    {
        glDeleteObjectARB(uiVertexHandle);
        throw;
    }

    link_program_(uiVertexHandle, uiFragmentHandle);
}

shader::~shader()
{
    if (uiProgramHandle_ != 0)
        glDeleteObjectARB(uiProgramHandle_);
}

void shader::bind()
{
    glUseProgramObjectARB(uiProgramHandle_);
    uiTextureBound_ = 0;
}

void shader::unbind()
{
    glUseProgramObjectARB(0);
    glActiveTexture(GL_TEXTURE0);
}

void shader::bind_texture(const std::string& sName, const texture& mTexture)
{
    int iID = get_parameter_id_(sName);
    if (iID == -1)
        return;

    glActiveTexture(GL_TEXTURE0 + uiTextureBound_);

    mTexture.bind();

    glUniform1iARB(iID, uiTextureBound_);

    ++uiTextureBound_;
}

int shader::get_parameter_id_(const std::string& sName)
{
    int iID = glGetUniformLocationARB(uiProgramHandle_, sName.c_str());
    if (iID == -1)
        std::cout << "# Warning # : shader : cannot find parameter '" << sName << "'." << std::endl;

    return iID;
}

uint shader::compile_vertex_shader_(const std::string& sFile)
{
    std::ifstream mFile(sFile);
    mFile.seekg(0, std::ios::end);
    uint uiLength = mFile.tellg();
    mFile.seekg(0);
    char* sSource = new char[uiLength+1];
    mFile.read(sSource, uiLength);
    mFile.close();
    sSource[uiLength] = 0;

    uint uiVertexHandle_ = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
    glShaderSourceARB(uiVertexHandle_, 1, (const GLcharARB**)&sSource, nullptr);
    glCompileShaderARB(uiVertexHandle_);

    GLint bSuccess;
    glGetObjectParameterivARB(uiVertexHandle_, GL_OBJECT_COMPILE_STATUS_ARB, &bSuccess);
    if (bSuccess == GL_FALSE)
    {
        char sError[1024];
        glGetInfoLogARB(uiVertexHandle_, sizeof(sError), 0, sError);
        glDeleteObjectARB(uiVertexHandle_);

        throw utils::exception("vertex_shader", "compiler error :\n"+std::string(sError));
    }

    return uiVertexHandle_;
}

uint shader::compile_fragment_shader_(const std::string& sFile)
{
    std::ifstream mFile(sFile);
    mFile.seekg(0, std::ios::end);
    uint uiLength = mFile.tellg();
    mFile.seekg(0);
    char* sSource = new char[uiLength+1];
    mFile.read(sSource, uiLength);
    mFile.close();
    sSource[uiLength] = 0;

    uint uiFragmentHandle_ = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
    glShaderSourceARB(uiFragmentHandle_, 1, (const GLcharARB**)&sSource, nullptr);
    glCompileShaderARB(uiFragmentHandle_);

    GLint bSuccess;
    glGetObjectParameterivARB(uiFragmentHandle_, GL_OBJECT_COMPILE_STATUS_ARB, &bSuccess);
    if (bSuccess == GL_FALSE)
    {
        char sError[1024];
        glGetInfoLogARB(uiFragmentHandle_, sizeof(sError), 0, sError);
        glDeleteObjectARB(uiFragmentHandle_);

        throw utils::exception("fragment_shader", "compiler error :\n"+std::string(sError));
    }

    return uiFragmentHandle_;
}

void shader::link_program_(uint uiVertex, uint uiFragment)
{
    uiProgramHandle_ = glCreateProgramObjectARB();
    glAttachObjectARB(uiProgramHandle_, uiVertex);
    glDeleteObjectARB(uiVertex);

    if (uiFragment)
    {
        glAttachObjectARB(uiProgramHandle_, uiFragment);
        glDeleteObjectARB(uiFragment);
    }

    glLinkProgramARB(uiProgramHandle_);
    GLint bSuccess;
    glGetObjectParameterivARB(uiProgramHandle_, GL_OBJECT_LINK_STATUS_ARB, &bSuccess);
    if (bSuccess == GL_FALSE)
    {
        // Oops... link errors
        char sError[1024];
        glGetInfoLogARB(uiProgramHandle_, sizeof(sError), 0, sError);
        glDeleteObjectARB(uiProgramHandle_);

        throw utils::exception("shader", "linker error :\n"+std::string(sError));
    }
}

bool shader::is_supported()
{
    glew::initialize();

    return (GLEW_ARB_shading_language_100 &&
            GLEW_ARB_shader_objects       &&
            GLEW_ARB_vertex_shader        &&
            GLEW_ARB_fragment_shader);
}
