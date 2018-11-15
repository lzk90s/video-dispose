#pragma once

#include "algo/stub/algo_stub_factory.h"
#include "vfilter/proc/abstract_algo_proc.h"

using namespace std;

namespace vf {

//人脸的单独出来，是因为人脸的效果还需要优化，避免影响其他算法的效果
class FaceAlgoProcessor : public AbstractAlgoProcessor {
public:
    FaceAlgoProcessor() {
        string vendor = GlobalSettings::getInstance().enableGosunAlgo ? "gosun" : "null";
        algo_ = algo::AlgoStubFactory::NewAlgoStub(vendor);
    }

protected:

    int32_t algoRoutine(ChannelSink &chl, uint64_t frameId, cv::Mat &frame) override {
        return trailAndRecognize(chl, frameId, frame);
    }

private:
    int32_t trailAndRecognize(ChannelSink &chl, uint64_t frameId, cv::Mat &frame) {
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

        ImageResult imageResult;
        FilterResult filterResult;
        ret = algo_->Trail(chl.GetChannelId(), frameId, bgr24, width, height, trailParam, imageResult, filterResult);
        if (0 != ret) {
            LOG_ERROR("Trail error, ret {}", ret);
            return ret;
        }

        for (auto &p : imageResult.faces) {
            //针对人脸特殊处理，目前人脸算法中没有去重，这样简单处理
            if (!chl.faceObjectSink.ObjectExist(p.guid)) {
                LOG_INFO("New face object {}", p.guid);
                chl.faceNotifier.OnRecognizedObject(chl.GetChannelId(), frame, p);
            }
        }
        chl.faceObjectSink.OnDetectedObjects(imageResult.faces);

        return 0;
    }

private:
    shared_ptr<algo::AlgoStub> algo_;
};

}