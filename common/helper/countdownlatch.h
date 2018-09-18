#pragma once

#include <mutex>
#include <condition_variable>

using namespace std;

class CountDownLatch {
public:
    explicit  CountDownLatch(int count)
        : mutex_(), condition_(), count_(count) {

    }
    ~CountDownLatch() {

    }

    void wait() {
        unique_lock<mutex> lock(mutex_);
        while (count_ > 0) {  //ֻҪ����ֵ����0��CountDownLatch��Ͳ�������֪���ȴ�����ֵΪ0
            condition_.wait(lock);
        }
    }

    void countDown() {
        unique_lock<mutex> lock(mutex_);
        --count_;
        if (count_ == 0) {
            condition_.notify_all();
        }
    }

    int getCount() const {
        unique_lock<mutex> lock(mutex_);
        return count_;
    }


private:
    mutable mutex mutex_;
    condition_variable condition_;
    int count_;
};