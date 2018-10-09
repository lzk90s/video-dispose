#pragma once

#include <cstdint>
#include "opencv/cv.h"

namespace vf {

using OnFrameHandler = function<void(uint32_t chanelId, uint64_t frameId, cv::Mat &frame)>;

class FrameHandler {
public:
    virtual void OnFrame(uint32_t chanelId, uint64_t frameId, cv::Mat &frame) = 0;
};

}