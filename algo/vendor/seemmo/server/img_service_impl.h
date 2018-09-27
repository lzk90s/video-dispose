#pragma once

#include <memory>
#include <string>

#include <gflags/gflags.h>
#include <butil/logging.h>
#include <brpc/server.h>

#include "common/helper/logger.h"
#include "algo/vendor/seemmo/rpc/service.pb.h"
#include "algo/vendor/seemmo/server/algo_loader.h"
#include "algo/vendor/seemmo/server/jpeg_helper.h"

namespace algo {
namespace seemmo {

class ImgProcServiceImpl : public ImgProcService {
public:
    ImgProcServiceImpl(algo::seemmo::AlgoLoader &algo) : algo_(algo) {
    };

    virtual ~ImgProcServiceImpl() {};

    virtual void Recognize(::google::protobuf::RpcController* controller,
                           const ::algo::seemmo::ImgRecognizeRequest* request, ::algo::seemmo::ImgRecognizeReply* response,
                           ::google::protobuf::Closure* done) {
        brpc::ClosureGuard done_guard(done);

        brpc::Controller* cntl =
            static_cast<brpc::Controller*>(controller);

        int32_t len = 10 * 1024 * 1024;
        int32_t width = 0, height = 0;
        unique_ptr<uint8_t[]> bgr24(new uint8_t[len] {0});
        JpegHelper::jpeg2rgb((uint8_t*)request->jpeg().c_str(), request->jpeg().length(), bgr24.get(), &len, &width, &height);

        //识别参数写死
        string param =
            "{\"Recognize\":{\"Feature\":{\"IsRec\":true},\"Person\":{\"IsRec\":true},\"Vehicle\":{\"Brand\":{\"IsRec\":true},\"Color\":{\"IsRec\":true},\"Plate\":{\"IsRec\":true}},\"ObjLocations\":[]}}";

        uint32_t bufLen = 1024 * 1024 * 5;
        unique_ptr<char[]> buf(new char[bufLen]);
        int ret = algo_.DetectRecognize(
                      /*((const uint8_t*)request->bgr24().data()*/ bgr24.get(),
                      width,
                      height,
                      param.c_str(),
                      buf.get(),
                      bufLen);
        if (0 != ret) {
            LOG_ERROR("Recognize image error, {}", ret);
            cntl->SetFailed("Recognize image error, code " + std::to_string(ret));
            return;
        }
        response->set_data(buf.get(), bufLen);
    }

private:
    AlgoLoader & algo_;
};

}
}