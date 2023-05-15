/*
 * @Author: ljy
 * @Date: 2023-05-14 10:16:33
 * @LastEditors: ljy
 * @LastEditTime: 2023-05-15 17:41:26
 * @FilePath: /MyThreadPool/ThreadPool.h
 * @Description: 线程池
 * Copyright (c) 2023 by ljy.sj@qq.com, All Rights Reserved. 
 */
#ifndef THREADPOOL_H
#define THREADPOOL_H
#include "Thread/PrimaryThread.h"
#include "Thread/SecondaryThread.h"
#include "Thread/MonitorThread.h"
#include "SafeDeque/SafeDeque.h"
#include "TaskPool/TaskPool.h"
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
        con_var_ptr_(std::make_shared<std::condition_variable>()), 
        stop_(std::make_shared<bool>(false)),
        tasks_(std::make_shared<SafeDeque<std::function<void()>>>()) {
            for(int i = 0; i < primary_num; i++) {
                // std::cout << i << std::endl;
                primary_threads_.push_back(std::make_shared<PrimaryThread>(tasks_, con_var_ptr_, stop_, i));
            }
        }

    ~ThreadPool() {
        *stop_ = true;
        std::thread([this]{
            std::ofstream out("notify_all", std::ofstream::app);
            for(int i = 0; i < 1000; i++) {
                con_var_ptr_->notify_all();
                usleep(1000);
                out << i + 1 << std::endl;
            }
        }).detach();
        for(auto &thread:primary_threads_) {
            thread->Close();
        }
        std::ofstream out("out.txt", std::ofstream::app);
        out << "over" << std::endl;
        // std::cout << "over" << std::endl;
    }

    // 向任务队列添加任务
    template<typename F, typename... Args>
    auto AddTask(F &&f, Args &&...args) 
        -> std::future<decltype(f(args...))>{
        using return_type = decltype(f(args...));
        std::function<return_type()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        auto task = std::make_shared<std::packaged_task<return_type()>>(func);
        std::future<return_type> res = task->get_future();
        tasks_->push_back(std::function<void()>([task]{(*task)();}));
        con_var_ptr_->notify_one();
        return res;
    }

private:
    std::shared_ptr<std::condition_variable> con_var_ptr_;
    std::shared_ptr<bool> stop_;
    std::shared_ptr<SafeDeque<std::function<void()>>> tasks_;
    std::vector<std::shared_ptr<PrimaryThread>> primary_threads_;       // 主线程集
    std::list<std::shared_ptr<SecondaryThread>> secondary_threads_;     // 辅助线程集
    uint max_secondary_num_;                                            // 辅助线程的最大数量
    std::shared_ptr<MonitorThread> monitor_thread_;                     // 监控线程
};

#endif
