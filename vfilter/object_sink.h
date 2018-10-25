#pragma once

#include <string>
#include <map>
#include <mutex>
#include <vector>

#include "common/helper/logger.h"
#include "algo/stub/object_type.h"
#include "vfilter/setting.h"


namespace vf {

//目标池
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
    vector<T>  CalcNeedRecognizeObjects(vector<T> &objs) {
        unique_lock<mutex> lck(mutex_);
        vector<T> recognizableObjs;
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
        return recognizableObjs;
    }

    //更新检测到的目标
    void UpdateDetectedObjects(vector<T> &objs) {
        unique_lock<mutex> lck(mutex_);

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
    }

    //更新识别到的目标
    void UpdateRecognizedObjects(vector<T> &objs) {
        unique_lock<mutex> lck(mutex_);
        for (auto &o : objs) {
            if (existObjs_.find(o.guid) != existObjs_.end()) {
                auto &lastObj = existObjs_[o.guid].obj2;
                //对比属性，根据属性的评分判断是否更新目标的属性
                for (auto &p : o.attrs) {
                    uint32_t attrKey = p.first;
                    if (lastObj.attrs.find(attrKey) != lastObj.attrs.end()) {
                        uint32_t currScore = p.second.score;
                        uint32_t lastScore = lastObj.attrs[attrKey].score;
                        if (currScore < lastScore) {
                            p.second = lastObj.attrs[attrKey];
                        }
                    }
                }
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
};

typedef ObjectSink<algo::BikeObject> BikeObjectSink;
typedef ObjectSink<algo::VehicleObject> VehicleObjectSink;
typedef ObjectSink<algo::PersonObject> PersonObjectSink;
typedef ObjectSink<algo::FaceObject> FaceObjectSink;

}