#pragma once

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <queue>

#include "context/thread_pool.h"

namespace logger {
namespace ctx {

using Task = std::function<void(void)>;
using TaskRunnerTag = uint64_t;
using RepeatedTaskId = uint64_t;

class Executor {
public:
    Executor();
    ~Executor();

    Executor(const Executor& other) = delete;
    Executor& operator=(const Executor& other) = delete;

    TaskRunnerTag AddTaskRunner(const TaskRunnerTag& tag);

    void PostTask(const TaskRunnerTag& runner_tag, Task task);

    template <typename F, typename... Args>
    auto PostTaskAndGetResult(const TaskRunnerTag& runner_tag, F&& f, Args&&... args)
        -> std::shared_ptr<std::future<std::result_of_t<F(Args...)>>> {
            ExecutorContext::TaskRunner* task_runner = executor_context_->GetTaskRunner(runner_tag);
            auto ret = task_runner->RunRetTask(std::forward<F>(f), std::forward<Args>(args)...);
            return ret;
        }
    
    template <typename R, typename P>
    void PostDelayedTask(const TaskRunnerTag& runner_tag, Task task,
                         const std::chrono::duration<R, P>& delta) {
        Task func = std::bind(&Executor::PostTask, this, runner_tag, std::move(task));

        executor_timer_->Start();
        executor_timer_->PostDelayedTask(std::move(func),
                                         std::chrono::duration_cast<std::chrono::microseconds>(delta));
    }

    template <typename R, typename P>
    RepeatedTaskId PostRepeatedTask(const TaskRunnerTag& runner_tag, Task task,
                                    const std::chrono::duration<R, P>& delta, uint64_t repeat_num) {
        Task func = std::bind(&Executor::PostTask, this, runner_tag, std::move(task));
        executor_timer_->Start();
        return executor_timer_->PostRepeatedTask(std::move(func),
                                                 std::chrono::duration_cast<std::chrono::microseconds>(delta),
                                                 repeat_num);
    }

    void CancelRepeatedTask(RepeatedTaskId task_id) { executor_timer_->CancelRepeatedTask(task_id); }
    
private:
    class ExecutorContext {
    public:
        ExecutorContext() = default;
        ~ExecutorContext() = default;

        ExecutorContext(const ExecutorContext&) = delete;
        ExecutorContext& operator=(const ExecutorContext&) = delete;

        TaskRunnerTag AddTaskRunner(const TaskRunnerTag& tag);
    private:
        using TaskRunner = ThreadPool;
        using TaskRunnerPtr = std::unique_ptr<TaskRunner>;
        friend class Executor;

        TaskRunner* GetTaskRunner(const TaskRunnerTag& tag);
        TaskRunnerTag GetNextRunnerTag();

        std::unordered_map<TaskRunnerTag, TaskRunnerPtr> task_runner_dict_;
        std::mutex mutex_;
    };

    class ExecutorTimer {
    public:
        struct InternalS {
            std::chrono::time_point<std::chrono::high_resolution_clock> time_point;
            Task task;
            RepeatedTaskId repeated_id;

            bool operator<(const InternalS& b) const { return time_point > b.time_point; }
        };

        ExecutorTimer();
        ~ExecutorTimer();

        ExecutorTimer(const ExecutorTimer& other) = delete;
        ExecutorTimer& operator=(const ExecutorTimer& other) = delete;

        bool Start();
        void Stop();

        void PostDelayedTask(Task task, const std::chrono::microseconds& delta);
        RepeatedTaskId PostRepeatedTask(Task task, const std::chrono::microseconds& delta, uint64_t repeat_num);
        void CancelRepeatedTask(RepeatedTaskId task_id);

    private:
        void Run_();
        void PostRepeatedTask_(Task task, const std::chrono::microseconds& delta,
                               RepeatedTaskId repeated_task_id, uint64_t repeated_num);
        void PostTask_(Task task, std::chrono::microseconds delta,
                       RepeatedTaskId repeated_task_id, uint64_t repeated_num);

        RepeatedTaskId GetNextRepeatedTaskId() { return repeated_task_id_ ++; }

        std::priority_queue<InternalS> queue_;
        std::mutex mutex_;
        std::condition_variable cond_;
        std::atomic<bool> running_;
        std::unique_ptr<ThreadPool> thread_pool_;
        
        std::atomic<RepeatedTaskId> repeated_task_id_;
        std::unordered_set<RepeatedTaskId> repeated_id_state_set_;
    };
private:
    std::unique_ptr<ExecutorContext> executor_context_;
    std::unique_ptr<ExecutorTimer> executor_timer_;
};


} // namespace ctx
} // namespace logger