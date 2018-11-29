#include <map>
#include <memory>

#include "common/helper/singleton.h"
#include "common/helper/color_conv_util.h"
#include "common/helper/timer.h"

#include "vfilter/core/app.h"
#include "vfilter/config/setting.h"
#include "vfilter/core/channel_sink_mng.h"
#include "vfilter/vfilter.h"

#include "vfilter/proc/default_algo_proc.h"
#include "vfilter/proc/face_algo_proc.h"

using namespace std;

//看门狗，虽然是多线程，没有加锁的必要
class Watchdog {
public:
    ~Watchdog() {
        timer.Expire();
    }

    void Watch(std::function<void()> die) {
        timer.StartTimer(1000, [=]() {
            //10秒钟还没有喂狗，表示程序可能出现问题了，强制 go die
            if (dogCount_++ > 10) {
                die();
            }
        });
    }

    void Feed() {
        dogCount_ = 0;
    }

private:
    Timer timer;
    uint32_t dogCount_;
};

static shared_ptr<vf::AbstractAlgoProcessor> defaultProcessor;
static shared_ptr<vf::AbstractAlgoProcessor> faceProcessor;
static Watchdog watchdog;

int32_t VFilter_Init() {
    vf::ThisApp::getInstance();
    defaultProcessor.reset(new vf::DefaultAlgoProcessor());
    faceProcessor.reset(new vf::FaceAlgoProcessor());
    //启动看门狗监听
    watchdog.Watch([]() {
        cout << "-------GO DIE, oops!---------" << endl;
        _exit(5);
    });
    return 0;
}

int32_t VFilter_Destroy() {
    vf::CSMS().sinks.clear();
    defaultProcessor.reset();
    faceProcessor.reset();
    return 0;
}

int32_t VFilter_Routine(uint32_t channelId,
                        uint8_t *y, uint32_t stride_y,
                        uint8_t *u, uint32_t stride_u,
                        uint8_t *v, uint32_t stride_v,
                        uint32_t width, uint32_t height) {
    if (vf::CSMS().sinks.find(channelId) == vf::CSMS().sinks.end()) {
        auto chl = make_shared<vf::ChannelSink>(channelId);
        defaultProcessor->LinkHandler(*chl);
        faceProcessor->LinkHandler(*chl);
        vf::CSMS().sinks[channelId] = chl;
        LOG_INFO("New channel {}, {}*{}", channelId, width, height);
    }

    //feed the watch dog
    watchdog.Feed();

    //allocate mat memory
    cv::Mat frame = cv::Mat(height, width, CV_8UC3);

    //YUV420P->BGR24
    I420ToBGR24Converter::Convert(y, stride_y, u, stride_u, v, stride_v, frame.data, width*3, width, height);

    //handle frame
    vf::CSMS().sinks[channelId]->HandleReceivedFrame(frame);

    //BGR24->YUV420P
    BGR24ToI420Converter::Convert(frame.data, width*3, y, stride_y, u, stride_u, v, stride_v, width, height);

    return 0;
}