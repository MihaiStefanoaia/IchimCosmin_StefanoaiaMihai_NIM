#include <iostream>

#include "fpm/ios.hpp"
#include "engine.h"
#include <ctime>
#include <set>
#include "argparse.hpp"

// map: function_name -> (domain_min, domain_max, function, fitness_function)
std::map<std::string, std::tuple<float,float,std::function<fixedpt(fixedpt*, uint32_t)>,std::function<double_t (fixedpt,uint32_t)>>> functions = {
        {"rastrigin",{-5.12, 5.12, rastrigin, rastrigin_fitness}},
};

std::map<std::string, HillclimbStrategies> hc_strats = {
        {"none",  HillclimbStrategies::NONE},
        {"first", HillclimbStrategies::FIRST_IMPROVEMENT},
        {"best",  HillclimbStrategies::BEST_IMPROVEMENT},
};

int main(int argc, char** argv) {
    std::srand(time(nullptr));

    auto parser = argparse::ArgumentParser("H1");
    std::string funcs = "{";
    for(auto& [k, _] : functions){
        funcs += k + ", ";
    }
    funcs.pop_back();
    funcs.pop_back();
    funcs += "}";

    parser.add_argument("--function", "-f")
        .help("Function to optimize for. Choose one of the following: " + funcs)
        .required()
        .action([=](const std::string& s){
            if(!functions.count(s))
                throw std::runtime_error("Invalid function name \"" + s + "\"\nChoose one of the following: " + funcs);
            return s;
        });
    parser.add_argument("--dimensions", "-d", "-dim")
        .help("Dimensions the engine will try optimizing for")
        .scan<'d',int>()
        .default_value(2);

    parser.add_argument("--population", "-p")
        .help("How many individuals the engine will work with")
        .scan<'d',int>()
        .default_value(100);

    parser.add_argument("--generations", "-g")
        .help("How many iterations the engine will run for")
        .scan<'d',int>()
        .default_value(1000);

    parser.add_argument("--tolerance", "-t")
        .help("Number below which the engine will accept the solution")
        .scan<'g',float>()
        .default_value(0.0f);

    parser.add_argument("--mutation_rate", "-m")
            .help("Probability a mutation will occur in a gene (expect 64 * dimensions * rate for every individual)")
            .scan<'g',float>()
            .default_value(0.00625f);


    parser.add_argument("--cuts", "--crossover_cuts", "-c")
        .help("How many cuts to do per chromosome in the crossover step. Set to -1 for random")
        .scan<'d',int>()
        .default_value(1);

    std::string hcs = "{";
    for(auto& [k, _] : hc_strats){
        hcs += k + ", ";
    }
    hcs.pop_back();
    hcs.pop_back();
    hcs += "}";

    parser.add_argument("--hillclimb_strategy", "-hc", "--hc_strat")
        .help("Strategy used in the hillclimb step. Choose one of the following: " + hcs)
        .default_value(std::string{"best"})
        .action([=](const std::string& s){
            if(!hc_strats.count(s))
                throw std::runtime_error("Invalid strategy \"" + s + "\"\nChoose one of the following: " + hcs);
            return s;
        });


    parser.add_argument("--log_frequency", "-fq")
        .help("The frequency at which to add to the json logs")
        .scan<'d',int>()
        .default_value(1);

    parser.add_argument("--log_file", "-lf")
        .help("Filename to dump the json at")
        .default_value(std::string{"log.json"});

    parser.add_argument("--disable_logs")
        .help("This is an option to just run the simulation without collecting any data (why?)")
        .default_value(false)
        .implicit_value(true);



    try{
        parser.parse_args(argc,argv);
    } catch(const std::runtime_error& err){
        std::cerr << err.what() << '\n';
        std::cerr << parser;
        exit(-1);
    }
    auto eng = engine();
    tie(eng.lowerBound, eng.upperBound, eng.optimize, eng.fitness) = functions[parser.get<std::string>("-f")];
    eng.dimensions = parser.get<int>("-d");
    eng.populationSize = parser.get<int>("-p");
    eng.generations = parser.get<int>("-g");
    eng.threshold = eng.fitness(fixedpt(parser.get<float>("-t")),eng.dimensions);
    eng.mutationRate = parser.get<float>("-m");
    eng.crossoverCuts = parser.get<int>("-c");
    eng.loggingFrequency = parser.get<int>("-fq");
    eng.strategy = hc_strats[parser.get<std::string>("--hc_strat")];
    eng.logFile = parser.get<std::string>("--log_file");
    eng.disableLogging = parser.get<bool>("--disable_logs");

    eng.run();
    return 0;
}
