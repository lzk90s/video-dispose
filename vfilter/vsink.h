#pragma once

#include <deque>
#include <cstdint>
#include <memory>
#include <chrono>
#include <map>
#include <mutex>

#include "opencv/cv.h"
#include "vfilter/vmixer.h"
#include "vfilter/target.h"

using namespace std;

namespace vf {

class VSink {
public:
    // ��֡ʱ���������룩
    const static int32_t FRAME_PICK_INTERNAL_MS = 200;
    // ��������֡��(5���ӵ�֡��)
    const static int32_t FRAME_CACHE_MAX_NUM = 5 * (1000 / FRAME_PICK_INTERNAL_MS);

public:
    VSink();

    // ���յ�һ֡����
    int32_t OnReceivedFrame(cv::Mat &frame);

    // ���յ��㷨����Ӧ
    int32_t OnReceivedAlgoReply();

private:
    bool needPickFrame();

    void pickFrame(cv::Mat &frame);

    void cacheFrame(cv::Mat &frame);

    void mixFrame(cv::Mat &frame);

private:
    chrono::steady_clock::time_point lastTime_;
    chrono::steady_clock::time_point currTime_;

    VMixer vmixer_;

    deque<cv::Mat> frameCache_;
    mutex frameCacheMutex_;
    TargetMap targetMap_;
    mutex targetMapMutex_;
};

}
