#include <iostream>
#include <memory>
#include <string>
#include <cassert>

#include <gflags/gflags.h>
#include <butil/logging.h>
#include <butil/time.h>
#include <brpc/channel.h>

#include "common/helper/logger.h"
#include "algo/vendor/seemmo/stub_impl/seemmo_stub.h"
#include "algo/vendor/seemmo/rpc/service.pb.h"
#include "algo/vendor/seemmo/stub_impl/filter_param_builder.h"
#include "algo/vendor/seemmo/stub_impl/filter_result_parser.h"
#include "algo/vendor/seemmo/stub_impl/rec_param_builder.h"
#include "algo/vendor/seemmo/stub_impl/rec_result_parser.h"
#include "algo/vendor/seemmo/stub_impl/detect_result_parser.h"
#include "algo/vendor/seemmo/shm/shmdata.h"

// service param
DEFINE_string(attachment, "foo", "Carry this along with requests");
DEFINE_string(protocol, "baidu_std", "Protocol type. Defined in src/brpc/options.proto");
DEFINE_string(connection_type, "single", "Connection type. Available values: single, pooled, short");
DEFINE_string(server, "0.0.0.0:7000", "IP Address of server");
DEFINE_string(load_balancer, "", "The algorithm for load balancing");
DEFINE_int32(timeout_ms, 2000, "RPC timeout in milliseconds");
DEFINE_int32(max_retry, 0, "Max retries(not including the first RPC)");
DEFINE_string(http_content_type, "application/json", "Content type of http request");

using namespace std;

namespace algo {
namespace seemmo {

class VideoProcClient {
public:
    VideoProcClient() {
        init();
    }

    ~VideoProcClient() {
        stub_.reset();
        channel_.reset();
    }

    int32_t Trail(
        uint32_t channelId,
        uint64_t frameId,
        const uint8_t *bgr24,
        uint32_t width,
        uint32_t height,
        const TrailParam &param,
        ImageResult &imageResult,
        FilterResult &filterResult
    ) {
        TrailRequest request;
        TrailReply reply;
        brpc::Controller cntl;

        // 初始化共享内存
        if (!SIMMNG::getInstance().Exist(channelId)) {
            SIMMNG::getInstance().Create(channelId);
        }

        uint8_t *bgr24Dst = SIMMNG::getInstance().Get(channelId)->GetBuffer().bgr24Buff1;

        request.set_videochl(channelId);
        request.set_timestamp(frameId);
        //request.set_bgr24((char*)bgr24, width*height * 3);		//bgr24图片字节大小为h*w*3
        memcpy(bgr24Dst, bgr24, width*height * 3);	//拷贝到共享内存中
        bgr24Dst[width*height * 3] = '\0';
        request.set_width(width);
        request.set_height(height);

        trail::DetectRegionPO regionPO;
        fillDetectRegion(param, regionPO);
        request.set_param(trail::FilterParamBuilder().Build(regionPO).c_str());

        stub_->Trail(&cntl, &request, &reply, NULL);
        if (cntl.Failed()) {
            LOG_ERROR("Trail error, {}", cntl.ErrorText());
            return cntl.ErrorCode();
        }

        // parse response message
        algo::seemmo::detect::DetectReplyPO detectRspPO;
        algo::seemmo::detect::DetectResponseParser().Parse(reply.data(), detectRspPO);
        trail::TrailReplyPO trailRspPO;
        trail::FilterResponseParser().Parse(reply.data(), trailRspPO);

        // convert
        fillDetectResult(imageResult, detectRspPO);
        fillFilterResult(filterResult, trailRspPO);

        return 0;
    }

