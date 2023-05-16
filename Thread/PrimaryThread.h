/*
 * @Author: ljy
 * @Date: 2023-05-14 10:17:04
 * @LastEditors: ljy
 * @LastEditTime: 2023-05-16 11:25:30
 * @FilePath: /MyThreadPool/Thread/PrimaryThread.h
 * @Description: 主线程，职责是不断取出任务并完成，取任务优先级是自己的任务队列、线程池的任务队列、邻居线程的任务队列，不可增加，不可删减
 * Copyright (c) 2023 by ljy.sj@qq.com, All Rights Reserved. 
 */
#ifndef PRIMARYTHREAD_H
#define PRIMARYTHREAD_H
#include "BaseThread.h"

class PrimaryThread: public BaseThread {
public:
    PrimaryThread(std::shared_ptr<SafeDeque<std::function<void()>>> pool_tasks, std::shared_ptr<std::mutex> pool_mutex_ptr, std::shared_ptr<std::condition_variable> con_var_ptr, std::shared_ptr<bool> stop, int num) : 
        BaseThread(pool_tasks, pool_mutex_ptr, con_var_ptr, stop, num) {}

    ~PrimaryThread() {}

private:
};

#endif