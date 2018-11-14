#pragma once

#include <chrono>

#include "vfilter/config/setting.h"

using namespace std;

namespace vf {

class FramePicker {
public:
    FramePicker()
        :lastTime_(chrono::steady_clock::now()),
         currTime_(chrono::steady_clock::now()),
         pickCnt(0) {

    }

    bool NeedPickFrame() {
        bool pickFlag = false;
        currTime_ = chrono::steady_clock::now();
        auto diffTime = std::chrono::duration_cast<std::chrono::milliseconds>(currTime_ - lastTime_).count();
        pickCnt++;

        //当时间和数量都达到的时候，才抽帧。
        //做时间限制，是为了避免高帧率的时候，疯狂抽帧。做数量限制，是为了避免低帧率的时候，抽到相同帧
        if (pickCnt >= GlobalSettings::getInstance().framePickInternalNum &&
                diffTime >= GlobalSettings::getInstance().minFramePickInternalMs) {
            // 时间更新&计数归零
            pickFlag = true;
            lastTime_ = currTime_;
            pickCnt = 0;
        }
        return pickFlag;
    }

private:
    //time for pick frame
    chrono::steady_clock::time_point lastTime_;
    chrono::steady_clock::time_point currTime_;
    uint32_t pickCnt;
};

}