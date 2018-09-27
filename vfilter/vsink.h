#pragma once

#include <deque>
#include <cstdint>
#include <memory>
#include <chrono>
#include <map>
#include <mutex>

#include "opencv/cv.h"
#include "vfilter/frame_cache.h"
#include "vfilter/mixer/bike_mixer.h"
#include "vfilter/mixer/person_mixer.h"
#include "vfilter/mixer/vehicle_mixer.h"

using namespace std;

namespace vf {

class VSink {
public:
    using OnFrameHandler =
        function<void (uint32_t chanelId, uint64_t frameId, uint8_t *bgr24, uint32_t width, uint32_t height)>;

public:
    VSink(uint32_t channelId)
        : channelId_(channelId),
          lastTime_(chrono::steady_clock::now()),
          currTime_(chrono::steady_clock::now()) {
    }

    // 处理接收到的帧
    int32_t HandleReceivedFrame(cv::Mat &frame) {
        if (needPickFrame()) {
            //抽帧&异步处理图像帧
            cv::Mat cloneFrame = frame.clone();
            FrameCache::FrameId fid = CacheFrame(cloneFrame);
            onFrameHandler_(channelId_, (uint64_t)fid, cloneFrame.data, cloneFrame.cols, cloneFrame.rows);
        }
        mixFrame(frame);
        return 0;
    }

    FrameCache::FrameId CacheFrame(cv::Mat &frame) {
        return frameCache_.Put(frame);
    }

    cv::Mat GetFrame(FrameCache::FrameId id, bool &exist) {
        return frameCache_.Get(id, exist);
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
        auto diffTime = std::chrono::duration_cast<std::chrono::milliseconds>(currTime_ - lastTime_).count();
        if (diffTime >= FrameCache::FRAME_PICK_INTERNAL_MS) {
            pickFlag = true;
            // 更新当前时间
            lastTime_ = currTime_;
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

    //callback
    OnFrameHandler onFrameHandler_;
};


}
