#include "ThreadPool.h"
#include <iostream>
using namespace std;

mutex anslock;
vector<int> ans;

int addf(int a, int b) {
    unique_lock<mutex> lock(anslock);
    ans.push_back(a + b);
    return a + b;
}

int main() {
    ThreadPool tp(10, 0);
    vector<future<int>> res;
    for(int i = 0; i < 1000; i++) {
        res.push_back(tp.AddTask(addf, i, 1000));
    }
    for(auto &f : res) {
        f.wait();
    }
    // cout << ans.size() << endl;
    return 0;
}