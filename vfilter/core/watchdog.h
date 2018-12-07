#pragma once

#include <cstdint>
#include "common/helper/timer.h"

using namespace std;

namespace vf {

//看门狗
class Watchdog {
public:
    Watchdog() {
        dogCount_ = INIT_COUNT;
    }

    ~Watchdog() {
        timer.Expire();
    }

    void Watch(std::function<void()> die) {
        timer.StartTimer(1000, [=]() {
            //10秒钟还没有喂狗，表示程序可能出现问题了，强制 go die
            //double check 替代加锁
            dogCount_--;
            if (dogCount_ <= 0) {
                if (dogCount_ <= 0) {
                    die();
                }
            }
        });
    }

    void Feed() {
        dogCount_ = INIT_COUNT;
    }

private:
    const int INIT_COUNT = 10;
    Timer timer;
    int32_t dogCount_;
};

}