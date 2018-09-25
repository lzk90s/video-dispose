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

#include "common/helper/logger.h"

#include "algo/vendor/seemmo/stub_impl/seemmo_stub.h"
#include "algo/vendor/seemmo/rpc/service.pb.h"
#include "algo/vendor/seemmo/rpc/service.grpc.pb.h"
#include "algo/vendor/seemmo/stub_impl/filter_param_builder.h"
#include "algo/vendor/seemmo/stub_impl/filter_result_parser.h"
#include "algo/vendor/seemmo/stub_impl/rec_param_builder.h"
#include "algo/vendor/seemmo/stub_impl/rec_result_parser.h"
#include "algo/vendor/seemmo/stub_impl/detect_result_parser.h"

using grpc::ChannelArguments;
using grpc::ChannelInterface;
using grpc::ClientContext;
using grpc::Status;

using namespace std;

namespace algo {
namespace seemmo {

class VideoProcClient {
public:
    VideoProcClient(std::shared_ptr<ChannelInterface> channel) : stub_(VideoProc::NewStub(channel)) {}

    void Shutdown() {
        stub_.reset();
    }

    int32_t Trail(
        int32_t videoChl,
        uint64_t timestamp,
        const uint8_t *bgr24,
        uint32_t width,
        uint32_t height,
        const TrailParam &param,
        DetectResult &detect,
        FilterResult &filter
    ) {

        // set timeout
        gpr_timespec timespec;
        timespec.tv_sec = 2;
        timespec.tv_nsec = 0;
        timespec.clock_type = GPR_TIMESPAN;

        TrailRequest request;
        request.set_videochl(videoChl);
        request.set_timestamp(timestamp);
        request.set_bgr24((char*)bgr24, width*height*3);		//bgr24图片字节大小为h*w*3
        request.set_width(width);
        request.set_height(height);
        request.set_param(trail::FilterParamBuilder().Build().c_str());

        // RPC
        TrailReply reply;
        ClientContext context;
        context.set_deadline(timespec);
        Status status = stub_->Trail(&context, request, &reply);
        if (!status.ok()) {
            LOG_ERROR("call trail method error, status {}, error {}", status.error_code(), status.error_message());
            return -1;
        }

        // parse response message
        detect::DetectReplyPO detectRspPO;
        detect::DetectResponseParser().Parse(reply.data(), detectRspPO);
        trail::TrailReplyPO trailRspPO;
        trail::FilterResponseParser().Parse(reply.data(), trailRspPO);
        // convert
        fillDetectResult(detect, detectRspPO);
        fillFilterResult(filter, trailRspPO);

        return 0;
    }

    int32_t Recognize(
        uint8_t *bgr24,
        uint32_t width,
        uint32_t height,
        const RecogParam &param,
        RecogResult &rec
    ) {
        // set timeout
        gpr_timespec timespec;
        timespec.tv_sec = 2;
        timespec.tv_nsec = 0;
        timespec.clock_type = GPR_TIMESPAN;

        RecognizeRequest request;
        request.set_bgr24((char*)bgr24, width*height * 3);		//bgr24图片字节大小为h*w*3
        request.set_width(width);
        request.set_height(height);

        vector<rec::LocationPO> locations;
        fillLocations(param, locations);
        request.set_param(rec::RecParamBuilder().Build(locations).c_str());

        // RPC
        RecognizeReply reply;
        ClientContext context;
        context.set_deadline(timespec);
        Status status = stub_->Recognize(&context, request, &reply);
        if (!status.ok()) {
            LOG_ERROR("call trail method error, status {}, error {}", status.error_code(), status.error_message());
            return -1;
        }

        // parse response message
        rec::RecogReplyPO recRspPO;
        rec::RecResultParser().Parse(reply.data(), recRspPO);

        //convert
        // 深瞐返回的识别结果中没有guid，这里根据入参手动填进去
        fillRecogResult(rec, recRspPO, param.obj.guid);

        return 0;
    }

private:
    void fillDetectResult(DetectResult &r, detect::DetectReplyPO &p) {
        if (p.Code != 0) {
            LOG_ERROR("detect reply error, code {}, msg {}", p.Code, p.Message);
            return;
        }
        if (p.ImageResults.empty()) {
            return;
        }

        //目前没有用批量方式，所以，只取第一个imageresult即可
        detect::ImageResultPO root = p.ImageResults.at(0);

        for (auto a : root.Bikes) {
            algo::BikeObject tmp;
            tmp.type = (algo::ObjectType)a.Type;
            tmp.guid = a.GUID;
            tmp.trail = a.Trail;
            if (a.Detect.Body.Rect.size() == 4) {
                tmp.detect.Set(a.Detect.Body.Rect[0], a.Detect.Body.Rect[1], a.Detect.Body.Rect[2], a.Detect.Body.Rect[3]);
            }
            r.bikes.push_back(tmp);
        }
        for (auto a : root.Pedestrains) {
            algo::PersonObject tmp;
            tmp.type = (algo::ObjectType)a.Type;
            tmp.guid = a.GUID;
            tmp.trail = a.Trail;
            if (a.Detect.Body.Rect.size() == 4) {
                tmp.detect.Set(a.Detect.Body.Rect[0], a.Detect.Body.Rect[1], a.Detect.Body.Rect[2], a.Detect.Body.Rect[3]);
            }
            r.pedestrains.push_back(tmp);
        }
        for (auto a : root.Vehicles) {
            algo::VehicleObject tmp;
            tmp.type = (algo::ObjectType)a.Type;
            tmp.guid = a.GUID;
            tmp.trail = a.Trail;
            if (a.Detect.Body.Rect.size() == 4) {
                tmp.detect.Set(a.Detect.Body.Rect[0], a.Detect.Body.Rect[1], a.Detect.Body.Rect[2], a.Detect.Body.Rect[3]);
            }
            r.vehicles.push_back(tmp);
        }
    }

