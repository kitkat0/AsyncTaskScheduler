#ifndef ASYNC_TASK_SCHEDULER_H
#define ASYNC_TASK_SCHEDULER_H

#include <functional>
#include <chrono>
#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>

class IAsyncTaskScheduler {
public:
    using TaskFunction = std::function<void()>;
    using ActionFunction = std::function<void()>;

    virtual ~IAsyncTaskScheduler() = default;

    virtual void execute_function(TaskFunction func, std::chrono::milliseconds delay) = 0;
    virtual void stop() = 0;
    virtual bool is_running() const = 0;
    virtual void wait_for_completion() = 0;

    virtual void set_on_task_start(ActionFunction action) = 0;
    virtual void set_on_task_complete(ActionFunction action) = 0;
    virtual void set_on_all_tasks_complete(ActionFunction action) = 0;

    virtual const std::string& get_name() const = 0;
};

class AsyncTaskScheduler : public IAsyncTaskScheduler {
public:
    explicit AsyncTaskScheduler(std::string name);
    ~AsyncTaskScheduler() override;

    void execute_function(TaskFunction func, std::chrono::milliseconds delay) override;
    void stop() override;
    bool is_running() const override;
    void wait_for_completion() override;

    void set_on_task_start(ActionFunction action) override;
    void set_on_task_complete(ActionFunction action) override;
    void set_on_all_tasks_complete(ActionFunction action) override;

    const std::string& get_name() const override;

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
    std::string name_;
};

class AsyncTaskSchedulerManager {
public:
    std::shared_ptr<IAsyncTaskScheduler> create(const std::string& name);
    bool remove(const std::string& name);
    bool stop(const std::string& name);
    std::shared_ptr<IAsyncTaskScheduler> get(const std::string& name);
    void stop_all();
    void remove_all();

private:
    std::unordered_map<std::string, std::shared_ptr<IAsyncTaskScheduler>> schedulers_;
    std::mutex mutex_;
};

#endif // ASYNC_TASK_SCHEDULER_H