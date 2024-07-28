# AsyncTaskScheduler

![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![Version](https://img.shields.io/badge/version-1.0.0-orange.svg)

Efficient and flexible asynchronous task management for C++.

## Key Features

- Multiple named schedulers
- Delayed task execution
- Event callbacks
- Thread-safe operations

## Advantages

- Simplifies complex asynchronous workflows
- Improves application responsiveness
- Reduces overhead in multi-threaded environments
- Easy integration with existing projects

## Quick Start

```cpp
#include "async_task_scheduler.h"
#include <iostream>
#include <chrono>

int main() {
    AsyncTaskSchedulerManager manager;

    auto scheduler = manager.create("MainScheduler");

    scheduler->set_on_task_start([]() {
        std::cout << "Task is starting" << std::endl;
    });

    scheduler->set_on_task_complete([]() {
        std::cout << "Task is completed" << std::endl;
    });

    scheduler->execute_function([]() {
        std::cout << "Hello from AsyncTaskScheduler!" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Working on something..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Almost there..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Done!" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Goodbye from AsyncTaskScheduler!" << std::endl;
    }, std::chrono::seconds(1));

    // Wait for all tasks to complete
    scheduler->wait_for_completion();

    // Stop and remove the scheduler
    /* 
    manager.stop("MainScheduler");
    manager.remove("MainScheduler");
    */

    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << "Exiting main" << std::endl;

    return 0;
}
```

## API Overview

```cpp
class AsyncTaskSchedulerManager {
public:
    std::shared_ptr<IAsyncTaskScheduler> create(const std::string& name);
    bool remove(const std::string& name);
    bool stop(const std::string& name);
};

class IAsyncTaskScheduler {
public:
    virtual void execute_function(TaskFunction func, std::chrono::milliseconds delay) = 0;
    virtual void wait_for_completion() = 0;
};
```

## Requirements

- C++17 or later
- Standard C++ libraries
