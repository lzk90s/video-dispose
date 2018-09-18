#pragma once

#include <string>
#include <map>
#include "vfilter/target.h"
#include "vfilter/cvx_text.h"

using namespace std;

namespace vf {

class VMixer {
public:
    VMixer();

    void MixFrame(cv::Mat &frame, TargetMap &tm);

private:
    CvxText text_;
};
}
