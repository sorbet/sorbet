#ifndef SRUBY_THREADQUEUE_H
#define SRUBY_THREADQUEUE_H

#include <condition_variable>
#include <list>
#include <mutex>

namespace ruby_typer {

template <class T> class ThreadQueue {
public:
    ThreadQueue() : closed_(false) {}

    void push(T &val) {
        std::unique_lock<std::mutex> locked(mutex_);
        queue_.push_back(val);
        cond_.notify_one();
    }

    void push(T &&val) {
        std::unique_lock<std::mutex> locked(mutex_);
        queue_.push_back(std::move(val));
        cond_.notify_one();
    }

    void close() {
        std::unique_lock<std::mutex> locked(mutex_);
        closed_ = true;
        cond_.notify_all();
    }

    bool pop(T *out) {
        std::unique_lock<std::mutex> locked(mutex_);
        while (queue_.empty() && !closed_)
            cond_.wait(locked);
        if (queue_.empty() && closed_)
            return false;
        *out = std::move(queue_.front());
        queue_.pop_front();
        return true;
    }

    bool try_pop(T *out) {
        std::unique_lock<std::mutex> locked(mutex_);
        if (queue_.empty())
            return false;
        *out = std::move(queue_.front());
        queue_.pop_front();
        return true;
    }

    ThreadQueue(const ThreadQueue &) = delete;
    ThreadQueue operator=(const ThreadQueue &) = delete;

protected:
    std::mutex mutex_;
    std::condition_variable cond_;
    bool closed_;
    std::list<T> queue_;
};

} // namespace ruby_typer
#endif // SRUBY_THREADQUEUE_H
