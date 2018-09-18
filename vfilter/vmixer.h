#pragma once

#include <map>
#include "vfilter/target.h"
#include "opencv/cv.h"
#include "opencv2/opencv.hpp"

using namespace std;

class VMixer {
public:

    void MixFrame(cv::Mat &frame);

private:
    map<string, DetectTarget> targetMap;
};