#pragma once

#ifndef H1_FUNCLIB_H
#define H1_FUNCLIB_H

#include "fpm/fixed.hpp"
#include "fpm/math.hpp"
#include "engine.h"
#include <sys/types.h>

using fixedpt = fpm::fixed<int64_t ,__int128_t ,24>;

fixedpt rastrigin(fixedpt* args, uint32_t n){
    auto ret = fixedpt (10l * n);
    for(int i = 0; i < n; i++){
        const auto& x = args[i];
        ret += x * x - 10l * fpm::cos(x * fixedpt::two_pi());
    }
    return ret;
}
double_t rastrigin_fitness(fixedpt f_x,uint32_t dimensions){
    if(f_x == fixedpt(0))
        return INFINITY;
    return 1.0/std::pow((double_t)f_x, dimensions);
}

fixedpt griewangk(fixedpt* args, uint32_t n){
    auto ret = fixedpt(1);
    auto prod = fixedpt(1);
    for(int i = 0; i < n; i++){
        const auto& x = args[i];
        ret  += (x / fixedpt(200)) * (x / fixedpt(20));
    }
    for(int i = 0; i < n; i++){
        prod *= fpm::cos(args[i] / fpm::sqrt(fixedpt(i+1)));
    }
    return ret - prod;
}

double_t griewangk_fitness(fixedpt f_x, uint32_t dimensions){
    if(f_x == fixedpt(0))
        return INFINITY;
    return 1.0/std::pow((double_t)f_x,dimensions);
}

fixedpt rosenbrock(fixedpt* args, uint32_t n){
    auto ret = fixedpt(0);
    for(auto i = 0; i < n - 1; i++){
        ret += 100 * fpm::pow(args[i+1] - args[i] * args[i],2) + fpm::pow(1 - args[i],2);
    }
    return ret;
}

double_t rosenbrock_fitness(fixedpt f_x,uint32_t dimensions){
    if(f_x == fixedpt(0))
        return INFINITY;
    return 1.0/std::pow((double_t)f_x,dimensions);
}

fixedpt michalewicz(fixedpt* args, uint32_t n){
    auto m = fixedpt(1);
    auto ret = fixedpt(0);
    for(auto i = 0; i < n; i++) {
        auto const& x = args[i];
        ret += fpm::sin(x) * fpm::pow(fpm::sin(fixedpt(i+1) * x * x / fixedpt::pi()),fixedpt (2 * m));
    }
    return fixedpt(-1) * ret;
}

double_t michalewicz_fitness(fixedpt f_x, uint32_t dimensions){
    return std::abs((double_t)f_x);
}

uint32_t  partition(double_t * arr, fixedpt* additionalArray, uint32_t  low, uint32_t  high){
    double_t pivot = arr[high];
    uint32_t  i = (low - 1);
    for(uint32_t  j = low;j <= high; j++){
        if(arr[j] < pivot){
            i++;
            std::swap(arr[i],arr[j]);
            std::swap(additionalArray[i],additionalArray[j]);
        }
    }
    std::swap(arr[i+1],arr[high]);
    std::swap(additionalArray[i+1],additionalArray[high]);
    return (i + 1);
}

void quickSort(double_t * arr, fixedpt* additionalArray, uint32_t  low, uint32_t high){
    if(low < high){
        uint32_t  pi = partition(arr, additionalArray, low, high);
        quickSort(arr, additionalArray, low, pi-1);
        quickSort(arr, additionalArray, pi+1, high);
    }
}

#endif //H1_FUNCLIB_H
