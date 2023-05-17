/*
 * @Author: ljy
 * @Date: 2023-05-14 10:18:54
 * @LastEditors: ljy
 * @LastEditTime: 2023-05-17 12:15:58
 * @FilePath: /MyThreadPool/Thread/MonitorThread.h
 * @Description: 监控线程，用于监控辅助线程，忙则增加辅助线程，闲则删减辅助线程
 * Copyright (c) 2023 by ljy.sj@qq.com, All Rights Reserved. 
 */
#ifndef MONITORTHREAD_H
#define MONITORTHREAD_H
#include "PrimaryThread.h"
#include "SecondaryThread.h"
#include "../OutPut.h"
#include <list>
#include <memory>
#include <unistd.h>
#include <functional>
#include <vector>
#include <iostream>

const uint AddSecondaryThreadsNum = 10; 

class MonitorThread {
public:
    MonitorThread(std::shared_ptr<SafeDeque<std::function<void()>>> pool_tasks,
                  std::shared_ptr<std::vector<std::shared_ptr<PrimaryThread>>> primary_threads,
                  std::shared_ptr<std::list<std::shared_ptr<SecondaryThread>>> secondary_threads,
                  uint max_secondary_num,
                  std::shared_ptr<SafeBase<uint64_t>> threads_num,
                  std::shared_ptr<std::mutex> pool_mutex_ptr,
                  std::shared_ptr<std::condition_variable> con_var_ptr,
                  std::shared_ptr<bool> start,
                  std::shared_ptr<bool> stop,
                  uint64_t num) :
        pool_tasks_(pool_tasks),
        primary_threads_(primary_threads),
        secondary_threads_(secondary_threads),
        max_secondary_num_(max_secondary_num),
        threads_num_(threads_num),
        pool_mutex_ptr_(pool_mutex_ptr),
        con_var_ptr_(con_var_ptr),
        start_(start),
        stop_(stop),
        num_(num) {
            MonitorOut <<"MonitorThread is start" << std::endl;
            BuildThread();
        }
    ~MonitorThread(){
        Close();
    }

    void Close() {
        try {
            thread_.join();
            MonitorOut << "~MonitorThread" << std::endl;
        } catch (const std::system_error &err) {
            // std::cerr << "thread.join() 错误: " << err.what() << std::endl;
        }
    }

private:
    std::shared_ptr<std::mutex> pool_mutex_ptr_;                                        // mutex
    std::shared_ptr<std::condition_variable> con_var_ptr_;                              // 条件变量
    std::shared_ptr<bool> start_;
    std::shared_ptr<bool> stop_;                                                        // 线程关闭信号
    std::shared_ptr<SafeDeque<std::function<void()>>> pool_tasks_;                      // 线程池的任务队列
    std::shared_ptr<std::vector<std::shared_ptr<PrimaryThread>>> primary_threads_;      // 主线程集
    std::shared_ptr<std::list<std::shared_ptr<SecondaryThread>>> secondary_threads_;    // 辅助线程集
    uint max_secondary_num_;                                                            // 辅助线程的最大数量
    std::shared_ptr<SafeBase<uint64_t>> threads_num_;                                   // 主线程 + 辅助线程 数目
    std::thread thread_;
    uint64_t num_;

    void BuildThread() {
        thread_ = std::thread([this] {
            while(true) {
                if(*start_) {
                    if(*stop_) break;
                    MonitorPrimaryThreads();
                    sleep(1);   // 防止新增加的线程还没运行就被下面的函数删除
                    MonitorSecondaryThreads();
                    MonitorOut << "primary_threads_.size(): " << primary_threads_->size() << std::endl;
                    MonitorOut << "secondary_threads_.size(): " << secondary_threads_->size() << std::endl;
                }
            }
        });
    }

    void MonitorPrimaryThreads() {
        uint free_cnt = 0;
        for(auto &pthread : *primary_threads_) {
            if(!*(pthread->is_running_->Get())) {
                free_cnt++;
            }
        }
        // 忙则增加辅助线程
        if(free_cnt == 0) {
            AddSecondaryThreads(AddSecondaryThreadsNum);
        }
    }

    void MonitorSecondaryThreads() {
        for(auto it = (*secondary_threads_).begin(); it != (*secondary_threads_).end(); ++it) {
            // 闲则删除该辅助线程
            if(!*((*it)->is_running_->Get())) {
                it = secondary_threads_->erase(it);
                --it;   // 抵消for循环中的++it
                threads_num_->Add(-1);
            }
        }
    }

    void AddSecondaryThreads(uint num) {
        if(secondary_threads_->size() >= max_secondary_num_) return ;
        if(max_secondary_num_ - secondary_threads_->size() < num) {
            num = max_secondary_num_ - secondary_threads_->size();
        }
        for (int i = 0; i < num; i++) {
            secondary_threads_->emplace_back(std::make_shared<SecondaryThread>(pool_tasks_, pool_mutex_ptr_, con_var_ptr_, stop_, num_++));
        }
        threads_num_->Add(num);
    }
};
#endif
