#ifndef H1_ENGINE_H
#define H1_ENGINE_H
#include "funclib.h"
#include <random>
#include <iostream>

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
        delete chromosomes;
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
                chromosomes[i] = chromosomes[i] > fixedpt(lowerBound) ? chromosomes[i] : fixedpt(lowerBound);
                chromosomes[i] = chromosomes[i] < fixedpt(upperBound) ? chromosomes[i] : fixedpt(upperBound);
            }

        }
    }
    void get_genes(const genome& other, uint32_t dimensions){
        for (auto i = 0; i < dimensions; i++){
            *(uint64_t*)(&this->chromosomes[i]) = *(uint64_t*)(&other.chromosomes[i]);
        }
    }
};

class engine {
public:
    genome* currentGeneration{};             // the main population on which the work is being done
    genome* nextGeneration{};                // buffer for the new generations
    double_t* fitnessScores{};                // list of the cumulative sum of the fitness scores
    int32_t* fitnessRanking{};                // buffer for the fitness rankings
    uint32_t populationSize{};                // amount of entities
    uint32_t dimensions{};                    // dimensions to optimize - also number of chromosomes
    uint32_t generations{};                   // max amount of times to run the algorithm
    double_t lowerBound{};                    // lower bound of the function domain
    double_t upperBound{};                    // upper bound of the function domain
    double_t threshold{};                     // the stopping point
    double_t currentMaximumFitness = -1;
    double_t selectionPressure{};             // lambda used by the exponential function for selection
    double_t mutationRate{};                  // chance between 0 and 1 for a bit to mutate
    std::function<fixedpt(fixedpt*, uint32_t)> optimize; // function to optimize for
    std::function<double_t (fixedpt,uint32_t)> fitness;           // fitness function
    genome* winner{};
    genome* tempWinner{};
    uint32_t printFrequency = 256;

    void run();

    void setup();
    void run_generation();
    engine()= default;
};

void engine::run_generation() {
    fitnessScores[0] = fitness(optimize(currentGeneration[0].chromosomes,dimensions),dimensions);
    for (auto i = 1; i < populationSize; i++){
        auto fitnessScore = fitness(optimize(currentGeneration[i].chromosomes,dimensions),dimensions);
        if(fitnessScore >= threshold){
            winner = &currentGeneration[i];
            return;
        }
        fitnessScores[i] = fitnessScores[i-1] + fitnessScore;
        if (fitnessScore > currentMaximumFitness){
            currentMaximumFitness = fitnessScore;
            tempWinner->get_genes(currentGeneration[i],dimensions);
        }
    }
    double_t randomSelector = (double)std::rand()/(double)RAND_MAX * fitnessScores[populationSize-1];
    for(auto newMember = 0; newMember < populationSize; newMember++){
        for(auto i = 0; i < populationSize; i++){
            if (fitnessScores[i] >= randomSelector){
                nextGeneration[newMember].get_genes(currentGeneration[i],dimensions);
                if (newMember < populationSize - 1) {
                    nextGeneration[newMember + 1].get_genes(nextGeneration[newMember], dimensions);
                    nextGeneration[newMember + 1].mutate(dimensions, mutationRate, lowerBound, upperBound);
                    newMember++;
                }
                break;
            }
        }
    }
}

void engine::setup() {
    winner = nullptr;
    tempWinner = new genome(dimensions,lowerBound,upperBound);

    currentGeneration = new genome[populationSize];
    nextGeneration = new genome[populationSize];
    fitnessScores = new double_t[populationSize];

    for(auto i = 0; i < populationSize; i++){
        currentGeneration[i] = genome(dimensions,lowerBound,upperBound);
        nextGeneration[i] = genome(dimensions,lowerBound,upperBound);
    }
}

void engine::run() {
    setup();
    for(auto generation = 0; generation < generations; generation++){
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
                if(fitnessScores[i] - fitnessScores[i-1] > delta){
                    delta = fitnessScores[i] - fitnessScores[i-1];
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
}


#endif //H1_ENGINE_H
