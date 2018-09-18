#include <chrono>
#include <memory>
#include "common/helper/threadpool.h"
#include "vfilter/vfilter.h"
#include "vfilter/vsink.h"
#include "vfilter/vmixer.h"

using namespace std;

class VFilter {
    static const int32_t FILTER_INTERNAL_MS = 200;

public:
    VFilter() : algoWorker(1), lastTp(chrono::steady_clock::now()), currTp(chrono::steady_clock::now()) {

    }
    ~VFilter() {

    }

    uint32_t FilterFlow(uint8_t *bgr24, uint32_t width, uint32_t height) {
        cv::Mat frame(height, width, CV_8UC3, (void*)bgr24);

//         if (needPickFrame()) {
//             pickFrame(mat);
//         }
//
        vmixer.MixFrame(frame);
    }

private:
    bool needPickFrame() {
        currTp = chrono::steady_clock::now();
        auto diffTp = std::chrono::duration_cast<std::chrono::milliseconds>(currTp - lastTp).count();
        return diffTp >= FILTER_INTERNAL_MS;
    }

    void pickFrame(shared_ptr<cv::Mat> mat) {
        vsink.StoreFrame(mat);
    }



private:
    chrono::steady_clock::time_point lastTp;
    chrono::steady_clock::time_point currTp;
    VMixer vmixer;
    VSink vsink;
    threadpool algoWorker;
};

VFilter vfilter;

int32_t VFilter_Open() {

}

int32_t VFilter_Close() {

}

int32_t VFilter_Routine(uint8_t *bgr24, uint32_t width, uint32_t height) {
    return vfilter.FilterFlow(bgr24, width, height);
}
