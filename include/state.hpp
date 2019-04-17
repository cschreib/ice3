#ifndef STATE_HPP
#define STATE_HPP

#include <utils.hpp>
#include <utils_wptr.hpp>
#include <input.hpp>

struct input_data
{
    input_data(utils::refptr<input::handler_impl> handler) :
        mInput(handler), fDelta(0.1), uiScreenWidth(128), uiScreenHeight(128),
        bScreenChanged(false)
    {}

    input::manager mInput;
    double fDelta;

    uint uiScreenWidth;
    uint uiScreenHeight;
    bool bScreenChanged;
};

class application;

class state
{
public :

    state(application& mApp, bool bPlay, bool bShow);
    virtual ~state();

    state(const state& mState) = delete;
    state& operator = (const state& mState) = delete;

    virtual void set_self(utils::wptr<state> pSelf);

    virtual void pause();
    virtual void play();
    bool         is_paused() const;
    virtual void update(input_data& mData) = 0;

    virtual void hide();
    virtual void show();
    bool         is_shown() const;
    virtual void render() = 0;

    virtual void ask_shutdown();
    bool         is_asking_for_shutdown() const;

protected :

    utils::wptr<state> pSelf_;

    bool bPaused_;
    bool bHidden_;
    bool bShutdown_;
};

#endif
