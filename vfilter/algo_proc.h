#pragma once

#include <functional>
#include "common/helper/threadpool.h"
#include "common/helper/logger.h"

#include "vfilter/setting.h"
#include "algo/stub/algo_stub.h"
#include "vfilter/frame_cache.h"
#include "vfilter/vsink.h"
#include "vfilter/frame_handler.h"


using namespace std;
using namespace algo;

namespace vf {

class AbstractAlgoProcessor : public  FrameHandler {
public:

    AbstractAlgoProcessor(VSink &sink, AlgoStub &algoStub)
        : tp_(1),	//业务线程是单线程，这样就不需要加锁，也避免后面的图片先比前面的图片去检测
          tpNtf_(1),	//通知线程，启1个。
          sink_(sink),
          algo_(algoStub) {
        sink_.RegisterFrameHandler(std::bind(&AbstractAlgoProcessor::OnFrame, this,
                                             std::placeholders::_1,
                                             std::placeholders::_2,
                                             std::placeholders::_3));
    }

    ~AbstractAlgoProcessor() {
    }

    void OnFrame(uint32_t chanelId, uint64_t frameId, cv::Mat &frame) override  {
        auto f1 = std::bind(&AbstractAlgoProcessor::algoRoutine, this, chanelId, frameId, frame);
        tp_.commit(f1);
    }

protected:

    int32_t algoRoutine(uint32_t channelId, uint64_t frameId, cv::Mat &frame) {
        ImageResult imageResult;
        FilterResult filterResult;
        int ret = 0;

        // 跟踪目标
        ret = trailObjects(channelId, frameId, frame, imageResult, filterResult);
        if (0 != ret) {
            return ret;
        }

        // 目标识别
        recognizeByImageResult(channelId, frame, imageResult);

        // 对择优结果进行识别
        recognizeByFilterResult(channelId, filterResult);

        return 0;
    }

    int32_t trailObjects(uint32_t channelId, uint64_t frameId, cv::Mat &frame, ImageResult &imageResult,
                         FilterResult &filterResult) {

        const uint8_t *bgr24 = frame.data;
        uint32_t width = frame.cols;
        uint32_t height = frame.rows;

        TrailParam trailParam;
        // roi区域设置为全图
        trailParam.roi.push_back(Point{ 0, 0 });
        trailParam.roi.push_back(Point{ 0, (int32_t)height });
        trailParam.roi.push_back(Point{ (int32_t)width, (int32_t)height });
        trailParam.roi.push_back(Point{ (int32_t)width, 0 });

        int32_t ret = algo_.Trail(channelId, frameId, bgr24, width, height, trailParam, imageResult, filterResult);
        if (0 != ret) {
            LOG_ERROR("Trail error, ret {}", ret);
            return ret;
        }

        return 0;
    }

    int32_t recognizeByImageResult(uint32_t channelId, cv::Mat &frame, ImageResult &imageResult) {
        int32_t ret = 0;
        RecogParam recParam;
        const uint8_t *bgr24 = frame.data;
        uint32_t width = frame.cols;
        uint32_t height = frame.rows;

        //根据检测结果，计算需要识别的目标
        vector<algo::BikeObject> toRecogBikeObjs;
        sink_.bikeObjectSink.CalcNeedRecognizeObjects(imageResult.bikes, toRecogBikeObjs);
        for (auto &p : toRecogBikeObjs) {
            RecogParam::ObjLocation loc;
            loc.type = p.type;
            loc.trail = p.trail;
            loc.detect = p.detect;
            recParam.locations.push_back(loc);
        }
        vector<algo::VehicleObject> toRecogVehicleObjs;
        sink_.vehicleObjectSink.CalcNeedRecognizeObjects(imageResult.vehicles, toRecogVehicleObjs);
        for (auto &p : toRecogVehicleObjs) {
            RecogParam::ObjLocation loc;
            loc.type = p.type;
            loc.trail = p.trail;
            loc.detect = p.detect;
            recParam.locations.push_back(loc);
        }
        vector<algo::PersonObject> toRecogPersonObjs;
        sink_.personObjectSink.CalcNeedRecognizeObjects(imageResult.pedestrains, toRecogPersonObjs);
        for (auto &p : toRecogPersonObjs) {
            RecogParam::ObjLocation loc;
            loc.type = p.type;
            loc.trail = p.trail;
            loc.detect = p.detect;
            recParam.locations.push_back(loc);
        }

        for (auto &p : imageResult.faces) {
            //针对人脸特殊处理，目前人脸算法中没有去重，这样简单处理
            if (!sink_.faceObjectSink.ObjectExist(p.guid)) {
                LOG_INFO("New face object {}", p.guid);
                sink_.faceNotifier.OnRecognizedObject(channelId, frame, p);
            }
        }

        //更新目标目标检测结果
        onDetectedObjects(imageResult);

        ImageResult recImageResult;
        //如果存在识别区域，则进行识别
        if (!recParam.locations.empty()) {
            ret = algo_.Recognize(channelId, bgr24, width, height, recParam, recImageResult);
            if (0 != ret) {
                LOG_ERROR("Recognize error, ret {}", ret);
                return ret;
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
        }

        //更新目标池
        onRecognizedObjects(recImageResult);

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
            cv::Mat frame = sink_.frameCache.Get(p.frameId, exist);
            if (!exist) {
                LOG_WARN("The saved frame {} not exist", p.frameId);
                return 0;
            }
            recognizeByFilterResultInner(channelId, frame, recParam);
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
            cv::Mat frame = sink_.frameCache.Get(p.frameId, exist);
            if (!exist) {
                LOG_WARN("The saved frame {} not exist", p.frameId);
                return 0;
            }
            recognizeByFilterResultInner(channelId, frame, recParam);
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
            cv::Mat frame = sink_.frameCache.Get(p.frameId, exist);
            if (!exist) {
                LOG_WARN("The saved frame {} not exist", p.frameId);
                return 0;
            }
            recognizeByFilterResultInner(channelId, frame, recParam);
        }

        //把不需要的帧手动释放掉
        for (auto &fid : filterResult.releasedFrames) {
            sink_.frameCache.ManualRelase(fid);
        }

        return 0;
    }

