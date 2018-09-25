#pragma once

#include <functional>
#include "common/helper/threadpool.h"
#include "common/helper/logger.h"

#include "algo/stub/algo_stub.h"
#include "vfilter/frame_cache.h"

using namespace std;
using namespace algo;

namespace vf {

class AsyncAlgoProcessor {
public:
    using OnTrailReplyCb = function<void()>;
    using OnRecReplyCb = function<void()>;

public:
    AsyncAlgoProcessor()
        : seemmoWorker_(1),
          gosunWorker_(1) {
        seemmoStub_ = AlgoStubFactory::CreateStub("seemmo");
        gosunStub_ =AlgoStubFactory::CreateStub("gosun");
    }

    ~AsyncAlgoProcessor() {
        AlgoStubFactory::FreeStub(seemmoStub_);
        AlgoStubFactory::FreeStub(gosunStub_);
    }

    void AsyncProcessFrame(uint32_t chanelId, uint64_t frameId, uint8_t *bgr24, uint32_t width, uint32_t height) {
        auto f1 = std::bind(&AsyncAlgoProcessor::SeemmoRoutine, this, chanelId, frameId, bgr24, width, height);
        //auto f2 = std::bind(&AsyncAlgoProcessor::GosunDo, this, chanelId, frameId, bgr24, width, height);
        seemmoWorker_.commit(f1);
        //gosunWorker_.commit(f2);
    }

    int32_t SeemmoRoutine(uint32_t channelId, uint64_t frameId, uint8_t *bgr24, uint32_t width, uint32_t height) {
        DetectResult detectResult;
        FilterResult filterResult;
        int32_t ret = 0;

        LOG_INFO("--START TRAIL : {}", frameId);
        TrailParam trailParam;
        ret = seemmoStub_->Trail(channelId, frameId, bgr24, width, height, trailParam, detectResult, filterResult);
        if (0 != ret) {
            LOG_ERROR("trail error, ret {}", ret);
            return ret;
        }
        LOG_INFO("--END TRAIL : {}", frameId);

        OnAlgoDetectReply(detectResult);

        OnAlgoFilterReply(filterResult);

        for (auto v : filterResult.bikes) {
            RecogParam recParam;
            recParam.ContextCode = v.contextCode;
            recParam.obj = v;

            // 从缓存中获取最佳的那一帧图像
            bool exist = false;
            cv::Mat fv = frameCache_.Get(v.frameId, exist);
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
            OnAlgoRecognizeReply(recResult);
        }
        for (auto v : filterResult.vehicles) {
            RecogParam recParam;
            recParam.ContextCode = v.contextCode;
            recParam.obj = v;

            // 从缓存中获取最佳的那一帧图像
            bool exist = false;
            cv::Mat fv = frameCache_.Get(v.frameId, exist);
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
            OnAlgoRecognizeReply(recResult);
        }
        for (auto v : filterResult.pedestrains) {
            RecogParam recParam;
            recParam.ContextCode = v.contextCode;
            recParam.obj = v;

            // 从缓存中获取最佳的那一帧图像
            bool exist = false;
            cv::Mat fv = frameCache_.Get(v.frameId, exist);
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
            OnAlgoRecognizeReply(recResult);
        }

        LOG_INFO("--END RECOG : {}", frameId);

        return 0;
    }

    FrameCache::FrameId CacheFrame(cv::Mat &frame) {
        return frameCache_.Put(frame);
    }

public:
    virtual void OnAlgoDetectReply(DetectResult &detectResult) {};

    virtual void OnAlgoFilterReply(FilterResult &filterResult) {};

    virtual void OnAlgoRecognizeReply(RecogResult &recResult) {};

private:
    AlgoStub *seemmoStub_;
    AlgoStub *gosunStub_;
    threadpool seemmoWorker_;
    threadpool gosunWorker_;

    FrameCache frameCache_;
};

}