#include "threading_policies.hpp"

#ifdef NO_STD_THREAD
    #pragma GCC system_header
    #include <boost/thread.hpp>
#else
    #include <thread>
#endif

namespace policies {
namespace sleep {

void do_sleep_(size_t duration)
{
#ifdef NO_STD_THREAD
    boost::this_thread::sleep(boost::posix_time::milliseconds(duration));
#else
    std::this_thread::sleep_for(std::chrono::milliseconds(duration));
#endif
}

}
}
