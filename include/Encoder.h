#ifndef _ENCODER_H
#define _ENCODER_H

#include <arduino.h>

class Encoder {
public:
    Encoder() : counter(0), _oldAB(0), _state(0) {}
    int32_t update(uint8_t inputAB);

    int32_t counter;      // Encoder value  
private:
    uint8_t _oldAB;     // Lookup table index
    int8_t _state;
};

#endif // _ENCODER_H
