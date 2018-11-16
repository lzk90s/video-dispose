#pragma once

#include <functional>
#include "common/helper/threadpool.h"

#include "vfilter/core/frame_cache.h"
#include "vfilter/core/channel_sink.h"
#include "vfilter/core/frame_handler.h"


namespace vf {

class AbstractAlgoProcessor : public  FrameHandler {
public:

    AbstractAlgoProcessor()
        : worker_(1) {
    }

    virtual ~AbstractAlgoProcessor() {
    }

    void LinkHandler(ChannelSink &chl) {
        chl.RegOnFrameHander(std::bind(&AbstractAlgoProcessor::OnFrame, this, std::placeholders::_1, std::placeholders::_2));
        chl.RegOnFrameEndHandler(std::bind(&AbstractAlgoProcessor::OnFrameEnd, this, std::placeholders::_1));
    }

    void OnFrame(ChannelSink &chl,  cv::Mat &frame) override {
        worker_.commit(std::bind(&AbstractAlgoProcessor::algoRoutine,this, ref(chl), chl.frameCache.AllocateEmptyFrame(),
                                 frame));
    }

    void OnFrameEnd(ChannelSink &chl) override {
        algoRoutineEnd(chl);
    }

protected:
    //algorithm routine
    virtual int32_t algoRoutine(ChannelSink &chl, uint64_t frameId, cv::Mat &frame) {
        return 0;
    }

    virtual int32_t algoRoutineEnd(ChannelSink &chl) {
        return 0;
    }

protected:
    threadpool worker_;
};

}