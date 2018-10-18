#include <map>
#include <memory>

#include "common/helper/singleton.h"
#include "common/helper/counttimer.h"

#include "vfilter/app.h"
#include "vfilter/setting.h"
#include "vfilter/vfilter.h"
#include "vfilter/vsink.h"

#include "vfilter/algo_proc.h"

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
    void SetChannelId(uint32_t channelId) {
        this->channelId_ = channelId;
    }

    uint32_t FilterFlow(uint8_t *bgr24, uint32_t width, uint32_t height) {
        cv::Mat frame = cv::Mat(height, width, CV_8UC3, (void*)bgr24);
        return vsink_.HandleReceivedFrame(frame);
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
    //��ʼ��
    //vf::ThisApp::getInstance();
    vf::GlobalSettings::getInstance();
    vf::VFilterManagerSingleton::getInstance();
    return 0;
}

int32_t VFilter_Destroy() {
    return 0;
}

int32_t VFilter_Routine(uint32_t channelId, uint8_t *bgr24, uint32_t width, uint32_t height) {
    //�ݲ�������Ŀǰ���Ƕ��̣߳������������ټ�
    if (vf::VFilterManagerSingleton::getInstance().vfMap.find(channelId) ==
            vf::VFilterManagerSingleton::getInstance().vfMap.end()) {
        LOG_INFO("New channel {}", channelId);
        vf::VFilterManagerSingleton::getInstance().vfMap[channelId] = make_shared<vf::VFilter>(channelId);
    }
    return vf::VFilterManagerSingleton::getInstance().vfMap[channelId]->FilterFlow(bgr24, width, height);
}