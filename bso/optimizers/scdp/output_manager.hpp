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
    scdp_config mConfig;    
    std::string mResultDir;
    mutable clock_t mBegin = 0;
    mutable clock_t mEnd = 0;
    
    bool createResultDir(std::string& dir) const;
    bool createStandardSubResultDir(std::string& dir, bool shortName = false) const;
    bool createSubResultDir(std::string& dir) const;
    void setResultDir();

    static std::string getCurrentDateTime();

public:
    explicit output_manager(const scdp_config& config);

    const std::string& getResultDir() const{return mResultDir;}

    void logError(const std::string& errorMessage,
                  unsigned int errorIteration,
                  bool verbose = false);

    void outputData(const OptimizationOutput& output);

    template<class T>
    void out(const T& t, const bool& e = false, const bool& i = false, const bool& verbose = false) const
    {
        if (!verbose) return;
        std::cout << t;
        mEnd = clock();
        if (i) std::cout << " (" << 1000*(mEnd-mBegin)/CLOCKS_PER_SEC << " ms)";
        if (e) std::cout << std::endl;
        mBegin = mEnd;
    }
};

}}} // namespace bso::optimizers::scdp

#include <bso/optimizers/scdp/output_manager.cpp>

#endif // SCDP_OUTPUT_MANAGER_HPP
