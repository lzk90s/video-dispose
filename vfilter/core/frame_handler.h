#pragma once

#include <cstdint>
#include "opencv/cv.h"
#include <memory>

#include "vfilter/core/channel_sink.h"

namespace video {
namespace filter {

class FrameHandler {
protected:
    virtual void onFrame(std::shared_ptr<ChannelSink> chl, cv::Mat &frame) = 0;

    virtual void onFrameEnd(std::shared_ptr<ChannelSink> chl) = 0;
};

}
}