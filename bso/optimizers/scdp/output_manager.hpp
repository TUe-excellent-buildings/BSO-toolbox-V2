#ifndef SCDP_OUTPUT_MANAGER_HPP
#define SCDP_OUTPUT_MANAGER_HPP

#include <ctime>
#include <iostream>
#include <string>
#include <vector>

#include <bso/optimizers/scdp/scdp_config.hpp>
#include <bso/utilities/data_point.hpp>

namespace bso { namespace spatial_design { class ms_N_building; } }

namespace bso { namespace optimizers { namespace scdp {

struct OptimizationOutput
{
    const scdp_config& config;
    const std::vector<bso::spatial_design::ms_N_building>& msDesigns;
    const std::vector<utilities::data_point>& designPerformances;
    const std::vector<std::string>& axisDesignPerformances;
    const std::vector<double>& sd_total_load;
    const std::vector<double>& exteriorSurfaceArea;
    const std::string& modification;
    const std::string& designName;
    const std::string& deleteMethod;
    unsigned int maxIterationsInSequence;
    unsigned int finalIteration;
    unsigned int RNGSeed;
};

class output_manager {
private:
    std::string mResultDir;

    bool createResultDir(std::string& dir, bool verbose) const;

    static std::string getCurrentDateTime();
    static clock_t sBegin;
    static clock_t sEnd;

public:
    explicit output_manager(const scdp_config& config);

    const std::string& getResultDir() const;

    static void logError(const std::string& errorMessage,
                  const std::string& inputFile,
                  const std::string& modification,
                  unsigned int iterations,
                  unsigned int errorIteration);

    void getSubResultDir(std::string& dir, std::string inputFile, std::string modification, unsigned int iterationNr, bool verbose) const;

    void outputData(const OptimizationOutput& output);

    template<class T>
    static void out(const T& t, const bool& e = false, const bool& i = false, const bool& verbose = false)
    {
        if (!verbose) return;
        std::cout << t;
        sEnd = clock();
        if (i) std::cout << " (" << 1000*(sEnd-sBegin)/CLOCKS_PER_SEC << " ms)";
        if (e) std::cout << std::endl;
        sBegin = sEnd;
    }
};

}}} // namespace bso::optimizers::scdp

#include <bso/optimizers/scdp/output_manager.cpp>

#endif // SCDP_OUTPUT_MANAGER_HPP
