/*
 * @Author: ljy
 * @Date: 2023-05-15 10:49:32
 * @LastEditors: ljy
 * @LastEditTime: 2023-05-15 11:57:48
 * @FilePath: /MyThreadPool/TaskPool/TaskPool.h
 * @Description: 公有任务池 各线程先访问私有任务队列 无任务则从公有任务池窃取 
 * Copyright (c) 2023 by ljy.sj@qq.com, All Rights Reserved. 
 */
#ifndef TASKPOOL_H
#define TASKPOOL_H
#include "../SafeDeque/SafeDeque.h"
#include <memory>
#include <future>


class TaskPool {
public:
    TaskPool() {}



private:
    std::shared_ptr<SafeDeque<std::function<void()>>> tasks_; // 线程池的任务队列
};

#endif
