#include "application.hpp"
#include <lxgui/utils_filesystem.hpp>
#include <lxgui/utils_string.hpp>
#include <iostream>
#include <sstream>

#include "color.hpp"

//#define USE_OIS

#ifdef USE_OIS
#include <lxgui/impl/ois_input_impl.hpp>
#else
#include <lxgui/impl/sfml_input_impl.hpp>
#endif

std::ostream& stamp(std::ostream& os)
{
    static sf::Clock c;
    double d = c.getElapsedTime().asSeconds();

    uint h = floor(d/3600.0f);
    uint m = floor(d/60.0f) - h*60;
    uint s = floor(d) - m*60 - h*3600;
    uint ms = floor(1000.0f*(d - s - m*60 - h*3600));

    std::string sh =  utils::to_string(h, 2);
    std::string sm =  utils::to_string(m, 2);
    std::string ss =  utils::to_string(s, 2);
    std::string sms = utils::to_string(ms, 4);

    return os << (sh + ":" + sm + ":" + ss + ":" + sms + " : ");
}

application::application() : mLog_("ice3.log")
{
    std::cout << stamp << "Application start." << std::endl;
}

application::log::log(const std::string& file)
{
    mLogFile_.open(file.c_str(), std::ios::out);
    pSavedBuffer_ = std::cout.rdbuf();
    std::cout.rdbuf(mLogFile_.rdbuf());
}

application::log::~log()
{
    std::cout.rdbuf(pSavedBuffer_);
}

application::~application()
{
    save_config();
    std::cout << stamp << "Application end." << std::endl;
}

void application::read_config()
{
    try { mLua_.do_file("default_config.lua"); }
    catch (lua::exception& e)
    {
        std::cout << "# Error # : " << e.get_description() << std::endl;
        throw utils::exception("application", "cannot load default config. Exiting.");
    }

    mLua_.push_global("game_options");
    mLua_.push_nil();
    while (mLua_.next())
    {
        if (mLua_.get_type() == lua::TYPE_TABLE)
        {
            if (lua_objlen(mLua_.get_state(), -1) == 3)
            {
                color c;
                mLua_.get_field(1, -1); c.r = mLua_.get_number(); mLua_.pop();
                mLua_.get_field(2, -1); c.g = mLua_.get_number(); mLua_.pop();
                mLua_.get_field(3, -1); c.b = mLua_.get_number(); mLua_.pop();
                lDefaultGameOptionList_[mLua_.get_string(-2)] = c;
            }
            else if (lua_objlen(mLua_.get_state(), -1) == 4)
            {
                color c;
                mLua_.get_field(1, -1); c.r = mLua_.get_number(); mLua_.pop();
                mLua_.get_field(2, -1); c.g = mLua_.get_number(); mLua_.pop();
                mLua_.get_field(3, -1); c.b = mLua_.get_number(); mLua_.pop();
                mLua_.get_field(4, -1); c.a = mLua_.get_number(); mLua_.pop();
                lDefaultGameOptionList_[mLua_.get_string(-2)] = c;
            }
        }
        else
            lDefaultGameOptionList_[mLua_.get_string(-2)] = mLua_.get_value();

        mLua_.pop();
    }
    mLua_.pop();

    if (utils::file_exists("saves/config.lua"))
    {
        try { mLua_.do_file("saves/config.lua"); }
        catch (lua::exception& e)
        {
            std::cout << "# Warning # : " << e.get_description() << std::endl;
        }
    }

    mLua_.push_global("game_options");
    mLua_.push_nil();
    while (mLua_.next())
    {
        if (mLua_.get_type() == lua::TYPE_TABLE)
        {
            if (lua_objlen(mLua_.get_state(), -1) == 3)
            {
                color c; c.a = 1.0f;
                mLua_.get_field(1, -1); c.r = mLua_.get_number(); mLua_.pop();
                mLua_.get_field(2, -1); c.g = mLua_.get_number(); mLua_.pop();
                mLua_.get_field(3, -1); c.b = mLua_.get_number(); mLua_.pop();
                lGameOptionList_[mLua_.get_string(-2)] = c;
            }
            else if (lua_objlen(mLua_.get_state(), -1) == 4)
            {
                color c;
                mLua_.get_field(1, -1); c.r = mLua_.get_number(); mLua_.pop();
                mLua_.get_field(2, -1); c.g = mLua_.get_number(); mLua_.pop();
                mLua_.get_field(3, -1); c.b = mLua_.get_number(); mLua_.pop();
                mLua_.get_field(4, -1); c.a = mLua_.get_number(); mLua_.pop();
                lGameOptionList_[mLua_.get_string(-2)] = c;
            }
        }
        else
            lGameOptionList_[mLua_.get_string(-2)] = mLua_.get_value();

        mLua_.pop();
    }
    mLua_.pop();

    bool bFullScreen = get_constant<bool>("fullscreen");
    if (bFullScreen)
    {
        uiScreenWidth_ = get_constant<double>("screen_width");
        uiScreenHeight_ = get_constant<double>("screen_height");
        mWindow_.create(sf::VideoMode(uiScreenWidth_, uiScreenHeight_, 32), "iCE3", sf::Style::Fullscreen);
    }
    else
    {
        uiScreenWidth_ = get_constant<double>("window_width");
        uiScreenHeight_ = get_constant<double>("window_height");
        mWindow_.create(sf::VideoMode(uiScreenWidth_, uiScreenHeight_, 32), "iCE3");
    }

    mWindow_.setVerticalSyncEnabled(get_constant<bool>("vsync"));

    sLocale_ = get_constant<std::string>("game_locale");

    pInputData_ = utils::refptr<input_data>(new input_data(
    #ifdef USE_OIS
        utils::refptr<input::handler_impl>(new input::ois_handler(
            utils::to_string((uint)mWindow_.getSystemHandle()), mWindow_.getSize().x, mWindow_.getSize().y)
        )
    #else
        utils::refptr<input::handler_impl>(new input::sfml_handler(mWindow_))
    #endif
    ));
    pInputData_->mInput.get_handler().set_manually_updated(true);
    pInputData_->fDelta = 0.1f;
    pInputData_->uiScreenWidth = uiScreenWidth_;
    pInputData_->uiScreenHeight = uiScreenHeight_;

    pInputData_->mInput.block_input("UNIT");
}