    void fillFilterResult(FilterResult &r, trail::TrailReplyPO &p) {
        if (p.Code != 0) {
            LOG_ERROR("filter reply error, code {}, msg {}", p.Code, p.Message);
            return;
        }
        if (p.FilterResults.empty()) {
            return;
        }

        //目前没有用批量方式，所以，只取第一个imageresult即可
        trail::FilterResultPO root = p.FilterResults.at(0);

        for (auto a : root.Bikes) {
            algo::BikeFilter tmp;
            tmp.type = (algo::ObjectType)a.Type;
            tmp.guid = a.GUID;
            tmp.trail = a.Trail;
            tmp.contextCode = a.ContextCode;
            tmp.frameId = a.Timestamp;
            if (a.Detect.Body.Rect.size() == 4) {
                tmp.detect.Set(a.Detect.Body.Rect[0], a.Detect.Body.Rect[1], a.Detect.Body.Rect[2], a.Detect.Body.Rect[3]);
            }
            r.bikes.push_back(tmp);
        }
        for (auto a : root.Pedestrains) {
            algo::PersonFilter tmp;
            tmp.type = (algo::ObjectType)a.Type;
            tmp.guid = a.GUID;
            tmp.trail = a.Trail;
            tmp.contextCode = a.ContextCode;
            tmp.frameId = a.Timestamp;
            if (a.Detect.Body.Rect.size() == 4) {
                tmp.detect.Set(a.Detect.Body.Rect[0], a.Detect.Body.Rect[1], a.Detect.Body.Rect[2], a.Detect.Body.Rect[3]);
            }
            r.pedestrains.push_back(tmp);
        }
        for (auto a : root.Vehicles) {
            algo::VehicleFilter tmp;
            tmp.type = (algo::ObjectType)a.Type;
            tmp.guid = a.GUID;
            tmp.trail = a.Trail;
            tmp.contextCode = a.ContextCode;
            tmp.frameId = a.Timestamp;
            if (a.Detect.Body.Rect.size() == 4) {
                tmp.detect.Set(a.Detect.Body.Rect[0], a.Detect.Body.Rect[1], a.Detect.Body.Rect[2], a.Detect.Body.Rect[3]);
            }
            r.vehicles.push_back(tmp);
        }
    }

