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
                                             std::placeholders::_2));
    }

    ~AbstractAlgoProcessor() {
    }

    void OnFrame(uint32_t chanelId, cv::Mat &frame) override  {
        tp_.commit(std::bind(&AbstractAlgoProcessor::algoRoutine,
                             this, chanelId, frameCache_.AllocateEmptyFrame(), frame.clone()));
    }

protected:

    //检测到目标
    virtual void onDetectedObjects(uint32_t channelId, uint64_t frameId, cv::Mat &frame, ImageResult &imageResult) {}

    //识别到目标
    virtual void onRecognizedObjects(uint32_t channelId, uint64_t frameId, cv::Mat &frame, ImageResult &imageResult) {}

    //识别到择优后的目标
    virtual void onRecognizedFilterObjects(uint32_t channelId, cv::Mat &frame, ImageResult &imageResult) {}

private:

    int32_t algoRoutine(uint32_t channelId, uint64_t frameId, cv::Mat &frame) {
        int ret = 0;

        FilterResult filterResult;
        ret = trailAndRecognize(channelId, frameId, frame, filterResult);
        if (0 != ret) {
            return ret;
        }

        ret = recognizeByFilterResult(channelId, filterResult);
        if (0 != ret) {
            return ret;
        }

        return 0;
    }

    //跟踪+识别
    int32_t trailAndRecognize(uint32_t channelId, uint64_t frameId, cv::Mat &frame, FilterResult &filterResult) {
        int32_t ret = 0;
        const uint8_t *bgr24 = frame.data;
        uint32_t width = frame.cols;
        uint32_t height = frame.rows;

        TrailParam trailParam;
        // roi区域设置为全图
        trailParam.roi.push_back(Point{ 0, 0 });
        trailParam.roi.push_back(Point{ 0, (int32_t)height });
        trailParam.roi.push_back(Point{ (int32_t)width, (int32_t)height });
        trailParam.roi.push_back(Point{ (int32_t)width, 0 });

        //检测
        ImageResult imageResult;
        ret = algo_.Trail(channelId, frameId, bgr24, width, height, trailParam, imageResult, filterResult);
        if (0 != ret) {
            LOG_ERROR("Trail error, ret {}", ret);
            return ret;
        }

        //根据检测结果，计算需要识别的目标
        RecogParam recParam;
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

        onDetectedObjects(channelId, frameId, frame, imageResult);
        onRecognizedObjects(channelId, frameId, frame, recImageResult);

        return 0;
    }

    //择优结果异步识别
    int32_t recognizeByFilterResult(uint32_t channelId, FilterResult &filterResult) {
        for (auto &p : filterResult.bikes) {
            bool exist = false;
            cv::Mat img = frameCache_.GetOneObjectImage(p.frameId, p.guid, exist);
            if (!exist) {
                LOG_WARN("The saved object {} in frame {} not exist", p.guid, p.frameId);
                continue;
            }

            RecogParam recParam;
            RecogParam::ObjLocation loc;
            loc.type = p.type;
            loc.ContextCode = p.contextCode;
            loc.trail = p.trail;
            loc.detect = {0, 0, img.cols, img.rows};
            recParam.locations.push_back(loc);

            recognizeByFilterResultInner(channelId, img, recParam);
        }

        for (auto &p : filterResult.vehicles) {
            bool exist = false;
            cv::Mat img = frameCache_.GetOneObjectImage(p.frameId, p.guid, exist);
            if (!exist) {
                LOG_WARN("The saved object {} in frame {} not exist", p.guid, p.frameId);
                continue;
            }

            RecogParam recParam;
            RecogParam::ObjLocation loc;
            loc.type = p.type;
            loc.ContextCode = p.contextCode;
            loc.trail = p.trail;
            loc.detect = { 0, 0, img.cols, img.rows };
            recParam.locations.push_back(loc);
            recognizeByFilterResultInner(channelId, img, recParam);
        }

        for (auto &p : filterResult.pedestrains) {
            bool exist = false;
            cv::Mat img = frameCache_.GetOneObjectImage(p.frameId, p.guid, exist);
            if (!exist) {
                LOG_WARN("The saved object {} in frame {} not exist", p.guid, p.frameId);
                continue;
            }

            RecogParam recParam;
            RecogParam::ObjLocation loc;
            loc.type = p.type;
            loc.ContextCode = p.contextCode;
            loc.trail = p.trail;
            loc.detect = { 0, 0, img.cols, img.rows };
            recParam.locations.push_back(loc);
            recognizeByFilterResultInner(channelId, img, recParam);
        }

        //把不需要的帧手动释放掉
        for (auto &fid : filterResult.releasedFrames) {
            frameCache_.ManualRelase(fid);
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
            onRecognizedFilterObjects(channelId, f, imageResult);

            return 0;
        });

        return 0;
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
    //frame cache
    FrameCache frameCache_;
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


    void onDetectedObjects(uint32_t channelId, uint64_t frameId, cv::Mat &frame, ImageResult &imageResult) override {
        //根据检测结果抠图
        FrameCache::ObjectImageMap objectImages;
        for (auto &p : imageResult.bikes) {
            int32_t x = p.detect[0], y = p.detect[1], w = p.detect[2], h = p.detect[3];
            cv::Rect  rect = cv::Rect(x, y, w, h);
            cv::Mat roi = frame(rect);
            cv::Mat img = roi.clone();
            objectImages[p.guid] = img;
        }
        for (auto &p : imageResult.pedestrains) {
            int32_t x = p.detect[0], y = p.detect[1], w = p.detect[2], h = p.detect[3];
            cv::Rect  rect = cv::Rect(x, y, w, h);
            cv::Mat roi = frame(rect);
            cv::Mat img = roi.clone();
            objectImages[p.guid] = img;
        }
        for (auto &p : imageResult.vehicles) {
            int32_t x = p.detect[0], y = p.detect[1], w = p.detect[2], h = p.detect[3];
            cv::Rect  rect = cv::Rect(x, y, w, h);
            cv::Mat roi = frame(rect);
            cv::Mat img = roi.clone();
            objectImages[p.guid] = img;
        }
        for (auto &p : imageResult.faces) {
            int32_t x = p.detect[0], y = p.detect[1], w = p.detect[2], h = p.detect[3];
            cv::Rect  rect = cv::Rect(x, y, w, h);
            cv::Mat roi = frame(rect);
            cv::Mat img = roi.clone();
            objectImages[p.guid] = img;
        }

        //保存目标抠图，后续的择优识别需要用到
        if (!objectImages.empty()) {
            frameCache_.SaveObjectImageInFrame(frameId, objectImages);
        }

        sink_.personObjectSink.UpdateDetectedObjects(imageResult.pedestrains);
        sink_.vehicleObjectSink.UpdateDetectedObjects(imageResult.vehicles);
        sink_.bikeObjectSink.UpdateDetectedObjects(imageResult.bikes);
        //sink_.faceObjectSink.UpdateDetectedObjects(imageResult.faces);
    }

    void onRecognizedObjects(uint32_t channelId, uint64_t frameId, cv::Mat &frame, ImageResult &imageResult) override {
        sink_.personObjectSink.UpdateRecognizedObjects(imageResult.pedestrains);
        sink_.vehicleObjectSink.UpdateRecognizedObjects(imageResult.vehicles);
        sink_.bikeObjectSink.UpdateRecognizedObjects(imageResult.bikes);
        //sink_.faceObjectSink.UpdateRecognizedObjects(imageResult.faces);
    }

    void onRecognizedFilterObjects(uint32_t channelId, cv::Mat &frame, ImageResult &imageResult) override {
        for (auto &p : imageResult.bikes) {
            sink_.bikeNotifier.OnRecognizedObject(channelId, frame, p);
        }
        for (auto &p : imageResult.pedestrains) {
            sink_.personNotifier.OnRecognizedObject(channelId, frame, p);
        }
        for (auto &p : imageResult.vehicles) {
            sink_.vehicleNotifier.OnRecognizedObject(channelId, frame, p);
        }
//         for (auto &p : imageResult.faces) {
//             sink_.faceNotifier.OnRecognizedObject(channelId, frame, p);
//         }
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
    void onDetectedObjects(uint32_t channelId, uint64_t frameId, cv::Mat &frame, ImageResult &imageResult) override {
        for (auto &p : imageResult.faces) {
            //针对人脸特殊处理，目前人脸算法中没有去重，这样简单处理
            if (!sink_.faceObjectSink.ObjectExist(p.guid)) {
                LOG_INFO("New face object {}", p.guid);
                sink_.faceNotifier.OnRecognizedObject(channelId, frame, p);
            }
        }

        sink_.faceObjectSink.UpdateDetectedObjects(imageResult.faces);
    }

private:
    AlgoStub *stub_;
};

}