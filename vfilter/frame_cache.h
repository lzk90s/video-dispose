#pragma once

#include <map>
#include <string>
#include <deque>
#include <mutex>

#include "opencv/cv.h"


using namespace std;

namespace vf {
class FrameCache {
public:
    typedef uint64_t FrameId;

    // 抽帧时间间隔（毫秒）
    const static int32_t FRAME_PICK_INTERNAL_MS = 80;
    // 抽帧间隔数目
    const static int32_t FRAME_PICK_INTERNAL_NUM = 3;
    // 缓存的最大帧数(30秒的帧数)
    const static int32_t FRAME_CACHE_MAX_NUM = 10 * (1000 / FRAME_PICK_INTERNAL_MS);

public:
    FrameCache() : currFrameId_(0) {}

    ~FrameCache() {
        unique_lock<mutex> lck(mutex_);
        deq_.clear();
        cache_.clear();
    }

    FrameId Put(cv::Mat &frame) {
        unique_lock<mutex> lck(mutex_);

        FrameId thisId = currFrameId_;
        cache_[currFrameId_] = frame;
        deq_.push_back(currFrameId_);
        currFrameId_++;
        freeExpiredFrame();
        return thisId;
    }

    cv::Mat Get(FrameCache::FrameId fid, bool &exist) {
        unique_lock<mutex> lck(mutex_);

        cv::Mat mat;
        if (cache_.find(fid) != cache_.end()) {
            mat = cache_[fid];
            exist = true;
        } else {
            exist = false;
        }
        return mat;
    }

private:
    void freeExpiredFrame() {
        while (deq_.size() > FRAME_CACHE_MAX_NUM) {
            cache_.erase(deq_.front());
            deq_.pop_front();
        }
    }


private:

    //frame cache
    FrameId currFrameId_;
    map<FrameId, cv::Mat> cache_;
    deque<FrameId> deq_;
    mutex mutex_;
};

}