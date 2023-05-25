#include "ThreadPool.h"
#include <iostream>
#include <memory>
#include <thread>
using namespace std;

mutex anslock;
vector<int> ans;

int addf(int a, int b) {
    unique_lock<mutex> lock(anslock);
    ans.push_back(a + b);
    return a + b;
}

int main() {
    shared_ptr<ThreadPool> tp = make_shared<ThreadPool>(0, 100);
    tp->SetStart();
    auto res = make_shared<vector<future<int>>>();
    for(int i = 0; i < 10000; i++) {
        res->push_back(tp->AddTask(addf, i, 1000));
        if(i != 0 && i % 100000 == 0) {
            sleep(5);
        }
    }
    for(auto &f : *res) {
        f.wait();
    }
    if(ans.size() != 10000) cout << "ans.size(): " << ans.size() << endl;
    return 0;
}