bool is_different(const lua::var& v1, const lua::var& v2)
{
    if (v1.is_of_type<std::string>() || v1.is_of_type<double>() || v1.is_of_type<bool>())
        return v1 != v2;
    else if (v1.is_of_type<color>())
        return v1.get<color>() != v2.get<color>();

    return true;
}

void application::save_config()
{
    std::ofstream mFile("saves/config.lua");
    if (mFile.is_open())
    {
        mFile << "-- Configuration overrides :" << std::endl;

        std::map<std::string, lua::var>::iterator iterChanged, iterDefault;
        foreach (iterChanged, lGameOptionList_)
        {
            iterDefault = lDefaultGameOptionList_.find(iterChanged->first);
            if (iterDefault == lDefaultGameOptionList_.end() ||
                is_different(iterDefault->second, iterChanged->second))
            {
                std::string sChanged;
                if (iterChanged->second.is_of_type<std::string>())
                    sChanged = "\""+iterChanged->second.get<std::string>()+"\"";
                else if (iterChanged->second.is_of_type<double>())
                    sChanged = utils::to_string(iterChanged->second.get<double>());
                else if (iterChanged->second.is_of_type<bool>())
                    sChanged = utils::to_string(iterChanged->second.get<bool>());
                else if (iterChanged->second.is_of_type<color>())
                {
                    color c = iterChanged->second.get<color>();
                    sChanged = "{";
                    sChanged += utils::to_string(c.r) + ", ";
                    sChanged += utils::to_string(c.g) + ", ";
                    sChanged += utils::to_string(c.b);
                    if (c.a != 1.0f)
                        sChanged + ", " + utils::to_string(c.a);
                    sChanged += "}";
                }
                else if (iterChanged->second.is_of_type<void>())
                    sChanged = "nil";
                else
                    continue;

                mFile << "game_options[\""+iterChanged->first+"\"] = "+sChanged+";" << std::endl;
            }
        }
    }
    else
        std::cout << "# Warning # : cannot save configuration to saves/config.lua." << std::endl;
}

lua::var application::get_constant(const std::string& sConstantName) const
{
    std::map<std::string, lua::var>::const_iterator iter = lGameOptionList_.find(sConstantName);
    if (iter == lGameOptionList_.end())
        return lua::var();
    return iter->second;
}

