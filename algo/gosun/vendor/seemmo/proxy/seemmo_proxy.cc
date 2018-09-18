#include <iostream>
#include <memory>
#include <string>
#include <cassert>

#include <grpc/grpc.h>
#include <grpc++/support/channel_arguments.h>
#include <grpc++/impl/codegen/channel_interface.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>
#include <grpc++/support/status.h>

#include "seemmo/service/rpc/service.pb.h"
#include "seemmo/service/rpc/service.grpc.pb.h"

using grpc::ChannelArguments;
using grpc::ChannelInterface;
using grpc::ClientContext;
using grpc::Status;
using gosun::video::VideoTrailRecReply;
using gosun::video::VideoTrailRecRequest;
using gosun::video::VideoProc;

using namespace std;

namespace algo {
namespace seemmo {

class VideoProcClient {
public:
    VideoProcClient(std::shared_ptr<ChannelInterface> channel) : stub_(VideoProc::NewStub(channel)) {}

    int32_t TrailAndRec(int32_t videoChl, uint64_t timeStamp, const uint8_t *bgr24, uint32_t height, uint32_t width) {
        gpr_timespec timespec;
        timespec.tv_sec = 2;	//设置阻塞时间为2秒
        timespec.tv_nsec = 0;
        timespec.clock_type = GPR_TIMESPAN;

        VideoTrailRecRequest request;

        VideoTrailRecReply reply;
        ClientContext context;
        context.set_deadline(timespec);
        Status status = stub_->TrailAndRec(&context, request, &reply);
        if (status.ok()) {
            //return reply.jsondata();
        } else {
            //return "Rpc failed, error " + std::to_string(status.error_code());
        }
    }
    void Shutdown() {
        stub_.reset();
    }
private:
    std::unique_ptr<VideoProc::Stub> stub_;
};


class AlgoStub {
public:
    AlgoStub() {
    }

    ~AlgoStub() {
        m_client.reset();
    }

    int32_t Open(string &address) {
        assert(!address.empty());
        grpc_init();
        m_client.reset(new VideoProcClient(grpc::CreateChannel(address, grpc::InsecureChannelCredentials())));
    }

    int32_t Close() {
        m_client->Shutdown();
        grpc_shutdown();
    }

    int32_t VideoTrailAndRec(int32_t videoChl, uint64_t timeStamp, const uint8_t *bgr24, uint32_t height, uint32_t width) {
        return m_client->TrailAndRec(videoChl, timeStamp, bgr24, height, width);
    }

private:
    shared_ptr<VideoProcClient> m_client;
};
}
}
