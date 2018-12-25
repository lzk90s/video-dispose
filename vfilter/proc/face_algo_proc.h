#pragma once

#include "common/helper/threadpool.h"

#include "algo/stub/algo_stub_factory.h"
#include "vfilter/proc/abstract_algo_proc.h"

namespace video {
namespace filter {

//人脸的单独出来，是因为人脸的效果还需要优化，避免影响其他算法的效果
class FaceAlgoProcessor : public AbstractAlgoProcessor {
public:
    FaceAlgoProcessor()
        : AbstractAlgoProcessor("face", G_CFG().gosunFramePickInterval),
          algo_(algo::AlgoStubFactory::NewAlgoStub("gosun")),
          worker_(1) {
    }

    ~FaceAlgoProcessor() {
        waitForComplete();
    }

protected:
    void onFrame(std::shared_ptr<ChannelSink> chl, cv::Mat &frame) override {
        uint64_t frameId = chl->frameCache.AllocateEmptyFrame();
        worker_.commit(std::bind(&FaceAlgoProcessor::algoRoutine, this, chl, frameId, frame));
    }

    void onFrameEnd(std::shared_ptr<ChannelSink> chl) override {
        waitForComplete();
    }

    void mixFrame(std::shared_ptr<ChannelSink> chl, cv::Mat &frame) override {
        chl->faceMixer.MixFrame(frame, chl->faceObjectSink);
        chl->faceObjectSink.IncreaseGofIdx();
    }

private:
    void waitForComplete() {
        while (true) {
            if ((worker_.taskCount() == 0)) {
                break;
            }
            LOG_INFO("Waiting for uncompleted tasks, count ({})", worker_.taskCount());
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    int32_t algoRoutine(std::shared_ptr<ChannelSink> chl, uint64_t frameId, cv::Mat &frame) {
        chl->watchdog.Feed();
        return trailAndRecognize(chl, frameId, frame);
    }

    int32_t trailAndRecognize(std::shared_ptr<ChannelSink> chl, uint64_t frameId, cv::Mat &frame) {
        int32_t ret = 0;
        const uint8_t *bgr24 = frame.data;
        uint32_t width = frame.cols;
        uint32_t height = frame.rows;

        algo::TrailParam trailParam;
        trailParam.roi.push_back(algo::Point{ 0, 0 });
        trailParam.roi.push_back(algo::Point{ 0, (int32_t)height });
        trailParam.roi.push_back(algo::Point{ (int32_t)width, (int32_t)height });
        trailParam.roi.push_back(algo::Point{ (int32_t)width, 0 });

        algo::ImageResult imageResult;
        algo::FilterResult filterResult;
        ret = algo_->Trail(chl->GetChannelId(), frameId, bgr24, width, height, trailParam, imageResult, filterResult);
        if (0 != ret) {
            LOG_ERROR("Face trail error, ret {}", ret);
            return ret;
        }

        for (auto &p : imageResult.faces) {
            //针对人脸特殊处理，目前人脸算法中没有去重，这样简单处理
            if (!chl->faceObjectSink.ObjectExist(p.guid)) {
                LOG_INFO("New face object {}", p.guid);
                chl->faceNotifier.OnRecognizedObject(chl->GetChannelId(), frame, p);
            }
        }
        chl->faceObjectSink.OnDetectedObjects(imageResult.faces);

        return 0;
    }

private:
    std::shared_ptr<algo::AlgoStub> algo_;
    std::threadpool worker_;
};

}
}
