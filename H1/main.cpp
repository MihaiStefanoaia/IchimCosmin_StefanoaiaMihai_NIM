#include <iostream>

#include "funclib.h"
#include "fpm/ios.hpp"

int main() {
    std::cout << sizeof(fixedpt) << '\n';
    for(float f = -1; f < 1; f += 0.0625){
        auto nr = fixedpt(f);
        std::cout << f << " : " << rastrigin(&nr,1) << '\n';
    }
    return 0;
}
