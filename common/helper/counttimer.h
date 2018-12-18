#pragma once

#include <iostream>
#include <string>
#include <chrono>

class CountTimer {
public:
    CountTimer(const std::string &name, int onlyShowOverTime = 0) : name_(name), onlyShowOverTime_(onlyShowOverTime) {
        start_ = std::chrono::steady_clock::now();
    }

    ~CountTimer() {
        end_ = std::chrono::steady_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::microseconds>(end_ - start_).count();
        if (diff > onlyShowOverTime_) {
            std::cout << "[" << name_ << "] -> " << diff << "us" << std::endl;
        }
    }

private:
    std::string name_;
    int onlyShowOverTime_;
    std::chrono::steady_clock::time_point start_;
    std::chrono::steady_clock::time_point end_;
};
