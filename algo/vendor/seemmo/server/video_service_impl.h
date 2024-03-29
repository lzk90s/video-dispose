#pragma once

#include <memory>
#include <string>

#include <gflags/gflags.h>
#include <butil/logging.h>
#include <brpc/server.h>

#include "common/helper/logger.h"
#include "algo/vendor/seemmo/rpc/service.pb.h"
#include "algo/vendor/seemmo/server/algo_loader.h"
#include "algo/vendor/seemmo/shm/shmdata.h"

namespace video {
namespace algo {
namespace seemmo {

class VideoProcServiceImpl : public VideoProcService {
public:
    VideoProcServiceImpl(AlgoLoader &algo) : algo_(algo) {
    };

    virtual ~VideoProcServiceImpl() {};

    virtual void Trail(::google::protobuf::RpcController* controller, const TrailRequest* request, TrailReply* response,
                       ::google::protobuf::Closure* done) {

        brpc::ClosureGuard done_guard(done);

        brpc::Controller* cntl =
            static_cast<brpc::Controller*>(controller);

        uint8_t *bgr24 = nullptr;
        if (request->bgr24().empty()) {
            bgr24 = SIMMNG::getInstance().CreateAndGet(request->videochl(), request->width(), request->height(),
                    true)->GetBuffer().bgr24Buff1;
        } else {
            bgr24 = (uint8_t*)request->bgr24().data();
        }

        uint32_t bufLen = 1024 * 1024 * 5;
        std::unique_ptr<char[]> buf(new char[bufLen]);
        int ret = algo_.Trail(
                      request->videochl(),
                      request->timestamp(),
                      bgr24,
                      request->width(),
                      request->height(),
                      request->param(),
                      buf.get(),
                      bufLen);
        if (0 != ret) {
            LOG_ERROR("Trail error, {}", ret);
            cntl->SetFailed("Trail error, code " + std::to_string(ret));
            return;
        }
        response->set_data(buf.get(), bufLen);
    }

    virtual void TrailEnd(::google::protobuf::RpcController* controller, const TrailEndRequest* request,
                          TrailEndReply* response, ::google::protobuf::Closure* done) {
        brpc::ClosureGuard done_guard(done);

        SIMMNG::getInstance().Delete(request->videochl());

        LOG_INFO("Trail end for channel {}", request->videochl());
        response->set_data("ok");
    }

    virtual void Recognize(::google::protobuf::RpcController* controller,const RecognizeRequest* request,
                           RecognizeReply* response,::google::protobuf::Closure* done) {
        brpc::ClosureGuard done_guard(done);

        brpc::Controller* cntl =
            static_cast<brpc::Controller*>(controller);

        uint8_t *bgr24 = nullptr;
        if (request->bgr24().empty()) {
            bgr24 = SIMMNG::getInstance().CreateAndGet(request->videochl(), request->width(), request->height(),
                    true)->GetBuffer().bgr24Buff2;
        } else {
            bgr24 = (uint8_t*)request->bgr24().data();
        }

        uint32_t bufLen = 1024 * 1024 * 5;
        std::unique_ptr<char[]> buf(new char[bufLen]);
        int ret = algo_.Recognize(
                      bgr24,
                      request->width(),
                      request->height(),
                      request->param(),
                      buf.get(),
                      bufLen);
        if (0 != ret) {
            LOG_ERROR("Recognize error, {}", ret);
            cntl->SetFailed("Trail error, code " + std::to_string(ret));
            return;
        }
        response->set_data(buf.get(), bufLen);
    }

private:
    AlgoLoader & algo_;
};
}
}
}