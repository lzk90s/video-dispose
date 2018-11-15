#pragma once

#include "algo/stub/algo_stub_factory.h"
#include "vfilter/proc/abstract_algo_proc.h"

using namespace std;

namespace vf {

class DefaultAlgoProcessor : public AbstractAlgoProcessor {
public:
    DefaultAlgoProcessor() : recogWorker_(1) {
        string vendor = GlobalSettings::getInstance().enableSeemmoAlgo ? "seemmo" : "null";
        algo_ = algo::AlgoStubFactory::NewAlgoStub(vendor);
    }

protected:

    int32_t algoRoutine(ChannelSink &chl, uint64_t frameId, cv::Mat &frame) override {
        return trailAndRecognize(chl, frameId, frame);
    }

    int32_t algoRoutineEnd(ChannelSink &chl) override {
        return algo_->TrailEnd(chl.GetChannelId());
    }

private:

    //跟踪+识别
    int32_t trailAndRecognize(ChannelSink &chl, uint64_t frameId, cv::Mat &frame) {
        int32_t ret = 0;
        uint32_t width = frame.cols;
        uint32_t height = frame.rows;

        TrailParam trailParam;
        // roi区域设置为全图
        trailParam.roi.push_back(Point{ 0, 0 });
        trailParam.roi.push_back(Point{ 0, (int32_t)height });
        trailParam.roi.push_back(Point{ (int32_t)width, (int32_t)height });
        trailParam.roi.push_back(Point{ (int32_t)width, 0 });

        //检测
        ImageResult imageResult;
        FilterResult filterResult;
        ret = algo_->Trail(chl.GetChannelId(), frameId, frame.data, width, height, trailParam, imageResult, filterResult);
        if (0 != ret) {
            LOG_ERROR("Trail error, ret {}", ret);
            return ret;
        }

        //保存目标的图片
        saveImage(chl, frameId, frame, imageResult);

        //识别目标
        asyncRecognizeObject(chl, frameId, frame, imageResult);

        //识别择优结果
        asyncRecognizeFilterObject(chl, frameId, filterResult);

        return 0;
    }

    void saveImage(ChannelSink &chl, uint64_t frameId, cv::Mat &frame, ImageResult &imageResult) {
        //根据检测结果抠图
        FrameCache::ObjectImageMap objectImages;
        for (auto &p : imageResult.bikes) {
            int32_t x = p.detect[0], y = p.detect[1], w = p.detect[2], h = p.detect[3];
            cv::Rect  rect = cv::Rect(x, y, w, h);
            cv::Mat roi = frame(rect);
            cv::Mat img = roi.clone();
            objectImages[p.guid] = img;
        }
        for (auto &p : imageResult.pedestrains) {
            int32_t x = p.detect[0], y = p.detect[1], w = p.detect[2], h = p.detect[3];
            cv::Rect  rect = cv::Rect(x, y, w, h);
            cv::Mat roi = frame(rect);
            cv::Mat img = roi.clone();
            objectImages[p.guid] = img;
        }
        for (auto &p : imageResult.vehicles) {
            int32_t x = p.detect[0], y = p.detect[1], w = p.detect[2], h = p.detect[3];
            cv::Rect  rect = cv::Rect(x, y, w, h);
            cv::Mat roi = frame(rect);
            cv::Mat img = roi.clone();
            objectImages[p.guid] = img;
        }
        for (auto &p : imageResult.faces) {
            int32_t x = p.detect[0], y = p.detect[1], w = p.detect[2], h = p.detect[3];
            cv::Rect  rect = cv::Rect(x, y, w, h);
            cv::Mat roi = frame(rect);
            cv::Mat img = roi.clone();
            objectImages[p.guid] = img;
        }

        //保存目标抠图，后续的择优识别需要用到
        if (!objectImages.empty()) {
            chl.frameCache_.SaveAllObjectImageInFrame(frameId, objectImages);
        }
    }

