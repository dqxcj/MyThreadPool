/*
 * @Author: ljy
 * @Date: 2023-05-14 10:17:31
 * @LastEditors: ljy
 * @LastEditTime: 2023-05-16 11:00:58
 * @FilePath: /MyThreadPool/Thread/SecondaryThread.h
 * @Description: 辅助线程，职责是不断从线程池的任务队列取出任务并完成，可增加，可删减
 * Copyright (c) 2023 by ljy.sj@qq.com, All Rights Reserved.
 */
#ifndef SECONDARYTHREAD_H
#define SECONDARYTHREAD_H
#include "BaseThread.h"

class SecondaryThread : public BaseThread {
public:
    SecondaryThread(std::shared_ptr<SafeDeque<std::function<void()>>> pool_tasks, std::shared_ptr<std::mutex> pool_mutex_ptr, std::shared_ptr<std::condition_variable> con_var_ptr, std::shared_ptr<bool> stop, int num) : 
        BaseThread(pool_tasks, pool_mutex_ptr, con_var_ptr, stop, num),
        stop_ST_(false) {
            
        BuildThead();
    }

    virtual ~SecondaryThread() {
        stop_ST_ = true;
        thread_.join();
    }

private:
    bool stop_ST_;
    virtual void BuildThead() {
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
                        con_var_ptr_->wait(lock);
                    }
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