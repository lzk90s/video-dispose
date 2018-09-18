#pragma once

#include <functional>

#include "algo/seemmo/stub/seemmo_stub.h"
#include "algo/gosun/stub/gosun_stub.h"

using namespace std;

namespace vf {

class AlgoAsyncCaller {
public:
    using OnReplyCallback = function<void()>;

public:
    AlgoAsyncCaller() : tp(1) {
    }

    void asyncProcessFrame(uint32_t videoChl, uint8_t *bgr24, uint32_t width, uint32_t height, OnReplyCallback onReply) {
        tp.commit(std::bind(AlgoAsyncCaller::Call, this), videoChl, bgr24, width, height, onReply);
    }

    int32_t Call(uint32_t videoChl, uint8_t *bgr24, uint32_t width, uint32_t height, OnReplyCallback onReply) {
        stub_->VideoTrailAndRec(videoChl, bgr24, width, height);
    }

private:
    algo::gosun::AlgoStub stub_;
    threadpool tp;
};

}