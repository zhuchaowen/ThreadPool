#pragma once
#include "TaskQueue.h"
#include <thread>
#include <unordered_map>
#include <atomic>
#include <mutex>
#include <condition_variable>

class ThreadPool {
private:
    TaskQueue* task_queue;  // 任务队列，保存待处理的任务

    std::thread manager_thread;    // 管理线程
    std::unordered_map<std::thread::id, std::thread> workers_thread;   // 工作线程
    int min_number; // 最小工作线程数量
    int max_number; // 最大工作线程数量
    // 以下三个变量要进行原子操作
    std::atomic<int> live_number;   // 存活的线程数量，即已经创建的工作线程数
    std::atomic<int> busy_number;   // 忙碌的线程数量，即正在工作的线程数
    std::atomic<int> exit_number;   // 待销毁的线程数量

    bool shutdown;  // 是否关闭线程池

    std::mutex workers_thread_lock; // 互斥访问工作线程
    std::condition_variable empty;  // 任务队列中是否没有任务，没有就要阻塞工作线程
public:
    ThreadPool(int min_num, int max_num);   // 确定线程池可创建的工作线程数范围
    ~ThreadPool();  // 销毁线程池

    // 向线程池中添加任务
    void add_task(Task task);
    void add_task(callback function, void* arg);
    // 获得存活的工作线程数
    int get_live_number();
    // 获得忙碌的工作线程数
    int get_busy_number();
private:
    // ThreadPool类内部调用的函数
    void manager(); // 管理线程
    void worker();  // 工作线程
    void thread_exit(); // 销毁线程
    // 工作线程从任务队列中取走一个任务
    void take_task();
};