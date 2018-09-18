#pragma  once

#include <cstdint>

class RectArea {
public:
    int32_t x, y, w, h;
};

class Attribute {

};

class DetectTarget {
public:

private:
    RectArea rect;
    Attribute attr;
};
