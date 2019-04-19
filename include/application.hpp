#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <lxgui/utils.hpp>
#include <lxgui/utils_refptr.hpp>
#include <lxgui/utils_wptr.hpp>
#include <lxgui/luapp_var.hpp>
#include <lxgui/luapp_state.hpp>
#include <map>
#include <string>
#include <deque>
#include <fstream>
#include <SFML/Window.hpp>
#include "state.hpp"

std::ostream& stamp(std::ostream&);

class application
{
public :

    application();
    ~application();

    application(const application&) = delete;
    application& operator = (const application&) = delete;

    void     read_config();
    void     save_config();
    lua::var get_constant(const std::string& sConstantName) const;
    void     set_constant(const std::string& sConstantName, const lua::var& vValue);
    bool     is_constant_defined(const std::string& sConstantName) const;
    template<class T>
    T        get_constant(const std::string& sConstantName) const
    {
        std::map<std::string, lua::var>::const_iterator iter = lGameOptionList_.find(sConstantName);
        if (iter == lGameOptionList_.end())
            return T();
        return iter->second.get<T>();
    }

    void           pop_state(state& pState);

    template<class T, typename ... Args>
    T& push_state(Args... args)
    {
        std::unique_ptr<T> p(new T(*this, args...));
        T& obj = *p;
        push_state_(std::move(p));
        return obj;
    }

    void start();

    void grab_mouse(bool bGrab);
    bool is_mouse_grabbed() const;

    const std::string& get_locale() const;
    const sf::Window& get_window() const;
    input_data& get_input_data() const;

private :

    class log
    {
    public :

        log(const std::string& file);
        ~log();

    private :

        std::ofstream   mLogFile_;
        std::streambuf* pSavedBuffer_;
    };

    void push_state_(std::unique_ptr<state> pState);

    std::map<std::string, lua::var> lDefaultGameOptionList_;
    std::map<std::string, lua::var> lGameOptionList_;

    log         mLog_;
    lua::state  mLua_;
    std::string sLocale_;

    uint uiScreenWidth_ = 0;
    uint uiScreenHeight_ = 0;

    mutable utils::refptr<input_data> pInputData_;

    std::vector<std::unique_ptr<state>> lStateStack_;

    sf::Window mWindow_;

    bool bMouseGrabbed_ = false;
};

#endif
