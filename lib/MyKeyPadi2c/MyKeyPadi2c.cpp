#include "MyKeyPadi2c.h"
#include <PCF8574.h>


MyKeyPadi2c::MyKeyPadi2c(PCF8574* pcf)
{
    this->pcf8574 = pcf;
    this->pcf8574->pinMode(P4, INPUT_PULLUP);
    this->pcf8574->pinMode(P5, INPUT_PULLUP);
    this->pcf8574->pinMode(P6, INPUT_PULLUP);
    
    this->pcf8574->pinMode(P0, OUTPUT,HIGH);
    this->pcf8574->pinMode(P1, OUTPUT,HIGH);
    this->pcf8574->pinMode(P2, OUTPUT,HIGH);
    this->pcf8574->pinMode(P3, OUTPUT,HIGH);
    
    iter_rown* p0 = (iter_rown*) malloc(sizeof(iter_rown));
    iter_rown* p1 = (iter_rown*) malloc(sizeof(iter_rown));
    iter_rown* p2 = (iter_rown*) malloc(sizeof(iter_rown));
    iter_rown* p3 = (iter_rown*) malloc(sizeof(iter_rown));
    
    p0->port = P0;
    p1->port = P1;
    p2->port = P2;
    p3->port = P3;

    this->_port = p0;
    this->_port->ptr = p1;
    this->_port = this->_port->ptr;
    this->_port->ptr = p2;
    this->_port = this->_port->ptr;
    this->_port->ptr = p3;
    this->_port = this->_port->ptr;
    this->_port->ptr = p0;
    this->_port = this->_port->ptr;
}

bool MyKeyPadi2c::begin(){
    if(this->pcf8574->begin()){
        this->_row = 0;
        return true;
    }
    return false; 
}

bool MyKeyPadi2c::read(){
    uint8_t row = 0;
    bool ret = false;
    uint8_t reg456 = 0;
    
    
    this->pcf8574->digitalWrite(this->_port->port,LOW);
    reg456 = ((0b01110000 ^ pcf8574->digitalReadAll()) & 0b01110000) >> 4;
    this->pcf8574->digitalWrite(this->_port->port,HIGH);
    if (reg456 > 0b0){
        switch (reg456)
        {
            case 0b001:
            row = 0;
            break;
            case 0b010:
            row = 1;
            break;
            case 0b100:
            row = 2;
            break;
            default:    
            break;
        } 
        _key = _keys[this->_port->port * 3 + row];            
        ret = true;
    }  
    this->_port = this->_port->ptr;
    return ret;
}

char MyKeyPadi2c::getKey(){
    return _key;
}

MyKeyPadi2c::~MyKeyPadi2c()
{
}

bool MyKeyPadi2c::isConnected(){
    return true;
}