#include "Buffer.h"

Buffer::Buffer(uint8_t size)
{    
    if (size > 0 && size <= 16){
        this->_size = size;
        this->_buffer = (uint8_t*)malloc(sizeof(uint8_t) * (this->_size + 1));
        this->_len = 0;
        this->_ptr = this->_buffer;
        *(this->_buffer) = 0;
        this->_ttl = 0;
    }
}

bool Buffer::add(uint8_t c){
    
    if(this->_len < this->_size){
        *(this->_buffer) = c;
        this->_buffer++;
        *(this->_buffer) = 0;
        this->_len++;
        this->_ttl = millis()/1000;
        return true;
    }
    return false;
}

void Buffer::clear(){
        this->_len = 0;
        this->_buffer = this->_ptr;
        *(this->_buffer) = 0;
}

u_int8_t* Buffer::toArrayChar(){ 
    
    return this->_ptr;
}

uint8_t Buffer::len(){
    return this->_len;
}

uint32_t Buffer::ttl(){
    return this->_ttl;
}