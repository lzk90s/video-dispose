#pragma once

#include <map>
#include <string>

#include "common/helper/logger.h"
#include "algo/stub/object_type.h"
#include "vfilter/mixer/vmixer.h"

using namespace std;
using namespace algo;

namespace vf {

class FaceMixer : public VMixer<algo::FaceObject> {
public:
    FaceMixer() : VMixer("face") {}

protected:
    void doMixFrame(cv::Mat &frame, vector<algo::FaceObject> &objs1, vector<algo::FaceObject> &objs2) override {
        //人脸的不做混流
    }
};

}