    int32_t Recognize(
        uint32_t channelId,
        const uint8_t *bgr24,
        uint32_t width,
        uint32_t height,
        const RecogParam &param,
        ImageResult &imageResult
    ) {
        RecognizeRequest request;
        RecognizeReply reply;
        brpc::Controller cntl;

        if (!SIMMNG::getInstance().Exist(channelId)) {
            SIMMNG::getInstance().Create(channelId);
        }

        uint8_t *bgr24Dst = SIMMNG::getInstance().Get(channelId)->GetBuffer().bgr24Buff2;

        request.set_videochl(channelId);
        //request.set_bgr24((char*)bgr24, width*height * 3);		//bgr24图片字节大小为h*w*3
        memcpy(bgr24Dst, bgr24, width*height * 3);	//图片拷贝到共享内存中
        bgr24Dst[width*height * 3] = '\0';
        request.set_width(width);
        request.set_height(height);

        vector<rec::LocationPO> locations;
        fillLocations(param, locations);
        request.set_param(rec::RecParamBuilder().Build(locations).c_str());

        stub_->Recognize(&cntl, &request, &reply, NULL);
        if (cntl.Failed()) {
            LOG_ERROR("Recognize error, {}", cntl.ErrorText());
            return cntl.ErrorCode();
        }

        // parse response message
        rec::RecogReplyPO recRspPO;
        rec::RecResultParser().Parse(reply.data(), recRspPO);

        //convert
        fillRecogResult(imageResult, recRspPO);

        return 0;
    }


private:
    void init() {
        channel_.reset(new brpc::Channel);

        brpc::ChannelOptions options;
        options.protocol = FLAGS_protocol;
        options.connection_type = FLAGS_connection_type;
        options.timeout_ms = FLAGS_timeout_ms/*milliseconds*/;
        options.max_retry = FLAGS_max_retry;
        if (channel_->Init(FLAGS_server.c_str(), FLAGS_load_balancer.c_str(), &options) != 0) {
            LOG_ERROR("Fail to initialize channel");
            throw runtime_error("brpc init error");
        }

        stub_.reset(new VideoProcService_Stub(channel_.get()));
        LOG_INFO("succeed to init seemmo algo stub");
    }

    void fillDetectResult(ImageResult &r, detect::DetectReplyPO &p) {
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
            tmp.detect = a.Detect.Body.Rect;
            r.bikes.push_back(tmp);
        }
        for (auto a : root.Pedestrains) {
            algo::PersonObject tmp;
            tmp.type = (algo::ObjectType)a.Type;
            tmp.guid = a.GUID;
            tmp.trail = a.Trail;
            tmp.detect = a.Detect.Body.Rect;
            r.pedestrains.push_back(tmp);
        }
        for (auto a : root.Vehicles) {
            algo::VehicleObject tmp;
            tmp.type = (algo::ObjectType)a.Type;
            tmp.guid = a.GUID;
            tmp.trail = a.Trail;
            tmp.detect = a.Detect.Body.Rect;
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
            tmp.detect = a.Detect.Body.Rect;
            r.bikes.push_back(tmp);
        }
        for (auto a : root.Pedestrains) {
            algo::PersonFilter tmp;
            tmp.type = (algo::ObjectType)a.Type;
            tmp.guid = a.GUID;
            tmp.trail = a.Trail;
            tmp.contextCode = a.ContextCode;
            tmp.frameId = a.Timestamp;
            tmp.detect = a.Detect.Body.Rect;
            r.pedestrains.push_back(tmp);
        }
        for (auto a : root.Vehicles) {
            algo::VehicleFilter tmp;
            tmp.type = (algo::ObjectType)a.Type;
            tmp.guid = a.GUID;
            tmp.trail = a.Trail;
            tmp.contextCode = a.ContextCode;
            tmp.frameId = a.Timestamp;
            tmp.detect = a.Detect.Body.Rect;
            r.vehicles.push_back(tmp);
        }
        r.releasedFrames = root.ReleaseCacheFrames;
    }

