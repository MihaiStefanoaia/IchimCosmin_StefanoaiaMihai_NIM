#ifndef H1_ENGINE_H
#define H1_ENGINE_H
#include "funclib.h"
#include <random>
#include <iostream>
#include <fstream>
#include "json.hpp"


class engine {
public:
//    buffers
    genome* currentGeneration{};              // the main population on which the work is being done
    genome* nextGeneration{};                 // buffer for the new generations
    double_t* fitnessScores{};                // list of the fitness scores
    double_t* fitnessScoresCumSum{};          // list of the cumulative sum of the fitness scores
    double_t* newGenerationFitnessScores;     // AAAAAAAAAaa
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
    uint32_t generationsBetweenHillclimbings = 1;
    double_t crossoverPercentage = 0.65;
    uint32_t loggingFrequency = 256;
    double_t selectionPercentage = 0.5;
    std::string logFile = "out.log";
    bool disableLogging = true;
//    various helper variables
    uint32_t currentGenerationsBetweenHillclimbings = 0;
    double_t currentMaximumFitness = -1;      // used for printing
    nlohmann::json log;
    std::string functionName = "";
    time_t startTime;
    time_t finishTime;

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
    // run the selection
    for(auto newMember = 0; newMember < populationSize; newMember++){
        double_t randomSelector = (double)std::rand()/(double)RAND_MAX * fitnessScoresCumSum[populationSize-1];
        for(auto i = 0; i < populationSize * selectionPercentage; i++){
            if (fitnessScoresCumSum[i] >= randomSelector){
                nextGeneration[newMember].get_genes(currentGeneration[i],dimensions);
                newGenerationFitnessScores[newMember] = fitnessScores[i];
                break;
            }
        }
    }
    for(uint32_t i = populationSize * selectionPercentage; i < populationSize; i++){
        uint32_t randomSelector = (double)std::rand()/(double)RAND_MAX * populationSize * selectionPercentage;
        nextGeneration[i].get_genes(nextGeneration[randomSelector],dimensions);
        nextGeneration[i].mutate(dimensions,mutationRate,lowerBound,upperBound);
        newGenerationFitnessScores[i] = fitness(optimize(nextGeneration[i].chromosomes,dimensions),dimensions);
    }
    // crossover
    quickSort(newGenerationFitnessScores, nextGeneration, 1, populationSize - 1);
    uint32_t crossoverPopulation = populationSize * crossoverPercentage;
    crossoverPopulation += (crossoverPopulation % 2) * (std::rand() > 0x3FFFFFFF ? 1 : -1);
    for(auto i = 0; i < crossoverPopulation; i+=2){
        nextGeneration[i].crossover(nextGeneration[i+1],dimensions,crossoverCuts, lowerBound, upperBound);
    }
    // mutation
    for(auto i = 0; i < populationSize; i++){
        nextGeneration[i].mutate(dimensions,mutationRate,lowerBound, upperBound);
    }
    // hill climb
    if (generationsBetweenHillclimbings > 0 && currentGenerationsBetweenHillclimbings == 0) {
        for (auto i = 0; i < populationSize; i++) {
            nextGeneration[i].hillclimb(dimensions, optimize, fitness, strategy, lowerBound, upperBound);
        }
        currentGenerationsBetweenHillclimbings = generationsBetweenHillclimbings - 1;
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
    newGenerationFitnessScores = new double_t[populationSize];

    for(auto i = 0; i < populationSize; i++){
        currentGeneration[i] = genome(dimensions,lowerBound,upperBound);
        nextGeneration[i] = genome(dimensions,lowerBound,upperBound);
    }

    log = {};
    log["function"] = functionName;
    log["dimensions"] = dimensions;
    log["progress"] = nlohmann::json::array();
    log["population"] = populationSize;
    log["tolerance"] = threshold;
    log["mutation_rate"] = mutationRate;
    log["frequency"] = loggingFrequency;

    // convert the threshold from the tolerance to the value of the fitness function required to pass
    threshold = fitness(fixedpt(threshold),dimensions);
}

void engine::run() {
    setup();
    startTime = std::time(nullptr);
    for(auto generation = 0; generation < generations || generations == -1; generation++){
        run_generation();
        if(generation % loggingFrequency == 0){
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
            log["progress"].push_back((float)optimize(tempBest->chromosomes,dimensions));
        }
        if (winner != nullptr){
            std::cout << "Finished after " << generation << " generations:\n";
            std::cout << "x = [";
            for(int i = 0; i < dimensions - 1; i++){
                std::cout << winner->chromosomes[i] << ", ";
            }
            std::cout << winner->chromosomes[dimensions-1] <<"]\n";
            std::cout << "function(x) = " << optimize(winner->chromosomes,dimensions) << "\n";

            log["status"] = "success";
            log["last_generation"] = generation;
            log["solution"] = nlohmann::json::array();
            for(int i = 0; i < dimensions; i++){
                log["solution"].push_back((float)winner->chromosomes[i]);
            }

            if(generation % loggingFrequency != 0){
                log["solution_value"] = (float)optimize(winner->chromosomes,dimensions);
            }

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

        log["status"] = "failure";
        log["last_generation"] = generations;
        log["solution"] = nlohmann::json::array();
        for(int i = 0; i < dimensions; i++){
            log["solution"].push_back((float)tempWinner->chromosomes[i]);
        }
        if(generations % loggingFrequency != 0){
            log["solution_value"] = (float)optimize(tempWinner->chromosomes,dimensions);
        }

    }
    finishTime = std::time(nullptr);
    log["runtime"] = finishTime - startTime;
    auto file_opened = std::ofstream(logFile);
    if(file_opened){
        file_opened << log.dump(2);
    }

}


#endif //H1_ENGINE_H
