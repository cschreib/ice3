#ifndef SHADER_HPP
#define SHADER_HPP

#include <lxgui/utils.hpp>
#include <lxgui/utils_wptr.hpp>
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

    uint uiProgramHandle_ = 0;
    bool bUseFragment_ = false;
    uint uiTextureBound_ = 0;
};

#endif
