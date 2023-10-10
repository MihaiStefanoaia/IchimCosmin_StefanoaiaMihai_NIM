//
// Created by mihai-pc on 10/10/23.
//

#ifndef H1_ENGINE_H
#define H1_ENGINE_H
#include "funclib.h"

class engine {
public:
    static fixedpt** population;
    static fixedpt** pop_buffer;
    static uint32_t  pop_size;
    static uint32_t  dimensions;
    static float_t   selection_pressure;
    static std::function<fixedpt(fixedpt*, int)> func;
    static uint32_t  generations;
};


#endif //H1_ENGINE_H
