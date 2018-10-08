#pragma once

#include "opencv/cv.h"

#include "algo/stub/object_type.h"
#include "vfilter/setting.h"
#include "vfilter/notifier/httplib.h"
#include "common/helper/logger.h"


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
        //��ͼ
        cv::Rect  rect = cv::Rect(x, y, w, h);
        cv::Mat roi = frame(rect);
        cv::Mat img = roi.clone();

        //����֪ͨ��Ϣ����ͨ��http����֪ͨ��Ϣ
        string msg = buildNotifyMsg(channelId, img, obj);
        if (!msg.empty()) {
            cli_->Post(getRequestURL().c_str(), msg, "application/json");
            LOG_DEBUG("Send {} notify msg {}", type_, msg);
        }
    }

protected:
    virtual string buildNotifyMsg(uint32_t channelId, cv::Mat &img, T &obj) {
        return "";
    }

    virtual string getRequestURL() {
        return "";
    }

private:
    string type_;
    unique_ptr<httplib::Client> cli_;
};


}