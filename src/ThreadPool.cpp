#include "ThreadPool.h"
#include <iostream>
#include <chrono>

const int NUMBER = 2;   // 每次创建或销毁两个线程
const std::thread::id NOT_THREAD = std::thread::id();   // 销毁的线程id

// 确定线程池可创建的工作线程数范围并初始化
ThreadPool::ThreadPool(int min_num, int max_num) : min_number(min_num), max_number(max_num)
{
    try {
        task_queue = new TaskQueue();
    } catch (const std::bad_alloc& e) {
        std::cerr << "task queue memory allocation failed: " << e.what() << std::endl;
        return;
    }

    try {
        workers_id.assign(max_num, NOT_THREAD);
    } catch (const std::bad_alloc& e) {
        std::cerr << "workers_thread memory allocation failed: " << e.what() << std::endl;
        delete task_queue;
        return;
    }

    busy_number = 0;
    live_number = min_num;
    exit_number = 0;

    shutdown = false;

    manager_thread = std::thread(manager);
    for (int i = 0; i < min_num; ++i) {
        std::thread worker_thread = std::thread(worker);
        workers_id[i] = worker_thread.get_id();
    }
}

// 销毁线程池
ThreadPool::~ThreadPool()
{
    shutdown = true;    // 关闭线程池

    // 阻塞回收管理线程
    manager_thread.join();

    // 唤醒工作线程
    for (int i = 0; i < live_number; ++i) {
        empty.notify_one();
    }

    // 释放任务队列
    if (task_queue) {
        delete task_queue;
    }
}

// 向线程池中添加任务
void ThreadPool::add_task(Task task)
{
    if (shutdown) {
        return;
    }

    task_queue->add_task(task);
    empty.notify_one();
}

void ThreadPool::add_task(callback function, void* arg)
{
    if (shutdown) {
        return;
    }

    task_queue->add_task(function, arg);
    empty.notify_one();
}

// 获得存活的工作线程数
inline int ThreadPool::get_live_number() 
{
    return live_number;
}

// 获得忙碌的工作线程数
inline int ThreadPool::get_busy_number()
{
    return busy_number;
}

// 管理线程，根据情况创建或销毁工作线程
void ThreadPool::manager()
{
    const int NUMBER = 2; // 每次创建或销毁2个线程

    while (!shutdown) {
        // 每3秒检查一次
        std::this_thread::sleep_for(std::chrono::seconds(3));
        
        // 当前任务数比工作的线程数多时创建线程
        int queue_size = task_queue->get_task_number();
		if (queue_size > live_number && live_number < max_number) {
			int counter = 0;
            for (int i = 0; i < max_number && counter < NUMBER && live_number < max_number; ++i) {
                std::unique_lock<std::mutex> lock(workers_id_lock);
                if (workers_id[i] == NOT_THREAD) {
                    std::thread worker_thread = std::thread(worker);
                    workers_id[i] = worker_thread.get_id();
                    counter++;
                    live_number++;
                }
            }
		}

        // 当前创建的线程数多于工作的线程数的2倍时销毁线程
		if (busy_number * 2 < live_number && live_number > min_number) {
			exit_number = NUMBER;
			for (int i = 0; i < NUMBER; ++i) {
				empty.notify_one(); // 唤醒阻塞的工作线程让其销毁
			}
		}
    }   
}

// 工作线程
void ThreadPool::worker()
{
    while (true) {
        {
            // 任务队列为空就阻塞
            std::unique_lock<std::mutex> lock(task_queue_lock);
            while (task_queue->empty() && !shutdown) {
                empty.wait(lock);
            }
        }

        // 需要销毁线程
        if (exit_number > 0) {
            exit_number--;
            if (live_number > min_number) {
                live_number--;
                thread_exit();
                return;
            }
        }

        if (shutdown) {
            thread_exit();
            return;
        }

        Task task = task_queue->take_task();

        busy_number++;

        std::cout << "thread " << std::this_thread::get_id() << " start working..." << std::endl;
        task.function(task.arg);
        delete task.arg;
        task.arg = nullptr;
        std::cout << "thread " << std::this_thread::get_id() << " end working..." << std::endl;

		busy_number--;
    }
}

// 销毁线程
void ThreadPool::thread_exit()
{
    std::thread::id tid = std::this_thread::get_id();
    for (int i = 0; i < max_number; ++i) {
        std::unique_lock<std::mutex> lock(workers_id_lock);
        if (workers_id[i] == tid) {
            workers_id[i] = NOT_THREAD;
            std::cout << "thread_exit() called, " << tid << " exiting..." << std::endl;
            break;
        }
    }
} 