void application::set_constant(const std::string& sConstantName, const lua::var& vValue)
{
    std::map<std::string, lua::var>::iterator iter = lGameOptionList_.find(sConstantName);
    if (iter != lGameOptionList_.end())
    {
        if (iter->second == vValue)
            return;
    }

    lGameOptionList_[sConstantName] = vValue;
}

bool application::is_constant_defined(const std::string& sConstantName) const
{
    return (lGameOptionList_.find(sConstantName) != lGameOptionList_.end());
}

void application::pop_state(state& mState)
{
    mState.mark_for_deletion_();
}

void application::push_state_(std::unique_ptr<state> pState)
{
    lStateStack_.push_back(std::move(pState));
}

void application::start()
{
    sf::Clock mFPSClock;
    sf::Clock mClock;

    bool bRunning = true;
    bool bFocus = true;

    utils::wptr<input::sfml_handler> pHandler = utils::wptr<input::sfml_handler>::dyn_cast(
        pInputData_->mInput.get_handler().get_impl()
    );

    while (bRunning)
    {
        sf::Event mEvent;
        while (mWindow_.pollEvent(mEvent))
        {
            if (mEvent.type      == sf::Event::Closed)
                bRunning = false;
            else if (mEvent.type == sf::Event::LostFocus)
                bFocus = false;
            else if (mEvent.type == sf::Event::GainedFocus)
                bFocus = true;
            else if (mEvent.type == sf::Event::Resized)
            {
                pInputData_->uiScreenWidth  = uiScreenWidth_  = mWindow_.getSize().x;
                pInputData_->uiScreenHeight = uiScreenHeight_ = mWindow_.getSize().y;
                pInputData_->bScreenChanged = true;
            }

        #ifndef USE_OIS
            pHandler->on_sfml_event(mEvent);
        #endif
        }

        /*std::ostringstream ss;
        for (size_t i = 0; i < (size_t)input::key::K_MAXKEY; ++i)
        {
            if (pInputData_->mInput.key_is_pressed((input::key::code)i))
                ss << i << " : " << pInputData_->mInput.get_key_name((input::key::code)i) << ", ";
        }

        if (!ss.str().empty())
            std::cout << "keys : " << ss.str() << std::endl;*/

        if (!bFocus)
        {
            sf::sleep(sf::seconds(0.1f));
            continue;
        }

        pInputData_->fDelta = mFPSClock.getElapsedTime().asSeconds();
        mFPSClock.restart();
        pInputData_->mInput.get_handler().update();
        pInputData_->mInput.update(pInputData_->fDelta);

        // Make a temporary copy of the state stack, which we use for iteration
        // To make sure the stack we iterate on doesn't get modified by new states
        // being pushed
        std::vector<state*> lTempStack;
        for (auto& pState : lStateStack_) {
            lTempStack.push_back(pState.get());
        }

        for (auto& pState : lTempStack)
        {
            if (!pState->is_paused())
                pState->update(*pInputData_);

            if (pState->is_asking_for_shutdown())
                bRunning = false;
        }

        for (auto& pState : lTempStack)
        {
            if (pState->is_shown())
                pState->render();
        }

        // Delete states that were marked for deletion
        std::vector<std::unique_ptr<state>>::iterator iter;
        foreach (iter, lStateStack_) {
            if ((*iter)->is_marked_for_deletion_()) {
                iter = lStateStack_.erase(iter);
            }
        }

        mWindow_.display();
    }

    std::cout << stamp << "Exiting main loop." << std::endl;

    lStateStack_.clear();
}

void application::grab_mouse(bool bGrab)
{
    if (bMouseGrabbed_ == bGrab)
        return;

    if (bGrab)
        mWindow_.setMouseCursorVisible(false);
    else
        mWindow_.setMouseCursorVisible(true);

    pInputData_->mInput.get_handler().get_impl()->toggle_mouse_grab();

    bMouseGrabbed_ = bGrab;
}

bool application::is_mouse_grabbed() const
{
    return bMouseGrabbed_;
}

const std::string& application::get_locale() const
{
    return sLocale_;
}

const sf::Window& application::get_window() const
{
    return mWindow_;
}

input_data& application::get_input_data() const
{
    return *pInputData_;
}
