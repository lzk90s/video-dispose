#pragma  once

#include <cstdint>
#include <map>
#include <memory>

using std::string;
using std::map;
using std::shared_ptr;

namespace vf {

class Rect {
public:
    int32_t x, y, w, h;

    Rect(const Rect &rhs) {
        this->x = rhs.x;
        this->y = rhs.y;
        this->w = rhs.w;
        this->h = rhs.h;
    }

    Rect() {
        x = y = w = h = -1;
    }

    void Reset() {
        x = y = w = h = -1;
    }
};

class Attribute {
public:
    string key;
    string value;
    int32_t score;
    bool visable;	//是否在流中可见（流叠加）

    Attribute() {
        visable = true;
        score = 0;
    }

    Attribute(const Attribute &rhs) {
        this->key = rhs.key;
        this->value = rhs.value;
        this->score = rhs.score;
    }
};

class Target {
public:
    Target(const string &id) {
        this->id = id;
    }

    Target &rect(const Rect &rect) {
        this->rect = rect;
        return *this;
    }

    Target &attrs(vector<Attribute> &attrs) {
        this->attrs = attrs;
        return *this;
    }

private:
    string id;
    Rect rect;
    vector<Attribute> attrs;
};

/* id, target*/
typedef map<string, shared_ptr<Target>> TargetMap;

}

