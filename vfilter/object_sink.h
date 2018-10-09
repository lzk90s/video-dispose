#pragma once

#include <string>
#include <map>
#include <mutex>
#include <vector>

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

    //����Ŀ��
    void SetDetectedObjects(vector<T> &objs) {
        unique_lock<mutex> lck(mutex_);

        for (auto &o : objs) {
            //���û���ҵ�������ӵ��Ѵ���Ŀ��������
            if (existObjs_.find(o.guid) == existObjs_.end()) {
                ObjectWithCounter newObj;
                newObj.obj1 = o;
                existObjs_[o.guid] = newObj;
                //callback
                if (nullptr != objAppearHandler_) {
                    objAppearHandler_(o.guid, (void*)&o);
                }
            } else {
                existObjs_[o.guid].obj1 = o;
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
                * �����µ�Ŀ���в����ڣ����������
                * 1. Ŀ���Ѿ���ȫ��ʧ
                * 2. Ŀ��ֻ���ڸպ���һ֡��û�м�⵽����������Ŀ���Ѿ���ʧ��
                * ��������������ͨ������Ŀ��ļ�������Ŀ�����Ϊ0��ʱ�򣬱�ʾĿ�������ʧ�ˡ���ɾ����
                */
                o.second.cnt = o.second.cnt - 1;
                if (o.second.cnt == 0) {
                    disappearedObjs.push_back(o.first);
                }
            }
        }

        //ɾ����ʧ��Ŀ��
        for (auto &o : disappearedObjs) {
            if (nullptr != objDisappearHandler_) {
                objDisappearHandler_(o);
            }
            existObjs_.erase(o);
        }
    }

    void SetRecognizedObjects(vector<T> &objs) {
        unique_lock<mutex> lck(mutex_);
        for (auto &o : objs) {
            if (existObjs_.find(o.guid) != existObjs_.end()) {
                existObjs_[o.guid].obj2 = o;
            }
        }
    }

    void GetLatestObjects(vector<T> &t1, vector<T> &t2) {
        unique_lock<mutex> lck(mutex_);
        for (auto &o : existObjs_) {
            t1.push_back(o.second.obj1);
            t2.push_back(o.second.obj2);
        }
    }

    bool ObjectExist(const string &objId) {
        unique_lock<mutex> lck(mutex_);
        return existObjs_.find(objId) != existObjs_.end();
    }

private:
    class ObjectWithCounter {
        typedef int32_t OjbectDisappearCounter;

    public:
        T obj1;		//obj1 �洢�����
        T obj2;		//obj2 �洢ʶ����
        OjbectDisappearCounter cnt;

        ObjectWithCounter() {
            cnt = GlobalSettings::getInstance().objectDisappearCount;
        }
    };

    mutex mutex_;
    //�Ѿ����ڵ�Ŀ��<id, obj>
    map<string, ObjectWithCounter> existObjs_;
    ObjectAppearHandler objAppearHandler_;
    ObjectDisappearHandler objDisappearHandler_;
};

typedef ObjectSink<algo::BikeObject> BikeObjectSink;
typedef ObjectSink<algo::VehicleObject> VehicleObjectSink;
typedef ObjectSink<algo::PersonObject> PersonObjectSink;
typedef ObjectSink<algo::FaceObject> FaceObjectSink;

}