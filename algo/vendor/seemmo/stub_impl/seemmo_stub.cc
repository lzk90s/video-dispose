#include <iostream>
#include <memory>
#include <string>
#include <cassert>

#include <butil/logging.h>
#include <butil/time.h>
#include <brpc/channel.h>

#include "common/helper/logger.h"
#include "common/helper/counttimer.h"

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

    int32_t TrailEnd(uint32_t channelId) {
        TrailEndRequest request;
        TrailEndReply reply;
        brpc::Controller cntl;

        if (SIMMNG::getInstance().Exist(channelId)) {
            SIMMNG::getInstance().Delete(channelId);
        }

        request.set_videochl(channelId);

        stub_->TrailEnd(&cntl, &request, &reply, NULL);
        if (cntl.Failed()) {
            LOG_ERROR("TrailEnd error, {}", cntl.ErrorText());
            return cntl.ErrorCode();
        }

        LOG_INFO("Trail end for channel {}", channelId);

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
        if (channel_->Init("seemmo-algo-server:7000", "", &options) != 0) {
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
        using AttrType = algo::PersonObject::AttrType;
        buildAttr(in.Age.Name, in.Age.Score, AttrType::AGE, out);
        buildAttr(in.Sex.Name, in.Sex.Score, AttrType::SEX, out);
        buildAttr(in.UpperColor.Name, in.UpperColor.Score, AttrType::UPPER_COLOR, out);
        buildAttr(in.BottomColor.Name, in.BottomColor.Score, AttrType::BOTTOM_COLOR, out);
        buildAttr(in.Orientation.Name, in.Orientation.Score, AttrType::ORIENTATION, out);
        buildAttr(in.Hair.Name, in.Hair.Score, AttrType::HAIR, out);
        buildAttr(in.Umbrella.Name, in.Umbrella.Score, AttrType::UMBERLLA, out);
        buildAttr(in.Hat.Name, in.Hat.Score, AttrType::HAT, out);
        buildAttr(in.UpperType.Name, in.UpperType.Score, AttrType::UPPER_TYPE, out);
        buildAttr(in.BottomType.Name, in.BottomType.Score, AttrType::BOTTOM_TYPE, out);
        buildAttr(in.Knapsack.Name, in.Knapsack.Score, AttrType::KNAPSACK, out);
        buildAttr(in.Bag.Name, in.Bag.Score, AttrType::BAG, out);
        buildAttr(in.Baby.Name, in.Baby.Score, AttrType::BABY, out);
        buildAttr(in.MessengerBag.Name, in.MessengerBag.Score, AttrType::MESSAGER_BAG, out);
        buildAttr(in.ShoulderBag.Name, in.ShoulderBag.Score, AttrType::SHOULDER_BAG, out);
        buildAttr(in.Glasses.Name, in.Glasses.Score, AttrType::GLASSES, out);
        buildAttr(in.Mask.Name, in.Mask.Score, AttrType::MASK, out);
        buildAttr(in.UpperTexture.Name, in.UpperTexture.Score, AttrType::UPPER_TEXTURE, out);
        buildAttr(in.Barrow.Name, in.Barrow.Score, AttrType::BARROW, out);
        buildAttr(in.TrolleyCase.Name, in.TrolleyCase.Score, AttrType::TROLLEY_CASE, out);
    }

    void recogAttributesConv(algo::seemmo::rec::VehicleAttributeGroup &in, algo::Attributes &out) {
        using AttrType = algo::VehicleObject::AttrType;
        buildAttr(in.Brand.Name, in.Brand.Score, AttrType::BRAND, out);
        buildAttr(in.Color.Name, in.Color.Score, AttrType::COLOR, out);
        buildAttr(in.Type.Name, in.Type.Score, AttrType::TYPE, out);
        buildAttr(in.Plate.Name, in.Plate.Score, AttrType::PLATE, out);
    }

    void buildAttr(const string &attrName, int32_t score, uint32_t attrType, algo::Attributes &out) {
        if (!attrName.empty()) {
            out[attrType] = algo::Attribute(attrName, score);
        } else {
            out[attrType] = algo::Attribute("", 0);
        }
    }

private:
    unique_ptr<brpc::Channel> channel_;
    unique_ptr<VideoProcService_Stub> stub_;
};

static VideoProcClient videoProcClient;

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
    CountTimer t1("SeemmoAlgoStub::Trail", 80 * 1000);
    return videoProcClient.Trail(channelId, frameId, bgr24, width, height, param, imageResult, filterResult);
}

int32_t SeemmoAlgoStub::TrailEnd(uint32_t channelId) {
    CountTimer t1("SeemmoAlgoStub::TrailEnd", 80 * 1000);
    return videoProcClient.TrailEnd(channelId);
}

int32_t SeemmoAlgoStub::Recognize(
    uint32_t channelId,
    const uint8_t *bgr24,
    uint32_t width,
    uint32_t height,
    const RecogParam &param,
    ImageResult &imageResult
) {
    CountTimer t1("SeemmoAlgoStub::Recognize", 80 * 1000);
    return videoProcClient.Recognize(channelId, bgr24, width, height, param, imageResult);
}

}
}
