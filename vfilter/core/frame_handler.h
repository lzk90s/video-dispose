#pragma once

#include <cstdint>
#include "opencv/cv.h"
#include <memory>

#include "vfilter/core/channel_sink.h"

namespace video {
namespace filter {

class FrameHandler {
public:
    virtual void OnFrame(std::shared_ptr<ChannelSink> chl, cv::Mat &frame) = 0;

    virtual void OnFrameEnd(std::shared_ptr<ChannelSink> chl) = 0;
};

}
}