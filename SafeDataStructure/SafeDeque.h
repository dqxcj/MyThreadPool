/*
 * @Author: ljy
 * @Date: 2023-05-15 10:01:37
 * @LastEditors: ljy
 * @LastEditTime: 2023-05-16 10:52:19
 * @FilePath: /MyThreadPool/SafeDataStructure/SafeDeque.h
 * @Description: 安全队列 给队列加锁
 * Copyright (c) 2023 by ljy.sj@qq.com, All Rights Reserved. 
 */

#ifndef SAFEDEQUE_H
#define SAFEDEQUE_H
#include <deque>
#include <mutex>
#include <memory>

template <typename T>
class SafeDeque {
public:
    SafeDeque()=default;
    ~SafeDeque() {}

    // 目前发现不可设为const 
    // 否则unique_lock会报错

    void emplace_back(T &&t) {
        std::unique_lock<std::mutex> lock(mutex_);
        deq_.emplace_back(std::forward<T>(t));
    }

    std::shared_ptr<T> pop_back() {
        std::unique_lock<std::mutex> lock(mutex_);
        if(deq_.empty()) return nullptr;
        auto res = deq_.back();
        deq_.pop_back();
        return std::make_shared<T>(std::move(res));
    }

    void emplace_front(T &&t) {
        std::unique_lock<std::mutex> lock(mutex_);
        deq_.emplace_front(std::forward<T>(t));
    }

    std::shared_ptr<T> pop_front() {
        std::unique_lock<std::mutex> lock(mutex_);
        if(deq_.empty()) return nullptr;
        auto res = deq_.front();
        deq_.pop_front();
        return std::make_shared<T>(std::move(res));
    }

    size_t size() {
        std::unique_lock<std::mutex> lock(mutex_);
        return deq_.size();
    }

    bool empty() {
        std::unique_lock<std::mutex> lock(mutex_);
        return deq_.empty();
    }

    std::deque<T> Steal(int num) {
        std::unique_lock<std::mutex> lock(mutex_);
        if(num > deq_.size()) num = deq_.size();
        std::deque<T> res;
        for(int i = 0; i < num; i++) {
            res.push_back(deq_.front());
            deq_.pop_front();
        }
        return res;
    }

private:
    std::deque<T> deq_;
    std::mutex mutex_;
};

#endif