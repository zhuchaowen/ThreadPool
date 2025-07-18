#include "TaskQueue.h"

TaskQueue::TaskQueue() 
{

}

TaskQueue::~TaskQueue() 
{

}

// 向任务队列中添加任务
void TaskQueue::add_task(Task task)
{
    std::unique_lock<std::mutex> lock(task_queue_mutex);
    task_queue.push(task);
}

void TaskQueue::add_task(callback function, void* arg)
{
    std::unique_lock<std::mutex> lock(task_queue_mutex);
    task_queue.push(Task(function, arg));
}

// 从任务队列中取出一个任务
Task TaskQueue::take_task()
{   
    Task task;

    task_queue_mutex.lock();
    if(!task_queue.empty()) {
        task = task_queue.front();
        task_queue.pop();
    }
    task_queue_mutex.unlock();

    return task;
}

// 判断任务队列是否为空
bool TaskQueue::empty()
{
    std::unique_lock<std::mutex> lock(task_queue_mutex);
    return task_queue.empty();
}

// 获得当前任务数，即队列的大小
int TaskQueue::get_task_number()
{
    std::unique_lock<std::mutex> lock(task_queue_mutex);
    return task_queue.size();
}