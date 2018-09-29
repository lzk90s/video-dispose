#pragma once

#include <deque>
#include <cstdint>
#include <memory>
#include <chrono>
#include <map>
#include <mutex>

#include "opencv/cv.h"

#include "vfilter/setting.h"
#include "vfilter/frame_cache.h"
#include "vfilter/mixer/bike_mixer.h"
#include "vfilter/mixer/person_mixer.h"
#include "vfilter/mixer/vehicle_mixer.h"

using namespace std;

namespace vf {

class VSink {
public:
    using OnFrameHandler = function<void (uint32_t chanelId, uint64_t frameId, cv::Mat &frame)>;

public:
    VSink(uint32_t channelId)
        : channelId_(channelId),
          lastTime_(chrono::steady_clock::now()),
          currTime_(chrono::steady_clock::now()),
          pickCnt(0) {
    }

    // 处理接收到的帧
    int32_t HandleReceivedFrame(cv::Mat &frame) {
        if (needPickFrame()) {
            //抽帧&异步处理图像帧
            cv::Mat cloneFrame = frame.clone();
            FrameCache::FrameId fid = frameCache_.Put(cloneFrame);
            onFrameHandler_(channelId_, (uint64_t)fid, cloneFrame);
        }
        mixFrame(frame);
        return 0;
    }

    FrameCache &GetFrameCache() {
        return frameCache_;
    }

    void SetFrameHandler(OnFrameHandler h) {
        this->onFrameHandler_ = h;
    }

    VehicleMixer &GetVehicleMixer() {
        return this->vehicleMixer_;
    }

    BikeMixer &GetBikeMixer() {
        return this->bikeMixer_;
    }

    PersonMixer &GetPersonMixer() {
        return this->personMixer_;
    }

protected:

    bool needPickFrame() {
        bool pickFlag = false;
        currTime_ = chrono::steady_clock::now();
        pickCnt++;
        auto diffTime = std::chrono::duration_cast<std::chrono::milliseconds>(currTime_ - lastTime_).count();
        //当时间和数量都达到的时候，才抽帧。
        //做时间限制，是为了避免高帧率的时候，疯狂抽帧。做数量限制，是为了避免低帧率的时候，抽到相同帧
        if (pickCnt>= GlobalSettings::getInstance().framePickInternalNum &&
                diffTime >= GlobalSettings::getInstance().framePickInternalMs) {
            // 时间更新&计数归零
            pickFlag = true;
            lastTime_ = currTime_;
            pickCnt = 0;
        }
        return pickFlag;
    }

    void mixFrame(cv::Mat &frame) {
        vehicleMixer_.MixFrame(frame);
        personMixer_.MixFrame(frame);
        bikeMixer_.MixFrame(frame);
    }

private:
    //channel id
    uint32_t channelId_;

    //mixer
    VehicleMixer vehicleMixer_;
    BikeMixer bikeMixer_;
    PersonMixer personMixer_;

    //frame cache
    FrameCache frameCache_;

    //time for pick frame
    chrono::steady_clock::time_point lastTime_;
    chrono::steady_clock::time_point currTime_;
    uint32_t pickCnt;

    //callback
    OnFrameHandler onFrameHandler_;
};


}
