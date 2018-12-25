#pragma once

#include <string>
#include <map>
#include <mutex>
#include <vector>

#include "common/helper/logger.h"
#include "algo/stub/object_type.h"
#include "vfilter/config/setting.h"

namespace video {
namespace filter {

//目标池
template<class T>
class ObjectSink {
public:

    ObjectSink() {
        gofIdx_ = 0;
        gofSize_ = 0;
    }

    std::vector<T> OnDetectedObjects(const std::vector<T> &objs) {
        std::unique_lock<std::mutex> lck(mutex_);
        std::vector<T> toRecObjs = calcNeedRecognizeObjects(objs);
        updateDetectedObjects(objs);
        return toRecObjs;
    }

    void OnRecognizedObjects(const std::vector<T> &objs) {
        std::unique_lock<std::mutex> lck(mutex_);
        updateRecognizedObjects(objs);
    }

    void IncreaseGofIdx() {
        std::unique_lock<std::mutex> lck(mutex_);
        gofIdx_++;
    }

    void GetMixableObjects(uint32_t frameWidth, uint32_t frameHeight, std::vector<T> &t1, std::vector<T> &t2) {
        std::unique_lock<std::mutex> lck(mutex_);
        for (auto &o : existObjs_) {
            if (o.second.Showable()) {
                T tmp = o.second.obj1;
                //修正抽帧导致的位置偏移
                tmp.detect = calcObjectRealRect(frameWidth, frameHeight, tmp.detect, tmp.trail);
                t1.push_back(tmp);
                t2.push_back(o.second.obj2);
            }
        }
    }

    bool ObjectExist(const std::string &objId) {
        std::unique_lock<std::mutex> lck(mutex_);
        return existObjs_.find(objId) != existObjs_.end();
    }

private:
    //根据抽帧后目标位置和偏移计算目标的真实位置
    algo::Rect calcObjectRealRect(uint32_t frameWidth, uint32_t frameHeight,
                                  const algo::Rect &rect, const algo::Shift &shift) {
        if (rect.size() != 4 || shift.size() != 2) {
            return rect;
        }

        int32_t x = rect[0], y = rect[1], w = rect[2], h = rect[3];
        int32_t sx = shift[0], sy = shift[1];
        int32_t wmax = (int32_t)frameWidth, hmax=(int32_t)frameHeight;

        if (gofSize_ > 0) {
            x += round((double)sx / (double)gofSize_) * gofIdx_;
            y += round((double)sy / (double)gofSize_) * gofIdx_;
            //计算后的坐标可能会超出画面，超出部分截断处理
            if (x < 0) {
                w += x;
                x = 0;
            }
            if (y < 0) {
                h += y;
                y = 0;
            }
            if (x + w > wmax) {
                w = wmax - x;
            }
            if (y + h > hmax) {
                h = hmax - y;
            }
            if (w < 0) {
                w = 0;
            }
            if (h < 0) {
                h = 0;
            }
        }

        return algo::Rect{ x,y,w,h };
    }

    std::vector<T>  calcNeedRecognizeObjects(const std::vector<T> &objs) {
        std::vector<T> toRecObjs;
        //计算需要重新识别的目标，以下情况需要从新识别
        // 1. 目标第一次出现【目标第一次出现，进行识别】
        // 2. 目标最新的评分比历史最高评分高【目标的评分高，说明可能清晰度更好，识别结果可能更准确】
        for (auto &o : objs) {
            if (existObjs_.find(o.guid) == existObjs_.end()) {
                toRecObjs.push_back(o);
            } else {
                auto &p = existObjs_[o.guid];
                if ((o.score > p.maxScore) && (o.score - p.maxScore > G_CFG().scoreDiff4ReRecognize)) {
                    toRecObjs.push_back(o);
                    p.maxScore = o.score;   //update max score
                }
            }
        }
        return toRecObjs;
    }

    void updateDetectedObjects(const std::vector<T> &objs) {
        for (auto &o : objs) {
            //如果没有找到，则添加到已存在目标容器中
            if (existObjs_.find(o.guid) == existObjs_.end()) {
                ObjectVO newObj;
                newObj.obj1 = o;
                existObjs_[o.guid] = newObj;
            } else {
                existObjs_[o.guid].obj1 = o;
                //重置计数
                existObjs_[o.guid].ResetCounter();
            }
        }

        std::vector<std::string> disappearedObjs;
        for (auto &o : existObjs_) {
            bool exist = false;
            for (auto &i : objs) {
                if (i.guid == o.second.obj1.guid) {
                    exist = true;
                    break;
                }
            }
            if (!exist) {
                /*
                * 在最新的目标中不存在，有两种情况
                * 1. 目标已经完全消失
                * 2. 目标只是在刚好这一帧中没有检测到，并不代表目标已经消失了
                * 针对这两种情况，通过减少目标的计数，当目标计数为0的时候，表示目标真的消失了。就删除掉
                */
                o.second.absentCount--;
                if (o.second.absentCount == 0) {
                    disappearedObjs.push_back(o.first);
                }
            }
        }

        //删掉消失的目标
        for (auto &o : disappearedObjs) {
            existObjs_.erase(o);
        }

        //更新gofsize，gofidx清0
        gofSize_ = gofIdx_;
        gofIdx_ = 0;
    }

    void updateRecognizedObjects(const std::vector<T> &objs) {
        for (auto &o : objs) {
            if (existObjs_.find(o.guid) != existObjs_.end()) {
                auto newObj = o;
                auto &oldObj = existObjs_[o.guid].obj2;
                //对比属性，根据属性的评分判断是否更新目标的属性
                for (auto &p : newObj.attrs) {
                    uint32_t attrKey = p.first;
                    if (oldObj.attrs.find(attrKey) != oldObj.attrs.end()) {
                        uint32_t newScore = p.second.score;
                        uint32_t oldScore = oldObj.attrs[attrKey].score;
                        if (newScore < oldScore) {
                            p.second = oldObj.attrs[attrKey];
                        }
                    }
                }
                existObjs_[o.guid].obj2 = newObj;
            }
        }
    }

private:
    class ObjectVO {
    public:
        T           obj1;           //obj1 存储检测结果，检测结果不带属性信息
        T           obj2;           //obj2 存储识别结果，识别结果带属性信息
        uint32_t    maxScore;       //目标最大分数，用来判断是否需要重新识别
        uint32_t    absentCount;    //消失计数

        ObjectVO() {
            maxScore = 0;
            absentCount = G_CFG().objectAbsentCount;
        }

        void ResetCounter() {
            absentCount = G_CFG().objectAbsentCount;
        }

        bool Showable() {
            return absentCount >= G_CFG().objectAbsentCount;
        }
    };

    std::mutex mutex_;
    //已经存在的目标<id, obj>
    std::map<std::string, ObjectVO> existObjs_;
    //因为是抽帧检测，所以，把相邻两次检测之间的帧认作一个gof（group of frame）
    //gofSize表示一个gof中的帧数目，gofidx表示一帧在当前gof中的序号
    int32_t gofSize_;
    int32_t gofIdx_;
};

typedef ObjectSink<algo::BikeObject> BikeObjectSink;
typedef ObjectSink<algo::VehicleObject> VehicleObjectSink;
typedef ObjectSink<algo::PersonObject> PersonObjectSink;
typedef ObjectSink<algo::FaceObject> FaceObjectSink;

}
}