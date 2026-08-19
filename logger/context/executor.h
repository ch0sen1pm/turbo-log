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
private:
    std::unique_ptr<ExecutorContext> executor_context_;
};


} // namespace ctx
} // namespace logger