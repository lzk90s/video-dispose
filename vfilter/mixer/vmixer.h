#pragma once

#include <string>
#include <map>
#include <mutex>


#include "common/helper/stringconv.h"
#include "opencv/cv.h"
#include "opencv2/opencv.hpp"
#include "vfilter/mixer/cvx_text.h"
#include "vfilter/mixer/cvx_text.h"

using namespace std;

namespace vf {
static const char *DEFAULT_FONT = "/usr/share/fonts/truetype/simsun.ttf";
static const int DEFAULT_FONT_SIZE = 20;

class VMixer {
public:
    VMixer() : text_(DEFAULT_FONT) {
        float p = 1; //Í¸Ã÷¶È
        cv::Scalar size{ 20, 0.5, 0.1, 0 };
        text_.setFont(nullptr, &size, nullptr, &p);
    }

    virtual void MixFrame(cv::Mat &frame) = 0;

protected:
    CvxText text_;
};


}