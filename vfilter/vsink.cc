
#include "common/helper/threadpool.h"
#include "vfilter/vsink.h"
#include "vfilter/async_algo_proc.h"

namespace vf {

VSink::VSink(uint32_t channelId)
    : channelId_(channelId),
      lastTime_(chrono::steady_clock::now()),
      currTime_(chrono::steady_clock::now()) {
}

int32_t VSink::OnReceivedFrame(cv::Mat &frame) {
    if (needPickFrame()) {
        //抽帧&异步处理图像帧
        cv::Mat cloneFrame = frame.clone();
        FrameCache::FrameId fid = CacheFrame(cloneFrame);
        AsyncProcessFrame(channelId_, (uint64_t)fid, cloneFrame.data, cloneFrame.cols, cloneFrame.rows);
    }
    mixFrame(frame);
    return 0;
}

void VSink::OnAlgoDetectReply(DetectResult &detectResult) {
    personMixer_.SetDetectedObjects(detectResult.pedestrains);
    vehicleMixer_.SetDetectedObjects(detectResult.vehicles);
    bikeMixer_.SetDetectedObjects(detectResult.bikes);
}

void VSink::OnAlgoFilterReply(FilterResult &filterResult) {
    for (auto s : filterResult.pedestrains) {
        LOG_INFO("RECV FILTER: {}, {}", s.guid, s.frameId);
    }
}

void VSink::OnAlgoRecognizeReply(RecogResult &recResult) {
    LOG_INFO("--111-----vehicles ---{}", recResult.vehicles.size());
    LOG_INFO("---111----pedestrains ---{}", recResult.pedestrains.size());
    LOG_INFO("---111----bikes ---{}", recResult.bikes.size());

    personMixer_.SetRecognizedObjects(recResult.pedestrains);
    vehicleMixer_.SetRecognizedObjects(recResult.vehicles);
    bikeMixer_.SetRecognizedObjects(recResult.bikes);
}

bool VSink::needPickFrame() {
    bool pickFlag = false;
    currTime_ = chrono::steady_clock::now();
    auto diffTime = std::chrono::duration_cast<std::chrono::milliseconds>(currTime_ - lastTime_).count();
    if (diffTime >= FrameCache::FRAME_PICK_INTERNAL_MS) {
        pickFlag = true;
        // 更新当前时间
        lastTime_ = currTime_;
    }
    return pickFlag;
}

void VSink::mixFrame(cv::Mat &frame) {
    this->vehicleMixer_.MixFrame(frame);
    this->personMixer_.MixFrame(frame);
    this->bikeMixer_.MixFrame(frame);
}

}
