#ifndef SCDP_OUTPUT_MANAGER_HPP
#define SCDP_OUTPUT_MANAGER_HPP

#include <ctime>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <unistd.h>

#include <bso/optimizers/scdp/scdp_config.hpp>
#include <bso/utilities/data_point.hpp>

namespace bso { namespace spatial_design { class ms_N_building; } }

namespace bso { namespace optimizers { namespace scdp {

// Struct to hold all relevant data for outputting results of the scdp_optimization in a single object.
// An object is generated in the scdp_optimizer after all iteration and passed to the output manager for outputting.
struct OptimizationOutput
{
    const std::vector<bso::spatial_design::ms_N_building>& msDesigns;
    const std::vector<utilities::data_point>& designPerformances;
    const std::vector<std::string>& axisDesignPerformances;
    const std::vector<double>& sd_total_load;
    const std::vector<double>& exteriorSurfaceArea;
    const std::string& modification;
    const bool weightedModification;
    const std::vector<double>& objectiveWeights;
    const std::string& designName;
    const std::string& deleteMethod;
    unsigned int maxIterationsInSequence;
    unsigned int finalIteration;
    unsigned int RNGSeed;
};

class output_manager {
private:
    scdp_config mConfig;    
    std::string mResultDir = ""; // will be set based on config options when outputData constructor is called
    std::string mVisualizationSubDir = ""; // subdirectory for visualizations, set based on config options when outputData constructor is called

    // Continues process/wall-clock timers for subsequential short processes 
    // (e.g. outputting data, writing files, simulations)
    // See functions startTimer and endTimer for outputting times 
    // over a series of short processes (i.e. a whole iteration or total time of the optimization).
    mutable clock_t mBegin = 0; 
    mutable clock_t mEnd = 0; 
    mutable std::chrono::steady_clock::time_point wallBegin = std::chrono::steady_clock::now();
    mutable std::chrono::steady_clock::time_point wallEnd = wallBegin;

    // Timer storage for series of short processes combined, e.g. for a whole iteration or, total time of the 
    // optimization. The label can be any string, e.g. "iteration_1", "total_time", etc.
    mutable std::map<std::string, std::chrono::steady_clock::time_point> mTimers; 
    
    // Helper functions for creating result directories based on the configuration options.
    bool createResultDir(std::string& dir) const;
    bool createStandardSubResultDir(std::string& dir, bool shortName = false) const;
    bool createSubResultDir(std::string& dir) const;
    void setResultDir();
    static std::string getCurrentDateTime();

public:
    explicit output_manager(const scdp_config& config);

    // Getter for the result directory path.
    const std::string& getResultDir() const{return mResultDir;}
    const std::string& getVisualizationSubDir() const{return mVisualizationSubDir;}

    // Logs error messages to a file named "error_log" in the result directory if it exists, 
    // otherwise in the current working directory.
    void logError(const std::string& errorMessage,
                  unsigned int errorIteration,
                  bool verbose = false);

    // Logs memory usage to std::cout with an optional label
    void log_memory_usage(const std::string& label = "") const;

    // Main function to output the results of the optimization, 
    // including writing files and printing to terminal as specified in the configuration.
    void outputData(const OptimizationOutput& output, bool verbose = false);

    // Time output functions for measuring the duration of processes and 
    // optionally printing the durations.
    template<class T>
    void out(const T& t, const bool& e = false, const bool& i = false, const bool& verbose = false, const bool& printWall = false) const
    {
        if (!verbose) return;
        std::cout << t;
        mEnd = clock();
        wallEnd = std::chrono::steady_clock::now();
        if (i) {
            double process_ms = 1000.0 * (mEnd - mBegin) / CLOCKS_PER_SEC;
            double wall_ms = std::chrono::duration_cast<std::chrono::milliseconds>(wallEnd - wallBegin).count();
            if(printWall){std::cout << " (process: " << process_ms << " ms, wall: " << wall_ms << " ms)";}
            else{std::cout << " (" << process_ms << " ms)";}
        }
        if (e) std::cout << std::endl;
        mBegin = mEnd;
        wallBegin = wallEnd;
    }
    void startTimer(const std::string& label) const;
    double endTimer(const std::string& label, bool print = true) const;
};

}}} // namespace bso::optimizers::scdp

#include <bso/optimizers/scdp/output_manager.cpp>

#endif // SCDP_OUTPUT_MANAGER_HPP
