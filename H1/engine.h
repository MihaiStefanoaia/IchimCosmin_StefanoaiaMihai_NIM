//
// Created by mihai-pc on 10/10/23.
//

#ifndef H1_ENGINE_H
#define H1_ENGINE_H
#include "funclib.h"

class genome{
public:
    fixedpt* chromosomes; // using raw pointer as it allows for some alignment, and the size is managed by the engine
    explicit genome(int size){
        chromosomes = new (std::align_val_t(64)) fixedpt[size];
    }
};

class engine {
public:
    static genome*  population;   // the main population on which the work is being done
    static genome*  pop_buffer;   // buffer for the new generations
    static uint32_t pop_size;     // amount of entities
    static uint32_t dimensions;   // dimensions to optimize - also number of chromosomes
    static uint32_t generations;  // max amount of times to run the algorithm
    static float_t  threshold;    // the stopping point
    static float_t  selection_pressure;
    static std::function<fixedpt(fixedpt*, uint32_t)> optimize; // function to optimize for
    static std::function<float_t(fixedpt)> fitness;             // fitness function
};


#endif //H1_ENGINE_H
