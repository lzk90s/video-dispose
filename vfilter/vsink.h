#pragma once

#include <deque>
#include <cstdint>
#include <memory>

#include "opencv/include/opencv/cv.h"
#include "opencv/include/opencv2/opencv.hpp"

using namespace std;


class VSink {
public:
    const int32_t CACHE_FRAME_MAX_NUM = 300;

public:
    void StoreFrame(shared_ptr<cv::Mat> mat) {
        vframeDeque.push_back(mat);
        freeExpiredFrame();
    }

private:
    void freeExpiredFrame() {

    }

private:
    deque<shared_ptr<cv::Mat>> vframeDeque;
};
