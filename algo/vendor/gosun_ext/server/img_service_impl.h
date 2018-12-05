#pragma once

#include <memory>
#include <string>

#include <gflags/gflags.h>
#include <butil/logging.h>
#include <brpc/server.h>

#include "common/helper/logger.h"
#include "common/helper/jpeg_helper.h"

#include "algo/vendor/gosun_ext/rpc/service.pb.h"
#include "algo/vendor/gosun_ext/server/algo_loader.h"


namespace algo {
namespace gosun_ext {

class ImgProcServiceImpl : public ImgProcService {
public:
    ImgProcServiceImpl(algo::gosun_ext::AlgoLoader &algo) : algo_(algo) {
        timestamp_ = 0;
    };

    virtual ~ImgProcServiceImpl() {};

    virtual void Recognize(::google::protobuf::RpcController* controller,
                           const ::algo::gosun_ext::ImgRecognizeRequest* request, ::algo::gosun_ext::ImgRecognizeReply* response,
                           ::google::protobuf::Closure* done) {
        brpc::ClosureGuard done_guard(done);

        brpc::Controller* cntl =
            static_cast<brpc::Controller*>(controller);

        Jpeg2BgrConverter converter;
        if (0 != converter.Convert((uint8_t*)request->imagedata().c_str(), request->imagedata().length())) {
            LOG_INFO("Failed to convert jpeg image");
            cntl->SetFailed(400, "jpeg decompress error");
            return;
        }

        //识别
        uint32_t bufLen2 = 1024 * 1024 * 5;
        unique_ptr<char[]> buf2(new char[bufLen2]);
        int ret = algo_.DetectRecognize(
                      converter.GetImgBuffer(),
                      converter.GetWidth(),
                      converter.GetHeight(),
                      request->calcparam(),
                      buf2.get(),
                      bufLen2);
        if (0 != ret) {
            LOG_ERROR("Recognize image error, {}", ret);
            cntl->SetFailed("DetectRecognize image error, code " + std::to_string(ret));
            return;
        }
        response->set_data(buf2.get(), bufLen2);
    }

private:

    string buildEmptyResponse() {
        string msg = "{\"Code\":0, \"Message\":\"ok\", \"ImageResults\":[]}";
        return msg;
    }

    string& replaceAll(string& str, const string&   oldValue, const string& newValue) {
        while (true) {
            string::size_type pos(0);
            if ((pos = str.find(oldValue)) != string::npos)
                str.replace(pos, oldValue.length(), newValue);
            else {
                break;
            }
        }
        return str;
    }

private:
    AlgoLoader & algo_;
    uint64_t timestamp_;
};

}
}