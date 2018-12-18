#pragma once

#include <map>
#include <string>
#include <deque>
#include <mutex>

#include "opencv/cv.h"

#include "vfilter/config/setting.h"

namespace video {
namespace filter {

//保存每一帧图像中的目标
class FrameCache {
public:
    typedef uint64_t FrameId;
    typedef std::map<std::string, cv::Mat> ObjectImageMap;

public:
    FrameCache() {
        currFrameId_ = 0;
        frameCacheMaxNum_ = G_CFG().frameCacheMaxNum;
        bufferedFrameType_ = G_CFG().bufferedFrameType;
    }

    ~FrameCache() {
        std::unique_lock<std::mutex> lck(mutex_);
        deq_.clear();
        cache_.clear();
    }

    FrameId AllocateEmptyFrame() {
        std::unique_lock<std::mutex> lck(mutex_);

        FrameId thisId = currFrameId_;
        cache_[currFrameId_] = std::map<std::string, cv::Mat> {};
        deq_.push_back(currFrameId_);
        currFrameId_++;

        freeExpiredFrame();
        return thisId;
    }

    void SaveAllObjectImageInFrame(FrameId fid, ObjectImageMap &imgs) {
        std::unique_lock<std::mutex> lck(mutex_);
        cache_[fid] = imgs;
    }

    cv::Mat GetObjectImageInFrame(FrameCache::FrameId fid, const std::string &objId, bool &exist) {
        std::unique_lock<std::mutex> lck(mutex_);

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
        std::unique_lock<std::mutex> lck(mutex_);

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
    std::map<FrameId, ObjectImageMap> cache_;
    std::deque<FrameId> deq_;
    std::mutex mutex_;
};

}
}