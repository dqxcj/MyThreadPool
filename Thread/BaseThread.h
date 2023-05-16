/*
 * @Author: ljy
 * @Date: 2023-05-16 10:28:03
 * @LastEditors: ljy
 * @LastEditTime: 2023-05-16 12:34:55
 * @FilePath: /MyThreadPool/Thread/BaseThread.h
 * @Description: 主线程和辅助线程的基类
 * Copyright (c) 2023 by ljy.sj@qq.com, All Rights Reserved. 
 */
#ifndef BASETHREAD_H
#define BASETHREAD_H
#include "../SafeDataStructure/SafeDeque.h"
#include "../SafeDataStructure/SafeBase.h"
#include "MonitorThread.h"
#include <memory>
#include <functional>
#include <mutex>
#include <thread>
#include <iostream>
#include <condition_variable>

class BaseThread {
friend class MonitorThread;
public:
    BaseThread(std::shared_ptr<SafeDeque<std::function<void()>>> pool_tasks, 
               std::shared_ptr<std::mutex> pool_mutex_ptr, 
               std::shared_ptr<std::condition_variable> con_var_ptr, 
               std::shared_ptr<bool> stop, 
               int num) : 
        own_tasks_(std::make_shared<SafeDeque<std::function<void()>>>()),
        pool_tasks_(pool_tasks),
        pool_mutex_ptr_(pool_mutex_ptr),
        con_var_ptr_(con_var_ptr),
        stop_(stop),
        num_(num),
        is_running_(std::make_shared<SafeBase<bool>>(false)) {
        BuildThead();
    }

    virtual ~BaseThread() {
    }



    std::shared_ptr<std::function<void()>> GetTaskFromOwnTasks() {
        // 这里判空并不是出于安全考虑，pop_front()内部会判空
        // 这里判空是处于性能考虑，对于空队列，没必要继续执行
        if (own_tasks_->empty()) return nullptr;   
        return own_tasks_->pop_front();
    }
    std::shared_ptr<std::function<void()>> GetTaskFromPoolTasks() {
        // 这里判空并不是出于安全考虑
        // 这里判空是处于性能考虑，对于空队列，没必要继续执行
        if(pool_tasks_->empty()) return nullptr;
        auto tasks = pool_tasks_->Steal(10);
        for(auto &task : tasks) {
            own_tasks_->emplace_back(std::move(task));
        }
        return GetTaskFromOwnTasks();
    }
    std::shared_ptr<std::function<void()>> GetTaskFromOthersTasks() {return nullptr;}

    void Close() {
        thread_.join();
    }

protected:
    std::shared_ptr<SafeDeque<std::function<void()>>> own_tasks_;
    std::shared_ptr<SafeDeque<std::function<void()>>> pool_tasks_;
    std::shared_ptr<std::mutex> pool_mutex_ptr_;
    std::shared_ptr<std::condition_variable> con_var_ptr_;
    std::shared_ptr<bool> stop_;
    std::thread thread_;
    std::shared_ptr<SafeBase<bool>> is_running_;
    int num_;

    virtual void BuildThead() {
        thread_ = std::thread([this]{
            while(true) {
                std::shared_ptr<std::function<void()>> task;  
                {
                    std::unique_lock<std::mutex> lock(*pool_mutex_ptr_);
                    while ((task = GetTaskFromOwnTasks()) == nullptr && (task = GetTaskFromPoolTasks()) == nullptr && (task = GetTaskFromOthersTasks()) == nullptr) {
                        if (*(stop_) && task == nullptr) {
                            break;
                        }
                        is_running_->Set(false);
                        con_var_ptr_->wait(lock);
                    }
                    is_running_->Set(true);
                }
                if (*(stop_) && task == nullptr) {
                    break;
                }
                if(task == nullptr) {
                    continue;
                }
                (*task)();
            }
        });
    }
};

#endif