    int32_t recognizeByFilterResultInner(uint32_t channelId, cv::Mat &frame, RecogParam &recParam) {
        //异步处理
        tpNtf_.commit([=]() {
            const uint8_t *bgr24 = frame.data;
            uint32_t width = frame.cols;
            uint32_t height = frame.rows;

            ImageResult imageResult;
            int32_t ret = algo_.Recognize(channelId, bgr24, width, height, recParam, imageResult);
            if (0 != ret) {
                LOG_ERROR("Recognize error, ret {}", ret);
                return ret;
            }

            cv::Mat f = frame;
            for (auto &p : imageResult.bikes) {
                sink_.bikeNotifier.OnRecognizedObject(channelId, f, p);
            }
            for (auto &p : imageResult.pedestrains) {
                sink_.personNotifier.OnRecognizedObject(channelId, f, p);
            }
            for (auto &p : imageResult.vehicles) {
                sink_.vehicleNotifier.OnRecognizedObject(channelId, f, p);
            }
            for (auto &p : imageResult.faces) {
                sink_.faceNotifier.OnRecognizedObject(channelId, f, p);
            }

            return 0;
        });

        return 0;
    }

    virtual void onDetectedObjects(ImageResult &imageResult) {
        sink_.personObjectSink.UpdateDetectedObjects(imageResult.pedestrains);
        sink_.vehicleObjectSink.UpdateDetectedObjects(imageResult.vehicles);
        sink_.bikeObjectSink.UpdateDetectedObjects(imageResult.bikes);
        sink_.faceObjectSink.UpdateDetectedObjects(imageResult.faces);
    }

    virtual void onRecognizedObjects(ImageResult &imageResult) {
        sink_.personObjectSink.UpdateRecognizedObjects(imageResult.pedestrains);
        sink_.vehicleObjectSink.UpdateRecognizedObjects(imageResult.vehicles);
        sink_.bikeObjectSink.UpdateRecognizedObjects(imageResult.bikes);
        sink_.faceObjectSink.UpdateRecognizedObjects(imageResult.faces);
    }

protected:
    //算法异步线程池
    threadpool tp_;
    //通知线程池
    threadpool tpNtf_;
    //sink
    VSink &sink_;
    //算法stub
    AlgoStub  &algo_;
};

class DefaultAlgoProcessor : public AbstractAlgoProcessor {
public:
    DefaultAlgoProcessor(VSink &vsink, AlgoStub *stub = NewAlgoStub(GlobalSettings::getInstance().enableSeemmoAlgo, false)):
        AbstractAlgoProcessor(vsink, *stub) {
        this->stub_ = stub;
    }

    ~DefaultAlgoProcessor() {
        FreeAlgoStub(stub_);
    }

private:
    AlgoStub *stub_;
};


//人脸的单独出来，是因为人脸的效果还需要优化，避免影响其他算法的效果
class FaceAlgoProcessor : public AbstractAlgoProcessor {
public:
    FaceAlgoProcessor(VSink &vsink, AlgoStub *stub = NewAlgoStub(false, GlobalSettings::getInstance().enableGosunAlgo))
        : AbstractAlgoProcessor(vsink, *stub) {
        this->stub_ = stub;
    }

    ~FaceAlgoProcessor() {
        FreeAlgoStub(stub_);
    }

protected:
    void onDetectedObjects(ImageResult &imageResult) override {
        sink_.faceObjectSink.UpdateDetectedObjects(imageResult.faces);
    }

    void onRecognizedObjects(ImageResult &imageResult) override {
        sink_.faceObjectSink.UpdateRecognizedObjects(imageResult.faces);
    }

private:
    AlgoStub *stub_;
};

}