#include "context/executor.h"

namespace logger {
namespace ctx {
Executor::Executor() {
    executor_context_ = std::make_unique<ExecutorContext>();
}

Executor::~Executor() {
    executor_context_.reset();
}

TaskRunnerTag Executor::AddTaskRunner(const TaskRunnerTag& tag) {
    return executor_context_->AddTaskRunner(tag);
}

void Executor::PostTask(const TaskRunnerTag& runner_tag, Task task) {
    ExecutorContext::TaskRunner* task_runner = executor_context_->GetTaskRunner(runner_tag);
    task_runner->RunTask(std::move(task));
}

TaskRunnerTag Executor::ExecutorContext::AddTaskRunner(const TaskRunnerTag& tag) {
    std::lock_guard<std::mutex> lock(mutex_);

    TaskRunnerTag latest_tag = tag;
    while (task_runner_dict_.find(latest_tag) != task_runner_dict_.end()) {
        latest_tag = GetNextRunnerTag();
    }

    TaskRunnerPtr runner = std::make_unique<ThreadPool>(1);
    runner->Start();
    task_runner_dict_.emplace(latest_tag, std::move(runner));
    return latest_tag;
}

TaskRunnerTag Executor::ExecutorContext::GetNextRunnerTag() {
    static uint64_t index = 0;
    ++ index;
    return index;
}

Executor::ExecutorContext::TaskRunner* Executor::ExecutorContext::GetTaskRunner(const TaskRunnerTag& tag) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (task_runner_dict_.find(tag) == task_runner_dict_.end()) {
        return nullptr;
    }

    return task_runner_dict_[tag].get();
}

} // namespace ctx
} // namespace logger