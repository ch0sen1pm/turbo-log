#pragma once

#include "context/executor.h"

#include <memory>

namespace logger {
namespace ctx {

class ExecutorManager;
class Context {
public:
    Executor* GetExecutor();

    static Context* GetInstance() {
        static Context* instance = new Context();
        return instance;
    }

    TaskRunnerTag NewTaskRunner(TaskRunnerTag tag);

    Context(const Context& other) = delete;
    Context& operator=(const Context& other) = delete;

private:
    Context();

    std::unique_ptr<ExecutorManager> executor_manager_;
};

} // namespace ctx
} // namespace logger


#define CONTEXT logger::ctx::Context::GetInstance()

#define EXECUTOR CONTEXT->GetExecutor()

#define NEW_TASK_RUNNER(tag) CONTEXT->NewTaskRunner(tag)

#define POST_TASK(runner_tag, task) EXECUTOR->PostTask(runner_tag, task)

#define WAIT_TASK_IDLE(runner_tag) EXECUTOR->PostTaskAndGetResult(runner_tag, []() {})->wait()

#define POST_REPEATED_TASK(runner_tag, task, delta, repeat_num) \
    EXECUTOR->PostRepeatedTask(runner_tag, task, delta, repeat_num)