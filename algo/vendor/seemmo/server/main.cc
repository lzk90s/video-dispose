#include <memory>
#include <string>

#include <gflags/gflags.h>
#include <butil/logging.h>
#include <brpc/server.h>

#include "common/helper/logger.h"
#include "algo/vendor/seemmo/rpc/service.pb.h"
#include "algo/vendor/seemmo/server/algo_loader.h"

//----------------server params----------------
DEFINE_bool(echo_attachment, true, "Echo attachment as well");
DEFINE_int32(port, 7000, "TCP Port of this server");
DEFINE_int32(idle_timeout_s, -1, "Connection will be closed if there is no "
             "read/write operations during the last `idle_timeout_s'");

//----------------algo params----------------
DEFINE_string(base_dir, "../../", "the base dir");
DEFINE_string(auth_server, "192.168.1.198:12821", "the address of auth server");
DEFINE_int32(gpu_dev, 0, "the gpu device");
DEFINE_int32(auth_type, 1, "the auth_type");


using namespace std;

namespace algo {
namespace seemmo {

class VideoProcServiceImpl : public VideoProcService {
public:
    VideoProcServiceImpl(algo::seemmo::AlgoLoader &algo) : algo_(algo) {
    };

    virtual ~VideoProcServiceImpl() {};

    virtual void Trail(::google::protobuf::RpcController* controller, const ::algo::seemmo::TrailRequest* request,
                       ::algo::seemmo::TrailReply* response, ::google::protobuf::Closure* done) {

        brpc::ClosureGuard done_guard(done);

        brpc::Controller* cntl =
            static_cast<brpc::Controller*>(controller);

        uint32_t bufLen = 1024 * 1024 * 5;
        unique_ptr<char[]> buf(new char[bufLen]);
        int ret = algo_.Trail(
                      request->videochl(),
                      request->timestamp(),
                      (const uint8_t*)request->bgr24().data(),
                      request->height(),
                      request->width(),
                      request->param(),
                      buf.get(),
                      bufLen);
        if (0 != ret) {
            LOG_ERROR("Trail error, {}", ret);
            return;
        }
        response->set_data(buf.get(), bufLen);
    }

    virtual void Recognize(::google::protobuf::RpcController* controller, const ::algo::seemmo::RecognizeRequest* request,
                           ::algo::seemmo::RecognizeReply* response, ::google::protobuf::Closure* done) {
        brpc::ClosureGuard done_guard(done);

        brpc::Controller* cntl =
            static_cast<brpc::Controller*>(controller);

        uint32_t bufLen = 1024 * 1024 * 5;
        unique_ptr<char[]> buf(new char[bufLen]);
        int ret = algo_.Recognize(
                      (const uint8_t*)request->bgr24().data(),
                      request->width(),
                      request->height(),
                      request->param(),
                      buf.get(),
                      bufLen);
        if (0 != ret) {
            LOG_ERROR("Recognize error, {}", ret);
            return;
        }
        response->set_data(buf.get(), bufLen);
    }

private:
    AlgoLoader &algo_;
};
}
}


int main(int argc, char* argv[]) {
    GFLAGS_NAMESPACE::ParseCommandLineFlags(&argc, &argv, true);

    // load algorithm
    algo::seemmo::AlgoLoader algo;
    algo.Load(FLAGS_base_dir, 4, 4, FLAGS_auth_type, FLAGS_auth_server, FLAGS_gpu_dev);

    // create rpc server
    brpc::Server server;

    // Instance of your service.
    algo::seemmo::VideoProcServiceImpl videoProcServiceImpl(algo);

    // Add the service into server. Notice the second parameter, because the
    // service is put on stack, we don't want server to delete it, otherwise
    // use brpc::SERVER_OWNS_SERVICE.
    if (server.AddService(&videoProcServiceImpl,
                          brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
        LOG(ERROR) << "Fail to add service";
        return -1;
    }

    // Start the server.
    brpc::ServerOptions options;
    options.idle_timeout_sec = FLAGS_idle_timeout_s;
    if (server.Start(FLAGS_port, &options) != 0) {
        LOG(ERROR) << "Fail to start EchoServer";
        return -1;
    }

    // Wait until Ctrl-C is pressed, then Stop() and Join() the server.
    server.RunUntilAskedToQuit();
    LOG(INFO) << "Quit";

    return 0;
}