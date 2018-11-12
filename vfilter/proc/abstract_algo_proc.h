#pragma once

#include <functional>
#include "common/helper/threadpool.h"

#include "algo/stub/algo_stub.h"
#include "vfilter/frame_cache.h"
#include "vfilter/vsink.h"
#include "vfilter/frame_handler.h"


namespace vf {

class AbstractAlgoProcessor : public  FrameHandler {
public:

    AbstractAlgoProcessor(VSink &sink)
        : worker_(1), //single worker thread
          sink_(sink) {
        sink_.RegisterFrameHandler(std::bind(&AbstractAlgoProcessor::OnFrame, this, std::placeholders::_1,
                                             std::placeholders::_2));
    }

    virtual ~AbstractAlgoProcessor() {
    }

    void OnFrame(uint32_t chanelId, cv::Mat &frame) override {
        worker_.commit(std::bind(&AbstractAlgoProcessor::algoRoutine,
                                 this, chanelId, frameCache_.AllocateEmptyFrame(), frame));
    }

protected:
    //algorithm routine
    virtual int32_t algoRoutine(uint32_t channelId, uint64_t frameId, cv::Mat &frame) {
        return -1;
    }

protected:
    threadpool worker_;
    VSink &sink_;
    FrameCache frameCache_;
};

}