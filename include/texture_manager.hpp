#ifndef TEXTUREMANAGER_HPP
#define TEXTUREMANAGER_HPP

#include <string>
#include <map>
#include <memory>
#include <lxgui/utils_refptr.hpp>
#include <lxgui/utils_wptr.hpp>
#include "texture.hpp"

class texture_manager
{
public :

    texture_manager();

    texture_manager(const texture_manager&) = delete;
    texture_manager& operator = (const texture_manager&) = delete;

    texture* get_texture(const std::string& sFile);

    static std::unique_ptr<texture> load_texture(const std::string& sFile);
    static void                     save_texture(const texture& mTex, const std::string& sFile);

private:

    std::map<std::string, std::unique_ptr<texture>> lTextureList_;

};

#endif
