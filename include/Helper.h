#ifndef HELPER_H
#define HELPER_H

#include <Arduino.h>

#define CLAMP(_V, _MIN, _MAX) (_V < _MIN ? _MIN : _V > _MAX ? _MAX : _V)

class RolingAcc {
public:
    RolingAcc() : size_(10) { reset(); }
    void reset();
    float avg(float v);

protected:
    float buffer_[10];
    float sum_;
    size_t size_;
    size_t fill_;
    size_t n_;
};


#endif // HELPER_H