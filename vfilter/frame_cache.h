#pragma once

#include <map>
#include <string>
#include <deque>
#include <mutex>

#include "opencv/cv.h"

#include "vfilter/setting.h"
#include "vfilter/buffered_frame.h"

using namespace std;

namespace vf {

class FrameCache {
public:
    typedef uint64_t FrameId;

public:
    FrameCache() {
        currFrameId_ = 0;
        frameCacheMaxNum_ = GlobalSettings::getInstance().frameCacheMaxNum;
        bufferedFrameType_ = GlobalSettings::getInstance().bufferedFrameType;
    }

    ~FrameCache() {
        unique_lock<mutex> lck(mutex_);
        deq_.clear();
        cache_.clear();
    }

    FrameId Put(cv::Mat &frame) {
        unique_lock<mutex> lck(mutex_);

        shared_ptr<BufferedFrame> bufferedFrame(BufferedFrameFactory::Create(bufferedFrameType_),[](BufferedFrame*ptr) {
            BufferedFrameFactory::Free(ptr);
        });
        bufferedFrame->Put(frame);

        FrameId thisId = currFrameId_;
        cache_[currFrameId_] = bufferedFrame;
        deq_.push_back(currFrameId_);
        currFrameId_++;

        freeExpiredFrame();
        return thisId;
    }

    cv::Mat Get(FrameCache::FrameId fid, bool &exist) {
        unique_lock<mutex> lck(mutex_);

        cv::Mat mat;
        if (cache_.find(fid) != cache_.end()) {
            mat = cache_[fid]->Get();
            exist = true;
        } else {
            exist = false;
        }
        return mat;
    }

    //手动释放一帧
    void ManualRelase(FrameCache::FrameId fid) {
        unique_lock<mutex> lck(mutex_);

        cache_.erase(fid);

        for (auto it = deq_.begin(); it != deq_.end();) {
            if (fid == *it) {
                deq_.erase(it++);
            } else {
                ++it;
            }
        }
    }

private:
    void freeExpiredFrame() {
        while (deq_.size() > frameCacheMaxNum_) {
            cache_.erase(deq_.front());
            deq_.pop_front();
        }
    }

private:

    uint32_t frameCacheMaxNum_;
    uint32_t bufferedFrameType_;
    FrameId currFrameId_;
    map<FrameId, shared_ptr<BufferedFrame>> cache_;
    deque<FrameId> deq_;
    mutex mutex_;
};

}