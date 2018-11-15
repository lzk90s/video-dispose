#include <map>
#include <memory>

#include "common/helper/singleton.h"
#include "common/helper/color_conv_util.h"

#include "vfilter/core/app.h"
#include "vfilter/config/setting.h"
#include "vfilter/core/channel_sink_mng.h"
#include "vfilter/vfilter.h"

#include "vfilter/proc/default_algo_proc.h"
#include "vfilter/proc/face_algo_proc.h"

using namespace std;

static shared_ptr<vf::AbstractAlgoProcessor> defaultProcessor;
static shared_ptr<vf::AbstractAlgoProcessor> faceProcessor;

int32_t VFilter_Init() {
    vf::ThisApp::getInstance();
    defaultProcessor.reset(new vf::DefaultAlgoProcessor());
    faceProcessor.reset(new vf::FaceAlgoProcessor());
    return 0;
}

int32_t VFilter_Destroy() {
    return 0;
}

int32_t VFilter_Routine(uint32_t channelId, uint8_t *y, uint8_t *u, uint8_t *v, uint32_t width, uint32_t height) {
    if (vf::CSMS().sinks.find(channelId) == vf::CSMS().sinks.end()) {
        auto chl = make_shared<vf::ChannelSink>(channelId);
        defaultProcessor->LinkHandler(*chl);
        faceProcessor->LinkHandler(*chl);
        vf::CSMS().sinks[channelId] = chl;
        LOG_INFO("New channel {}", channelId);
    }

    unique_ptr<uint8_t[]> img(new uint8_t[width*height * 3] { 0 });
    //YUV420P->BGR24
    I420ToBGR24Converter::Convert(y, u, v, img.get(), width, height);
    cv::Mat frame = cv::Mat(height, width, CV_8UC3, (void*)img.get());
    vf::CSMS().sinks[channelId]->HandleReceivedFrame(frame);
    //BGR24->YUV420P
    BGR24ToI420Converter::Convert(y, u, v, frame.data, frame.cols, frame.rows);

    return 0;
}