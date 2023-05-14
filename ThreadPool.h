/*
 * @Author: ljy
 * @Date: 2023-05-14 10:16:33
 * @LastEditors: ljy
 * @LastEditTime: 2023-05-14 12:49:49
 * @FilePath: /MyThreadPool/ThreadPool.h
 * @Description: 线程池
 * Copyright (c) 2023 by ljy.sj@qq.com, All Rights Reserved. 
 */
#ifndef THREADPOOL_H
#define THREADPOOL_H
#include "Thread/PrimaryThread.h"
#include "Thread/SecondaryThread.h"
#include "Thread/MonitorThread.h"
#include <memory>
#include <deque>
#include <vector>
#include <list>
#include <future>
#include <mutex>
#include <functional>
#include <thread>
#include <iostream>
using namespace std;

class ThreadPool {
public:
    // 假如线程池的线程数量为num，则 primary_num <= num <= primary_num + max_secondary_num
    // 即主线程不可增删，辅助线程可增删
    ThreadPool(uint primary_num, uint max_secondary_num):
        tasks_(make_shared<std::deque<std::function<void()>>>())
     {
        for(int i = 0; i < primary_num; i++) {
            tests_threads.emplace_back(std::thread([this, i]{
                while(true) {
                    std::function<void()> task; 
                    {
                        unique_lock<mutex> lock(mutex_);
                        while(tasks_->empty()) {
                            if(stop) break;
                            con_var_.wait(lock);
                        }
                        if(stop) break;
                        task = tasks_->front();
                        tasks_->pop_front();
                        cout << "thread " << i << " " << ++cnt << endl;
                    }
                    task();
                }
            }));
        }
    }

    ~ThreadPool() {
        {
            unique_lock<mutex> lock(mutex_);
            stop = true;
            cout << "stop " << cnt << endl; 
        }
        con_var_.notify_all();
        for(auto &t : tests_threads) {
            t.join();
            cout << "stop" << endl;
        }
    }

    // 向任务队列添加任务
    template<typename F, typename... Args>
    auto AddTask(F &&f, Args &&...args) 
        -> std::future<decltype(f(args...))>{
    using return_type = decltype(f(args...));
    function<return_type()> func = bind(std::forward<F>(f), std::forward<Args>(args)...);
    auto task = make_shared<packaged_task<return_type()>>(func);
    future<return_type> res = task->get_future();
    {
        unique_lock<mutex> lock(mutex_);
        tasks_->push_back(std::function<void()>([task]{(*task)();}));
    }
    con_var_.notify_one();
    return res;
}

// private:
    std::shared_ptr<std::deque<std::function<void()>>> tasks_;          // 线程池的任务队列
    std::vector<std::thread> tests_threads;
    std::vector<std::shared_ptr<PrimaryThread>> primary_threads_;       // 主线程集
    std::list<std::shared_ptr<SecondaryThread>> secondary_threads_;     // 辅助线程集
    uint max_secondary_num_;                                            // 辅助线程的最大数量
    std::shared_ptr<MonitorThread> monitor_thread_;                     // 监控线程
    std::mutex mutex_;
    std::condition_variable con_var_;
    bool stop = false;
    int cnt = 0;
};

#endif
