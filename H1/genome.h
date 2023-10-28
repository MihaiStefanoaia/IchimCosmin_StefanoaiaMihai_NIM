#ifndef H1_GENOME_H
#define H1_GENOME_H

#include "fpm/fixed.hpp"
#include "fpm/math.hpp"

using fixedpt = fpm::fixed<int64_t ,__int128_t ,40>;

enum HillclimbStrategies {
    NONE,
    BEST_IMPROVEMENT,
    FIRST_IMPROVEMENT
};

class genome{
public:
    fixedpt* chromosomes = nullptr; // using raw pointer as it allows for some alignment, and the size is managed by the engine
    explicit genome(uint32_t size = 0, double_t lowerBound = 0, double_t upperBound = 0){
        if(size == 0){
            chromosomes = nullptr;
            return;
        }
        chromosomes = new fixedpt[size];
        for(int i = 0; i < size; i++){
            chromosomes[i] = fixedpt(lowerBound + (double_t)std::rand()/(double_t)RAND_MAX * (upperBound - lowerBound));
        }
    }
    ~genome(){
        if(chromosomes != nullptr)
            return;
        delete[] chromosomes;
    }
    void mutate(uint32_t dimensions, double_t chance, double_t lowerBound = 0, double_t upperBound = 0){
        if (chromosomes == nullptr)
            return;
        for (auto i = 0; i < dimensions; i++){
            for (int j = 0 ; j < 64; j++) {
                auto r = std::rand();
                if ((double_t) r / (double_t) RAND_MAX > chance)
                    continue;
                auto &modify = *(uint64_t *) &chromosomes[i];
                modify ^= 1ul << j;
            }
            chromosomes[i] = chromosomes[i] > fixedpt(lowerBound) ? chromosomes[i] : fixedpt(lowerBound);
            chromosomes[i] = chromosomes[i] < fixedpt(upperBound) ? chromosomes[i] : fixedpt(upperBound);
        }
    }
    void get_genes(const genome& other, uint32_t dimensions){
        for (auto i = 0; i < dimensions; i++){
            *(uint64_t*)(&this->chromosomes[i]) = *(uint64_t*)(&other.chromosomes[i]);
        }
    }
    void hillclimb(uint32_t dimensions, const std::function<fixedpt(fixedpt*, uint32_t)>& optimizeFunction, const std::function<double_t (fixedpt, uint32_t)>& fitnessFunction, HillclimbStrategies strategy = BEST_IMPROVEMENT, double_t lowerBound = 0, double_t upperBound = 0){
        if(strategy == HillclimbStrategies::NONE)
            return;
        std::pair<int32_t ,double_t> change = {-1,fitnessFunction(optimizeFunction(this->chromosomes,dimensions),dimensions)};
        for(int i = 0; i < dimensions; i++) {
            for (int j = 0 ; j < 64; j++) {
                auto& modify = *(uint64_t *) &chromosomes[i];
                modify ^= 1ul << j;
                if(chromosomes[i] > fixedpt(upperBound) || chromosomes[i] < fixedpt(lowerBound)){
                    modify ^= 1ul << j;
                    continue;
                }
                auto fitness = fitnessFunction(optimizeFunction(this->chromosomes,dimensions),dimensions);
                if(fitness > change.second){
                    change = {i * 64 + j, fitness};
                    if(strategy == HillclimbStrategies::FIRST_IMPROVEMENT)
                        return;
                }
                modify ^= 1ul << j;
            }
        }
        if(change.first == -1)
            return;
        *(uint64_t *) &chromosomes[change.first / 64] ^= 1ul << change.first % 64;
    }
    genome(genome &t, uint32_t size = 0){
        chromosomes = new fixedpt[size];
        for(int i = 0; i < size; i++){
            chromosomes[i] = t.chromosomes[i];
        }
    }

    void crossover(genome& other, uint32_t dimensions, uint32_t cuts = 1, double_t lowerBound = 0, double_t upperBound = 0, double_t crossoverRate = 1){
        if(cuts == 0 && (double)std::rand()/(double)RAND_MAX > crossoverRate)
            return;
        for(int i = 0; i < dimensions; i++){
            uint64_t mask = 0;
            if(cuts == -1){
                mask = std::rand();
            } else {
                for(int c = 0; c < cuts; c++){
                    mask ^= (1 << (std::rand() % 64)) - 1;
                }
            }
            auto& modify = *(uint64_t *) &chromosomes[i];
            auto& modify_o = *(uint64_t *) &other.chromosomes[i];
            uint64_t buf = modify;
            modify   = (modify &  mask) | (modify_o & ~mask);
            modify_o = (buf    & ~mask) | (modify_o &  mask);
            chromosomes[i] = chromosomes[i] > fixedpt(lowerBound) ? chromosomes[i] : fixedpt(lowerBound);
            chromosomes[i] = chromosomes[i] < fixedpt(upperBound) ? chromosomes[i] : fixedpt(upperBound);
            other.chromosomes[i] = other.chromosomes[i] > fixedpt(lowerBound) ? other.chromosomes[i] : fixedpt(lowerBound);
            other.chromosomes[i] = other.chromosomes[i] < fixedpt(upperBound) ? other.chromosomes[i] : fixedpt(upperBound);
        }
    }

    void swap(genome& other) {
        std::swap(this->chromosomes, other.chromosomes);
    }
};

#endif //H1_GENOME_H
