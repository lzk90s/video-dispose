#pragma once

#include <deque>
#include <cstdint>
#include <memory>
#include <chrono>
#include <map>
#include <mutex>
#include <vector>

#include "opencv/cv.h"

#include "vfilter/frame_handler.h"
#include "vfilter/object_sink.h"
#include "vfilter/frame_picker.h"

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
    VSink(uint32_t channelId) {
        this->channelId_ = channelId;
    }

    // ������յ���֡
    int32_t HandleReceivedFrame(cv::Mat &frame) {
        if (framePicker_.NeedPickFrame()) {
            cv::Mat cloneFrame = frame.clone();
            for (auto &h : onFrameHandlers_) {
                h(channelId_, cloneFrame);
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

    void mixFrame(cv::Mat &frame) {
        vehicleMixer.MixFrame(frame, vehicleObjectSink);
        personMixer.MixFrame(frame, personObjectSink);
        bikeMixer.MixFrame(frame, bikeObjectSink);
        faceMixer.MixFrame(frame, faceObjectSink);
    }

private:
    //channel id
    uint32_t channelId_;
    //frame picker
    FramePicker framePicker_;
    //callback
    vector<OnFrameHandler> onFrameHandlers_;
};


}