    void fillRecogResult(ImageResult &r, rec::RecogReplyPO &p) {
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
            tmp.detect = a.Detect.Body.Rect;
            if (!a.Persons.empty()) {
                algo::Attribute attr;
                for (auto b : a.Persons) {
                    algo::PersonObject tmpPerson;
                    tmpPerson.type = (algo::ObjectType)a.Type;
                    tmpPerson.detect = a.Detect.Body.Rect;
                    recogAttributesConv(b.Recognize, tmpPerson.attrs);
                    tmp.persons.push_back(tmpPerson);
                }
            }
            r.bikes.push_back(tmp);
        }
        for (auto a : root.Pedestrains) {
            algo::PersonObject tmp;
            tmp.type = (algo::ObjectType)a.Type;
            tmp.detect = a.Detect.Body.Rect;
            recogAttributesConv(a.Recognize, tmp.attrs);
            r.pedestrains.push_back(tmp);
        }
        for (auto a : root.Vehicles) {
            algo::VehicleObject tmp;
            tmp.type = (algo::ObjectType)a.Type;
            tmp.detect = a.Detect.Body.Rect;
            tmp.attrs.brand.WithName(a.Recognize.Brand.Name);
            tmp.attrs.color.WithName(a.Recognize.Color.Name);
            tmp.attrs.type.WithName(a.Recognize.Type.Name);
            tmp.attrs.plate.WithName(a.Recognize.Plate.Name);
            r.vehicles.push_back(tmp);
        }
    }

    void fillDetectRegion(const TrailParam &r, trail::DetectRegionPO &regionPO) {
        regionPO.regions = r.roi;
    }

    void fillLocations(const RecogParam &r, vector<rec::LocationPO> &locPOArray) {
        for (auto loc : r.locations) {
            rec::LocationPO l;
            l.ContextCode = loc.ContextCode;
            l.Rect = loc.detect;
            l.Trail = loc.trail;
            l.Type = loc.type;
            locPOArray.push_back(l);
        }
    }

    void recogAttributesConv(algo::seemmo::rec::PersonAttributeGroupPO &in, algo::PersonObject::tagAttributeGroup &out) {
        out.age.WithName(in.Age.Name);
        out.sex.WithName(in.Sex.Name);
        out.upperColor.WithName(in.UpperColor.Name);
        out.bottomColor.WithName(in.BottomColor.Name);
        out.orientation.WithName(in.Orientation.Name);
        out.hair.WithName(in.Hair.Name);
        out.umbrella.WithName(in.Umbrella.Name);
        out.hat.WithName(in.Hat.Name);
        out.upperType.WithName(in.UpperType.Name);
        out.bottomType.WithName(in.BottomType.Name);
        out.knapsack.WithName(in.Knapsack.Name);
        out.bag.WithName(in.Bag.Name);
        out.baby.WithName(in.Baby.Name);
        out.messengerBag.WithName(in.MessengerBag.Name);
        out.shoulderBag.WithName(in.ShoulderBag.Name);
        out.glasses.WithName(in.Glasses.Name);
        out.mask.WithName(in.Mask.Name);
        out.upperTexture.WithName(in.UpperTexture.Name);
        out.barrow.WithName(in.Barrow.Name);
        out.trolleyCase.WithName(in.TrolleyCase.Name);
    }

    void recogAttributesConv(algo::seemmo::rec::VehicleAttributeGroup &in, algo::VehicleObject::tagAttributeGroup &out) {
        out.brand.WithName(in.Brand.Name);
        out.color.WithName(in.Color.Name);
        out.type.WithName(in.Type.Name);
        out.plate.WithName(in.Plate.Name);
    }

private:
    unique_ptr<brpc::Channel> channel_;
    unique_ptr<VideoProcService_Stub> stub_;
};

VideoProcClient videoProcClient;

SeemmoAlgoStub::SeemmoAlgoStub()
    : algo::AlgoStub("seemmo") {
}

SeemmoAlgoStub::~SeemmoAlgoStub() {
}

int32_t SeemmoAlgoStub::Trail(
    uint32_t channelId,
    uint64_t frameId,
    const uint8_t *bgr24,
    uint32_t width,
    uint32_t height,
    const TrailParam &param,
    ImageResult &imageResult,
    FilterResult &filterResult
) {
    return videoProcClient.Trail(channelId, frameId, bgr24, width, height, param, imageResult, filterResult);
}

int32_t SeemmoAlgoStub::Recognize(
    uint32_t channelId,
    const uint8_t *bgr24,
    uint32_t width,
    uint32_t height,
    const RecogParam &param,
    ImageResult &imageResult
) {
    return videoProcClient.Recognize(channelId, bgr24, width, height, param, imageResult);
}

}
}
