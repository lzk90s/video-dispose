#pragma once

#include <cstdint>
#include <chrono>
#include <vector>

#include "vfilter/core/object_sink.h"
#include "vfilter/core/frame_picker.h"
#include "vfilter/core/frame_cache.h"
#include "vfilter/core/watchdog.h"

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

    //watchdog
    Watchdog watchdog;

public:
    ChannelSink(uint32_t channelId) {
        this->channelId_ = channelId;
        watchdog.Watch([=]() {
            cout << "** OOPS! " << channelId_ << " die abnormally **" << endl;
            _exit(5);
        });
    }

    ~ChannelSink() {
    }

    uint32_t GetChannelId() {
        return this->channelId_;
    }

    void MixFrame(cv::Mat &frame) {
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
};


}
