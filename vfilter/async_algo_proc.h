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
        : tp_(1),	//ҵ���߳��ǵ��̣߳������Ͳ���Ҫ������Ҳ��������ͼƬ�ȱ�ǰ���ͼƬȥ���
          tpNotify_(2),	//֪ͨ�̣߳���2������������
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

        // ����Ŀ��
        ret = trailObjects(channelId, frameId, bgr24, width, height, imageResult, filterResult);
        if (0 != ret) {
            return ret;
        }

        // ��֡ʶ��
        if (++recogFrameCnt >= GlobalSettings::getInstance().frameRecogPickInternalNum) {
            recogFrameCnt = 0;
            recognizeByImageResult(channelId, bgr24, width, height, imageResult);
        }

        // ��filterresult��������첽ʶ�𣬲����Ľ��
        recognizeByFilterResult(channelId, filterResult);

        return 0;
    }

    int32_t trailObjects(uint32_t channelId, uint64_t frameId, const uint8_t *bgr24, uint32_t width, uint32_t height,
                         ImageResult &imageResult, FilterResult &filterResult) {

        TrailParam trailParam;
        // roi��������Ϊȫͼ
        trailParam.roi.push_back(Point{ 0, 0 });
        trailParam.roi.push_back(Point{ 0, (int32_t)height });
        trailParam.roi.push_back(Point{ (int32_t)width, (int32_t)height });
        trailParam.roi.push_back(Point{ (int32_t)width, 0 });
        int32_t ret = seemmoStub_->Trail(channelId, frameId, bgr24, width, height, trailParam, imageResult, filterResult);
        if (0 != ret) {
            LOG_ERROR("Trail error, ret {}", ret);
            return ret;
        }

        //���»�����еĶ���
        sink_.GetPersonMixer().SetDetectedObjects(imageResult.pedestrains);
        sink_.GetVehicleMixer().SetDetectedObjects(imageResult.vehicles);
        sink_.GetBikeMixer().SetDetectedObjects(imageResult.bikes);

        return 0;
    }

    int32_t recognizeByImageResult(uint32_t channelId, const uint8_t *bgr24, uint32_t width, uint32_t height,
                                   ImageResult &imageResult) {
        int32_t ret = 0;
        RecogParam recParam;

        //������a��sdk�ֲᣬ����ͼ��ʶ��ֻ����type��trail,detect
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
        //�������ʶ�����������ʶ��
        if (!recParam.locations.empty()) {
            ret = seemmoStub_->Recognize(channelId, bgr24, width, height, recParam, recImageResult);
            if (0 != ret) {
                LOG_ERROR("Recognize error, ret {}", ret);
                return ret;
            }
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

        // ����mixer�еļ��������
        sink_.GetPersonMixer().SetRecognizedObjects(recImageResult.pedestrains);
        sink_.GetVehicleMixer().SetRecognizedObjects(recImageResult.vehicles);
        sink_.GetBikeMixer().SetRecognizedObjects(recImageResult.bikes);

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
            //�����㷨�ӿ�ʶ��
            ImageResult imageResult;
            int32_t ret = seemmoStub_->Recognize(channelId, bgr24, width, height, recParam, imageResult);
            if (0 != ret) {
                LOG_ERROR("Recognize error, ret {}", ret);
                return ret;
            }

            //����ʶ������ͼ
            LOG_INFO("-----------RECOG RESULT {} {} {}", imageResult.pedestrains.size(), imageResult.vehicles.size(),
                     imageResult.bikes.size());

            return 0;
        });

        return 0;
    }

private:
    //��a�㷨stub
    AlgoStub * seemmoStub_;
    threadpool tp_;
    //֪ͨ�̳߳�
    threadpool tpNotify_;
    //sink
    VSink &sink_;
    //ʶ��֡����
    uint32_t recogFrameCnt;
};

}