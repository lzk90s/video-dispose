#pragma once

#include <cstdint>
#include "opencv/cv.h"
#include <memory>

#include "vfilter/core/channel_sink.h"

namespace vf {

class FrameHandler {
public:
    virtual void OnFrame(shared_ptr<ChannelSink> chl, cv::Mat &frame) = 0;

    virtual void OnFrameEnd(shared_ptr<ChannelSink> chl) = 0;
};

}