    void fillRecogResult(RecogResult &r, rec::RecogReplyPO &p, const string objId) {
        if (p.Code != 0) {
            LOG_ERROR("filter reply error, code {}, msg {}", p.Code, p.Message);
            return;
        }
        if (p.ImageResults.empty()) {
            return;
        }

        //目前没有用批量方式，所以，只取第一个imageresult即可
        rec::ImageResultPO root = p.ImageResults.at(0);

        for (auto a : root.Bikes) {
            algo::BikeObject tmp;
            tmp.type = (algo::ObjectType)a.Type;
            tmp.guid = objId;
            if (a.Detect.Body.Rect.size() == 4) {
                tmp.detect.Set(a.Detect.Body.Rect[0], a.Detect.Body.Rect[1], a.Detect.Body.Rect[2], a.Detect.Body.Rect[3]);
            }

            if (!a.Persons.empty()) {
                algo::Attribute attr;
                for (auto b : a.Persons) {

                }
            }

            r.bikes.push_back(tmp);
        }
        for (auto a : root.Pedestrains) {
            algo::PersonObject tmp;
            tmp.type = (algo::ObjectType)a.Type;
            tmp.guid = objId;
            if (a.Detect.Body.Rect.size() == 4) {
                tmp.detect.Set(a.Detect.Body.Rect[0], a.Detect.Body.Rect[1], a.Detect.Body.Rect[2], a.Detect.Body.Rect[3]);
            }

            tmp.attrs.age.WithName(a.Recognize.Age.Name);
            tmp.attrs.sex.WithName(a.Recognize.Sex.Name);
            tmp.attrs.upperColor.WithName(a.Recognize.UpperColor.Name);
            tmp.attrs.bottomColor.WithName(a.Recognize.BottomColor.Name);
            tmp.attrs.orientation.WithName(a.Recognize.Orientation.Name);
            tmp.attrs.hair.WithName(a.Recognize.Hair.Name);
            tmp.attrs.umbrella.WithName(a.Recognize.Umbrella.Name);
            tmp.attrs.hat.WithName(a.Recognize.Hat.Name);
            tmp.attrs.upperType.WithName(a.Recognize.UpperType.Name);
            tmp.attrs.bottomType.WithName(a.Recognize.BottomType.Name);
            tmp.attrs.knapsack.WithName(a.Recognize.Knapsack.Name);
            tmp.attrs.bag.WithName(a.Recognize.Bag.Name);
            tmp.attrs.baby.WithName(a.Recognize.Baby.Name);
            tmp.attrs.messengerBag.WithName(a.Recognize.MessengerBag.Name);
            tmp.attrs.shoulderBag.WithName(a.Recognize.ShoulderBag.Name);
            tmp.attrs.glasses.WithName(a.Recognize.Glasses.Name);
            tmp.attrs.mask.WithName(a.Recognize.Mask.Name);
            tmp.attrs.upperTexture.WithName(a.Recognize.UpperTexture.Name);
            tmp.attrs.barrow.WithName(a.Recognize.Barrow.Name);
            tmp.attrs.trolleyCase.WithName(a.Recognize.TrolleyCase.Name);

            r.pedestrains.push_back(tmp);
        }
        for (auto a : root.Vehicles) {
            algo::VehicleObject tmp;
            tmp.type = (algo::ObjectType)a.Type;
            tmp.guid = objId;
            if (a.Detect.Body.Rect.size() == 4) {
                tmp.detect.Set(a.Detect.Body.Rect[0], a.Detect.Body.Rect[1], a.Detect.Body.Rect[2], a.Detect.Body.Rect[3]);
            }
            tmp.attrs.brand.WithName(a.Recognize.Brand.Name);
            tmp.attrs.color.WithName(a.Recognize.Color.Name);
            tmp.attrs.type.WithName(a.Recognize.Type.Name);
            tmp.attrs.plate.WithName(a.Recognize.Plate.Name);
            r.vehicles.push_back(tmp);
        }
    }

    void fillLocations(const RecogParam &r, vector<rec::LocationPO> &loc) {
        rec::LocationPO l;
        l.GUID = r.obj.guid;
        l.ContextCode = r.ContextCode;
        l.Rect.push_back(r.obj.detect.x);
        l.Rect.push_back(r.obj.detect.y);
        l.Rect.push_back(r.obj.detect.w);
        l.Rect.push_back(r.obj.detect.h);
        l.Trail = r.obj.trail;
        l.Type = r.obj.type;
        loc.push_back(l);
    }

private:
    std::unique_ptr<VideoProc::Stub> stub_;
};


class SeemmoAlgoStub : public algo::AlgoStub {
public:
    SeemmoAlgoStub(): algo::AlgoStub("seemmo") {
        init();
    }

    ~SeemmoAlgoStub() {
        client_->Shutdown();
        grpc_shutdown();
    }

    int32_t Trail(
        uint32_t channelId,
        uint64_t frameId,
        uint8_t *bgr24,
        uint32_t width,
        uint32_t height,
        const TrailParam &param,
        DetectResult &detect,
        FilterResult &filter)  override {
        return  client_->Trail(channelId, frameId, bgr24, width, height, param, detect, filter);
    }

    int32_t Recognize(
        uint8_t *bgr24,
        uint32_t width,
        uint32_t height,
        const RecogParam &param,
        RecogResult &rec) override  {
        return client_->Recognize(bgr24, width, height, param, rec);
    }

private:
    void init() {
        string address = "unix:/tmp/a.sock";
        assert(!address.empty());
        grpc_init();
        client_.reset(new VideoProcClient(grpc::CreateChannel(address, grpc::InsecureChannelCredentials())));
    }

private:
    unique_ptr<VideoProcClient> client_;
};

algo::AlgoStub* NewAlgoStub() {
    return new SeemmoAlgoStub();
}

void FreeAlgoStub(algo::AlgoStub *&stub) {
    if (stub->GetVendor() == "seemmo") {
        delete stub;
        stub = nullptr;
    }
}
}
}
