#ifndef WORKER_THREAD_HPP
#define WORKER_THREAD_HPP

#include <vector>

#include "lock_free_queue.hpp"
#include "threading_policies.hpp"
#include "worker_thread_fwd.hpp"

#ifdef NO_STD_THREAD
    #pragma GCC system_header
    #include <boost/thread.hpp>
    #include <boost/functional.hpp>

    typedef boost::thread thread_t;
#else
    #include <thread>
    #include <functional>

    typedef std::thread thread_t;
#endif

template<typename T, size_t N, typename sleep_policy>
class worker_thread_t<T, N, policies::task::with_output, sleep_policy>
{
    typedef typename policies::task::argument_of<T>::type argument;
    typedef typename policies::task::result_of<T>::type   result;

    lock_free_queue<argument> in[N];
    lock_free_queue<result>   out;

    std::atomic<bool> running, waiting;

    thread_t     thread;
    sleep_policy sleeper;

    std::unique_ptr<T> worker;

public :

    worker_thread_t() : running(false), waiting(false) {}

    template<typename ... Args>
    explicit worker_thread_t(Args&&... args) : running(false), waiting(false),
        worker(new T(std::forward<Args>(args)...)) {}

    ~worker_thread_t() {
        abort();
    }

    // Disable copy
    worker_thread_t(const worker_thread_t& t) = delete;
    worker_thread_t& operator = (const worker_thread_t& t) = delete;

    // Get worker instance
    T& get_worker() {
        return *worker;
    }

    // Get worker instance
    const T& get_worker() const {
        return *worker;
    }

    // Get the number of priority queues
    size_t get_num_queues() const {
        return N;
    }

    // Order a new task
    void add_task(size_t priority, const argument& data) {
        in[priority].push(data);
    }

    // Retreive result
    bool consume(result& res) {
        return out.pop(res);
    }

    // Start working
    void start() {
        abort();
        running = true;
    #ifdef NO_STD_THREAD
        thread = boost::thread(boost::bind(&worker_thread_t::work_, this));
    #else
        thread = std::thread(&worker_thread_t::work_, this);
    #endif
    }

    // Start working with new worker
    template<typename ... Args>
    void start_new(Args&&... args) {
        abort();
        worker = std::unique_ptr<T>(new T(std::forward<Args>(args)...));
        start();
    }

    // Wait for all tasks to complete
    void wait() {
        if (thread.joinable())
        {
            waiting = true;
            thread.join();
        }
    }

    // Clear the task queue
    void abort() {
        if (thread.joinable())
        {
            running = false;
            thread.join();
        }
    }

private :

    // Do the actual work
    void work_() {
        argument data;
        size_t i;
        bool empty = true;

        while (running && !(waiting && empty))
        {
            sleeper.sleep_if(empty);

            empty = true;
            for (i = 0; i < N; ++i)
            {
                if (in[i].pop(data))
                {
                    (*worker)(data);
                    empty = false;
                    break;
                }
            }
        }

        for (i = 0; i < N; ++i)
            in[i].clear();
    }
};

template<typename T, size_t N, typename sleep_policy>
class worker_thread_t<T, N, policies::task::no_output, sleep_policy>
{
    typedef typename policies::task::argument_of<T>::type argument;

    lock_free_queue<argument> in[N];

    std::atomic<bool> running, waiting;
    bool              empty;

    thread_t     thread;
    sleep_policy sleeper;

    std::unique_ptr<T> worker;

public :

    worker_thread_t() : running(false), waiting(false), empty(true) {}

    template<typename ... Args>
    explicit worker_thread_t(Args&&... args) : running(false), waiting(false), empty(true),
        worker(new T(std::forward<Args>(args)...)) {}

    ~worker_thread_t() {
        abort();
    }

    // Disable copy
    worker_thread_t(const worker_thread_t& t) = delete;
    worker_thread_t& operator = (const worker_thread_t& t) = delete;

    // Get the number of priority queues
    size_t get_num_queues() const {
        return N;
    }

    // Get worker instance
    T& get_worker() {
        return *worker;
    }

    // Get worker instance
    const T& get_worker() const {
        return *worker;
    }

    // Order a new task
    void add_task(size_t priority, const argument& data) {
        in[priority].push(data);
        empty = false;
    }

    // Start working
    void start() {
        abort();
        running = true;
    #ifdef NO_STD_THREAD
        thread = boost::thread(boost::bind(&worker_thread_t::work_, this));
    #else
        thread = std::thread(&worker_thread_t::work_, this);
    #endif
    }

    // Start working with new task parameters
    template<typename ... Args>
    void start_new(Args&&... args) {
        abort();
        worker = std::unique_ptr<T>(new T(std::forward<Args>(args)...));
        start();
    }

    // Wait for all tasks to complete
    void wait() {
        if (thread.joinable())
        {
            waiting = true;
            thread.join();
        }
    }

    // Clear the task queue
    void abort() {
        if (thread.joinable())
        {
            running = false;
            thread.join();
        }
    }

private :

