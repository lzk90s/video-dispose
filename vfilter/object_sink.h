#pragma once

#include <string>
#include <map>
#include <mutex>
#include <vector>

#include "common/helper/logger.h"
#include "algo/stub/object_type.h"
#include "vfilter/setting.h"


namespace vf {

template<class T>
class ObjectSink {
public:
    using ObjectAppearHandler = function<void(const string&, void *)> ;
    using ObjectDisappearHandler = function<void(const string&)>;

public:

    void SetObjectAppearHandler(ObjectAppearHandler h) {
        this->objAppearHandler_ = h;
    }

    void SetObjectDisappearHandler(ObjectDisappearHandler h) {
        this->objDisappearHandler_ = h;
    }

    //计算需要识别的目标
    void CalcNeedRecognizeObjects(vector<T> &objs, vector<T> &recognizableObjs) {
        unique_lock<mutex> lck(mutex_);

        for (auto &o : objs) {
            // 1. 目标第一次出现
            // 2. 目标最新的评分比上一次高
            if (existObjs_.find(o.guid) == existObjs_.end()) {
                recognizableObjs.push_back(o);
            } else {
                uint32_t currScore = o.score;
                uint32_t lastScore = existObjs_[o.guid].obj1.score;
                if ((currScore > lastScore) &&
                        (currScore - lastScore > GlobalSettings::getInstance().scoreDiff4ReRecognize)) {
                    recognizableObjs.push_back(o);
                }
            }
        }
    }

    //设置目标
    void UpdateDetectedObjects(vector<T> &objs) {
        unique_lock<mutex> lck(mutex_);

        for (auto &o : objs) {
            //如果没有找到，则添加到已存在目标容器中
            if (existObjs_.find(o.guid) == existObjs_.end()) {
                ObjectWithCounter newObj;
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
    }

    void UpdateRecognizedObjects(vector<T> &objs) {
        unique_lock<mutex> lck(mutex_);
        for (auto &o : objs) {
            if (existObjs_.find(o.guid) != existObjs_.end()) {
                existObjs_[o.guid].obj2 = o;
            }
        }
    }

    void GetShowableObjects(vector<T> &t1, vector<T> &t2) {
        unique_lock<mutex> lck(mutex_);
        for (auto &o : existObjs_) {
            if (o.second.Showable()) {
                t1.push_back(o.second.obj1);
                t2.push_back(o.second.obj2);
            }
        }
    }

    bool ObjectExist(const string &objId) {
        unique_lock<mutex> lck(mutex_);
        return existObjs_.find(objId) != existObjs_.end();
    }

private:
    class ObjectWithCounter {
        typedef uint32_t OjbectDisappearCounter;

    public:
        T obj1;		//obj1 存储检测结果
        T obj2;		//obj2 存储识别结果
        OjbectDisappearCounter cnt;

        ObjectWithCounter() {
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
    map<string, ObjectWithCounter> existObjs_;
    ObjectAppearHandler objAppearHandler_;
    ObjectDisappearHandler objDisappearHandler_;
};

typedef ObjectSink<algo::BikeObject> BikeObjectSink;
typedef ObjectSink<algo::VehicleObject> VehicleObjectSink;
typedef ObjectSink<algo::PersonObject> PersonObjectSink;
typedef ObjectSink<algo::FaceObject> FaceObjectSink;

}