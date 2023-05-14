#include "ThreadPool.h"
#include <functional>
#include <thread>
using namespace std;

// 传入的参数可设置线程数量
// 假如线程池的线程数量为num，则 primary_num <= num <= primary_num + max_secondary_num
// 即主线程不可增删，辅助线程可增删
// ThreadPool::ThreadPool(uint primary_num, uint max_secondary_num) 
//     //: primary_threads_(primary_num, make_shared<PrimaryThread>()),
//     // max_secondary_num_(max_secondary_num),
//     // monitor_thread_(make_shared<MonitorThread>())  
//     {
//         for(int i = 0; i < primary_num; i++) {
//             tests_threads.emplace_back(std::thread([this]{
//                 while(true) {
//                     unique_lock<mutex> lock(mutex_);
//                     while(tasks_->empty()) {
//                         con_var_.wait(lock);
//                     }
//                     auto task = tasks_->front();
//                     tasks_->pop_front();
//                     task();
//                 }
//             }));
//         }
//     }

// 向任务队列添加任务
// 任务的返回值可以通过future获取
// template <typename F, typename... Args>
// auto ThreadPool::AddTask(F &&f, Args &&...args)
//     -> future<decltype(f(args...))> {
//     using return_type = decltype(f(args...));
//     function<return_type()> func = bind(std::forward<F>(f), std::forward<Args>(args)...);
//     auto task = make_shared<packaged_task<return_type()>>(func);
//     future<return_type> res = task.get_future();
//     tasks_->push_back([task]{task();});
//     con_var_.notify_one();
//     return res;
// }