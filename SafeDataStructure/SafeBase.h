/*
 * @Author: ljy
 * @Date: 2023-05-15 22:47:59
 * @LastEditors: ljy
 * @LastEditTime: 2023-05-15 22:58:23
 * @FilePath: /MyThreadPool/SafeDataStructure/SafeBase.h
 * @Description: 加锁的基础数据类型
 * Copyright (c) 2023 by ljy.sj@qq.com, All Rights Reserved. 
 */

#ifndef SAFEBASE_H
#define SAFEBASE_H
#include <mutex>
#include <memory>

template <typename T>
class SafeBase {
public:
    SafeBase(T &&t): t_ptr_(std::make_shared<T>(std::move(t))) {}
    
    std::shared_ptr<T> Get() {
        std::unique_lock<std::mutex> lock(mutex_);
        return t_ptr_;
    }

    std::shared_ptr<T> Get() const {
        std::unique_lock<std::mutex> lock(mutex_);
        return t_ptr_;
    }

    void Set(T &&t) {
        std::unique_lock<std::mutex> lock(mutex_);
        t_ptr_ = std::make_shared<T>(std::move(t));
    }

private:
    std::shared_ptr<T> t_ptr_;
    std::mutex mutex_;
};

#endif