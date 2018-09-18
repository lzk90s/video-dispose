#include <memory>
#include <iostream>
#include <string>
#include <thread>
#include <sstream>
#include <cstdint>
#include <cassert>


#include <grpc++/grpc++.h>
#include <grpc/support/log.h>
#include "algo/seemmo/service/rpc/service.grpc.pb.h"

#include "common/helper/singleton.h"
#include "algo/seemmo/service/rpcserver/server.h"
#include "algo/seemmo/service/rpcserver/algo_loader.h"

using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerCompletionQueue;
using grpc::Status;

using gosun::video::VideoTrailRecRequest;
using gosun::video::VideoTrailRecReply;
using gosun::video::VideoProc;

using namespace std;

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
        builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());
        builder.RegisterService(&service_);
        cq_ = builder.AddCompletionQueue();
        server_ = builder.BuildAndStart();

        cout << "Server listening on " << serverAddress << endl;

        new CallData(&service_, cq_.get(), ServerImpl::CallData::SS_TrailAndRec);

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
            SS_TrailAndRec = 0,
        };

    public:
        CallData(VideoProc::AsyncService* service, ServerCompletionQueue* cq, ServiceType s_type)
            :service_(service), cq_(cq), s_type_(s_type), responder_(&ctx_), status_(CREATE) {
            Process();
        }

        void Process() {
            if (status_ == CREATE) {
                status_ = PROCESS;
                switch (s_type_) {
                case ServerImpl::CallData::SS_TrailAndRec:
                    service_->RequestTrailAndRec(&ctx_, &request_, &responder_, cq_, cq_, this);
                    break;
                default:
                    break;
                }
            } else if (status_ == PROCESS) {
                status_ = FINISH;
                new CallData(service_, cq_, this->s_type_);

                switch (s_type_) {
                case ServerImpl::CallData::SS_TrailAndRec: {
                    //Algo_VideoTrailAndRec(request_.videochl(), request_.timestamp(), (uint8_t*)request_.rgbimg(), request_.height(), request_.width());
                    status_ = FINISH;
                    responder_.Finish(reply_, Status::OK, this);
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
        VideoTrailRecRequest request_;
        VideoTrailRecReply reply_;
        ::grpc::ServerAsyncResponseWriter<VideoTrailRecReply> responder_;

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
