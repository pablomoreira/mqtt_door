#ifndef BUFFER_H
#define BUFFER_H

#include "Arduino.h"

class Buffer
{
  public:
    Buffer(uint8_t);
    bool add(uint8_t);
    void clear();
    uint8_t len();
    uint8_t* toArrayChar();
    uint32_t ttl();
  private:
    uint8_t* _buffer;
    uint8_t* _ptr;
    uint8_t _len;
    uint8_t _size;
    uint32_t _ttl;
};

#endif