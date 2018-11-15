#pragma once

#include <cstdint>
#include "opencv/cv.h"
#include <memory>

#include "vfilter/core/channel_sink.h"

namespace vf {

class FrameHandler {
public:
    virtual void OnFrame(ChannelSink &chl, cv::Mat &frame) = 0;

    virtual void OnFrameEnd(ChannelSink &chl) = 0;
};

}