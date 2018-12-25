#pragma once

#include <cstdint>

namespace video {
namespace filter {

class FramePicker {
public:
    FramePicker(uint32_t pickInterval) : pickInterval_(pickInterval) {
        pickCnt_ = 0;
    }

    bool NeedPickFrame() {
        bool pickFlag = false;
        pickCnt_++;
        if (pickCnt_ >= pickInterval_) {
            pickFlag = true;
            pickCnt_ = 0;
        }
        return pickFlag;
    }

private:
    uint32_t pickInterval_;
    uint32_t pickCnt_;
};

}
}