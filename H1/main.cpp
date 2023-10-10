#include <iostream>

#include "fpm/ios.hpp"
#include "engine.h"
#include <ctime>

int main() {
//    std::cout << sizeof(fixedpt) << '\n';
//    for(float x = -1; x < 1; x += 0.0625){
//        for(float y = -1; y < 1; y += 0.0625) {
//            fixedpt nr[] = {fixedpt(x),fixedpt(y)};
//            std::cout << y << " : " << rastrigin(nr, 2) << '\n';
//        }
//    }
    std::srand(time(nullptr));
    auto eng = engine();
    eng.dimensions = 3;
    eng.populationSize = 1000;
    eng.generations = 100000;
    eng.lowerBound = -5.12;
    eng.upperBound =  5.12;
    eng.optimize = rastrigin;
    eng.fitness  = rastrigin_fitness;
    eng.selectionPressure = 0;
    eng.mutationRate = 0.00625;
    eng.threshold = INFINITY;
    eng.printFrequency = 512;

    eng.run();
    return 0;
}
