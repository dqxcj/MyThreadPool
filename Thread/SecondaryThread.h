/*
 * @Author: ljy
 * @Date: 2023-05-14 10:17:31
 * @LastEditors: ljy
 * @LastEditTime: 2023-05-17 12:11:08
 * @FilePath: /MyThreadPool/Thread/SecondaryThread.h
 * @Description: 辅助线程，职责是不断从线程池的任务队列取出任务并完成，可增加，可删减
 * Copyright (c) 2023 by ljy.sj@qq.com, All Rights Reserved.
 */
#ifndef SECONDARYTHREAD_H
#define SECONDARYTHREAD_H
#include "../SafeDataStructure/SafeDeque.h"
#include "../SafeDataStructure/SafeBase.h"
#include "../OutPut.h"
#include <memory>
#include <functional>
#include <mutex>
#include <thread>
#include <iostream>
#include <condition_variable>

class MonitorThread;

class SecondaryThread {
friend class MonitorThread;
public:
    SecondaryThread(std::shared_ptr<SafeDeque<std::function<void()>>> pool_tasks, std::shared_ptr<std::mutex> pool_mutex_ptr, std::shared_ptr<std::condition_variable> con_var_ptr, std::shared_ptr<bool> stop, int num) : 
        own_tasks_(std::make_shared<SafeDeque<std::function<void()>>>()),
        pool_tasks_(pool_tasks),
        pool_mutex_ptr_(pool_mutex_ptr),
        con_var_ptr_(con_var_ptr),
        stop_(stop),
        num_(num),
        is_running_(std::make_shared<SafeBase<bool>>(false)),
        stop_ST_(false) {
        MonitorOut << num_ << " is start" << std::endl;
        BuildThead();
    }

    ~SecondaryThread() {
        stop_ST_ = true;
        Close();
    }

    void Close() {
        try {
            thread_.join();
            MonitorOut << num_ << " ~SecondaryThread" << std::endl;
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

private:
    std::shared_ptr<SafeDeque<std::function<void()>>> own_tasks_;
    std::shared_ptr<SafeDeque<std::function<void()>>> pool_tasks_;
    std::shared_ptr<std::mutex> pool_mutex_ptr_;
    std::shared_ptr<std::condition_variable> con_var_ptr_;
    std::shared_ptr<bool> stop_;
    std::thread thread_;
    std::shared_ptr<SafeBase<bool>> is_running_;
    int num_;
    bool stop_ST_;
    void BuildThead() {
        thread_ = std::thread([this]{
            while(true) {
                // 关闭一个辅助线程
                if(stop_ST_) {
                    ClearOwnTasks();
                    break;
                }

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


    // 清空当前线程的队列
    // 注意并没有全程持有队列的锁
    // 这是为了让邻居线程可以偷走当前队列的元素
    void ClearOwnTasks() {
        std::shared_ptr<std::function<void()>> task;
        while((task = GetTaskFromOwnTasks()) != nullptr) {
            (*task)();
        }
    }
};
#endif