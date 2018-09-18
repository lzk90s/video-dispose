
#include "common/helper/threadpool.h"
#include "vfilter/vsink.h"
#include "vfilter/algo_async_caller.h"

namespace vf {

VSink::VSink() : lastTime_(chrono::steady_clock::now()), currTime_(chrono::steady_clock::now()) {

}

int32_t VSink::OnReceivedFrame(cv::Mat &frame) {
    if (needPickFrame()) {
        //Âú×ã³éÖ¡Ìõ¼þ£¬³éÖ¡
        pickFrame(frame);
    }
    mixFrame(frame);
    return 0;
}

bool VSink::needPickFrame() {
    currTime_ = chrono::steady_clock::now();
    auto diffTime = std::chrono::duration_cast<std::chrono::milliseconds>(currTime_ - lastTime_).count();
    return diffTime >= FRAME_PICK_INTERNAL_MS;
}

void VSink::pickFrame(cv::Mat &frame) {
    cv::Mat cloneFrame = frame.clone();
    cacheFrame(cloneFrame);

}

void VSink::cacheFrame(cv::Mat &frame) {
    lock_guard<std::mutex> lck(frameCacheMutex_);
    frameCache_.push_back(frame);
    //¶ªµô¹ýÆÚµÄÖ¡
    if (frameCache_.size() > FRAME_CACHE_MAX_NUM) {
        frameCache_.pop_front();
    }
}

void VSink::mixFrame(cv::Mat &frame) {
    TargetMap tmp;
    {
        lock_guard<std::mutex> lck(targetMapMutex_);
        // copy
        tmp = targetMap_;
    }
    vmixer_.MixFrame(frame, tmp);
}

}
