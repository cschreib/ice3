#ifndef THREADING_POLICIES_HPP
#define THREADING_POLICIES_HPP

#include <lxgui/utils.hpp>
#include <memory>

namespace policies {
namespace task {
    template<typename T>
    struct argument_of
    {
        template<typename R, typename U>
        static U return_arg(R (T::*func)(U) const) {}

        typedef typename std::remove_const<
            typename std::remove_reference<
                decltype(return_arg(&T::operator()))
            >::type
        >::type type;
    };

    template<typename T>
    struct result_of
    {
        typedef typename std::remove_const<
            typename std::remove_reference<
                decltype((*(T*)nullptr)(typename argument_of<T>::type()))
            >::type
        >::type type;
    };

    struct no_output {};
    struct with_output {};

    template<typename result>
    struct detect_
    {
        typedef with_output policy;
    };

    template<>
    struct detect_<void>
    {
        typedef no_output policy;
    };

    template<typename T>
    struct detect
    {
        typedef typename detect_<typename result_of<T>::type>::policy policy;
    };
}

namespace sleep {
    struct no_sleep
    {
        void sleep_if(bool condition) {}
        void sleep() {}
    };

    void do_sleep_(size_t duration);

    template<size_t N>
    struct sleep_for
    {
        void sleep_if(bool condition) {
            if (condition)
                sleep();
        }

        void sleep() {
            do_sleep_(N);
        }
    };

    template<>
    struct sleep_for<0>
    {
        void sleep_if(bool condition) {}
        void sleep() {}
    };
}
}

#endif