    //检测到目标
    void asyncRecognizeObject(ChannelSink &chl, uint64_t frameId, cv::Mat &frame, ImageResult &imageResult) {
        ChannelSink *chlPtr = &chl;
        recogWorker_.commit([=]() {
            cv::Mat f = frame;
            algo::RecogParam recParam;

            auto bikeObjs = chlPtr->bikeObjectSink.OnDetectedObjects(imageResult.bikes);
            for (auto &p : bikeObjs) {
                RecogParam::ObjLocation loc;
                loc.type = p.type;
                loc.trail = p.trail;
                loc.detect = p.detect;
                loc.guid = p.guid;
                recParam.locations.push_back(loc);
            }
            auto vehicleObjs = chlPtr->vehicleObjectSink.OnDetectedObjects(imageResult.vehicles);
            for (auto &p : vehicleObjs) {
                RecogParam::ObjLocation loc;
                loc.type = p.type;
                loc.trail = p.trail;
                loc.detect = p.detect;
                loc.guid = p.guid;
                recParam.locations.push_back(loc);
            }
            auto personObjs = chlPtr->personObjectSink.OnDetectedObjects(imageResult.pedestrains);
            for (auto &p : personObjs) {
                RecogParam::ObjLocation loc;
                loc.type = p.type;
                loc.trail = p.trail;
                loc.detect = p.detect;
                loc.guid = p.guid;
                recParam.locations.push_back(loc);
            }

            if (recParam.locations.empty()) {
                return;
            }

            ImageResult recImageResult;
            int32_t ret = algo_->Recognize(chlPtr->GetChannelId(), frame.data, frame.cols, frame.rows, recParam, recImageResult);
            if (0 != ret) {
                LOG_ERROR("Recognize error, ret {}", ret);
                return;
            }

            size_t sz = recImageResult.bikes.size() + recImageResult.vehicles.size() + recImageResult.pedestrains.size();
            if ( sz != recParam.locations.size()) {
                LOG_WARN("The recognize result size is error, request num {}, rsp num {}", recParam.locations.size(), sz);
                return;
            }

            auto itr = recParam.locations.begin();
            //深瞐识别时没有返回guid，这里手动设置一下
            //注意：这里的顺序和上面的存入recparam的顺序必须一致，否则对应关系会错误
            for (uint32_t idx = 0; idx < recImageResult.bikes.size(); idx++, ++itr) {
                recImageResult.bikes[idx].guid = (*itr).guid;
            }
            for (uint32_t idx = 0; idx < recImageResult.vehicles.size(); idx++, ++itr) {
                recImageResult.vehicles[idx].guid = (*itr).guid;
            }
            for (uint32_t idx = 0; idx < recImageResult.pedestrains.size(); idx++, ++itr) {
                recImageResult.pedestrains[idx].guid = (*itr).guid;
            }

            chlPtr->vehicleObjectSink.OnRecognizedObjects(recImageResult.vehicles);
            chlPtr->bikeObjectSink.OnRecognizedObjects(recImageResult.bikes);
            chlPtr->personObjectSink.OnRecognizedObjects(recImageResult.pedestrains);
        });
    }

    void asyncRecognizeFilterObject(ChannelSink &chl, uint64_t frameId, FilterResult &filterResult) {
        ChannelSink *chlPtr = &chl;
        recogWorker_.commit([=]() {
            for (auto &p : filterResult.bikes) {
                bool exist = false;
                cv::Mat img = chlPtr->frameCache_.GetObjectImageInFrame(p.frameId, p.guid, exist);
                if (!exist) {
                    LOG_WARN("The saved object {} in frame {} not exist", p.guid, p.frameId);
                    continue;
                }

                RecogParam recParam;
                RecogParam::ObjLocation loc;
                loc.type = p.type;
                loc.ContextCode = p.contextCode;
                loc.trail = p.trail;
                loc.detect = { 0, 0, img.cols, img.rows };
                recParam.locations.push_back(loc);
                recognizeFilterObjectInner(*chlPtr, img, recParam);
            }

            for (auto &p : filterResult.vehicles) {
                bool exist = false;
                cv::Mat img = chlPtr->frameCache_.GetObjectImageInFrame(p.frameId, p.guid, exist);
                if (!exist) {
                    LOG_WARN("The saved object {} in frame {} not exist", p.guid, p.frameId);
                    continue;
                }

                RecogParam recParam;
                RecogParam::ObjLocation loc;
                loc.type = p.type;
                loc.ContextCode = p.contextCode;
                loc.trail = p.trail;
                loc.detect = { 0, 0, img.cols, img.rows };
                recParam.locations.push_back(loc);
                recognizeFilterObjectInner(*chlPtr, img, recParam);
            }

            for (auto &p : filterResult.pedestrains) {
                bool exist = false;
                cv::Mat img = chlPtr->frameCache_.GetObjectImageInFrame(p.frameId, p.guid, exist);
                if (!exist) {
                    LOG_WARN("The saved object {} in frame {} not exist", p.guid, p.frameId);
                    continue;
                }

                RecogParam recParam;
                RecogParam::ObjLocation loc;
                loc.type = p.type;
                loc.ContextCode = p.contextCode;
                loc.trail = p.trail;
                loc.detect = { 0, 0, img.cols, img.rows };
                recParam.locations.push_back(loc);
                recognizeFilterObjectInner(*chlPtr, img, recParam);
            }

            //把不需要的帧手动释放掉
            for (auto &fid : filterResult.releasedFrames) {
                chlPtr->frameCache_.ManualRelase(fid);
            }
        });
    }

    int32_t recognizeFilterObjectInner(ChannelSink &chl, cv::Mat &frame, RecogParam &recParam) {
        if (recParam.locations.empty()) {
            return 0;
        }

        ImageResult imageResult;
        int32_t ret = algo_->Recognize(chl.GetChannelId(), frame.data, frame.cols, frame.rows, recParam, imageResult);
        if (0 != ret) {
            LOG_ERROR("Recognize error, ret {}", ret);
            return ret;
        }

        for (auto &p : imageResult.bikes) {
            chl.bikeNotifier.OnRecognizedObject(chl.GetChannelId(), frame, p);
        }
        for (auto &p : imageResult.pedestrains) {
            chl.personNotifier.OnRecognizedObject(chl.GetChannelId(), frame, p);
        }
        for (auto &p : imageResult.vehicles) {
            chl.vehicleNotifier.OnRecognizedObject(chl.GetChannelId(), frame, p);
        }

        return 0;
    }

private:
    shared_ptr<algo::AlgoStub> algo_;
    threadpool recogWorker_;
};

}