#pragma once

#include <string>
#include <map>
#include <mutex>
#include <vector>

#include "common/helper/logger.h"
#include "algo/stub/object_type.h"
#include "vfilter/setting.h"


namespace vf {

//Ŀ���
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

    //������Ҫʶ���Ŀ��
    vector<T>  CalcNeedRecognizeObjects(vector<T> &objs) {
        unique_lock<mutex> lck(mutex_);
        vector<T> recognizableObjs;
        for (auto &o : objs) {
            // 1. Ŀ���һ�γ���
            // 2. Ŀ�����µ����ֱ���һ�θ�
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

    //���¼�⵽��Ŀ��
    void UpdateDetectedObjects(vector<T> &objs) {
        unique_lock<mutex> lck(mutex_);

        for (auto &o : objs) {
            //���û���ҵ�������ӵ��Ѵ���Ŀ��������
            if (existObjs_.find(o.guid) == existObjs_.end()) {
                ObjectVO newObj;
                newObj.obj1 = o;
                existObjs_[o.guid] = newObj;
                if (nullptr != objAppearHandler_) {
                    objAppearHandler_(o.guid, (void*)&o);	//callback
                }
            } else {
                existObjs_[o.guid].obj1 = o;
                //���ü���
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
                objDisappearHandler_(o);	//callback
            }
            existObjs_.erase(o);
        }
    }

    //����ʶ�𵽵�Ŀ��
    void UpdateRecognizedObjects(vector<T> &objs) {
        unique_lock<mutex> lck(mutex_);
        for (auto &o : objs) {
            if (existObjs_.find(o.guid) != existObjs_.end()) {
                auto &lastObj = existObjs_[o.guid].obj2;
                //�Ա����ԣ��������Ե������ж��Ƿ����Ŀ�������
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

        T obj1;		//obj1 �洢����������������������Ϣ
        T obj2;		//obj2 �洢ʶ������ʶ������������Ϣ
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
    //�Ѿ����ڵ�Ŀ��<id, obj>
    map<string, ObjectVO> existObjs_;
    ObjectAppearHandler objAppearHandler_;
    ObjectDisappearHandler objDisappearHandler_;
};

typedef ObjectSink<algo::BikeObject> BikeObjectSink;
typedef ObjectSink<algo::VehicleObject> VehicleObjectSink;
typedef ObjectSink<algo::PersonObject> PersonObjectSink;
typedef ObjectSink<algo::FaceObject> FaceObjectSink;

}