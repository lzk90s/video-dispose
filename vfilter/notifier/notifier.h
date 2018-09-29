#pragma once

#include "opencv/cv.h"

#include "algo/stub/object_type.h"
#include "vfilter/setting.h"
#include "vfilter/notifier/http_client.h"


namespace vf {

template<class T>
class Notifier {
public:
    Notifier(const string &type, const string &url) {
        this->type = type;
        this->url = url;
        if (this->url.empty()) {
            throw runtime_error("The notify url is empty");
        }
    }

    void OnRecognizedObject(cv::Mat &frame, T &obj) {
        if (obj.detect.empty() || obj.detect.size() != 4) {
            return;
        }

        int32_t x = obj.detect[0], y = obj.detect[1], w = obj.detect[2], h = obj.detect[3];
        //抠图
        cv::Rect  rect = cv::Rect(x, y, w, h);
        cv::Mat roi = frame(rect);
        cv::Mat img = roi.clone();

        //生成通知消息，并通过http发送通知消息
        string msg = buildNotifyMsg(img, obj);
        HttpClient::SendReq(url, [](const string &msg) {});
    }

protected:
    virtual string buildNotifyMsg(cv::Mat &img, T &obj) {
        return "";
    }

private:
    string type;
    string url;
};


}