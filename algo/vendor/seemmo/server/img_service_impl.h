#pragma once

#include <memory>
#include <string>

#include <gflags/gflags.h>
#include <butil/logging.h>
#include <brpc/server.h>

#include "common/helper/logger.h"
#include "common/helper/jpeg_helper.h"

#include "algo/vendor/seemmo/rpc/service.pb.h"
#include "algo/vendor/seemmo/server/algo_loader.h"

#include "algo/vendor/seemmo/stub_impl/detect_param_builder.h"
#include "algo/vendor/seemmo/stub_impl/detect_result_parser.h"
#include "algo/vendor/seemmo/stub_impl/rec_param_builder.h"

namespace algo {
namespace seemmo {

class ImgProcServiceImpl : public ImgProcService {
public:
    ImgProcServiceImpl(algo::seemmo::AlgoLoader &algo) : algo_(algo) {
        timestamp_ = 0;
    };

    virtual ~ImgProcServiceImpl() {};

    virtual void Recognize(::google::protobuf::RpcController* controller,
                           const ::algo::seemmo::ImgRecognizeRequest* request, ::algo::seemmo::ImgRecognizeReply* response,
                           ::google::protobuf::Closure* done) {
        brpc::ClosureGuard done_guard(done);

        brpc::Controller* cntl =
            static_cast<brpc::Controller*>(controller);

        Jpeg2BgrConverter converter;
        if (0 != converter.Convert((uint8_t*)request->jpeg().c_str(), request->jpeg().length())) {
            LOG_INFO("Failed to convert jpeg image");
            cntl->SetFailed(400, "jpeg decompress error");
            return;
        }

        int ret = 0;

        //检测（全图）
        uint32_t bufLen1 = 1024 * 1024 * 5;
        unique_ptr<char[]> buf1(new char[bufLen1]);
        detect::DetectRegionPO detectRegionPo;
        detectRegionPo.regions.push_back({ 0, 0 });
        detectRegionPo.regions.push_back({ 0, converter.GetHeight() });
        detectRegionPo.regions.push_back({ converter.GetWidth(), converter.GetHeight() });
        detectRegionPo.regions.push_back({ converter.GetWidth(), 0 });

        ret = algo_.Trail(
                  -1,
                  timestamp_++,
                  converter.GetImgBuffer(),
                  converter.GetWidth(),
                  converter.GetHeight(),
                  detect::DetectParamBuilder().Build(detectRegionPo),
                  buf1.get(),
                  bufLen1);
        if (0 != ret) {
            LOG_ERROR("Trail image error, {}", ret);
            cntl->SetFailed("Trail image error, code " + std::to_string(ret));
            return;
        }

        //根据检测结果生成识别参数
        vector<rec::LocationPO> locs;
        detect::DetectReplyPO detectReply;
        detect::DetectResponseParser().Parse(buf1.get(), detectReply);
        buildRecLocationByImageResult(detectReply, locs);

        if (locs.empty()) {
            response->set_data(buildEmptyResponse().c_str());
            return;
        }

        //识别
        uint32_t bufLen2 = 1024 * 1024 * 5;
        unique_ptr<char[]> buf2(new char[bufLen2]);
        ret = algo_.Recognize(
                  converter.GetImgBuffer(),
                  converter.GetWidth(),
                  converter.GetHeight(),
                  rec::RecParamBuilder().Build(locs).c_str(),
                  buf2.get(),
                  bufLen2);
        if (0 != ret) {
            LOG_ERROR("Recognize image error, {}", ret);
            cntl->SetFailed("Recognize image error, code " + std::to_string(ret));
            return;
        }
        response->set_data(buf2.get(), bufLen2);
    }

private:
    void buildRecLocationByImageResult(detect::DetectReplyPO &p, vector<rec::LocationPO> &locs) {
        if (p.Code == 0) {
            //目前没有用批量方式，所以，只取第一个imageresult即可
            detect::ImageResultPO root = p.ImageResults.at(0);
            for (auto &a : root.Bikes) {
                rec::LocationPO tmp;
                tmp.Type = a.Type;
                tmp.Trail = a.Trail;
                tmp.Rect = a.Detect.Body.Rect;
                locs.push_back(tmp);
            }
            for (auto &a : root.Pedestrains) {
                rec::LocationPO tmp;
                tmp.Type = a.Type;
                tmp.Trail = a.Trail;
                tmp.Rect = a.Detect.Body.Rect;
                locs.push_back(tmp);
            }
            for (auto &a : root.Vehicles) {
                rec::LocationPO tmp;
                tmp.Type = a.Type;
                tmp.Trail = a.Trail;
                tmp.Rect = a.Detect.Body.Rect;
                locs.push_back(tmp);
            }
        }
    }

    string buildEmptyResponse() {
        string msg = "{\"Code\":0, \"Message\":\"ok\", \"ImageResults\":[]}";
        return msg;
    }

private:
    AlgoLoader & algo_;
    uint64_t timestamp_;
};

}
}