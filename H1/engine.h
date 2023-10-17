#ifndef H1_ENGINE_H
#define H1_ENGINE_H
#include "funclib.h"
#include <random>
#include <iostream>
#include <tuple>

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

    void crossover(genome& other, uint32_t dimensions, uint32_t cuts = 1, double_t lowerBound = 0, double_t upperBound = 0){
        if(cuts == 0)
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
};

class engine {
public:
    genome* currentGeneration{};              // the main population on which the work is being done
    genome* nextGeneration{};                 // buffer for the new generations
    double_t* fitnessScores{};                // list of the fitness scores
    std::tuple<double_t, int>* fitnessScoresCumSum{};   // list of the cumulative sum of the fitness scores
    int32_t* fitnessRanking{};                // buffer for the fitness rankings
    uint32_t populationSize{};                // amount of entities
    uint32_t dimensions{};                    // dimensions to optimize - also number of chromosomes
    uint32_t generations{};                   // max amount of times to run the algorithm
    double_t lowerBound{};                    // lower bound of the function domain
    double_t upperBound{};                    // upper bound of the function domain
    double_t threshold{};                     // the stopping point
    double_t currentMaximumFitness = -1;      // used for printing
    double_t mutationRate{};                  // chance between 0 and 1 for a bit to mutate
    std::function<fixedpt(fixedpt*, uint32_t)> optimize; // function to optimize for
    std::function<double_t (fixedpt,uint32_t)> fitness;           // fitness function
    genome* winner{};
    genome* tempWinner{};
    uint32_t printFrequency = 256;
    HillclimbStrategies strategy = NONE;
    uint32_t crossoverCuts = 1;
    double_t elitistPercentageInFollowingGeneration = 10;

    void run();

    void setup();
    void run_generation();
    void cleanup() const;
    engine()= default;
};

void engine::run_generation() {
    // generate fitness scores for each member of the generation
    fitnessScores[0] = fitness(optimize(currentGeneration[0].chromosomes,dimensions),dimensions);
    fitnessScoresCumSum[0] = std::tuple<double_t, int>(fitnessScores[0], 0);
    for (auto i = 1; i < populationSize; i++){
        auto fitnessScore = fitness(optimize(currentGeneration[i].chromosomes,dimensions),dimensions);
        fitnessScores[i] = fitnessScore;
        if(fitnessScore >= threshold){
            winner = &currentGeneration[i];
            return;
        }
        fitnessScoresCumSum[i] = std::tuple<double_t, int>(get<0>(fitnessScoresCumSum[i-1]) + fitnessScore, i);
        if (fitnessScore > currentMaximumFitness){
            currentMaximumFitness = fitnessScore;
            tempWinner->get_genes(currentGeneration[i],dimensions);
        }
    }
    // do the elitist selection
    // sorting of fitnessScores
    quickSort(fitnessScores, currentGeneration->chromosomes, 1, populationSize - 1);
    int elitistCount = (elitistPercentageInFollowingGeneration * populationSize) / 100;
    for (auto i = 0; i < elitistCount; i++){
        nextGeneration[i].get_genes(currentGeneration[populationSize - 1 - i], dimensions);
    }
    // after the elitist selection the crossover process should be done (the candidates are to be selected using the roulette strategy)
    // the crossover should be followed by the mutation process
    // run the selection
    double_t randomSelector = (double)std::rand()/(double)RAND_MAX * get<0>(fitnessScoresCumSum[populationSize-1]);
    for(auto newMember = elitistCount; newMember < populationSize; newMember++){
        for(auto i = 0; i < populationSize; i++){
            if (get<0>(fitnessScoresCumSum[i]) >= randomSelector){
                nextGeneration[newMember].get_genes(currentGeneration[get<1>(fitnessScoresCumSum[i])],dimensions);
                if (newMember < populationSize - 1) {
                    nextGeneration[newMember + 1].get_genes(nextGeneration[newMember], dimensions);
                    nextGeneration[newMember + 1].mutate(dimensions, mutationRate, lowerBound, upperBound);
                    newMember++;
                }
                break;
            }
        }
    }
    // crossover
    for(auto i = 0; i < populationSize - populationSize % 2; i+=2){
        nextGeneration[i].crossover(nextGeneration[i+1],dimensions,crossoverCuts, lowerBound, upperBound);
    }
    // hill climb
    for(auto i = 0; i < populationSize; i++){
        nextGeneration[i].hillclimb(dimensions, optimize, fitness, strategy,lowerBound, upperBound);
    }
}

void engine::setup() {
    winner = nullptr;
    tempWinner = new genome(dimensions,lowerBound,upperBound);

    currentGeneration = new genome[populationSize];
    nextGeneration = new genome[populationSize];
    fitnessScores = new double_t[populationSize];
    fitnessScoresCumSum = new std::tuple<double_t, int>[populationSize];

    for(auto i = 0; i < populationSize; i++){
        currentGeneration[i] = genome(dimensions,lowerBound,upperBound);
        nextGeneration[i] = genome(dimensions,lowerBound,upperBound);
    }
}

void engine::run() {
    setup();
    for(auto generation = 0; generation < generations || generations == -1; generation++){
        run_generation();
        if(generation % printFrequency == 0){
            std::cout << "At generation " << generation << "...\n";
//            std::cout << "Best version:\n";
//            std::cout << "x = [";
//            for(int i = 0; i < dimensions - 1; i++){
//                std::cout << tempWinner->chromosomes[i] << ", ";
//            }
//            std::cout << tempWinner->chromosomes[dimensions-1] <<"]\n";
//            std::cout << "function(x) = " << optimize(tempWinner->chromosomes,dimensions) << "\n";

            std::cout << "Best version from this generation:\n";
            double_t delta = fitnessScores[0];
            genome* tempBest = &currentGeneration[0];
            for(int i = 1; i < populationSize; i++){
                if(fitnessScores[i]  > delta){
                    delta = fitnessScores[i];
                    tempBest = &currentGeneration[i];
                }
            }
            std::cout << "x = [";
            for(int i = 0; i < dimensions - 1; i++){
                std::cout << tempBest->chromosomes[i] << ", ";
            }
            std::cout << tempBest->chromosomes[dimensions-1] <<"]\n";
            std::cout << "function(x) = " << optimize(tempBest->chromosomes,dimensions) << "\n";
            std::cout << "fitness (x) = " << fitness(optimize(tempBest->chromosomes,dimensions),dimensions) << "\n";
        }
        if (winner != nullptr){
            std::cout << "Finished after " << generation << " generations:\n";
            std::cout << "x = [";
            for(int i = 0; i < dimensions - 1; i++){
                std::cout << winner->chromosomes[i] << ", ";
            }
            std::cout << winner->chromosomes[dimensions-1] <<"]\n";
            std::cout << "function(x) = " << optimize(winner->chromosomes,dimensions) << "\n";
            break;
        }
        std::swap(currentGeneration, nextGeneration);
    }
    if (tempWinner != nullptr && winner == nullptr){
        std::cout << "Best version:\n";
        std::cout << "x = [";
        for(int i = 0; i < dimensions - 1; i++){
            std::cout << tempWinner->chromosomes[i] << ", ";
        }
        std::cout << tempWinner->chromosomes[dimensions-1] <<"]\n";
        std::cout << "function(x) = " << optimize(tempWinner->chromosomes,dimensions) << "\n";
    }

    cleanup();
}

void engine::cleanup() const {
    delete tempWinner;
    delete[] currentGeneration;
    delete[] nextGeneration;
    delete[] fitnessScores;
    delete[] fitnessScoresCumSum;
}


#endif //H1_ENGINE_H
