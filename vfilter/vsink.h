#pragma once

#include <deque>
#include <cstdint>
#include <memory>
#include <chrono>
#include <map>
#include <mutex>

#include "opencv/cv.h"
#include "vfilter/async_algo_proc.h"
#include "vfilter/mixer/bike_mixer.h"
#include "vfilter/mixer/person_mixer.h"
#include "vfilter/mixer/vehicle_mixer.h"

using namespace std;

namespace vf {

class VSink : public AsyncAlgoProcessor {
public:
    VSink(uint32_t channelId);

    // 接收到一帧数据
    int32_t OnReceivedFrame(cv::Mat &frame);


protected:
    void OnAlgoDetectReply(DetectResult &detectResult) override;

    void OnAlgoFilterReply(FilterResult &filterResult) override;

    void OnAlgoRecognizeReply(RecogResult &recResult) override;

private:
    bool needPickFrame();

    void mixFrame(cv::Mat &frame);

private:
    chrono::steady_clock::time_point lastTime_;
    chrono::steady_clock::time_point currTime_;

    uint32_t channelId_;

    VehicleMixer vehicleMixer_;
    BikeMixer bikeMixer_;
    PersonMixer personMixer_;
};

}