    // Do the actual work
    void work_() {
        argument data;
        size_t i;
        bool empty = true;

        while (running && !(waiting && empty))
        {
            sleeper.sleep_if(empty);

            empty = true;
            for (i = 0; i < N; ++i)
            {
                if (in[i].pop(data))
                {
                    (*worker)(data);
                    empty = false;
                    break;
                }
            }
        }

        for (i = 0; i < N; ++i)
            in[i].clear();
    }
};

template<typename sleep_policy>
class multitask_worker_thread;

class worker_base
{
template<typename sleep_policy>
friend class multitask_worker_thread;
protected :

public :

    worker_base() {}
    virtual ~worker_base() {};

    // Disable copy
    worker_base(const worker_base& w) = delete;
    worker_base& operator = (const worker_base& w) = delete;

protected :

    virtual void work_(bool& empty) = 0;
    virtual void clear_() = 0;
};

template<typename T, size_t N>
class worker_t<T, N, policies::task::with_output> : public worker_base
{
protected :

    typedef typename policies::task::argument_of<T>::type argument;
    typedef typename policies::task::result_of<T>::type   result;

    lock_free_queue<argument> in[N];
    lock_free_queue<result>   out;

    std::unique_ptr<T> worker;

public :

    worker_t() = default;
    ~worker_t() final = default;

    template <typename ... Args>
    explicit worker_t(Args&&... args) : worker(new T(std::forward<Args>(args)...)) {}

    // Get the number of priority queues
    size_t get_num_queues() const {
        return N;
    }
    // Get worker instance
    T& get_worker() {
        return *worker;
    }

    // Get worker instance
    const T& get_worker() const {
        return *worker;
    }

    // Order a new task
    void add_task(size_t priority, const argument& data) {
        in[priority].push(data);
    }

    // Retreive result
    bool consume(result& res) {
        return out.pop(res);
    }

    // Start working with new task parameters
    template<typename ... Args>
    void start_new(Args&&... args) {
        worker = std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }

protected :

    void work_(bool& empty) override final {
        argument data;
        size_t i;

        for (i = 0; i < N; ++i)
        {
            if (in[i].pop(data))
            {
                out.push((*worker)(data));
                empty = false;
                return;
            }
        }
    }

    void clear_() override final {
        for (size_t i = 0; i < N; ++i)
            in[i].clear();
    }
};

template<typename T, size_t N>
class worker_t<T, N, policies::task::no_output> : public worker_base
{
protected :

    typedef typename policies::task::argument_of<T>::type argument;

    lock_free_queue<argument> in[N];

    std::unique_ptr<T> worker;

public :

    worker_t() = default;
    ~worker_t() final = default;

    template <typename ... Args>
    explicit worker_t(Args&&... args) : worker(new T(std::forward<Args>(args)...)) {}

    // Get the number of priority queues
    size_t get_num_queues() const {
        return N;
    }

    // Get worker instance
    T& get_worker() {
        return *worker;
    }

    // Get worker instance
    const T& get_worker() const {
        return *worker;
    }

    // Order a new task
    void add_task(size_t priority, const argument& data) {
        in[priority].push(data);
    }

    // Start working with new task parameters
    template<typename ... Args>
    void start_new(Args&&... args) {
        worker = std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }

protected :

    void work_(bool& empty) override final {
        argument data;
        size_t i;

        for (i = 0; i < N; ++i)
        {
            if (in[i].pop(data))
            {
                (*worker)(data);
                empty = false;
                return;
            }
        }
    }

    void clear_() override final {
        for (size_t i = 0; i < N; ++i)
            in[i].clear();
    }
};

template<typename sleep_policy = policies::sleep::no_sleep>
class multitask_worker_thread
{
    std::vector<worker_base*> workers;

    std::atomic<bool> running, waiting;

    thread_t     thread;
    sleep_policy sleeper;

public :

    multitask_worker_thread() : running(false), waiting(false) {}

    template<typename ... Workers>
    multitask_worker_thread(Workers&... args) : running(false), waiting(false) {
        push_worker_(args...);
    }

    ~multitask_worker_thread() {
        abort();
    }

    template<typename ... Workers>
    void set_workers(Workers&... args) {
        push_worker_(args...);
    }

    void add_worker(worker_base& w) {
        push_worker_(w);
    }

    // Start working
    void start() {
        abort();
        running = true;
    #ifdef NO_STD_THREAD
        thread = boost::thread(boost::bind(&multitask_worker_thread::work_, this));
    #else
        thread = std::thread(&multitask_worker_thread::work_, this);
    #endif
    }

    // Wait for all tasks to complete
    void wait() {
        if (thread.joinable())
        {
            waiting = true;
            thread.join();
        }
    }

    // Clear the task queue
    void abort() {
        if (thread.joinable())
        {
            running = false;
            thread.join();
        }
    }

private :

    void work_() {
        bool empty = true;

        while (running && !(waiting && empty))
        {
            sleeper.sleep_if(empty);

            empty = true;
            for (auto w : workers)
                w->work_(empty);
        }

        for (auto w : workers)
            w->clear_();
    }

    template<typename ... Workers>
    void push_worker_(worker_base& worker, Workers&... args) {
        workers.push_back(&worker);
        push_worker_(args...);
    }

    void push_worker_(worker_base& worker) {
        workers.push_back(&worker);
    }
};

#endif
