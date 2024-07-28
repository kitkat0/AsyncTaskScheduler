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