#pragma once

#include <chrono>

#include "vfilter/config/setting.h"

using namespace std;

namespace vf {

class FramePicker {
public:
    FramePicker() {
        pickCnt_ = 0;
    }

    bool NeedPickFrame() {
        bool pickFlag = false;
        pickCnt_++;
        if (pickCnt_ >= G_CFG().framePickInternalNum) {
            pickFlag = true;
            pickCnt_ = 0;
        }
        return pickFlag;
    }

private:
    uint32_t pickCnt_;
};

}