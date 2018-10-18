#pragma once

#include <iostream>
#include <string>
#include <chrono>

using namespace std;

class CountTimer {
public:
    CountTimer(const string &name, int onlyShowOverTime = 0) : name_(name), onlyShowOverTime_(onlyShowOverTime) {
        start_ = std::chrono::steady_clock::now();
    }

    ~CountTimer() {
        end_ = std::chrono::steady_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::microseconds>(end_ - start_).count();
        if (diff > onlyShowOverTime_) {
            cout << "[" << name_ << "] -> " << diff << "us" << endl;
        }
    }

private:
    string name_;
    int onlyShowOverTime_;
    std::chrono::steady_clock::time_point start_;
    std::chrono::steady_clock::time_point end_;
};
