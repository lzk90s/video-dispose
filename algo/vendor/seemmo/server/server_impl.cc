#include <memory>
#include <iostream>
#include <string>
#include <thread>
#include <sstream>
#include <cstdint>
#include <cassert>
#include <vector>

#include <grpc++/grpc++.h>
#include <grpc/support/log.h>

#include "common/helper/singleton.h"

#include "algo/vendor/seemmo/rpc/service.grpc.pb.h"
#include "algo/vendor/seemmo/server/server.h"
#include "algo/vendor/seemmo/server/algo_loader.h"

using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerCompletionQueue;
using grpc::Status;


using namespace std;

namespace algo {
namespace seemmo {

AlgoLoader algoLoader;

class ServerImpl final : public RpcServer {
public:
    ~ServerImpl() {
        server_->Shutdown();
        cq_->Shutdown();
    }

    void Run(const string &serverAddress, const string &algoCfgPath) override {
        assert(!serverAddress.empty());
        assert(!algoCfgPath.empty());

        algoLoader.Load(algoCfgPath);

        startRpcServer(serverAddress);
    }

private:
    void startRpcServer(const string &serverAddress) {
        ServerBuilder builder;
        //设置最大消息大小为20M
        builder.SetMaxReceiveMessageSize(20 * 1024 * 1024);
        builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());
        builder.RegisterService(&service_);
        cq_ = builder.AddCompletionQueue();
        server_ = builder.BuildAndStart();

        cout << "Server listening on " << serverAddress << endl;

        new CallData(&service_, cq_.get(), ServerImpl::CallData::SS_Trail);
        new CallData(&service_, cq_.get(), ServerImpl::CallData::SS_Recognize);

        const int THREAD_NUM = 8;
        // using multi thread
        for (int i = 0; i < THREAD_NUM; i++) {
            std::shared_ptr<std::thread> t(new std::thread(ServerImpl::ThreadHandlerRPC, (void*)this));
            threads_.push_back(t);
        }

        // join
        for (auto itr = threads_.begin(); itr != threads_.end(); ++itr) {
            (*itr)->join();
        }
    }

private:

    class CallData {
    public:
        enum ServiceType {
            SS_Trail = 0,
            SS_Recognize = 1,
        };

    public:
        CallData(VideoProc::AsyncService* service, ServerCompletionQueue* cq, ServiceType s_type)
            :service_(service), cq_(cq), s_type_(s_type), trailResponder_(&ctx_), recResponder_(&ctx_), status_(CREATE) {
            Process();
        }

        void Process() {
            if (status_ == CREATE) {
                status_ = PROCESS;
                switch (s_type_) {
                case SS_Trail:
                    service_->RequestTrail(&ctx_, &trailRequest_, &trailResponder_, cq_, cq_, this);
                    break;
                case SS_Recognize:
                    service_->RequestRecognize(&ctx_, &recRequest_, &recResponder_, cq_, cq_, this);
                    break;
                default:
                    break;
                }
            } else if (status_ == PROCESS) {
                status_ = FINISH;
                new CallData(service_, cq_, this->s_type_);

                switch (s_type_) {
                case SS_Trail: {
                    uint32_t bufLen = 1024 * 1024 * 5;
                    unique_ptr<char[]> buf(new char[bufLen]);
                    int ret = algoLoader.Trail(
                                  trailRequest_.videochl(),
                                  trailRequest_.timestamp(),
                                  (const uint8_t*)trailRequest_.bgr24().data(),
                                  trailRequest_.height(),
                                  trailRequest_.width(),
                                  trailRequest_.param(),
                                  buf.get(),
                                  bufLen);
                    if (0 != ret) {
                        cout << "tail failed, ret = " << ret << endl;
                        trailResponder_.Finish(trailReply_, Status::CANCELLED, this);
                    } else {
                        trailReply_.set_data(buf.get(), bufLen);
                        trailResponder_.Finish(trailReply_, Status::OK, this);
                    }
                }
                break;
                case SS_Recognize: {
                    uint32_t bufLen = 1024 * 1024 * 5;
                    unique_ptr<char[]> buf(new char[bufLen]);
                    int ret = algoLoader.Recognize(
                                  (const uint8_t*)recRequest_.bgr24().data(),
                                  recRequest_.width(),
                                  recRequest_.height(),
                                  recRequest_.param(),
                                  buf.get(),
                                  bufLen);
                    if (0 != ret) {
                        cout << "tail failed, ret = " << ret << endl;
                        recResponder_.Finish(recReply_, Status::CANCELLED, this);
                    } else {
                        cout << "----OK " << buf.get() << endl;
                        recReply_.set_data(buf.get(), bufLen);
                        recResponder_.Finish(recReply_, Status::OK, this);
                    }
                }
                break;
                default:
                    break;
                }
            } else {
                GPR_ASSERT(status_ == FINISH);
                delete this;
            }
        }

    private:
        VideoProc::AsyncService* service_;
        ServerCompletionQueue* cq_;
        ServerContext ctx_;
        ServiceType s_type_;

        TrailRequest trailRequest_;
        TrailReply trailReply_;
        ::grpc::ServerAsyncResponseWriter<TrailReply> trailResponder_;

        RecognizeRequest recRequest_;
        RecognizeReply recReply_;
        ::grpc::ServerAsyncResponseWriter<RecognizeReply> recResponder_;

        enum CallStatus { CREATE, PROCESS, FINISH };
        CallStatus status_;
    };

    static unsigned ThreadHandlerRPC(void* lparam) {
        ServerImpl* impl = (ServerImpl*)lparam;
        impl->HandleRPCS();
        return 1;
    }

    void HandleRPCS() {
        void* tag;
        bool ok;
        while (true) {
            GPR_ASSERT(cq_->Next(&tag, &ok));
            //GPR_ASSERT(ok);
            if (ok) {
                static_cast<CallData*>(tag)->Process();
            }
        }
    }

private:
    std::shared_ptr<ServerCompletionQueue> cq_;
    VideoProc::AsyncService service_;
    std::shared_ptr<Server> server_;
    std::vector<std::shared_ptr<std::thread>> threads_;
};


RpcServer *NewRpcServer() {
    return &Singleton<ServerImpl>::getInstance();
}
}
}

