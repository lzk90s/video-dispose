#pragma once

#include <iostream>
#include <string>
#include <chrono>

using namespace std;

class CountTimer {
public:
    CountTimer(const string &name) : name_(name) {
        start_ = std::chrono::steady_clock::now();
    }

    ~CountTimer() {
        end_ = std::chrono::steady_clock::now();
        cout << "[" << name_ << "] -> " << std::chrono::duration_cast<std::chrono::milliseconds>(end_ - start_).count() << "ms" << endl;
    }

private:
    string name_;
    std::chrono::steady_clock::time_point start_;
    std::chrono::steady_clock::time_point end_;
};
