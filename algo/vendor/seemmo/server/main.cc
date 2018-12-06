#include <memory>
#include <string>

#include <gflags/gflags.h>
//#include <butil/logging.h>
#include <brpc/server.h>

#include "algo/vendor/seemmo/server/algo_loader.h"
#include "algo/vendor/seemmo/server/video_service_impl.h"
#include "algo/vendor/seemmo/server/img_service_impl.h"

//----------------server params----------------
DEFINE_bool(echo_attachment, true, "Echo attachment as well");
DEFINE_int32(port, 7000, "TCP Port of this server");
DEFINE_int32(idle_timeout_s, -1, "Connection will be closed if there is no "
             "read/write operations during the last `idle_timeout_s'");

//----------------algo params----------------
DEFINE_string(base_dir, "/root/seemmo_sdk", "the base dir");
DEFINE_string(auth_server, "192.168.1.198:12821", "the address of auth server");
DEFINE_int32(img_core_num, 20, "the number of image core");
DEFINE_int32(video_core_num, 20, "the number of video core");
DEFINE_int32(img_thr_num, 1, "the number of image thread");
DEFINE_int32(video_thr_num, 3, "the number of video thread");
DEFINE_int32(gpu_dev, 0, "the gpu device");
DEFINE_int32(auth_type, 1, "the auth_type");


using namespace std;

int main(int argc, char* argv[]) {
    GFLAGS_NAMESPACE::ParseCommandLineFlags(&argc, &argv, true);

    // load algorithm
    algo::seemmo::AlgoLoader algo;
    algo.Load(FLAGS_base_dir, FLAGS_img_thr_num, FLAGS_video_thr_num,FLAGS_img_core_num, FLAGS_video_core_num,
              FLAGS_auth_type, FLAGS_auth_server, FLAGS_gpu_dev);

    // create rpc server
    brpc::Server server;

    // Instance of your service.
    algo::seemmo::VideoProcServiceImpl videoProcServiceImpl(algo);
    algo::seemmo::ImgProcServiceImpl imgProcServiceImpl(algo);

    // Add the service into server. Notice the second parameter, because the
    // service is put on stack, we don't want server to delete it, otherwise
    // use brpc::SERVER_OWNS_SERVICE.
    if (server.AddService(&videoProcServiceImpl,
                          brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
        cout << "Fail to add service";
        return -1;
    }
    if (server.AddService(&imgProcServiceImpl,
                          brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
        cout << "Fail to add service";
        return -1;
    }

    // Start the server.
    brpc::ServerOptions options;
    options.idle_timeout_sec = FLAGS_idle_timeout_s;
    if (server.Start(FLAGS_port, &options) != 0) {
        cout << "Fail to start EchoServer";
        return -1;
    }

    // Wait until Ctrl-C is pressed, then Stop() and Join() the server.
    server.RunUntilAskedToQuit();
    cout << "Quit";

    return 0;
}
