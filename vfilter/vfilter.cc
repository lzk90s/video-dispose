#include <map>
#include <memory>

#include "common/helper/singleton.h"

#include "vfilter/app.h"
#include "vfilter/vfilter.h"
#include "vfilter/vsink.h"

using namespace std;

namespace vf {

class VFilter {
public:
    VFilter(uint32_t channelId) : channelId_(channelId), vsink_(channelId) {

    }
    void SetChannelId(uint32_t channelId) {
        this->channelId_ = channelId;
    }

    uint32_t FilterFlow(uint8_t *bgr24, uint32_t width, uint32_t height) {
        cv::Mat frame = cv::Mat(height, width, CV_8UC3, (void*)bgr24);
        return vsink_.OnReceivedFrame(frame);
    }

private:
    uint32_t channelId_;
    VSink vsink_;
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
    vf::ThisApp::getInstance();
    vf::VFilterManagerSingleton::getInstance();
    return 0;
}

int32_t VFilter_Destroy() {
    return 0;
}

int32_t VFilter_Routine(uint32_t channelId, uint8_t *bgr24, uint32_t width, uint32_t height) {
    //暂不加锁，目前不是多线程，后续有需求再加
    if (vf::VFilterManagerSingleton::getInstance().vfMap.find(channelId) ==
            vf::VFilterManagerSingleton::getInstance().vfMap.end()) {
        vf::VFilterManagerSingleton::getInstance().vfMap[channelId] = make_shared<vf::VFilter>(channelId);
    }
    return vf::VFilterManagerSingleton::getInstance().vfMap[channelId]->FilterFlow(bgr24, width, height);
}