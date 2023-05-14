/*
 * @Author: ljy
 * @Date: 2023-05-14 10:17:04
 * @LastEditors: ljy
 * @LastEditTime: 2023-05-14 11:40:39
 * @FilePath: /MyThreadPool/Thread/PrimaryThread.h
 * @Description: 主线程，职责是不断取出任务并完成，不可增加，不可删减
 * Copyright (c) 2023 by ljy.sj@qq.com, All Rights Reserved. 
 */
#ifndef PRIMARYTHREAD_H
#define PRIMARYTHREAD_H
#include <memory>
#include <deque>
#include <functional>

class PrimaryThread {
public:
    PrimaryThread(std::shared_ptr<std::deque<std::function<void()>>> pool_tasks): pool_tasks_(pool_tasks) {
        while(true) {
            if(!pool_tasks_->empty()) {
                auto task = pool_tasks_->front();
                pool_tasks_->pop_front();
                task();
            } else {

            }
        }
    }
private:
    std::shared_ptr<std::deque<std::function<void()>>> own_tasks_;
    std::shared_ptr<std::deque<std::function<void()>>> pool_tasks_;
};

#endif