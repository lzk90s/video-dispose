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
        : seemmoWorker_(1), sink_(sink) {
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
        seemmoWorker_.commit(f1);
    }

    int32_t AlgoRoutine(uint32_t channelId, uint64_t frameId, uint8_t *bgr24, uint32_t width, uint32_t height) {
        DetectResult detectResult;
        FilterResult filterResult;
        int32_t ret = 0;

        LOG_INFO("--StartTrail, timestamp: {}", frameId);
        TrailParam trailParam;
        ret = seemmoStub_->Trail(channelId, frameId, bgr24, width, height, trailParam, detectResult, filterResult);
        if (0 != ret) {
            LOG_ERROR("trail error, ret {}", ret);
            return ret;
        }

        onAlgoDetectReply(detectResult);

        onAlgoFilterReply(filterResult);

        for (auto v : filterResult.bikes) {
            RecogParam recParam;
            recParam.ContextCode = v.contextCode;
            recParam.obj = v;

            // 从缓存中获取最佳的那一帧图像
            bool exist = false;
            cv::Mat fv = sink_.GetFrame(v.frameId, exist);
            if (!exist) {
                LOG_WARN("the frame {} not exist!", v.frameId);
                continue;
            }

            RecogResult recResult;
            ret = seemmoStub_->Recognize((uint8_t*)fv.data, fv.rows, fv.cols, recParam, recResult);
            if (0 != ret) {
                LOG_ERROR("recognize error, ret {}", ret);
                return ret;
            }
            onAlgoRecognizeReply(recResult);
        }
        for (auto v : filterResult.vehicles) {
            RecogParam recParam;
            recParam.ContextCode = v.contextCode;
            recParam.obj = v;

            // 从缓存中获取最佳的那一帧图像
            bool exist = false;
            cv::Mat fv = sink_.GetFrame(v.frameId, exist);
            if (!exist) {
                LOG_WARN("the frame {} not exist!", v.frameId);
                continue;
            }

            RecogResult recResult;
            ret = seemmoStub_->Recognize((uint8_t*)fv.data, fv.rows, fv.cols, recParam, recResult);
            if (0 != ret) {
                LOG_ERROR("recognize error, ret {}", ret);
                return ret;
            }
            onAlgoRecognizeReply(recResult);
        }
        for (auto v : filterResult.pedestrains) {
            RecogParam recParam;
            recParam.ContextCode = v.contextCode;
            recParam.obj = v;

            // 从缓存中获取最佳的那一帧图像
            bool exist = false;
            cv::Mat fv = sink_.GetFrame(v.frameId, exist);
            if (!exist) {
                LOG_WARN("the frame {} not exist!", v.frameId);
                continue;
            }

            RecogResult recResult;
            ret = seemmoStub_->Recognize((uint8_t*)fv.data, fv.rows, fv.cols, recParam, recResult);
            if (0 != ret) {
                LOG_ERROR("recognize error, ret {}", ret);
                return ret;
            }
            onAlgoRecognizeReply(recResult);
        }

        return 0;
    }


private:

    void onAlgoDetectReply(DetectResult &detectResult) {
        sink_.GetPersonMixer().SetDetectedObjects(detectResult.pedestrains);
        sink_.GetVehicleMixer().SetDetectedObjects(detectResult.vehicles);
        sink_.GetBikeMixer().SetDetectedObjects(detectResult.bikes);
    }

    void onAlgoFilterReply(FilterResult &filterResult) {
        for (auto s : filterResult.pedestrains) {
            LOG_INFO("Received person FilterResults: {}, {}", s.guid, s.frameId);
        }
        for (auto s : filterResult.vehicles) {
            LOG_INFO("Received vehicle FilterResults: {}, {}", s.guid, s.frameId);
        }
    }

    void onAlgoRecognizeReply(RecogResult &recResult) {
        LOG_INFO("---111-----vehicles ---{}", recResult.vehicles.size());
        LOG_INFO("---111----pedestrains ---{}", recResult.pedestrains.size());
        LOG_INFO("---111----bikes ---{}", recResult.bikes.size());

        sink_.GetPersonMixer().SetRecognizedObjects(recResult.pedestrains);
        sink_.GetVehicleMixer().SetRecognizedObjects(recResult.vehicles);
        sink_.GetBikeMixer().SetRecognizedObjects(recResult.bikes);
    }

private:
    AlgoStub *seemmoStub_;
    threadpool seemmoWorker_;
    VSink &sink_;
};

}