#pragma once

#include "opencv/cv.h"

#include "common/helper/logger.h"

#include "vfilter/config/setting.h"
#include "vfilter/notifier/httplib.h"


namespace video {
namespace filter {

template<class T>
class Notifier {
public:
    Notifier(const std::string &type) {
        this->type_ = type;

        std::string host = G_CFG().notifyServerHost;
        if (host.empty()) {
            throw std::runtime_error("The notify url is empty");
        }
        std::string addr, port;
        addr = host.substr(0, host.find_last_of(":"));
        port = host.substr(host.find_last_of(":") + 1, host.length() - host.find_last_of(":"));
        if (addr.empty() || port.empty()) {
            throw std::runtime_error("The notify address is invalid");
        }

        cli_.reset(new httplib::Client(addr.c_str(), atoi(port.c_str())));
    }

    void OnRecognizedObject(uint32_t channelId, cv::Mat &frame, T &obj) {
        if (obj.detect.empty() || obj.detect.size() != 4) {
            return;
        }

        int32_t x = obj.detect[0], y = obj.detect[1], w = obj.detect[2], h = obj.detect[3];
        if (x < 0 || y < 0 || w <= 0 || h <= 0) {
            LOG_WARN("Invalid object with x={}, y={}, w={}, h={}", x, y, w, h);
            return;
        }

        //过滤掉无效的图片
        if (isInvalidPicture(w, h)) {
            LOG_DEBUG("The {} picture is too small, ignore it, width {}, height{}", type_, w, h);
            return;
        }

        //抠图
        cv::Rect  rect = cv::Rect(x, y, w, h);
        cv::Mat roi = frame(rect);
        cv::Mat img = roi.clone();

        //生成通知消息，并通过http发送通知消息
        std::string msg = buildNotifyMsg(channelId, img, obj);
        if (!msg.empty()) {
            cli_->Post(getRequestURL().c_str(), msg, "application/json");
            LOG_DEBUG("Send {} notify msg {}", type_, msg);
        }
    }

protected:
    //生成通知消息
    virtual std::string buildNotifyMsg(uint32_t channelId, cv::Mat &img, T &obj) = 0;

    //获取http请求url
    virtual std::string getRequestURL() = 0;

    //判断是否是无效的图片
    virtual bool isInvalidPicture(uint32_t width, uint32_t height) = 0;

private:
    std::string type_;
    std::unique_ptr<httplib::Client> cli_;
};


}
}