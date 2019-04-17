#include "state.hpp"

state::state(application& mApp, bool bPlay, bool bShow) :
    bPaused_(!bPlay), bHidden_(!bShow), bShutdown_(false)
{
}

state::~state()
{
}

void state::set_self(utils::wptr<state> pSelf)
{
    pSelf_ = pSelf;
}

void state::pause()
{
    bPaused_ = true;
}

void state::play()
{
    bPaused_ = false;
}

bool state::is_paused() const
{
    return bPaused_;
}

void state::hide()
{
    bHidden_ = true;
}

void state::show()
{
    bHidden_ = false;
}

bool state::is_shown() const
{
    return !bHidden_;
}

void state::ask_shutdown()
{
    bShutdown_ = true;
}

bool state::is_asking_for_shutdown() const
{
    return bShutdown_;
}
