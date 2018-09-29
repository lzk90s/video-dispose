#pragma once

#include <functional>
#include "common/helper/threadpool.h"
#include "common/helper/logger.h"

#include "vfilter/setting.h"
#include "algo/stub/algo_stub.h"
#include "vfilter/frame_cache.h"
#include "vfilter/vsink.h"

using namespace std;
using namespace algo;

namespace vf {

class AsyncAlgoProcessor  {
public:

    AsyncAlgoProcessor(VSink &sink)
        : tp_(1),	//业务线程是单线程，这样就不需要加锁，也避免后面的图片先比前面的图片去检测
          tpNotify_(2),	//通知线程，启2个。避免阻塞
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
        auto f1 = std::bind(&AsyncAlgoProcessor::algoRoutine, this, chanelId, frameId, bgr24, width, height);
        tp_.commit(f1);
    }

private:
    int32_t algoRoutine(uint32_t channelId, uint64_t frameId, uint8_t *bgr24, uint32_t width, uint32_t height) {
        ImageResult imageResult;
        FilterResult filterResult;
        int ret = 0;

        // 跟踪目标
        ret = trailObjects(channelId, frameId, bgr24, width, height, imageResult, filterResult);
        if (0 != ret) {
            return ret;
        }

        // 跳帧识别
        if (++recogFrameCnt >= GlobalSettings::getInstance().frameRecogPickInternalNum) {
            recogFrameCnt = 0;
            recognizeByImageResult(channelId, bgr24, width, height, imageResult);
        }

        // 对filterresult结果进行异步识别，不关心结果
        recognizeByFilterResult(channelId, filterResult);

        return 0;
    }

    int32_t trailObjects(uint32_t channelId, uint64_t frameId, const uint8_t *bgr24, uint32_t width, uint32_t height,
                         ImageResult &imageResult, FilterResult &filterResult) {

        TrailParam trailParam;
        // roi区域设置为全图
        trailParam.roi.push_back(Point{ 0, 0 });
        trailParam.roi.push_back(Point{ 0, (int32_t)height });
        trailParam.roi.push_back(Point{ (int32_t)width, (int32_t)height });
        trailParam.roi.push_back(Point{ (int32_t)width, 0 });
        int32_t ret = seemmoStub_->Trail(channelId, frameId, bgr24, width, height, trailParam, imageResult, filterResult);
        if (0 != ret) {
            LOG_ERROR("Trail error, ret {}", ret);
            return ret;
        }

        //更新混合器中的对象
        sink_.GetPersonMixer().SetDetectedObjects(imageResult.pedestrains);
        sink_.GetVehicleMixer().SetDetectedObjects(imageResult.vehicles);
        sink_.GetBikeMixer().SetDetectedObjects(imageResult.bikes);

        return 0;
    }

    int32_t recognizeByImageResult(uint32_t channelId, const uint8_t *bgr24, uint32_t width, uint32_t height,
                                   ImageResult &imageResult) {
        int32_t ret = 0;
        RecogParam recParam;

        //根据深瞐的sdk手册，单张图的识别，只设置type和trail,detect
        for (auto &p : imageResult.bikes) {
            RecogParam::ObjLocation loc;
            loc.type = p.type;
            loc.trail = p.trail;
            loc.detect = p.detect;
            recParam.locations.push_back(loc);
        }
        for (auto &p : imageResult.vehicles) {
            RecogParam::ObjLocation loc;
            loc.type = p.type;
            loc.trail = p.trail;
            loc.detect = p.detect;
            recParam.locations.push_back(loc);
        }
        for (auto &p : imageResult.pedestrains) {
            RecogParam::ObjLocation loc;
            loc.type = p.type;
            loc.trail = p.trail;
            loc.detect = p.detect;
            recParam.locations.push_back(loc);
        }

        ImageResult recImageResult;
        //如果存在识别区域，则进行识别
        if (!recParam.locations.empty()) {
            ret = seemmoStub_->Recognize(channelId, bgr24, width, height, recParam, recImageResult);
            if (0 != ret) {
                LOG_ERROR("Recognize error, ret {}", ret);
                return ret;
            }
        }

        //更新guid，深瞐识别时没有返回guid
        for (uint32_t idx = 0; idx < recImageResult.bikes.size(); idx++) {
            recImageResult.bikes[idx].guid = imageResult.bikes[idx].guid;
        }
        for (uint32_t idx = 0; idx < recImageResult.vehicles.size(); idx++) {
            recImageResult.vehicles[idx].guid = imageResult.vehicles[idx].guid;
        }
        for (uint32_t idx = 0; idx < recImageResult.pedestrains.size(); idx++) {
            recImageResult.pedestrains[idx].guid = imageResult.pedestrains[idx].guid;
        }

        // 更新mixer中的检测结果缓存
        sink_.GetPersonMixer().SetRecognizedObjects(recImageResult.pedestrains);
        sink_.GetVehicleMixer().SetRecognizedObjects(recImageResult.vehicles);
        sink_.GetBikeMixer().SetRecognizedObjects(recImageResult.bikes);

        return 0;
    }

