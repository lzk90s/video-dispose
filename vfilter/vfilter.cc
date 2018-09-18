#include "vfilter/vfilter.h"
#include "vfilter/vsink.h"

using namespace std;

namespace vf {

class VFilter {
public:
    uint32_t FilterFlow(uint8_t *bgr24, uint32_t width, uint32_t height) {
        cv::Mat frame(height, width, CV_8UC3, (void*)bgr24);
        return m_vsink.OnReceivedFrame(frame);
    }

private:
    VSink m_vsink;
};
}


static vf::VFilter vfilter;

int32_t VFilter_Open() {
    return 0;
}

int32_t VFilter_Close() {
    return 0;
}

int32_t VFilter_Routine(uint8_t *bgr24, uint32_t width, uint32_t height) {
    return vfilter.FilterFlow(bgr24, width, height);
}