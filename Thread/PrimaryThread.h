/*
 * @Author: ljy
 * @Date: 2023-05-14 10:17:04
 * @LastEditors: ljy
 * @LastEditTime: 2023-05-17 10:30:35
 * @FilePath: /MyThreadPool/Thread/PrimaryThread.h
 * @Description: 主线程，职责是不断取出任务并完成，取任务优先级是自己的任务队列、线程池的任务队列、邻居线程的任务队列，不可增加，不可删减
 * Copyright (c) 2023 by ljy.sj@qq.com, All Rights Reserved. 
 */
#ifndef PRIMARYTHREAD_H
#define PRIMARYTHREAD_H
#ifndef DEBUG
// #define DEBUG

#include "../SafeDataStructure/SafeDeque.h"
#include "../SafeDataStructure/SafeBase.h"
#include <memory>
#include <functional>
#include <mutex>
#include <thread>
#include <iostream>
#include <condition_variable>

class MonitorThread;
class PrimaryThread {
friend class MonitorThread;
public:
    PrimaryThread(std::shared_ptr<SafeDeque<std::function<void()>>> pool_tasks, 
               std::shared_ptr<std::mutex> pool_mutex_ptr, 
               std::shared_ptr<std::condition_variable> con_var_ptr, 
               std::shared_ptr<bool> start,
               std::shared_ptr<bool> stop, 
               int num) : 
        own_tasks_(std::make_shared<SafeDeque<std::function<void()>>>()),
        pool_tasks_(pool_tasks),
        pool_mutex_ptr_(pool_mutex_ptr),
        con_var_ptr_(con_var_ptr),
        start_(start),
        stop_(stop),
        num_(num),
        is_running_(std::make_shared<SafeBase<bool>>(false)) {
        BuildThead();
    }

    ~PrimaryThread() {
        Close();
    }

    void Close() {
        try {
            thread_.join();
            std::cout << num_ << " ~PrimaryThread" << std::endl;
        } catch (const std::system_error &err) {
            // std::cerr << "thread.join() 错误: " << err.what() << std::endl;
        }
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

protected:
    std::shared_ptr<SafeDeque<std::function<void()>>> own_tasks_;
    std::shared_ptr<SafeDeque<std::function<void()>>> pool_tasks_;
    std::shared_ptr<std::mutex> pool_mutex_ptr_;
    std::shared_ptr<std::condition_variable> con_var_ptr_;
    std::shared_ptr<bool> start_;
    std::shared_ptr<bool> stop_;
    std::thread thread_;
    std::shared_ptr<SafeBase<bool>> is_running_;
    int num_;

    void BuildThead() {
        thread_ = std::thread([this]{
            while(true) {
                if(*start_) {
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
                        #ifdef DEBUG
                        std::cout << num_ << "is running" << std::endl;
                        #endif
                    }
                    if (*(stop_) && task == nullptr) {
                        break;
                    }
                    if(task == nullptr) {
                        continue;
                    }
                    (*task)();
                }
            }
        });
    }
};

#endif
#endif