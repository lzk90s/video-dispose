#pragma once

#include "common/helper/threadpool.h"

#include "algo/stub/algo_stub_factory.h"
#include "vfilter/proc/abstract_algo_proc.h"

namespace video {
namespace filter {

class DefaultAlgoProcessor : public AbstractAlgoProcessor {
public:
    DefaultAlgoProcessor()
        : AbstractAlgoProcessor("seemmo", G_CFG().seemmoFramePickInterval),
          algo_(algo::AlgoStubFactory::NewAlgoStub("seemmo")),
          worker_(1),
          recogWorker_(1) {
    }

    ~DefaultAlgoProcessor() {
        waitForComplete();
    }

protected:
    void onFrame(std::shared_ptr<ChannelSink> chl, cv::Mat &frame) override {
        uint64_t frameId = chl->frameCache.AllocateEmptyFrame();
        worker_.commit(std::bind(&DefaultAlgoProcessor::algoRoutine, this, chl, frameId, frame));
    }

    void onFrameEnd(std::shared_ptr<ChannelSink> chl) override {
        waitForComplete();
        algo_->TrailEnd(chl->GetChannelId());
    }

    void mixFrame(std::shared_ptr<ChannelSink> chl, cv::Mat &frame) override {
        chl->vehicleMixer.MixFrame(frame, chl->vehicleObjectSink);
        chl->personMixer.MixFrame(frame, chl->personObjectSink);
        chl->bikeMixer.MixFrame(frame, chl->bikeObjectSink);
        chl->vehicleObjectSink.IncreaseGofIdx();
        chl->personObjectSink.IncreaseGofIdx();
        chl->bikeObjectSink.IncreaseGofIdx();
    }

private:
    void waitForComplete() {
        while (true) {
            if ((worker_.taskCount() == 0) && (recogWorker_.taskCount() == 0)) {
                break;
            }
            LOG_INFO("Waiting for uncompleted tasks, count ({}, {})", worker_.taskCount(), recogWorker_.taskCount());
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    int32_t algoRoutine(std::shared_ptr<ChannelSink> chl, uint64_t frameId, cv::Mat &frame) {
        chl->watchdog.Feed();
        return trailAndRecognize(chl, frameId, frame);
    }

    //跟踪+识别
    int32_t trailAndRecognize(std::shared_ptr<ChannelSink> chl, uint64_t frameId, cv::Mat &frame) {
        int32_t ret = 0;
        uint32_t width = frame.cols;
        uint32_t height = frame.rows;

        algo::TrailParam trailParam;
        // roi区域设置为全图
        trailParam.roi.push_back(algo::Point{ 0, 0 });
        trailParam.roi.push_back(algo::Point{ 0, (int32_t)height });
        trailParam.roi.push_back(algo::Point{ (int32_t)width, (int32_t)height });
        trailParam.roi.push_back(algo::Point{ (int32_t)width, 0 });

        //检测
        algo::ImageResult imageResult;
        algo::FilterResult filterResult;
        ret = algo_->Trail(chl->GetChannelId(), frameId, frame.data, width, height, trailParam, imageResult, filterResult);
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

    void saveImage(std::shared_ptr<ChannelSink> chl, uint64_t frameId, cv::Mat &frame, algo::ImageResult &imageResult) {
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
            chl->frameCache.SaveAllObjectImageInFrame(frameId, objectImages);
        }
    }

    //检测到目标
    void asyncRecognizeObject(std::shared_ptr<ChannelSink> chl, uint64_t frameId, cv::Mat &frame,
                              algo::ImageResult &imageResult) {
        recogWorker_.commit([=]() {
            cv::Mat f = frame;
            algo::RecogParam recParam;

            auto bikeObjs = chl->bikeObjectSink.OnDetectedObjects(imageResult.bikes);
            for (auto &p : bikeObjs) {
                algo::RecogParam::ObjLocation loc;
                loc.type = p.type;
                loc.trail = p.trail;
                loc.detect = p.detect;
                loc.guid = p.guid;
                recParam.locations.push_back(loc);
            }
            auto vehicleObjs = chl->vehicleObjectSink.OnDetectedObjects(imageResult.vehicles);
            for (auto &p : vehicleObjs) {
                algo::RecogParam::ObjLocation loc;
                loc.type = p.type;
                loc.trail = p.trail;
                loc.detect = p.detect;
                loc.guid = p.guid;
                recParam.locations.push_back(loc);
            }
            auto personObjs = chl->personObjectSink.OnDetectedObjects(imageResult.pedestrains);
            for (auto &p : personObjs) {
                algo::RecogParam::ObjLocation loc;
                loc.type = p.type;
                loc.trail = p.trail;
                loc.detect = p.detect;
                loc.guid = p.guid;
                recParam.locations.push_back(loc);
            }

            if (recParam.locations.empty()) {
                return;
            }

            algo::ImageResult recImageResult;
            int32_t ret = algo_->Recognize(chl->GetChannelId(), frame.data, frame.cols, frame.rows, recParam, recImageResult);
            if (0 != ret) {
                LOG_ERROR("Recognize error, ret {}", ret);
                return;
            }

            size_t sz = recImageResult.bikes.size() + recImageResult.vehicles.size() + recImageResult.pedestrains.size();
            if (sz != recParam.locations.size()) {
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

            chl->vehicleObjectSink.OnRecognizedObjects(recImageResult.vehicles);
            chl->bikeObjectSink.OnRecognizedObjects(recImageResult.bikes);
            chl->personObjectSink.OnRecognizedObjects(recImageResult.pedestrains);
        });
    }

    void asyncRecognizeFilterObject(std::shared_ptr<ChannelSink> chl, uint64_t frameId, algo::FilterResult &filterResult) {

        recogWorker_.commit([=]() {
            for (auto &p : filterResult.bikes) {
                bool exist = false;
                cv::Mat img = chl->frameCache.GetObjectImageInFrame(p.frameId, p.guid, exist);
                if (!exist) {
                    LOG_WARN("The saved object {} in frame {} not exist", p.guid, p.frameId);
                    continue;
                }

                algo::RecogParam recParam;
                algo::RecogParam::ObjLocation loc;
                loc.type = p.type;
                loc.ContextCode = p.contextCode;
                loc.trail = p.trail;
                loc.detect = { 0, 0, img.cols, img.rows };
                recParam.locations.push_back(loc);
                recognizeFilterObjectInner(chl, img, recParam);
            }

            for (auto &p : filterResult.vehicles) {
                bool exist = false;
                cv::Mat img = chl->frameCache.GetObjectImageInFrame(p.frameId, p.guid, exist);
                if (!exist) {
                    LOG_WARN("The saved object {} in frame {} not exist", p.guid, p.frameId);
                    continue;
                }

                algo::RecogParam recParam;
                algo::RecogParam::ObjLocation loc;
                loc.type = p.type;
                loc.ContextCode = p.contextCode;
                loc.trail = p.trail;
                loc.detect = { 0, 0, img.cols, img.rows };
                recParam.locations.push_back(loc);
                recognizeFilterObjectInner(chl, img, recParam);
            }

            for (auto &p : filterResult.pedestrains) {
                bool exist = false;
                cv::Mat img = chl->frameCache.GetObjectImageInFrame(p.frameId, p.guid, exist);
                if (!exist) {
                    LOG_WARN("The saved object {} in frame {} not exist", p.guid, p.frameId);
                    continue;
                }

                algo::RecogParam recParam;
                algo::RecogParam::ObjLocation loc;
                loc.type = p.type;
                loc.ContextCode = p.contextCode;
                loc.trail = p.trail;
                loc.detect = { 0, 0, img.cols, img.rows };
                recParam.locations.push_back(loc);
                recognizeFilterObjectInner(chl, img, recParam);
            }

            //把不需要的帧手动释放掉
            for (auto &fid : filterResult.releasedFrames) {
                chl->frameCache.ManualRelase(fid);
            }
        });
    }

    int32_t recognizeFilterObjectInner(std::shared_ptr<ChannelSink> chl, cv::Mat &frame, algo::RecogParam &recParam) {
        if (recParam.locations.empty()) {
            return 0;
        }

        algo::ImageResult imageResult;
        int32_t ret = algo_->Recognize(chl->GetChannelId(), frame.data, frame.cols, frame.rows, recParam, imageResult);
        if (0 != ret) {
            LOG_ERROR("Recognize error, ret {}", ret);
            return ret;
        }

        for (auto &p : imageResult.bikes) {
            chl->bikeNotifier.OnRecognizedObject(chl->GetChannelId(), frame, p);
        }
        for (auto &p : imageResult.pedestrains) {
            chl->personNotifier.OnRecognizedObject(chl->GetChannelId(), frame, p);
        }
        for (auto &p : imageResult.vehicles) {
            chl->vehicleNotifier.OnRecognizedObject(chl->GetChannelId(), frame, p);
        }

        return 0;
    }

private:
    std::shared_ptr<algo::AlgoStub> algo_;
    std::threadpool worker_;
    std::threadpool recogWorker_;
};

}
}