#ifndef MY_KEYPADI2C_H
#define MY_KEYPADI2C_H
#include <PCF8574.h>

struct iter_rown
{
    uint8_t port;
    iter_rown* ptr;
};

class MyKeyPadi2c
{

private:
    PCF8574* pcf8574;
    char _keys[16] = {'1','2','3','4','5','6','7','8','9','*','0','#'};
    uint8_t _row;
    char _key;
    iter_rown* _port;
public:
    MyKeyPadi2c(PCF8574* pcf);
    ~MyKeyPadi2c();
    bool begin();
    bool read();
    char getKey();
    bool isConnected();
};

#endif

