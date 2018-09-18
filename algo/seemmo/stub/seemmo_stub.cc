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

#include "algo/seemmo/service/rpc/service.pb.h"
#include "algo/seemmo/service/rpc/service.grpc.pb.h"

using grpc::ChannelArguments;
using grpc::ChannelInterface;
using grpc::ClientContext;
using grpc::Status;
using gosun::video::VideoTrailRecReply;
using gosun::video::VideoTrailRecRequest;
using gosun::video::VideoProc;

using namespace std;


class SeemmoVideoProcClient {
public:
    SeemmoVideoProcClient(std::shared_ptr<ChannelInterface> channel) : stub_(VideoProc::NewStub(channel)) {}

    int32_t TrailAndRec(int32_t videoChl, uint64_t timeStamp, const uint8_t *rgbImg, uint32_t height, uint32_t width) {
        VideoTrailRecRequest request;

        gpr_timespec timespec;
        timespec.tv_sec = 2;//设置阻塞时间为2秒
        timespec.tv_nsec = 0;
        timespec.clock_type = GPR_TIMESPAN;

        //request.set_name(user);
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


static SeemmoVideoProcClient *client = nullptr;

int32_t SeemmoStubOpen(const string &address) {
    assert(!address.empty());
    grpc_init();
    client = new SeemmoVideoProcClient(grpc::CreateChannel(address, grpc::InsecureChannelCredentials()));
}

int32_t SeemmoStubClose() {
    if (nullptr != client) {
        client->Shutdown();
        delete client;
        client = nullptr;
        grpc_shutdown();
    }
}

int32_t SeemmoStubVideoTrailAndRec(int32_t videoChl, uint64_t timeStamp, const uint8_t *rgbImg, uint32_t height, uint32_t width) {
    if (nullptr == client) {
        return -1;
    }
    return client->TrailAndRec(videoChl, timeStamp, rgbImg, height, width);
}
