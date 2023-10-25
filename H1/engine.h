#ifndef H1_ENGINE_H
#define H1_ENGINE_H
#include "funclib.h"
#include <random>
#include <iostream>
#include "json.hpp"


class engine {
public:
//    buffers
    genome* currentGeneration{};              // the main population on which the work is being done
    genome* nextGeneration{};                 // buffer for the new generations
    double_t* fitnessScores{};                // list of the fitness scores
    double_t* fitnessScoresCumSum{};          // list of the cumulative sum of the fitness scores
    int32_t* fitnessRanking{};                // buffer for the fitness rankings
    genome* winner{};
    genome* tempWinner{};
// parameters
    double_t lowerBound{};                    // lower bound of the function domain
    double_t upperBound{};                    // upper bound of the function domain
    std::function<fixedpt(fixedpt*, uint32_t)> optimize; // function to optimize for
    std::function<double_t (fixedpt,uint32_t)> fitness;           // fitness function
    uint32_t dimensions{};                    // dimensions to optimize - also number of chromosomes
    uint32_t populationSize{};                // amount of entities
    uint32_t generations{};                   // max amount of times to run the algorithm
    double_t threshold{};                     // the stopping point
    double_t mutationRate{};                  // chance between 0 and 1 for a bit to mutate
    uint32_t crossoverCuts = 1;
    HillclimbStrategies strategy = NONE;
    uint32_t generationsBetweenHillclimbings = 25;
    uint32_t currentGenerationsBetweenHillclimbings = 25;
    double_t crossoverPercentage = 0.2;
    uint32_t crossoverWindow = 10;
    uint32_t loggingFrequency = 256;
    std::string logFile = "out.log";
    bool disableLogging = true;
//    various helper variables
    double_t currentMaximumFitness = -1;      // used for printing
    nlohmann::json log;

    void run();

    void setup();
    void run_generation();
    engine()= default;
};

void engine::run_generation() {
    // generate fitness scores for each member of the generation
    fitnessScores[0] = fitness(optimize(currentGeneration[0].chromosomes,dimensions),dimensions);
    fitnessScoresCumSum[0] = fitnessScores[0];
    for (auto i = 1; i < populationSize; i++){
        auto fitnessScore = fitness(optimize(currentGeneration[i].chromosomes,dimensions),dimensions);
        fitnessScores[i] = fitnessScore;
        if(fitnessScore >= threshold){
            winner = &currentGeneration[i];
            return;
        }
        fitnessScoresCumSum[i] = fitnessScoresCumSum[i-1] + fitnessScore;
        if (fitnessScore > currentMaximumFitness){
            currentMaximumFitness = fitnessScore;
            tempWinner->get_genes(currentGeneration[i],dimensions);
        }
    }
    auto * newGenerationFitnessScores = new double_t[populationSize];
    // run the selection
    for(auto newMember = 0; newMember < populationSize; newMember++){
        double_t randomSelector = (double)std::rand()/(double)RAND_MAX * fitnessScoresCumSum[populationSize-1];
        for(auto i = 0; i < populationSize; i++){
            if (fitnessScoresCumSum[i] >= randomSelector){
                nextGeneration[newMember].get_genes(currentGeneration[i], dimensions);
                newGenerationFitnessScores[newMember] = fitnessScores[i];
//                if (newMember < populationSize - 1) {
//                    nextGeneration[newMember + 1].get_genes(nextGeneration[newMember], dimensions);
//                    nextGeneration[newMember + 1].mutate(dimensions, mutationRate, lowerBound, upperBound);
//                    newMember++;
//                }
                break;
            }
        }
    }
    // crossover (aici trebuie selectia aia pentru crossover)
    quickSort(newGenerationFitnessScores, nextGeneration,  1, populationSize);
    for(auto i = 0; i < populationSize * crossoverPercentage; i+=1){
        auto initialPosition = static_cast<int>((double)std::rand()/(double)RAND_MAX * (populationSize - populationSize * (1-crossoverPercentage)) + populationSize * (1-crossoverPercentage));
        auto crossoverPartnerPosition = static_cast<int>((double)std::rand()/(double)RAND_MAX * (2 * crossoverWindow) + ((-1) * crossoverWindow));
        while (crossoverPartnerPosition == 0){
            crossoverPartnerPosition = static_cast<int>((double)std::rand()/(double)RAND_MAX * (2 * crossoverWindow) + ((-1) * crossoverWindow));
        }
        nextGeneration[initialPosition].crossover(nextGeneration[crossoverPartnerPosition],dimensions,crossoverCuts, lowerBound, upperBound);
    }
    // hill climb
    if (currentGenerationsBetweenHillclimbings == 0) {
        for (auto i = 0; i < populationSize; i++) {
            nextGeneration[i].hillclimb(dimensions, optimize, fitness, strategy, lowerBound, upperBound);
        }
        currentGenerationsBetweenHillclimbings = generationsBetweenHillclimbings;
    }
    else{
        currentGenerationsBetweenHillclimbings -= 1;
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
        if(generation % loggingFrequency == 0){
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
}


#endif //H1_ENGINE_H
