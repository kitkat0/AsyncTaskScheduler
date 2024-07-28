#include "async_task_scheduler.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>

struct ScheduledTask {
    IAsyncTaskScheduler::TaskFunction func;
    std::chrono::steady_clock::time_point execute_time;

    bool operator>(const ScheduledTask& other) const {
        return execute_time > other.execute_time;
    }
};

class AsyncTaskScheduler::Impl {
public:
    explicit Impl(const std::string& name) : running_(true), pending_tasks_(0), name_(name) {
        worker_thread_ = std::thread(&Impl::run, this);
    }

    ~Impl() {
        stop();
    }

    void execute_function(IAsyncTaskScheduler::TaskFunction func, std::chrono::milliseconds delay) {
        auto execute_time = std::chrono::steady_clock::now() + delay;
        ScheduledTask task{ std::move(func), execute_time };

        {
            std::lock_guard<std::mutex> lock(mutex_);
            tasks_.push(std::move(task));
            pending_tasks_++;
        }
        cv_.notify_one();
    }

    void stop() {
        running_ = false;
        cv_.notify_one();
        if (worker_thread_.joinable()) {
            worker_thread_.join();
        }
    }

    bool is_running() const {
        return running_;
    }

    void wait_for_completion() {
        std::unique_lock<std::mutex> lock(mutex_);
        completion_cv_.wait(lock, [this] { return pending_tasks_ == 0; });
    }

    void set_on_task_start(IAsyncTaskScheduler::ActionFunction action) {
        on_task_start_ = std::move(action);
    }

    void set_on_task_complete(IAsyncTaskScheduler::ActionFunction action) {
        on_task_complete_ = std::move(action);
    }

    void set_on_all_tasks_complete(IAsyncTaskScheduler::ActionFunction action) {
        on_all_tasks_complete_ = std::move(action);
    }

    const std::string& get_name() const {
        return name_;
    }

private:
    void run() {
        while (running_) {
            std::unique_lock<std::mutex> lock(mutex_);

            if (tasks_.empty()) {
                cv_.wait(lock, [this] { return !running_ || !tasks_.empty(); });
            }
            else {
                auto now = std::chrono::steady_clock::now();
                if (tasks_.top().execute_time <= now) {
                    auto task = std::move(tasks_.top());
                    tasks_.pop();
                    lock.unlock();

                    if (on_task_start_) on_task_start_();
                    task.func();
                    if (on_task_complete_) on_task_complete_();

                    lock.lock();
                    pending_tasks_--;
                    if (pending_tasks_ == 0) {
                        if (on_all_tasks_complete_) {
                            lock.unlock();
                            on_all_tasks_complete_();
                            lock.lock();
                        }
                        completion_cv_.notify_all();
                    }
                }
                else {
                    cv_.wait_until(lock, tasks_.top().execute_time);
                }
            }
        }
    }

    std::atomic<bool> running_;
    std::atomic<int> pending_tasks_;
    std::thread worker_thread_;
    std::priority_queue<ScheduledTask, std::vector<ScheduledTask>, std::greater<ScheduledTask>> tasks_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::condition_variable completion_cv_;
    IAsyncTaskScheduler::ActionFunction on_task_start_;
    IAsyncTaskScheduler::ActionFunction on_task_complete_;
    IAsyncTaskScheduler::ActionFunction on_all_tasks_complete_;
    std::string name_;
};

AsyncTaskScheduler::AsyncTaskScheduler(std::string name)
    : pimpl_(std::make_unique<Impl>(name)), name_(std::move(name)) {}

AsyncTaskScheduler::~AsyncTaskScheduler() = default;

void AsyncTaskScheduler::execute_function(TaskFunction func, std::chrono::milliseconds delay) {
    pimpl_->execute_function(std::move(func), delay);
}

void AsyncTaskScheduler::stop() {
    pimpl_->stop();
}

bool AsyncTaskScheduler::is_running() const {
    return pimpl_->is_running();
}

void AsyncTaskScheduler::wait_for_completion() {
    pimpl_->wait_for_completion();
}

void AsyncTaskScheduler::set_on_task_start(ActionFunction action) {
    pimpl_->set_on_task_start(std::move(action));
}

void AsyncTaskScheduler::set_on_task_complete(ActionFunction action) {
    pimpl_->set_on_task_complete(std::move(action));
}

void AsyncTaskScheduler::set_on_all_tasks_complete(ActionFunction action) {
    pimpl_->set_on_all_tasks_complete(std::move(action));
}

const std::string& AsyncTaskScheduler::get_name() const {
    return name_;
}

// AsyncTaskSchedulerManager implementation
std::shared_ptr<IAsyncTaskScheduler> AsyncTaskSchedulerManager::create(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = schedulers_.find(name);
    if (it != schedulers_.end()) {
        return it->second;  // Return existing scheduler if it already exists
    }

    auto scheduler = std::make_shared<AsyncTaskScheduler>(name);
    schedulers_[name] = scheduler;
    return scheduler;
}

bool AsyncTaskSchedulerManager::remove(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = schedulers_.find(name);
    if (it != schedulers_.end()) {
        it->second->stop();
        schedulers_.erase(it);
        return true;
    }
    return false;
}

bool AsyncTaskSchedulerManager::stop(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = schedulers_.find(name);
    if (it != schedulers_.end()) {
        it->second->stop();
        return true;
    }
    return false;
}

std::shared_ptr<IAsyncTaskScheduler> AsyncTaskSchedulerManager::get(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = schedulers_.find(name);
    if (it != schedulers_.end()) {
        return it->second;
    }
    return nullptr;
}

void AsyncTaskSchedulerManager::stop_all() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& pair : schedulers_) {
        pair.second->stop();
    }
}

void AsyncTaskSchedulerManager::remove_all() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& pair : schedulers_) {
        pair.second->stop();
    }
    schedulers_.clear();
}