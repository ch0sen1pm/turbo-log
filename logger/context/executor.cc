#include "context/executor.h"

namespace logger {
namespace ctx {
Executor::Executor() {
    executor_context_ = std::make_unique<ExecutorContext>();
    executor_timer_ = std::make_unique<ExecutorTimer>();
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

Executor::ExecutorTimer::ExecutorTimer() {
    thread_pool_ = std::make_unique<ThreadPool>(1);
    repeated_task_id_.store(0);
    running_.store(false);
}

Executor::ExecutorTimer::~ExecutorTimer() {
    Stop();
}

bool Executor::ExecutorTimer::Start() {
    if (running_) {
        return true;
    }
    running_.store(true);
    bool ret = thread_pool_->Start();
    thread_pool_->RunTask(&Executor::ExecutorTimer::Run_, this);
    return ret;
}

void Executor::ExecutorTimer::Stop() {
    running_.store(false);
    cond_.notify_all();
    thread_pool_.reset();
}

void Executor::ExecutorTimer::Run_() {
    while (running_.load()) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            cond_.wait(lock);
            continue;
        }

        auto s = queue_.top();
        auto diff = s.time_point - std::chrono::high_resolution_clock::now();

        if (std::chrono::duration_cast<std::chrono::microseconds>(diff).count() > 0) {
            cond_.wait_for(lock, diff);
            continue;
        } else {
            queue_.pop();
            lock.unlock();
            s.task();
        }
    }
}

void Executor::ExecutorTimer::PostDelayedTask(Task task, const std::chrono::microseconds& delta) {
    InternalS s;
    s.time_point = std::chrono::high_resolution_clock::now() + delta;
    s.task = std::move(task);
    {
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.push(s);
        cond_.notify_all();
    }
}

RepeatedTaskId Executor::ExecutorTimer::PostRepeatedTask(Task task,
                                                         const std::chrono::microseconds& delta,
                                                         uint64_t repeat_num) {
    RepeatedTaskId id = GetNextRepeatedTaskId();
    repeated_id_state_set_.insert(id);
    PostRepeatedTask_(std::move(task), delta, id, repeat_num);
    return id;
}

void Executor::ExecutorTimer::CancelRepeatedTask(RepeatedTaskId task_id) {
    repeated_id_state_set_.erase(task_id);
}

void Executor::ExecutorTimer::PostTask_(Task task,
                                        std::chrono::microseconds delta,
                                        RepeatedTaskId repeated_task_id,
                                        uint64_t repeate_num) {
    PostRepeatedTask_(std::move(task), delta, repeated_task_id, repeate_num);
}

void Executor::ExecutorTimer::PostRepeatedTask_(Task task,
                                                const std::chrono::microseconds& delta,
                                                RepeatedTaskId repeated_task_id,
                                                uint64_t repeat_num) {
    if (repeated_id_state_set_.find(repeated_task_id) == repeated_id_state_set_.end() || repeat_num == 0) {
        return;
    }
    task();

    Task func = std::bind(&Executor::ExecutorTimer::PostTask_, this,
                          std::move(task), delta, repeated_task_id, repeat_num - 1);
    
    InternalS s;
    s.time_point = std::chrono::high_resolution_clock::now() + delta;
    s.repeated_id = repeated_task_id;
    s.task = std::move(func);

    {
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.push(s);
        cond_.notify_all();
    }
}


} // namespace ctx
} // namespace logger