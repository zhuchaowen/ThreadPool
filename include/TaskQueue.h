#pragma once
#include <queue>
#include <mutex>

// 回调函数，即每个线程执行的任务函数
using callback = void* (*)(void*);

// 任务结构体，包括要执行的函数及函数参数
struct Task {
    callback function;
    void* arg;

    Task() : function(nullptr), arg(nullptr) {}
    Task(callback func, void* a) : function(func), arg(a) {}
};

// 任务队列类
class TaskQueue {
private:
    std::queue<Task> task_queue;    // 任务队列
    std::mutex task_queue_mutex;    // 互斥访问任务队列
public:
    TaskQueue();
    ~TaskQueue();

    // 向任务队列中添加任务
    void add_task(Task task);
    void add_task(callback function, void* arg);
    // 从任务队列中取出一个任务
    Task take_task();
    // 判断任务队列是否为空
    bool empty();
};