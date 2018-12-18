#pragma once

#include "algo/stub/object_type.h"
#include "vfilter/mixer/vmixer.h"

namespace video {
namespace filter {

class FaceMixer : public VMixer<algo::FaceObject> {
public:
    FaceMixer() : VMixer("face") {}

protected:
    void doMixFrame(cv::Mat &frame, std::vector<algo::FaceObject> &objs1, std::vector<algo::FaceObject> &objs2) override {
        //人脸的不做混流
    }
};

}
}