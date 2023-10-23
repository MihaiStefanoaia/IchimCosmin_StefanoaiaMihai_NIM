#include <iostream>

#include "fpm/ios.hpp"
#include "engine.h"
#include <ctime>
#include <set>
#include "argparse.hpp"

#include "fpm/fixed.hpp"
#include "fpm/math.hpp"
#include <fstream>
#include "lut.h"

// map: function_name -> (domain_min, domain_max, function, fitness_function)
std::map<std::string, std::tuple<float,float,std::function<fixedpt(fixedpt*, uint32_t)>,std::function<double_t (fixedpt,uint32_t)>>> functions = {
        {"rastrigin",{-5.12, 5.12, rastrigin, rastrigin_fitness}},
};

std::map<std::string, HillclimbStrategies> hc_strats = {
        {"none",  HillclimbStrategies::NONE},
        {"first", HillclimbStrategies::FIRST_IMPROVEMENT},
        {"best",  HillclimbStrategies::BEST_IMPROVEMENT},
};

void createLUT() {
    auto start  = fixedpt (0l);
    auto end = fixedpt (8l);
    auto step = fixedpt (8l);
    step /= fixedpt(16384l);
    auto cos_values = new fixedpt[16384];
    for (int i = 0; i < 16384; i += 1) {
        cos_values[i] = fpm::cos((start + (fixedpt(i) * step)));
    }
    std::ofstream header_file("lut.h");
    header_file << "#ifndef LUT_H\n";
    header_file << "#define LUT_H\n\n";
    header_file << "#include \"fpm/fixed.hpp\"\n";
    header_file << "#include \"fpm/math.hpp\"\n";
    header_file << "#include <sys/types.h>\n";
    header_file << "using fixedpt = fpm::fixed<int64_t ,__int128_t ,48>;\n";
    header_file << "uint64_t * cos_values = new uint64_t []{\n";
    for (int i = 0; i < 16384; i += 1) {
        header_file << "    " << *(uint64_t*)(&cos_values[i]) << ",\n";
    }
    header_file << "};\n\n";
    header_file << "#endif // LUT_H\n";
    header_file.close();
    std::cout << "lut.h generated successfully." << std::endl;
    return;
}

//int main(int argc, char** argv) {
// //    createLUT();
//    std::cout<< sin_lut(fixedpt::pi()/fixedpt(4) - fixedpt::two_pi());
//}

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
