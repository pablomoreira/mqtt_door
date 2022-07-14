#include "Arduino.h"
#include "Digital.h"

Signal::Signal(uint8 pin,bool state)
{
  pinMode(pin, OUTPUT);
  _pin = pin;
  _state = state;
  _grade = 0;
  digitalWrite(_pin,state);
}
void Signal::high()
{
    _state = HIGH;
    digitalWrite(_pin,HIGH);
}
void Signal::low()
{
    _state = LOW;
    digitalWrite(_pin,LOW);
}
void Signal::change(){
    _state = ! _state;
    digitalWrite(_pin,_state);
    //Serial.printf("led to %d\n", _state);
}
void Signal::setblink(uint8 num){
    if(_num/2 != num){
        _num = num * 2;
        //Serial.printf("led to state %d blink %d \n", _num,num);
        _index = 0;
        low();
    }
}
void Signal::togle(){
    if(_index < _num){
       change();
    }
   _index++;
   if(_index >= 49){
    _index = 0;
    this->low();
   }
}

void Signal::upGrade(uint8 grade){
    if(_grade < grade){
        setblink(grade);
        _grade = grade;
    }
}
void Signal::downGrade(uint8 grade){
    if(_grade > grade){
        setblink(grade);
        _grade = grade;
    }
}