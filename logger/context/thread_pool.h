#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace logger {
namespace ctx {
class ThreadPool {
public:
    explicit ThreadPool(uint32_t thread_count) {
        this->is_available_.store(false);
        this->is_shutdown_.store(false);
        this->thread_count_ = thread_count;
    }

    ThreadPool(const ThreadPool& other) = delete;
    ThreadPool& operator=(const ThreadPool& other) = delete;

    ~ThreadPool() { Stop(); }

    bool Start();

    void Stop();

    template <typename F, typename... Args>
    void RunTask(F&& f, Args&&... args) {
        if (this->is_shutdown_.load() || !this->is_available_.load()) {
            return;
        }

        auto task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        {
            std::lock_guard<std::mutex> lock(this->task_mutex_);
            this->tasks_.emplace([task]() { task(); });
        }
        this->task_cv_.notify_all();
    }

    template <typename F, typename... Args>
    auto RunRetTask(F&& f, Args&&... args)
        -> std::shared_ptr<std::future<std::result_of_t<F(Args...)>>> {
            if (this->is_shutdown_.load() || !this->is_available_.load()) {
                return nullptr;
            }

            using return_type = std::result_of_t<F(Args...)>;

            auto task = std::make_shared<std::packaged_task<return_type()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            );

            std::future<return_type> res = task->get_future();
            {
                std::lock_guard<std::mutex> lock(this->task_mutex_);
                this->tasks_.emplace([task]() { (*task)(); });
            }
            this->task_cv_.notify_all();
            return std::make_shared<std::future<std::result_of_t<F(Args...)>>>(std::move(res));
        }

private:
    void AddThread();

    using ThreadPtr = std::shared_ptr<std::thread>;
    using Task = std::function<void()>;

    struct ThreadInfo {
        ThreadInfo() = default;
        ~ThreadInfo();
        ThreadPtr ptr{nullptr};
    };
    using ThreadInfoPtr = std::shared_ptr<ThreadInfo>;

    std::vector<ThreadInfoPtr> worker_threads_;
    std::queue<Task> tasks_;

    std::mutex task_mutex_;
    std::condition_variable task_cv_;

    std::atomic<uint32_t> thread_count_;
    std::atomic<bool> is_shutdown_;
    std::atomic<bool> is_available_;
};


} // namespace ctx
} // namespace logger