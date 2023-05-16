/*
 * @Author: ljy
 * @Date: 2023-05-14 10:18:54
 * @LastEditors: ljy
 * @LastEditTime: 2023-05-16 12:37:16
 * @FilePath: /MyThreadPool/Thread/MonitorThread.h
 * @Description: 监控线程，用于监控辅助线程，忙则增加辅助线程，闲则删减辅助线程
 * Copyright (c) 2023 by ljy.sj@qq.com, All Rights Reserved. 
 */
#ifndef MONITORTHREAD_H
#define MONITORTHREAD_H
#include "PrimaryThread.h"
#include "SecondaryThread.h"
#include <list>
#include <memory>
#include <unistd.h>
#include <functional>

const uint AddSecondaryThreadsNum = 10; 

class MonitorThread {
public:
    MonitorThread(std::shared_ptr<SafeDeque<std::function<void()>>> pool_tasks,
                  std::shared_ptr<std::vector<std::shared_ptr<PrimaryThread>>> primary_threads,
                  std::shared_ptr<std::list<std::shared_ptr<SecondaryThread>>> secondary_threads,
                  uint max_secondary_num,
                  std::shared_ptr<uint64_t> threads_num,
                  std::function<void(uint)> func) :
        pool_tasks_(pool_tasks),
        primary_threads_(primary_threads),
        secondary_threads_(secondary_threads),
        max_secondary_num_(max_secondary_num),
        threads_num_(threads_num),
        add_secondary_threads_(func) {

            BuildThread();
        }
    ~MonitorThread(){}

    void Close() {
        thread_.join();
    }
private:
    std::shared_ptr<SafeDeque<std::function<void()>>> pool_tasks_;                      // 线程池的任务队列
    std::shared_ptr<std::vector<std::shared_ptr<PrimaryThread>>> primary_threads_;      // 主线程集
    std::shared_ptr<std::list<std::shared_ptr<SecondaryThread>>> secondary_threads_;    // 辅助线程集
    uint max_secondary_num_;                                                            // 辅助线程的最大数量
    std::shared_ptr<uint64_t> threads_num_;                                             // 主线程 + 辅助线程 数目
    std::thread thread_;
    std::function<void(uint)> add_secondary_threads_;

    void BuildThread() {
        thread_ = std::thread([this] {
            while(true) {
                MonitorPrimaryThreads();
                MonitorSecondaryThreads();
                sleep(1);
            }
        });
    }

    void MonitorPrimaryThreads() {
        uint free_cnt = 0;
        for(auto &pthread : *primary_threads_) {
            auto bthread = static_cast<std::shared_ptr<BaseThread>>(pthread);
            if(!bthread->is_running_) {
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
            auto bthread = static_cast<std::shared_ptr<BaseThread>>(*it);
            // 闲则删除该辅助线程
            if(!bthread->is_running_) {
                secondary_threads_->erase(it);
            }
        }
    }

    void AddSecondaryThreads(uint num) {
        add_secondary_threads_(num);
    }
};
#endif
