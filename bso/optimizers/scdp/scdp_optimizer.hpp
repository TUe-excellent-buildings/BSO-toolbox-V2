#ifndef SCDP_OPTIMIZER_HPP
#define SCDP_OPTIMIZER_HPP

#include <vector>

#include <bso/optimizers/scdp/output_manager.hpp>
#include <bso/optimizers/scdp/scdp_config.hpp>
#include <bso/building_physics/bp_model.hpp>
#include <bso/spatial_design/ms_N_building.hpp>
#include <bso/structural_design/sd_model.hpp>
#include <bso/utilities/data_point.hpp>
#include <bso/optimizers/scdp/performance_evaluator.hpp>

namespace bso { namespace optimizers { namespace scdp {

class scdp_optimizer 
{
private:
    scdp_config mConfig; // Configuration object containing all parameters and options for the optimization process.
    output_manager mOutputManager; // Output manager for handling all output related tasks, such as logging and saving results.
    
    // Data structures to hold the current state of the optimization
    unsigned int currentIteration = 0;
    std::vector<spatial_design::ms_N_building> mMSDesigns;
    std::vector<structural_design::sd_model> mSDModels;
    std::vector<building_physics::bp_model> mBPModels;
    std::vector<utilities::data_point> mDesignPerformances;

    // Store the initial designs values which should be constant for 
    // each design during the optimization process.
    unsigned int nSpaces = 0;
    double floorArea = 0.0; 
    double averageWidthSpace = 0.0;

    // Store the performance data for analysis and for export, 
    // this is updated after each iteration and used for the next iteration.
    std::vector<utilities::data_point> designPerformances;
	std::vector<std::string> axisDesignPerformances = {"sd","bp"};
	std::vector<double> sd_total_load;
	std::vector<double> exteriorSurfaceArea; 
    
    // Internal methods for optimization steps
    void initializeDesign();
    void getDesignResults();
    void deleteSpaces(spatial_design::ms_N_building& ms, 
                      std::vector<utilities::data_point>& performances, 
                      const std::string& modificationType,
                      const utilities::data_point& distopianPoint);
    void scaleDesign(spatial_design::ms_N_building& ms);
    void splitSpaces(spatial_design::ms_N_building& ms, 
                      const std::vector<utilities::data_point>& performances,
                      const std::string& modification, const int i, 
                      const std::vector<double>& objectiveWeights);
    void modifyDesign(spatial_design::ms_N_building& ms, unsigned int iteration);
    void enforceConstraints(spatial_design::ms_N_building& ms);
    void visualizeMsw(const std::vector<utilities::data_point>& performances, 
                      const int interationNr, const std::string& modificationType, 
                      const std::vector<double>& objectiveWeights);

    // Different Methods to run one iteration of the optimization process
    void runSplitDeleteIteration(const std::string& modificationType);
    void runModificationIteration(const std::string& modificationType);
    
    // Methods to run all iteration in a certain strategies
    void runSplitDeleteOnlyIterations();
    void runModificationOnlyIterations();
    void runAlternatingIterations();

    // Output the requested results, either use after optimization finished
    // or after an error.
    void outputResults();

public:
    scdp_optimizer(scdp_config config);

    void runOptimization();
        
    const auto& getDesigns() const { return mMSDesigns; }
    const auto& getSDModels() const { return mSDModels; }
    const auto& getBPModels() const { return mBPModels; }
    const auto& getPerformances() const { return mDesignPerformances; }
};

}}} // namespace bso::optimizers::scdp

#include <bso/optimizers/scdp/scdp_optimizer.cpp>

#endif // SCDP_OPTIMIZER_HPP