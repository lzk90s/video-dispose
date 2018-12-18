#pragma once

#include <functional>

#include "vfilter/core/frame_cache.h"
#include "vfilter/core/channel_sink.h"
#include "vfilter/core/frame_handler.h"
#include "vfilter/core/frame_picker.h"

namespace video {
namespace filter {

class AbstractAlgoProcessor : public  FrameHandler {
public:

    AbstractAlgoProcessor(const std::string &name) {
        name_ = name;
    }

    virtual ~AbstractAlgoProcessor() {
    }

    // 处理接收到的帧
    int32_t OnChannelReceivedFrame(std::shared_ptr<ChannelSink> chl, cv::Mat &frame) {
        if (framePicker_.NeedPickFrame()) {
            cv::Mat cloneFrame = frame.clone();
            OnFrame(chl, cloneFrame);
        }
        chl->MixFrame(frame);
        return 0;
    }

    int32_t OnChannelClose(std::shared_ptr<ChannelSink> chl) {
        OnFrameEnd(chl);
        return 0;
    }

private:
    std::string name_;
    //frame picker
    FramePicker framePicker_;
};

}
}