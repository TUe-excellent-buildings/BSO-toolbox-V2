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
    scdp_config mConfig; // Configuration object set by input arguments in the terminal, it contains all parameters and options for the optimization process.
    output_manager mOutputManager; // Output manager for handling all output related tasks, such as logging and outputting results.
    
    // Data structures to hold the current state of the optimization, i.e. .back() is the current design, model, or performance.
    unsigned int mCurrentIteration = 0;
    std::vector<spatial_design::ms_N_building> mMSDesigns;
    std::vector<structural_design::sd_model> mSDModels;
    std::vector<building_physics::bp_model> mBPModels;
    std::vector<daylight::dl_model> mDLModels;
    std::string getActiveObjective() const; // Get the active objective token ("sd", "bp" or "dl") for the current iteration based on the parsed modificationSequence.

    // Store the initial designs values which should be constant for 
    // each design during the optimization process.
    unsigned int mNrSpaces = 0;
    double mFloorArea = 0.0; 

    // Store the performance data for analysis and for export, 
    // this is updated after each iteration and used for the next iteration.
    std::vector<utilities::data_point> mDesignPerformances; // Four dimensions are used always in the same order: sd, bp, dl (ADFbuilding), dl (score)
    std::vector<std::string> mAxisDesignPerformances = {"sd","bp","dl_ADFbuilding_inverse", "dl_score"};
    std::vector<bool> mActiveObjectives = {false, false, false, false}; // Track which of the four performance dimensions are actually activefor this specific scdp_config setting, corresponding to: [0]=sd, [1]=bp, [2]=dl_ADFbuilding, [3]=dl_score.

    // Additional data to store for export and analysis, these are updated after each iteration.
	std::vector<double> mSdTotalLoad;
	std::vector<double> mExteriorSurfaceArea;
    
    // Internal methods for optimization steps
    void initializeDesign();
    void getDesignResults();    
    std::vector<utilities::data_point> getIntermediateResults(spatial_design::ms_N_building& ms, const std::string& objective);
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
                      const int interationNr, const std::vector<double>& objectiveWeights,
                      std::string titleAddition = "");

    // Different Methods to run one iteration of the optimization process
    void runSplitDeleteIteration(const std::string& modificationType);
    void runModificationIteration(const std::string& modificationType);
    
    // Methods to run all iteration in a certain strategies
    void runSplitDeleteOnlyIterations();
    void runModificationOnlyIterations();
    void runAlternatingIterations();

    // Output the requested results, either use after optimization finished or after an error.
    void outputResults();

public:
    scdp_optimizer(scdp_config config);

    void runOptimization();
        
    const auto& getDesigns() const { return mMSDesigns; }
    const auto& getSDModels() const { return mSDModels; }
    const auto& getBPModels() const { return mBPModels; }
    const auto& getDLModels() const { return mDLModels; }
    const auto& getPerformances() const { return mDesignPerformances; }
};

}}} // namespace bso::optimizers::scdp

#include <bso/optimizers/scdp/scdp_optimizer.cpp>

#endif // SCDP_OPTIMIZER_HPP