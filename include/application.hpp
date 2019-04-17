#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <utils.hpp>
#include <utils_refptr.hpp>
#include <utils_wptr.hpp>
#include <luapp_var.hpp>
#include <luapp_state.hpp>
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

    void           pop_state(utils::wptr<state> pState);

    template<class T, typename ... Args>
    utils::wptr<T> push_state(Args... args)
    {
        utils::refptr<T> p(new T(*this, args...));
        p->set_self(p);
        push_state_(p);
        return p;
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

    void push_state_(utils::refptr<state> pState);

    std::map<std::string, lua::var> lDefaultGameOptionList_;
    std::map<std::string, lua::var> lGameOptionList_;

    log         mLog_;
    lua::state  mLua_;
    std::string sLocale_;

    uint uiScreenWidth_;
    uint uiScreenHeight_;

    mutable utils::refptr<input_data> pInputData_;

    std::deque<utils::refptr<state>> lStateStack_;

    sf::Window mWindow_;

    bool bMouseGrabbed_;
};

#endif