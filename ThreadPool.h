/*
 * @Author: ljy
 * @Date: 2023-05-14 10:16:33
 * @LastEditors: ljy
 * @LastEditTime: 2023-05-17 10:25:21
 * @FilePath: /MyThreadPool/ThreadPool.h
 * @Description: 线程池
 * Copyright (c) 2023 by ljy.sj@qq.com, All Rights Reserved. 
 */
#ifndef THREADPOOL_H
#define THREADPOOL_H
#include "Thread/PrimaryThread.h"
#include "Thread/SecondaryThread.h"
#include "Thread/MonitorThread.h"
#include "SafeDataStructure/SafeDeque.h"
#include "SafeDataStructure/SafeBase.h"
#include <memory>
#include <deque>
#include <vector>
#include <list>
#include <future>
#include <mutex>
#include <functional>
#include <thread>
#include <iostream>
#include <unistd.h>
#include <fstream>

class ThreadPool {
public:
    // 假如线程池的线程数量为num，则 primary_num <= num <= primary_num + max_secondary_num
    // 即主线程不可增删，辅助线程可增删
    ThreadPool(uint primary_num, uint max_secondary_num): 
        pool_mutex_ptr_(std::make_shared<std::mutex>()),
        con_var_ptr_(std::make_shared<std::condition_variable>()), 
        start_(std::make_shared<bool>(false)),
        stop_(std::make_shared<bool>(false)),
        tasks_(std::make_shared<SafeDeque<std::function<void()>>>()),
        max_secondary_num_(max_secondary_num),
        threads_num_(std::make_shared<SafeBase<uint64_t>>(primary_num)),
        primary_threads_(std::make_shared<std::vector<std::shared_ptr<PrimaryThread>>>()),
        secondary_threads_(std::make_shared<std::list<std::shared_ptr<SecondaryThread>>>()),
        num_(0) {

            // 构造主线程集
            for(int i = 0; i < primary_num; i++) {
                primary_threads_->push_back(std::make_shared<PrimaryThread>(tasks_, pool_mutex_ptr_, con_var_ptr_, start_, stop_, num_++));
            }
            // 构造监控线程
            monitor_thread_ = std::make_shared<MonitorThread>(tasks_, primary_threads_, secondary_threads_, max_secondary_num_, threads_num_, pool_mutex_ptr_, con_var_ptr_, start_, stop_, num_);
            
        }

    ~ThreadPool() {
        *stop_ = true;
        // 唤醒所有线程
        {
            std::unique_lock<std::mutex> lock(*pool_mutex_ptr_);
            con_var_ptr_->notify_all();
        }

        // 等待每个线程结束
        for(auto &thread : *primary_threads_) {
            thread->Close();
        }

        for(auto &thread : *secondary_threads_) {
            thread->Close();
        }

        monitor_thread_->Close();
        std::ofstream out("out.txt", std::ofstream::app);
        out << "over" << std::endl;
    }

    // 向任务队列添加任务
    template<typename F, typename... Args>
    auto AddTask(F &&f, Args &&...args) 
        -> std::future<decltype(f(args...))>{
        using return_type = decltype(f(args...));
        std::function<return_type()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        auto task = std::make_shared<std::packaged_task<return_type()>>(func);
        std::future<return_type> res = task->get_future();
        tasks_->emplace_back(std::function<void()>([task]{(*task)();}));
        con_var_ptr_->notify_one();
        return res;
    }

    void SetStart() {
        *start_ = true;
    }

private:
    std::shared_ptr<std::mutex> pool_mutex_ptr_;                                         // mutex
    std::shared_ptr<std::condition_variable> con_var_ptr_;                               // 条件变量
    std::shared_ptr<bool> start_;                                                        // 线程开始工作信号
    std::shared_ptr<bool> stop_;                                                         // 线程关闭信号
    std::shared_ptr<SafeDeque<std::function<void()>>> tasks_;                            // 任务队列
    std::shared_ptr<std::vector<std::shared_ptr<PrimaryThread>>> primary_threads_;       // 主线程集
    std::shared_ptr<std::list<std::shared_ptr<SecondaryThread>>> secondary_threads_;     // 辅助线程集
    uint max_secondary_num_;                                                             // 辅助线程的最大数量
    std::shared_ptr<MonitorThread> monitor_thread_;                                      // 监控线程
    std::shared_ptr<SafeBase<uint64_t>> threads_num_;                                    // 主线程 + 辅助线程 数目
    uint64_t num_;
};

#endif
