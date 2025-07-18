#include "ThreadPool.h"
#include <iostream>
#include <chrono>

void* test(void* arg)
{
    int num = *(int*)arg;
    std::cout << "thread " << std::this_thread::get_id() << "is working, number is " << num << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));

    return nullptr;
}

int main()
{
    ThreadPool* pool = new ThreadPool(3, 10);
    for (int i = 0; i < 100; ++i) {
        int* num = new int(i + 100);
        pool->add_task(test, num);
    }

    std::this_thread::sleep_for(std::chrono::seconds(20));

    return 0;
}