#ifndef WORKER_THREAD_FWD_HPP
#define WORKER_THREAD_FWD_HPP

#include "threading_policies.hpp"

template<typename T, size_t N, typename task_policy, typename sleep_policy>
class worker_thread_t;

template<typename T, size_t N = 1, typename sleep_policy = policies::sleep::no_sleep>
using worker_thread = worker_thread_t<T, N, typename policies::task::detect<T>::policy, sleep_policy>;

template<typename S>
class multitask_worker_thread;

template<typename T, size_t N, typename task_policy>
class worker_t;

template<typename T, size_t N = 1>
using worker = worker_t<T, N, typename policies::task::detect<T>::policy>;

#endif
