//
// Created by mihai-pc on 10/8/23.
//

#ifndef H1_FUNCLIB_H
#define H1_FUNCLIB_H

#include "fpm/fixed.hpp"
#include "fpm/math.hpp"
#include <vector>
#include "sys/types.h"

using fixedpt = fpm::fixed<int64_t,__int128_t,24>;

fixedpt rastrigin(const std::vector<fixedpt>& args){
    auto ret = fixedpt (10l * args.size());
    for(const auto& x : args){
        ret += x * x - 10l * fpm::cos(x * fixedpt::two_pi());
    }
    return ret;
}

fixedpt griewangk(const std::vector<fixedpt>& args){
    auto ret = fixedpt(1);
    auto prod = fixedpt(1);
    for(const auto& x : args){
        ret  += x * x / fixedpt(4000);
    }
    for(auto i = 0; i < args.size(); i++){
        prod *= fpm::cos(args[i] / fpm::sqrt(fixedpt(i)));
    }
    return ret + prod;
}

fixedpt rosenbrock(const std::vector<fixedpt>& args){
    auto ret = fixedpt(0);
    for(auto i = 0; i < args.size() - 1; i++){
        ret += 100 * fpm::pow(args[i+1] - args[i] * args[i],2) + fpm::pow(1 - args[i],2);
    }
    return ret;
}

template<float m>
fixedpt michalewicz(const std::vector<fixedpt>& args){
    auto ret = fixedpt(0);
    for(auto i = 0; i < args.size(); i++) {
        auto const& x = args[i];
        ret += fpm::sin(x) * fpm::pow(fpm::sin(fixedpt(i) * x * x / fixedpt::pi()),fixedpt (2 * m));
    }
    return fixedpt(-1) * ret;
}


#endif //H1_FUNCLIB_H
