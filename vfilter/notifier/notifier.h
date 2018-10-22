#pragma once

#include "opencv/cv.h"

#include "algo/stub/object_type.h"
#include "common/helper/logger.h"

#include "vfilter/setting.h"
#include "vfilter/notifier/httplib.h"
#include "vfilter/setting.h"


namespace vf {

template<class T>
class Notifier {
public:
    Notifier(const string &type) {
        this->type_ = type;

        string host = GlobalSettings::getInstance().notifyServerHost;
        if (host.empty()) {
            throw runtime_error("The notify url is empty");
        }
        string addr, port;
        addr = host.substr(0, host.find_last_of(":"));
        port = host.substr(host.find_last_of(":") + 1, host.length() - host.find_last_of(":"));
        if (addr.empty() || port.empty()) {
            throw runtime_error("The notify address is invalid");
        }

        cli_.reset(new httplib::Client(addr.c_str(), atoi(port.c_str())));
    }

    void OnRecognizedObject(uint32_t channelId, cv::Mat &frame, T &obj) {
        if (obj.detect.empty() || obj.detect.size() != 4) {
            return;
        }

        int32_t x = obj.detect[0], y = obj.detect[1], w = obj.detect[2], h = obj.detect[3];

        //过滤掉无效的图片
        if (isInvalidPicture(w, h)) {
            LOG_DEBUG("The picture is too small, ignore it, width {}, height{}", w, h);
            return;
        }

        //抠图
        cv::Rect  rect = cv::Rect(x, y, w, h);
        cv::Mat roi = frame(rect);
        cv::Mat img = roi.clone();

        //生成通知消息，并通过http发送通知消息
        string msg = buildNotifyMsg(channelId, img, obj);
        if (!msg.empty()) {
            cli_->Post(getRequestURL().c_str(), msg, "application/json");
            LOG_DEBUG("Send {} notify msg {}", type_, msg);
        }
    }

protected:
    //生成通知消息
    virtual string buildNotifyMsg(uint32_t channelId, cv::Mat &img, T &obj) = 0;

    //获取http请求url
    virtual string getRequestURL() = 0;

    //判断是否是无效的图片
    virtual bool isInvalidPicture(uint32_t width, uint32_t height) = 0;

private:
    string type_;
    unique_ptr<httplib::Client> cli_;
};


}