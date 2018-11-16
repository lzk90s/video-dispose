#pragma once

#include <cstdint>
#include <chrono>
#include <vector>

#include "vfilter/core/object_sink.h"
#include "vfilter/core/frame_picker.h"
#include "vfilter/core/frame_cache.h"

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

//通道池（1个通道一个对象）
class ChannelSink {
public:
    using OnFrameHandler = function<void(ChannelSink &chl, cv::Mat &frame)>;
    using OnFrameEndHandler = function<void(ChannelSink &chl)>;

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

    //frame cache
    FrameCache frameCache;

public:
    ChannelSink(uint32_t channelId) {
        this->channelId_ = channelId;
    }

    ~ChannelSink() {
        for (auto &h : onFrameEndHandlers_) {
            h(*this);
        }
    }

    // 处理接收到的帧
    int32_t HandleReceivedFrame(cv::Mat &frame) {
        if (framePicker_.NeedPickFrame()) {
            cv::Mat cloneFrame = frame.clone();
            for (auto &h : onFrameHandlers_) {
                h(*this, cloneFrame);
            }
        }
        mixFrame(frame);
        return 0;
    }

    void RegOnFrameHander(OnFrameHandler h) {
        this->onFrameHandlers_.push_back(h);
    }

    void RegOnFrameEndHandler(OnFrameEndHandler h) {
        this->onFrameEndHandlers_.push_back(h);
    }

    uint32_t GetChannelId() {
        return this->channelId_;
    }

protected:

    void mixFrame(cv::Mat &frame) {
        vehicleMixer.MixFrame(frame, vehicleObjectSink);
        personMixer.MixFrame(frame, personObjectSink);
        bikeMixer.MixFrame(frame, bikeObjectSink);
        faceMixer.MixFrame(frame, faceObjectSink);

        vehicleObjectSink.IncreaseGofIdx();
        personObjectSink.IncreaseGofIdx();
        bikeObjectSink.IncreaseGofIdx();
        faceObjectSink.IncreaseGofIdx();
    }

private:
    //channel id
    uint32_t channelId_;
    //frame picker
    FramePicker framePicker_;
    //callback
    vector<OnFrameHandler> onFrameHandlers_;
    vector<OnFrameEndHandler> onFrameEndHandlers_;
};


}
