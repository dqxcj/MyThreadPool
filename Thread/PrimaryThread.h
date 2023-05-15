/*
 * @Author: ljy
 * @Date: 2023-05-14 10:17:04
 * @LastEditors: ljy
 * @LastEditTime: 2023-05-15 23:11:53
 * @FilePath: /MyThreadPool/Thread/PrimaryThread.h
 * @Description: 主线程，职责是不断取出任务并完成，不可增加，不可删减
 * Copyright (c) 2023 by ljy.sj@qq.com, All Rights Reserved. 
 */
#ifndef PRIMARYTHREAD_H
#define PRIMARYTHREAD_H
#include "../SafeDataStructure/SafeDeque.h"
#include "../SafeDataStructure/SafeBase.h"
#include <memory>
#include <functional>
#include <mutex>
#include <thread>
#include <iostream>
#include <condition_variable>

class PrimaryThread {
public:
    PrimaryThread(std::shared_ptr<SafeDeque<std::function<void()>>> pool_tasks, std::shared_ptr<std::mutex> pool_mutex_ptr, std::shared_ptr<std::condition_variable> con_var_ptr, std::shared_ptr<bool> stop, int num) : 
        own_tasks_(std::make_shared<SafeDeque<std::function<void()>>>()),
        pool_tasks_(pool_tasks),
        pool_mutex_ptr_(pool_mutex_ptr),
        con_var_ptr_(con_var_ptr),
        stop_(stop),
        num_(num)
    {
        thread_ = std::thread([this]{
                while(true) {
                    std::shared_ptr<std::function<void()>> task;  
                    {
                        std::unique_lock<std::mutex> lock(*pool_mutex_ptr_);
                        con_var_ptr_->wait(lock, [task, this]{
                            return *(stop_) || !own_tasks_->empty() || !pool_tasks_->empty(); 
                        });
                    }
                    if((task = GetTaskFromOwnTasks()) == nullptr && (task = GetTaskFromPoolTasks()) == nullptr && (task = GetTaskFromOthersTasks()) == nullptr) {
                        ;
                    }
                    if (*(stop_) && task == nullptr) {
                        break;
                    }
                    if(task == nullptr) {
                        continue;
                    }
                    (*task)();
                }
                // std::cout << num_ << " while over" << std::endl;
            });
    }

    ~PrimaryThread() {
    }

    std::shared_ptr<std::function<void()>> GetTaskFromOwnTasks() {
        if(own_tasks_->empty()) return nullptr;
        return std::make_shared<std::function<void()>>(own_tasks_->pop_front());
    }
    std::shared_ptr<std::function<void()>> GetTaskFromPoolTasks() {
        if(pool_tasks_->empty()) return nullptr;
        auto tasks = pool_tasks_->Steal(10);
        for(auto &task : tasks) {
            own_tasks_->push_back(std::move(task));
        }
        return GetTaskFromOwnTasks();
    }
    std::shared_ptr<std::function<void()>> GetTaskFromOthersTasks() {return nullptr;}

    void Close() {
        thread_.join();
        // std::cout << num_ << " is closed" << std::endl;
    }

private:
    std::shared_ptr<SafeDeque<std::function<void()>>> own_tasks_;
    std::shared_ptr<SafeDeque<std::function<void()>>> pool_tasks_;
    std::shared_ptr<std::mutex> pool_mutex_ptr_;
    std::shared_ptr<std::condition_variable> con_var_ptr_;
    std::shared_ptr<bool> stop_;
    std::thread thread_;
    int num_;
};

#endif