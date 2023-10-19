#ifndef H1_ENGINE_H
#define H1_ENGINE_H
#include "funclib.h"
#include <random>
#include <iostream>
#include <tuple>


class engine {
public:
    genome* currentGeneration{};              // the main population on which the work is being done
    genome* nextGeneration{};                 // buffer for the new generations
    double_t* fitnessScores{};                // list of the fitness scores
    double_t* fitnessScoresCumSum{};   // list of the cumulative sum of the fitness scores
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
    for (auto i = 1; i < populationSize; i++){
        auto fitnessScore = fitness(optimize(currentGeneration[i].chromosomes,dimensions),dimensions);
        fitnessScores[i] = fitnessScore;
        if(fitnessScore >= threshold){
            winner = &currentGeneration[i];
            return;
        }
        if (fitnessScore > currentMaximumFitness){
            currentMaximumFitness = fitnessScore;
            tempWinner->get_genes(currentGeneration[i],dimensions);
        }
    }
    // do the elitist selection
    // sorting of fitnessScores - only done if the elitist selection is done
    if(elitistPercentageInFollowingGeneration != 0){

    }
        quickSort(fitnessScores, currentGeneration, 1, populationSize - 1);

    //get the cumulative sum of the sorted fitness scores
    fitnessScoresCumSum[0] = fitnessScores[0];
    for (auto i = 1; i < populationSize; i++) {
        fitnessScoresCumSum[i] = fitnessScoresCumSum[i-1] + fitnessScores[i];
    }

    int elitistCount = (elitistPercentageInFollowingGeneration * populationSize) / 100;
    for (auto i = 0; i < elitistCount; i++){
        nextGeneration[i].get_genes(currentGeneration[populationSize - 1 - i], dimensions);
//        std::cout << "fitness_scores = [";
//        for(int i = 0; i < populationSize - 1; i++){
//            std::cout << fitnessScores[i] << ", ";
//        }
//        std::cout << fitnessScores[populationSize-1] << "]\n";
//        exit(0);
    }
    // after the elitist selection the crossover process should be done (the candidates are to be selected using the roulette strategy)
    for(auto newMember = elitistCount; newMember < populationSize; newMember++){
        double_t randomSelector = (double)std::rand()/(double)RAND_MAX * fitnessScoresCumSum[populationSize-1];
        for(auto i = 0; i < populationSize; i++){
            if (fitnessScoresCumSum[i] >= randomSelector){
                nextGeneration[newMember].get_genes(currentGeneration[i],dimensions);
            }
        }
    }
    for (auto i = elitistCount - elitistCount % 2; i < populationSize - populationSize % 2; i += 2){
        nextGeneration[i].crossover(nextGeneration[i+1],dimensions,crossoverCuts, lowerBound, upperBound);
    }
    // the crossover should be followed by the mutation process
    for (auto i = 0; i < populationSize; i++){
        nextGeneration[i].mutate(dimensions, mutationRate, lowerBound, upperBound);
    }
//    double_t randomSelector = (double)std::rand()/(double)RAND_MAX * get<0>(fitnessScoresCumSum[populationSize-1]);
//    for(auto newMember = elitistCount; newMember < populationSize; newMember++){
//        for(auto i = 0; i < populationSize; i++){
//            if (get<0>(fitnessScoresCumSum[i]) >= randomSelector){
//                nextGeneration[newMember].get_genes(currentGeneration[get<1>(fitnessScoresCumSum[i])],dimensions);
//                if (newMember < populationSize - 1) {
//                    nextGeneration[newMember + 1].get_genes(nextGeneration[newMember], dimensions);
//                    nextGeneration[newMember + 1].mutate(dimensions, mutationRate, lowerBound, upperBound);
//                    newMember++;
//                }
//                break;
//            }
//        }
//    }
//    // crossover
//    for(auto i = 0; i < populationSize - populationSize % 2; i+=2){
//        nextGeneration[i].crossover(nextGeneration[i+1],dimensions,crossoverCuts, lowerBound, upperBound);
//    }
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
    fitnessScoresCumSum = new double_t[populationSize];

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
