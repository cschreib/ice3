#ifndef SHADER_HPP
#define SHADER_HPP

#include <utils.hpp>
#include <utils_wptr.hpp>
#include <string>

class texture;

class shader
{
public :

    shader(const std::string& sVertexFile);
    shader(const std::string& sVertexFile, const std::string& sFragmentFile);
    ~shader();

    void bind();
    void unbind();

    void bind_texture(const std::string&, utils::wptr<texture> pTexture);

    static bool is_supported();

private :

    uint compile_vertex_shader_(const std::string& sFile);
    uint compile_fragment_shader_(const std::string& sFile);
    void link_program_(uint uiVertex, uint uiFragment);

    int  get_parameter_id_(const std::string& sParam);

    uint uiProgramHandle_;
    bool bUseFragment_;
    uint uiTextureBound_;
};

#endif