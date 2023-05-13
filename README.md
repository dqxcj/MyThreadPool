## 项目描述
用C++11（及以上）实现的简易线程池。采用了主辅线程的思想，支持自动扩缩线程池；采用local-thread、lock-free、work-stealing、批量添加任务等技术优化线程池的性能。

## 接口
### ThreadPool
```cpp
ThreadPool()    // 构造函数 可以指定数量 指定是否开启扩缩容
AddTask()       // 添加任务
~ThreadPool()   //析构函数 需要唤醒所有线程
```