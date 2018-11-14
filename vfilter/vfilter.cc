#include <map>
#include <memory>

#include "common/helper/singleton.h"
#include "common/helper/color_conv_util.h"

#include "vfilter/config/setting.h"
#include "vfilter/core/vsink.h"
#include "vfilter/vfilter.h"
#include "vfilter/proc/default_algo_proc.h"
#include "vfilter/proc/face_algo_proc.h"

using namespace std;

namespace vf {

class VFilter {
public:
    VFilter(uint32_t channelId)
        : channelId_(channelId),
          vsink_(channelId),
          defAlgoProcessor_(vsink_),
          faceAlgoProcessor_(vsink_) {
    }

    uint32_t FilterFlow(uint8_t *y, uint8_t *u, uint8_t *v, uint32_t width, uint32_t height) {
        unique_ptr<uint8_t[]> img(new uint8_t[width*height * 3] {0});
        //YUV420P->BGR24
        I420ToBGR24Converter::Convert(y, u, v, img.get(), width, height);
        cv::Mat frame = cv::Mat(height, width, CV_8UC3, (void*)img.get());
        vsink_.HandleReceivedFrame(frame);
        //BGR24->YUV420P
        BGR24ToI420Converter::Convert(y, u, v, frame.data, frame.cols, frame.rows);
        return 0;
    }

private:
    uint32_t channelId_;
    VSink vsink_;
    DefaultAlgoProcessor defAlgoProcessor_;
    FaceAlgoProcessor faceAlgoProcessor_;
};

class VFilterManager {
public:
    map<uint32_t, shared_ptr<VFilter>> vfMap;

    ~VFilterManager() {
        vfMap.clear();
    }
};

typedef Singleton<vf::VFilterManager> VFilterManagerSingleton;
}


int32_t VFilter_Init() {
    //初始化
    //vf::ThisApp::getInstance();
    vf::GlobalSettings::getInstance();
    vf::VFilterManagerSingleton::getInstance();
    return 0;
}

int32_t VFilter_Destroy() {
    return 0;
}

int32_t VFilter_Routine(uint32_t channelId, uint8_t *y, uint8_t *u, uint8_t *v, uint32_t width, uint32_t height) {
    //暂不加锁，目前不是多线程，后续有需求再加
    if (vf::VFilterManagerSingleton::getInstance().vfMap.find(channelId) ==
            vf::VFilterManagerSingleton::getInstance().vfMap.end()) {
        LOG_INFO("New channel {}", channelId);
        vf::VFilterManagerSingleton::getInstance().vfMap[channelId] = make_shared<vf::VFilter>(channelId);
    }
    return vf::VFilterManagerSingleton::getInstance().vfMap[channelId]->FilterFlow(y, u, v, width, height);
}