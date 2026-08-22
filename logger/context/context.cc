#include "context/context.h"

namespace logger {
namespace ctx {

class ExecutorManager {
public:
    ExecutorManager() { executor_ = std::make_unique<Executor>(); }

    ~ExecutorManager() { executor_.reset(); }

    Executor* GetExecutor() { return executor_.get(); }

    TaskRunnerTag NewTaskRunner(TaskRunnerTag tag) { return executor_->AddTaskRunner(tag); }

private:
    std::unique_ptr<Executor> executor_;
};

Context::Context() : executor_manager_(std::make_unique<ExecutorManager>()) {}

Executor* Context::GetExecutor() {
    return executor_manager_->GetExecutor();
}

TaskRunnerTag Context::NewTaskRunner(TaskRunnerTag tag) {
    return executor_manager_->NewTaskRunner(tag);
}

} // namespace ctx
} // namespace logger

