#ifndef signal_h
#define signal_h

#include "Arduino.h"

class Signal
{
  public:
    Signal(uint8 pin,bool state);
    void high();
    void low();
    void change();
    void upGrade(uint8_t);
    void downGrade(uint8_t);
    void togle();
  private:
    int _pin;
    uint8_t _state;
    uint8 _num;
    uint8 _index;
    void setblink(uint8 num);
    uint8_t _grade;
};

#endif