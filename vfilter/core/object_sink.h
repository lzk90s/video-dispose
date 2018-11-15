#pragma once

#include <string>
#include <map>
#include <mutex>
#include <vector>

#include "common/helper/logger.h"
#include "algo/stub/object_type.h"
#include "vfilter/config/setting.h"


namespace vf {

//目标池
template<class T>
class ObjectSink {
public:
    using ObjectAppearHandler = function<void(const string&, void *)> ;
    using ObjectDisappearHandler = function<void(const string&)>;

public:

    ObjectSink() {
        gofIdx_ = 0;
        gofSize_ = 0;
        objAppearHandler_ = nullptr;
        objDisappearHandler_ = nullptr;
        lastTime_ = chrono::steady_clock::now();
        currTime_ = chrono::steady_clock::now();
    }

    void SetObjectAppearHandler(ObjectAppearHandler h) {
        this->objAppearHandler_ = h;
    }

    void SetObjectDisappearHandler(ObjectDisappearHandler h) {
        this->objDisappearHandler_ = h;
    }

    vector<T> OnDetectedObjects(const vector<T> &objs) {
        unique_lock<mutex> lck(mutex_);
        vector<T> toRecObjs = calcNeedRecognizeObjects(objs);
        updateDetectedObjects(objs);
        return toRecObjs;
    }

    void OnRecognizedObjects(const vector<T> &objs) {
        unique_lock<mutex> lck(mutex_);
        updateRecognizedObjects(objs);
    }

    void IncreaseGofIdx() {
        unique_lock<mutex> lck(mutex_);
        gofIdx_++;
    }

    void GetShowableObjects(vector<T> &t1, vector<T> &t2) {
        unique_lock<mutex> lck(mutex_);
        for (auto &o : existObjs_) {
            if (o.second.Showable()) {
                T tmp = o.second.obj1;
                //修正抽帧导致的位置偏移
                tmp.detect = fixObjectRect(tmp.detect, tmp.trail);
                t1.push_back(tmp);
                t2.push_back(o.second.obj2);
            }
        }
    }

    bool ObjectExist(const string &objId) {
        unique_lock<mutex> lck(mutex_);
        return existObjs_.find(objId) != existObjs_.end();
    }

private:
    //目标位移修正, shift表示的是当前帧与上一帧之间的位移
    //目标的移动一般都是有规律的，因为是抽帧的，所以用目标当前区域和位移来计算出预估的位置
    algo::Rect fixObjectRect(algo::Rect &rect, algo::Shift &shift) {
        if (rect.size() != 4 || shift.size() != 2) {
            return rect;
        }

        int32_t x = rect[0], y = rect[1], w = rect[2], h = rect[3];
        int32_t gofSize = (int32_t)gofSize_;
        if (gofSize > 0) {
            int32_t gofIdx = gofIdx_ > gofSize_ ? (int32_t)gofSize_ : (int32_t)gofIdx_;
            int32_t sx = shift[0], sy = shift[1];
            x += round((double)sx / (double)gofSize) * gofIdx;
            y += round((double)sy / (double)gofSize) * gofIdx;
            //计算后，可能会导致x，或者y小于0，小于0时，设置为0
            x = (x < 0) ? 0 : x;
            y = (y < 0) ? 0 : y;
        }
        return algo::Rect{ x,y,w,h };
    }

    //计算需要识别的目标
    vector<T>  calcNeedRecognizeObjects(const vector<T> &objs) {
        vector<T> toRecObjs;
        // 1. 超过全识别时间间隔【有时候第一次识别是识别错的，定时刷新当前目标的全部结果】
        // 2. 目标第一次出现【目标第一次出现，进行识别】
        // 3. 目标最新的评分比上一次高【目标的评分高，说明可能清晰度更好，识别结果可能更准确】
        currTime_ = chrono::steady_clock::now();
        auto diffTime = std::chrono::duration_cast<std::chrono::milliseconds>(currTime_ - lastTime_).count();
        if (diffTime >= GlobalSettings::getInstance().fullRecognizeInternalMs) {
            toRecObjs = objs;
            lastTime_ = currTime_;
        } else {
            for (auto &o : objs) {
                if (existObjs_.find(o.guid) == existObjs_.end()) {
                    toRecObjs.push_back(o);
                } else {
                    uint32_t currScore = o.score;
                    uint32_t lastScore = existObjs_[o.guid].obj1.score;
                    if ((currScore > lastScore) && (currScore - lastScore > GlobalSettings::getInstance().scoreDiff4ReRecognize)) {
                        toRecObjs.push_back(o);
                    }
                }
            }
        }
        return toRecObjs;
    }

    void updateDetectedObjects(const vector<T> &objs) {
        for (auto &o : objs) {
            //如果没有找到，则添加到已存在目标容器中
            if (existObjs_.find(o.guid) == existObjs_.end()) {
                ObjectVO newObj;
                newObj.obj1 = o;
                existObjs_[o.guid] = newObj;
                if (nullptr != objAppearHandler_) {
                    objAppearHandler_(o.guid, (void*)&o);	//callback
                }
            } else {
                existObjs_[o.guid].obj1 = o;
                //重置计数
                existObjs_[o.guid].ResetCounter();
            }
        }

        vector<string> disappearedObjs;
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
                o.second.cnt = o.second.cnt - 1;
                if (o.second.cnt == 0) {
                    disappearedObjs.push_back(o.first);
                }
            }
        }

        //删掉消失的目标
        for (auto &o : disappearedObjs) {
            if (nullptr != objDisappearHandler_) {
                objDisappearHandler_(o);	//callback
            }
            existObjs_.erase(o);
        }

        //更新gofsize，gofidx清0
        gofSize_ = gofIdx_;
        gofIdx_ = 0;
    }

    void updateRecognizedObjects(const vector<T> &objs) {
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
        typedef uint32_t DisappearCounter;

        T obj1;		//obj1 存储检测结果，检测结果不带属性信息
        T obj2;		//obj2 存储识别结果，识别结果带属性信息
        DisappearCounter cnt;

        ObjectVO() {
            cnt = GlobalSettings::getInstance().objectDisappearCount;
        }

        void ResetCounter() {
            cnt = GlobalSettings::getInstance().objectDisappearCount;
        }

        bool Showable() {
            return cnt >= GlobalSettings::getInstance().objectDisappearCount;
        }
    };

    mutex mutex_;
    //已经存在的目标<id, obj>
    map<string, ObjectVO> existObjs_;
    ObjectAppearHandler objAppearHandler_;
    ObjectDisappearHandler objDisappearHandler_;
    //因为是抽帧检测，所以，把相邻两次检测之间的帧认作一个gof（group of frame），gofSize表示一个gof中的帧数目，gofidx表示一帧在当前gof中的序号
    uint32_t gofSize_;
    uint32_t gofIdx_;
    //time for pick frame
    chrono::steady_clock::time_point lastTime_;
    chrono::steady_clock::time_point currTime_;
};

typedef ObjectSink<algo::BikeObject> BikeObjectSink;
typedef ObjectSink<algo::VehicleObject> VehicleObjectSink;
typedef ObjectSink<algo::PersonObject> PersonObjectSink;
typedef ObjectSink<algo::FaceObject> FaceObjectSink;

}