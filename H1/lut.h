#ifndef LUT_H
#define LUT_H

#include "fpm/fixed.hpp"
#include "fpm/math.hpp"
#include <sys/types.h>
#include "lut_arr.h"

fixedpt fractionary_part(fixedpt x){
    auto& xi = *(uint64_t*)(&x);
    xi &= 0x000000FFFFFFFFFF;
    return x;
}

fixedpt interpolate_amt(fixedpt x){
    auto& xi = *(uint64_t*)(&x);
    xi <<= 11;
    return fractionary_part(x);
}

fixedpt interpolate(fixedpt x, fixedpt low, fixedpt high){
    return low + (high - low) * interpolate_amt(x);
}

std::tuple<uint64_t, bool> hash(fixedpt x){
    auto step = fixedpt (8l);
    step /= fixedpt(16384l);
    return std::tuple<uint64_t, bool>(static_cast<uint64_t>(x/step), fractionary_part(x / step) == fixedpt(0l));
}

fixedpt cos_lut(fixedpt x){
//    if (x > fixedpt::two_pi() || x < fixedpt(0)){
//        x = x - fixedpt::two_pi() * (static_cast<int>(x / fixedpt::two_pi()));
//        if (x < fixedpt(0)){
//            x += fixedpt::two_pi();
//        }
//    }
    x = fpm::abs(x);
    x /= fixedpt::two_pi();
    x = fractionary_part(x);
    x *= fixedpt::two_pi();

    auto [idx, exact] = hash(x);
    if(exact)
        return *(fixedpt*)&cos_values[idx];
    auto low = *(fixedpt*)&cos_values[idx];
    auto high = *(fixedpt*)&cos_values[(idx + 1) & 16383];
    return interpolate(x, low, high);
}

fixedpt sin_lut(fixedpt x){
    return cos_lut(x - fixedpt::half_pi());
}

#endif // LUT_H
