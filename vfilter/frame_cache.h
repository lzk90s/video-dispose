#pragma once

#include <map>
#include <string>
#include <deque>
#include <mutex>

#include "opencv/cv.h"

#include "common/helper/logger.h"

#include "vfilter/setting.h"
#include "vfilter/buffered_frame.h"

using namespace std;

namespace vf {

//保存每一帧图像中的目标
class FrameCache {
public:
    typedef uint64_t FrameId;
    typedef map<string, cv::Mat> ObjectImageMap;

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

    FrameId AllocateEmptyFrame() {
        unique_lock<mutex> lck(mutex_);

        FrameId thisId = currFrameId_;
        cache_[currFrameId_] = map<string, cv::Mat> {};
        deq_.push_back(currFrameId_);
        currFrameId_++;

        freeExpiredFrame();
        return thisId;
    }

    void SaveObjectImageInFrame(FrameId fid, ObjectImageMap &imgs) {
        unique_lock<mutex> lck(mutex_);
        cache_[fid] = imgs;
    }

    cv::Mat GetOneObjectImage(FrameCache::FrameId fid, const string &objId, bool &exist) {
        unique_lock<mutex> lck(mutex_);

        cv::Mat mat;
        if (cache_.find(fid) != cache_.end()) {
            auto &m = cache_[fid];
            if (m.find(objId) != m.end()) {
                exist = true;
                mat = m[objId];
            }
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
    map<FrameId, ObjectImageMap> cache_;
    deque<FrameId> deq_;
    mutex mutex_;
};

}