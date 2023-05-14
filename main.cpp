#include "ThreadPool.h"
#include <iostream>
using namespace std;

int addf(int a, int b) {
    return a + b;
}

int main() {
    ThreadPool tp(10, 0);
    vector<future<int>> res;
    for(int i = 0; i < 100; i++) {
        res.push_back(tp.AddTask(addf, i, 1000));
    }
    for(auto &f : res) {
        f.wait();
    }
    return 0;
}