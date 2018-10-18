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
        : tp_(1),	//ҵ���߳��ǵ��̣߳������Ͳ���Ҫ������Ҳ��������ͼƬ�ȱ�ǰ���ͼƬȥ���
          tpNtf_(1),	//֪ͨ�̣߳���1����
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

        // ����Ŀ��
        ret = trailObjects(channelId, frameId, frame, imageResult, filterResult);
        if (0 != ret) {
            return ret;
        }

        // Ŀ��ʶ��
        recognizeByImageResult(channelId, frame, imageResult);

        // �����Ž������ʶ��
        recognizeByFilterResult(channelId, filterResult);

        return 0;
    }

    int32_t trailObjects(uint32_t channelId, uint64_t frameId, cv::Mat &frame, ImageResult &imageResult,
                         FilterResult &filterResult) {

        const uint8_t *bgr24 = frame.data;
        uint32_t width = frame.cols;
        uint32_t height = frame.rows;

        TrailParam trailParam;
        // roi��������Ϊȫͼ
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

        //���ݼ������������Ҫʶ���Ŀ��
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
            //����������⴦��Ŀǰ�����㷨��û��ȥ�أ������򵥴���
            if (!sink_.faceObjectSink.ObjectExist(p.guid)) {
                LOG_INFO("New face object {}", p.guid);
                sink_.faceNotifier.OnRecognizedObject(channelId, frame, p);
            }
        }

        //����Ŀ��Ŀ������
        onDetectedObjects(imageResult);

        ImageResult recImageResult;
        //�������ʶ�����������ʶ��
        if (!recParam.locations.empty()) {
            ret = algo_.Recognize(channelId, bgr24, width, height, recParam, recImageResult);
            if (0 != ret) {
                LOG_ERROR("Recognize error, ret {}", ret);
                return ret;
            }

            //����guid����aʶ��ʱû�з���guid
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

        //����Ŀ���
        onRecognizedObjects(recImageResult);

        return 0;
    }

    //����filterresult�����첽ʶ��
    int32_t recognizeByFilterResult(uint32_t channelId, FilterResult &filterResult) {
        //������a��sdk�ֲᣬʶ����ٽ������Ҫ����type��trail��rect��contextcode
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

        //�Ѳ���Ҫ��֡�ֶ��ͷŵ�
        for (auto &fid : filterResult.releasedFrames) {
            sink_.frameCache.ManualRelase(fid);
        }

        return 0;
    }

    int32_t recognizeByFilterResultInner(uint32_t channelId, cv::Mat &frame, RecogParam &recParam) {
        //�첽����
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
    //�㷨�첽�̳߳�
    threadpool tp_;
    //֪ͨ�̳߳�
    threadpool tpNtf_;
    //sink
    VSink &sink_;
    //�㷨stub
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


//�����ĵ�������������Ϊ������Ч������Ҫ�Ż�������Ӱ�������㷨��Ч��
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