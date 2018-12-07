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

static shared_ptr<vf::AbstractAlgoProcessor> defaultProcessor;
static shared_ptr<vf::AbstractAlgoProcessor> faceProcessor;

int32_t VFilter_Init() {
    vf::ThisApp::getInstance();
    if (vf::G_CFG().enableSeemmoAlgo) {
        defaultProcessor.reset(new vf::DefaultAlgoProcessor());
    }
    if (vf::G_CFG().enableGosunAlgo) {
        faceProcessor.reset(new vf::FaceAlgoProcessor());
    }
    return 0;
}

int32_t VFilter_Destroy() {
    //close all channel
    for (auto &p : vf::CSMS().sinks) {
        if (nullptr != defaultProcessor) {
            defaultProcessor->OnChannelClose(p.second);
        }
        if (nullptr != faceProcessor) {
            faceProcessor->OnChannelClose(p.second);
        }
    }
    vf::CSMS().sinks.clear();
    if (nullptr != defaultProcessor) {
        defaultProcessor.reset();
    }
    if (nullptr != faceProcessor) {
        faceProcessor.reset();
    }
    return 0;
}

int32_t VFilter_Routine(uint32_t channelId,
                        uint8_t *y, uint32_t stride_y,
                        uint8_t *u, uint32_t stride_u,
                        uint8_t *v, uint32_t stride_v,
                        uint32_t width, uint32_t height) {
    if (vf::CSMS().sinks.find(channelId) == vf::CSMS().sinks.end()) {
        auto chl = make_shared<vf::ChannelSink>(channelId);
        vf::CSMS().sinks[channelId] = chl;
        LOG_INFO("New channel {}, {}*{}", channelId, width, height);
    }

    //allocate mat memory
    cv::Mat frame = cv::Mat(height, width, CV_8UC3);

    //YUV420P->BGR24
    I420ToBGR24Converter::Convert(y, stride_y, u, stride_u, v, stride_v, frame.data, width*3, width, height);

    //handle frame
    auto chl = vf::CSMS().sinks[channelId];
    if (nullptr != defaultProcessor) {
        defaultProcessor->OnChannelReceivedFrame(chl, frame);
    }
    if (nullptr != faceProcessor) {
        faceProcessor->OnChannelReceivedFrame(chl, frame);
    }

    //BGR24->YUV420P
    BGR24ToI420Converter::Convert(frame.data, width*3, y, stride_y, u, stride_u, v, stride_v, width, height);

    return 0;
}