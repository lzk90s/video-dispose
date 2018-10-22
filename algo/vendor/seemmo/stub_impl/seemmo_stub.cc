#include <iostream>
#include <memory>
#include <string>
#include <cassert>

#include <butil/logging.h>
#include <butil/time.h>
#include <brpc/channel.h>

#include "common/helper/logger.h"
#include "algo/vendor/seemmo/stub_impl/seemmo_stub.h"
#include "algo/vendor/seemmo/rpc/service.pb.h"
#include "algo/vendor/seemmo/stub_impl/detect_param_builder.h"
#include "algo/vendor/seemmo/stub_impl/filter_result_parser.h"
#include "algo/vendor/seemmo/stub_impl/rec_param_builder.h"
#include "algo/vendor/seemmo/stub_impl/rec_result_parser.h"
#include "algo/vendor/seemmo/stub_impl/detect_result_parser.h"
#include "algo/vendor/seemmo/shm/shmdata.h"

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
            SIMMNG::getInstance().Create(channelId, width, height, false);
        }

        uint8_t *bgr24Dst = SIMMNG::getInstance().Get(channelId)->GetBuffer().bgr24Buff1;

        request.set_videochl(channelId);
        request.set_timestamp(frameId);
        //request.set_bgr24((char*)bgr24, width*height * 3);		//bgr24图片字节大小为h*w*3
        memcpy(bgr24Dst, bgr24, width*height * 3);	//拷贝到共享内存中
        bgr24Dst[width*height * 3] = '\0';
        request.set_width(width);
        request.set_height(height);

        detect::DetectRegionPO regionPO;
        fillDetectRegion(param, regionPO);
        request.set_param(detect::DetectParamBuilder().Build(regionPO).c_str());

        stub_->Trail(&cntl, &request, &reply, NULL);
        if (cntl.Failed()) {
            LOG_ERROR("Trail error, {}", cntl.ErrorText());
            return cntl.ErrorCode();
        }

        // parse detect result
        detect::DetectReplyPO detectRspPO;
        detect::DetectResponseParser().Parse(reply.data(), detectRspPO);
        // parse filter result
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
            SIMMNG::getInstance().Create(channelId, width, height, false);
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
        options.protocol = "baidu_std";
        options.connection_type = "single";
        options.timeout_ms = 2000/*milliseconds*/;
        options.max_retry = 0;
        if (channel_->Init("0.0.0.0:7000", "", &options) != 0) {
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

        for (auto &a : root.Bikes) {
            if (a.GUID.empty()) {
                continue;
            }
            algo::BikeObject tmp;
            tmp.type = (algo::ObjectType)a.Type;
            tmp.guid = a.GUID;
            tmp.trail = a.Trail;
            tmp.detect = a.Detect.Body.Rect;
            tmp.score = a.Detect.Body.Score;
            r.bikes.push_back(tmp);
        }
        for (auto &a : root.Pedestrains) {
            if (a.GUID.empty()) {
                continue;
            }
            algo::PersonObject tmp;
            tmp.type = (algo::ObjectType)a.Type;
            tmp.guid = a.GUID;
            tmp.trail = a.Trail;
            tmp.detect = a.Detect.Body.Rect;
            tmp.score = a.Detect.Body.Score;
            r.pedestrains.push_back(tmp);
        }
        for (auto &a : root.Vehicles) {
            if (a.GUID.empty()) {
                continue;
            }
            algo::VehicleObject tmp;
            tmp.type = (algo::ObjectType)a.Type;
            tmp.guid = a.GUID;
            tmp.trail = a.Trail;
            tmp.detect = a.Detect.Body.Rect;
            tmp.score = a.Detect.Body.Score;
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

        for (auto &a : root.Bikes) {
            algo::BikeFilter tmp;
            tmp.type = (algo::ObjectType)a.Type;
            tmp.guid = a.GUID;
            tmp.trail = a.Trail;
            tmp.contextCode = a.ContextCode;
            tmp.frameId = a.Timestamp;
            tmp.detect = a.Detect.Body.Rect;
            tmp.score = a.Detect.Body.Score;
            r.bikes.push_back(tmp);
        }
        for (auto &a : root.Pedestrains) {
            algo::PersonFilter tmp;
            tmp.type = (algo::ObjectType)a.Type;
            tmp.guid = a.GUID;
            tmp.trail = a.Trail;
            tmp.contextCode = a.ContextCode;
            tmp.frameId = a.Timestamp;
            tmp.detect = a.Detect.Body.Rect;
            tmp.score = a.Detect.Body.Score;
            r.pedestrains.push_back(tmp);
        }
        for (auto &a : root.Vehicles) {
            algo::VehicleFilter tmp;
            tmp.type = (algo::ObjectType)a.Type;
            tmp.guid = a.GUID;
            tmp.trail = a.Trail;
            tmp.contextCode = a.ContextCode;
            tmp.frameId = a.Timestamp;
            tmp.detect = a.Detect.Body.Rect;
            tmp.score = a.Detect.Body.Score;
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

        for (auto &a : root.Bikes) {
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
        for (auto &a : root.Pedestrains) {
            algo::PersonObject tmp;
            tmp.type = (algo::ObjectType)a.Type;
            tmp.detect = a.Detect.Body.Rect;
            recogAttributesConv(a.Recognize, tmp.attrs);
            r.pedestrains.push_back(tmp);
        }
        for (auto &a : root.Vehicles) {
            algo::VehicleObject tmp;
            tmp.type = (algo::ObjectType)a.Type;
            tmp.detect = a.Detect.Body.Rect;
            recogAttributesConv(a.Recognize, tmp.attrs);
            r.vehicles.push_back(tmp);
        }
    }

    void fillDetectRegion(const TrailParam &r, detect::DetectRegionPO &regionPO) {
        regionPO.regions = r.roi;
    }

    void fillLocations(const RecogParam &r, vector<rec::LocationPO> &locPOArray) {
        for (auto &loc : r.locations) {
            rec::LocationPO l;
            l.ContextCode = loc.ContextCode;
            l.Rect = loc.detect;
            l.Trail = loc.trail;
            l.Type = loc.type;
            locPOArray.push_back(l);
        }
    }

    void recogAttributesConv(algo::seemmo::rec::PersonAttributeGroupPO &in, algo::Attributes &out) {
        out[algo::PersonObject::AttrType::AGE] = algo::Attribute(in.Age.Name, in.Age.Score);
        out[algo::PersonObject::AttrType::SEX] = algo::Attribute(in.Sex.Name, in.Sex.Score);
        out[algo::PersonObject::AttrType::UPPER_COLOR] = algo::Attribute(in.UpperColor.Name, in.UpperColor.Score);
        out[algo::PersonObject::AttrType::BOTTOM_COLOR] = algo::Attribute(in.BottomColor.Name, in.BottomColor.Score);
        out[algo::PersonObject::AttrType::ORIENTATION] = algo::Attribute(in.Orientation.Name, in.Orientation.Score);
        out[algo::PersonObject::AttrType::HAIR] = algo::Attribute(in.Hair.Name, in.Hair.Score);
        out[algo::PersonObject::AttrType::UMBERLLA] = algo::Attribute(in.Umbrella.Name, in.Umbrella.Score);
        out[algo::PersonObject::AttrType::HAT] = algo::Attribute(in.Hat.Name, in.Hat.Score);
        out[algo::PersonObject::AttrType::UPPER_TYPE] = algo::Attribute(in.UpperType.Name, in.UpperType.Score);
        out[algo::PersonObject::AttrType::BOTTOM_TYPE] = algo::Attribute(in.BottomType.Name, in.BottomType.Score);
        out[algo::PersonObject::AttrType::KNAPSACK] = algo::Attribute(in.Knapsack.Name, in.Knapsack.Score);
        out[algo::PersonObject::AttrType::BAG] = algo::Attribute(in.Bag.Name, in.Bag.Score);
        out[algo::PersonObject::AttrType::BABY] = algo::Attribute(in.Baby.Name, in.Baby.Score);
        out[algo::PersonObject::AttrType::MESSAGER_BAG] = algo::Attribute(in.MessengerBag.Name, in.MessengerBag.Score);
        out[algo::PersonObject::AttrType::SHOULDER_BAG] = algo::Attribute(in.ShoulderBag.Name, in.ShoulderBag.Score);
        out[algo::PersonObject::AttrType::GLASSES] = algo::Attribute(in.Glasses.Name, in.Glasses.Score);
        out[algo::PersonObject::AttrType::MASK] = algo::Attribute(in.Mask.Name, in.Mask.Score);
        out[algo::PersonObject::AttrType::UPPER_TEXTURE] = algo::Attribute(in.UpperTexture.Name, in.UpperTexture.Score);
        out[algo::PersonObject::AttrType::BARROW] = algo::Attribute(in.Barrow.Name, in.Barrow.Score);
        out[algo::PersonObject::AttrType::TROLLEY_CASE] = algo::Attribute(in.TrolleyCase.Name, in.TrolleyCase.Score);
    }

    void recogAttributesConv(algo::seemmo::rec::VehicleAttributeGroup &in, algo::Attributes &out) {
        out[algo::VehicleObject::AttrType::BRAND] = algo::Attribute(in.Brand.Name, in.Brand.Score);
        out[algo::VehicleObject::AttrType::COLOR] = algo::Attribute(in.Color.Name, in.Color.Score);
        out[algo::VehicleObject::AttrType::TYPE] = algo::Attribute(in.Type.Name, in.Type.Score);
        out[algo::VehicleObject::AttrType::PLATE] =algo::Attribute(in.Plate.Name, in.Plate.Score);
    }

private:
    unique_ptr<brpc::Channel> channel_;
    unique_ptr<VideoProcService_Stub> stub_;
};

VideoProcClient videoProcClient;

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
