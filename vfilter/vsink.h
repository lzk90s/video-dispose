#pragma once

#include <deque>
#include <cstdint>
#include <memory>
#include <chrono>
#include <map>
#include <mutex>
#include <vector>

#include "opencv/cv.h"

#include "vfilter/setting.h"
#include "vfilter/frame_cache.h"
#include "vfilter/frame_handler.h"
#include "vfilter/object_sink.h"

#include "vfilter/mixer/bike_mixer.h"
#include "vfilter/mixer/person_mixer.h"
#include "vfilter/mixer/vehicle_mixer.h"
#include "vfilter/mixer/face_mixer.h"

#include "vfilter/notifier/bike_notifier.h"
#include "vfilter/notifier/person_notifier.h"
#include "vfilter/notifier/vehicle_notifier.h"
#include "vfilter/notifier/face_notifier.h"


using namespace std;

namespace vf {

class VSink {
public:
    //object sink
    VehicleObjectSink vehicleObjectSink;
    BikeObjectSink bikeObjectSink;
    PersonObjectSink personObjectSink;
    FaceObjectSink faceObjectSink;

    //mixer
    VehicleMixer vehicleMixer;
    BikeMixer bikeMixer;
    PersonMixer personMixer;
    FaceMixer faceMixer;

    //notifier
    VehicleNotifier vehicleNotifier;
    BikeNotifier bikeNotifier;
    PersonNotifier personNotifier;
    FaceNotifier faceNotifier;

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
            for (auto &h : onFrameHandlers_) {
                h(channelId_, frame);
            }
        }
        mixFrame(frame);
        return 0;
    }

    void RegisterFrameHandler(OnFrameHandler h) {
        onFrameHandlers_.push_back(h);
    }

    uint32_t GetChannelId() {
        return this->channelId_;
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
        vehicleMixer.MixFrame(frame, vehicleObjectSink);
        personMixer.MixFrame(frame, personObjectSink);
        bikeMixer.MixFrame(frame, bikeObjectSink);
        faceMixer.MixFrame(frame, faceObjectSink);
    }

private:
    //channel id
    uint32_t channelId_;

    //time for pick frame
    chrono::steady_clock::time_point lastTime_;
    chrono::steady_clock::time_point currTime_;
    uint32_t pickCnt;

    //callback
    vector<OnFrameHandler> onFrameHandlers_;
};


}