    //根据filterresult进行异步识别
    int32_t recognizeByFilterResult(uint32_t channelId, FilterResult &filterResult) {

        //根据深瞐的sdk手册，识别跟踪结果，需要设置type，trail，rect，contextcode
        for (auto &p : filterResult.bikes) {
            RecogParam recParam;
            RecogParam::ObjLocation loc;
            loc.type = p.type;
            loc.trail = p.trail;
            loc.ContextCode = p.contextCode;
            loc.detect = p.detect;
            recParam.locations.push_back(loc);

            bool exist = false;
            cv::Mat frame = sink_.GetFrame(p.frameId, exist);
            if (!exist) {
                LOG_WARN("The saved frame {} not exist", p.frameId);
                return 0;
            }
            asyncRecognizeByFilterResult(channelId, frame.data, frame.cols, frame.rows, recParam);
        }

        for (auto &p : filterResult.vehicles) {
            RecogParam recParam;
            RecogParam::ObjLocation loc;
            loc.type = p.type;
            loc.trail = p.trail;
            loc.ContextCode = p.contextCode;
            loc.detect = p.detect;
            recParam.locations.push_back(loc);

            bool exist = false;
            cv::Mat frame = sink_.GetFrame(p.frameId, exist);
            if (!exist) {
                LOG_WARN("The saved frame {} not exist", p.frameId);
                return 0;
            }
            asyncRecognizeByFilterResult(channelId, frame.data, frame.cols, frame.rows, recParam);
        }

        for (auto &p : filterResult.pedestrains) {
            RecogParam recParam;
            RecogParam::ObjLocation loc;
            loc.type = p.type;
            loc.trail = p.trail;
            loc.ContextCode = p.contextCode;
            loc.detect = p.detect;
            recParam.locations.push_back(loc);

            bool exist = false;
            cv::Mat frame = sink_.GetFrame(p.frameId, exist);
            if (!exist) {
                LOG_WARN("The saved frame {} not exist", p.frameId);
                return 0;
            }
            asyncRecognizeByFilterResult(channelId, frame.data, frame.cols, frame.rows, recParam);
        }

        return 0;
    }

    int32_t asyncRecognizeByFilterResult(uint32_t channelId, const uint8_t *bgr24, uint32_t width, uint32_t height,
                                         RecogParam &recParam) {
        tpNotify_.commit([this, channelId, bgr24, width, height, recParam]() {
            //调用算法接口识别
            ImageResult imageResult;
            int32_t ret = seemmoStub_->Recognize(channelId, bgr24, width, height, recParam, imageResult);
            if (0 != ret) {
                LOG_ERROR("Recognize error, ret {}", ret);
                return ret;
            }

            //根据识别结果抠图
            LOG_INFO("-----------RECOG RESULT {} {} {}", imageResult.pedestrains.size(), imageResult.vehicles.size(),
                     imageResult.bikes.size());

            return 0;
        });

        return 0;
    }

private:
    //深瞐算法stub
    AlgoStub * seemmoStub_;
    threadpool tp_;
    //通知线程池
    threadpool tpNotify_;
    //sink
    VSink &sink_;
    //识别帧计数
    uint32_t recogFrameCnt;
};

}