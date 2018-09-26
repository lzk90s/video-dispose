#pragma once

#include <functional>
#include "common/helper/threadpool.h"
#include "common/helper/logger.h"

#include "algo/stub/algo_stub.h"
#include "vfilter/frame_cache.h"
#include "vfilter/vsink.h"

using namespace std;
using namespace algo;

namespace vf {

class AsyncAlgoProcessor  {
public:

    AsyncAlgoProcessor(VSink &sink)
        : tp_(2),
          sink_(sink) {
        recogFrameCnt = 0;
        seemmoStub_ = AlgoStubFactory::CreateStub("seemmo");
        sink.SetFrameHandler(std::bind(&AsyncAlgoProcessor::AsyncProcessFrame, this,
                                       std::placeholders::_1,
                                       std::placeholders::_2,
                                       std::placeholders::_3,
                                       std::placeholders::_4,
                                       std::placeholders::_5));
    }

    ~AsyncAlgoProcessor() {
        AlgoStubFactory::FreeStub(seemmoStub_);
    }

    void AsyncProcessFrame(uint32_t chanelId, uint64_t frameId, uint8_t *bgr24, uint32_t width, uint32_t height)  {
        auto f1 = std::bind(&AsyncAlgoProcessor::AlgoRoutine, this, chanelId, frameId, bgr24, width, height);
        tp_.commit(f1);
    }

    int32_t AlgoRoutine(uint32_t channelId, uint64_t frameId, uint8_t *bgr24, uint32_t width, uint32_t height) {
        ImageResult imageResult;
        FilterResult filterResult;
        int32_t ret = 0;

        // 检测+跟踪
        if (true) {
            TrailParam trailParam;
            // roi区域设置为全图
            trailParam.roi.push_back(Point{ 0, 0 });
            trailParam.roi.push_back(Point{ 0, (int32_t)height });
            trailParam.roi.push_back(Point{ (int32_t)width, (int32_t)height });
            trailParam.roi.push_back(Point{ (int32_t)width, 0 });
            ret = seemmoStub_->Trail(channelId, frameId, bgr24, width, height, trailParam, imageResult, filterResult);
            if (0 != ret) {
                LOG_ERROR("Trail error, ret {}", ret);
                return ret;
            }
        }

        // 跳帧识别，不关心处理结果
        if (++recogFrameCnt >= 2) {
            recogFrameCnt = 0;
            auto f2 = std::bind(&AsyncAlgoProcessor::AsyncRecognizeByImageResult, this, channelId, bgr24, width, height,
                                imageResult);
            tp_.commit(f2);
        }

        // 对filterresult结果进行异步识别
        if (true) {
        }

        return 0;
    }

    //根据imageresult进行异步识别
    int32_t AsyncRecognizeByImageResult(uint32_t channelId, uint8_t *bgr24, uint32_t width, uint32_t height,
                                        ImageResult &imageResult) {
        int32_t ret = 0;
        RecogParam recParam;

        //根据深瞐的sdk手册，单张图的识别，只设置type和trail
        for (auto p : imageResult.bikes) {
            RecogParam::ObjLocation loc;
            loc.type = p.type;
            loc.trail = p.trail;
            loc.detect = p.detect;
            recParam.locations.push_back(loc);
        }
        for (auto p : imageResult.vehicles) {
            RecogParam::ObjLocation loc;
            loc.type = p.type;
            loc.trail = p.trail;
            loc.detect = p.detect;
            recParam.locations.push_back(loc);
        }
        for (auto p : imageResult.pedestrains) {
            RecogParam::ObjLocation loc;
            loc.type = p.type;
            loc.trail = p.trail;
            loc.detect = p.detect;
            recParam.locations.push_back(loc);
        }

        //如果存在识别区域，则进行识别
        if (!recParam.locations.empty()) {
            ImageResult recImageResult;
            ret = seemmoStub_->Recognize(channelId, bgr24, width, height, recParam, recImageResult);
            if (0 != ret) {
                LOG_ERROR("Recognize error, ret {}", ret);
                return ret;
            }

            // 更新mixer中的检测结果缓存
            sink_.GetPersonMixer().SetObjects(recImageResult.pedestrains);
            sink_.GetVehicleMixer().SetObjects(recImageResult.vehicles);
            sink_.GetBikeMixer().SetObjects(recImageResult.bikes);
        }

        return 0;
    }

    //根据filterresult进行异步识别
    int32_t AsyncRecognizeByFilterResult(uint32_t channelId, uint8_t *bgr24, uint32_t width, uint32_t height,
                                         FilterResult &filterResult) {
        int32_t ret = 0;
        RecogParam recParam;

        //根据深瞐的sdk手册，识别跟踪结果，需要设置type，trail，rect，contextcode
        for (auto p : filterResult.bikes) {
            RecogParam::ObjLocation loc;
            loc.type = p.type;
            loc.trail = p.trail;
            loc.ContextCode = p.contextCode;
            loc.detect = p.detect;
            recParam.locations.push_back(loc);
        }
        for (auto p : filterResult.vehicles) {
            RecogParam::ObjLocation loc;
            loc.type = p.type;
            loc.trail = p.trail;
            loc.ContextCode = p.contextCode;
            loc.detect = p.detect;
            recParam.locations.push_back(loc);
        }
        for (auto p : filterResult.pedestrains) {
            RecogParam::ObjLocation loc;
            loc.type = p.type;
            loc.trail = p.trail;
            loc.ContextCode = p.contextCode;
            loc.detect = p.detect;
            recParam.locations.push_back(loc);
        }

        //如果存在识别区域，则进行识别
        if (!recParam.locations.empty()) {
            ImageResult imageResult;
            ret = seemmoStub_->Recognize(channelId, bgr24, width, height, recParam, imageResult);
            if (0 != ret) {
                LOG_ERROR("Recognize error, ret {}", ret);
                return ret;
            }

            // 抠图
        }

        return 0;
    }

private:
    //深瞐算法stub
    AlgoStub * seemmoStub_;
    threadpool tp_;
    //sink
    VSink &sink_;
    //识别帧计数
    int32_t recogFrameCnt;
};

}