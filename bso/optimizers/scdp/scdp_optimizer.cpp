#ifndef SCDP_OPTIMIZER_CPP
#define SCDP_OPTIMIZER_CPP

#include <algorithm>
#include <cmath>
#include <chrono>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <unistd.h> // for getpid()

#include <bso/spatial_design/ms_N_building.hpp>
#include <bso/spatial_design/cf_building.hpp>
#include <bso/utilities/geometry.hpp>
#include <bso/utilities/geometry_functions.hpp>
#include <bso/utilities/data_point.hpp>
#include <bso/utilities/clustering.hpp>
#include <bso/utilities/data_export.hpp>
#include <bso/structural_design/sd_model.hpp>
#include <bso/building_physics/bp_model.hpp>
#include <bso/grammar/grammar.hpp>
#include <bso/visualization/visualization.hpp>

#include <bso/optimizers/scdp/performance_evaluator.hpp>

namespace bso { namespace optimizers { namespace scdp {

scdp_optimizer::scdp_optimizer(scdp_config config)
        : mConfig(std::move(config)),
            mOutputManager(mConfig)
{
        // Initialize which objectives are active for this run
        // Order: [sd, bp, dl_ADFbuilding, dl_score]
        mActiveObjectives[0] = mConfig.getRunSDSimulation();
        mActiveObjectives[1] = mConfig.getRunBPSimulation();
        mActiveObjectives[2] = mConfig.getRunDLSimulation();
        mActiveObjectives[3] = mConfig.getRunDLSimulation();

    if (!mConfig.getVisualizations().empty()) 
    {
        visualization::initVisualization(mConfig.getArgc(), mConfig.getArgv());
	    mOutputManager.out("Initialized visualization", true, true, mConfig.getVerbose());
    }
    initializeDesign();
    getDesignResults();
}

// Helper: determine which objective token ("sd", "bp" or "dl") is
// active for the current iteration (mCurrentIteration), based on the
// parsed modification sequence in the config. If no sequence is
// available, fall back to the raw modification string.
std::string scdp_optimizer::getActiveObjective() const
{
    const auto& modSequence = mConfig.getModificationSequence();
    if (!modSequence.empty())
    {
        std::size_t pos = static_cast<std::size_t>(mCurrentIteration) % modSequence.size();
        return modSequence[pos];
    }
    else
    {
        std::stringstream errorMessage;
        errorMessage << "The modification sequence is empty, thus no active objective can be determined.\n"
                     << "(bso/optimizers/scdp/scdp_optimizer.cpp)\n";
        throw std::invalid_argument(errorMessage.str());
    }
    
}

void scdp_optimizer::runOptimization()
{
    // Run the optimization process for the specified number of iterations
    unsigned int iterations = mConfig.getIterations();
    if (iterations < 0)
    {
        std::stringstream errorMessage;
        errorMessage << "Error, expected zero or a positive number of iterations, received 0.\n"
                     << "Please specify the number of iterations using the -i or --iterations argument.\n"
                     << "(bso/optimizers/scdp/scdp_optimizer.cpp)." << std::endl;
        throw std::invalid_argument(errorMessage.str());
    }

    // Select and execute the iteration strategy
    std::string strategy = mConfig.getIterationStrategy();
    if (strategy == "split_delete")
    {
        runSplitDeleteOnlyIterations();
    }
    else if (strategy == "modification")
    {
        runModificationOnlyIterations();
    }
    else if (strategy == "alternating")
    {
        runAlternatingIterations();
    }
    else
    {
        std::stringstream errorMessage;
        errorMessage << "Error, unknown iteration strategy: " << strategy << "\n"
                     << "Expected one of: split_delete, modification, alternating\n"
                     << "Please specify the iteration strategy using the -as or --iterationStrategy argument.\n";
        throw std::invalid_argument(errorMessage.str());
    }

    // After the optimization process is finished, output the results
    outputResults();
}

void scdp_optimizer::outputResults()
{
    // Build filtered copies of performances and axis labels that only
    // include the objectives that are actually active in this run.
    std::vector<unsigned int> activeIndices;
    std::vector<std::string> activeAxisLabels;

    // Order correspondence: 0->sd, 1->bp, 2->dl_ADFbuilding, 3->dl_score
    if (mActiveObjectives[0])
    {
        activeIndices.push_back(0);
        activeAxisLabels.push_back(mAxisDesignPerformances[0]);
    }
    if (mActiveObjectives[1])
    {
        activeIndices.push_back(1);
        activeAxisLabels.push_back(mAxisDesignPerformances[1]);
    }
    if (mActiveObjectives[2])
    {
        activeIndices.push_back(2);
        activeAxisLabels.push_back(mAxisDesignPerformances[2]);
    }
    if (mActiveObjectives[3])
    {
        activeIndices.push_back(3);
        activeAxisLabels.push_back(mAxisDesignPerformances[3]);
    }

    std::vector<utilities::data_point> filteredPerformances;
    if (!activeIndices.empty())
    {
        filteredPerformances.reserve(mDesignPerformances.size());
        for (const auto& dp : mDesignPerformances)
        {
            utilities::data_point reduced(static_cast<unsigned int>(activeIndices.size()));
            for (unsigned int k = 0; k < activeIndices.size(); ++k)
            {
                reduced(k) = dp(activeIndices[k]);
            }
            filteredPerformances.push_back(reduced);
        }
    }

    const auto& perfRef = activeIndices.empty() ? mDesignPerformances : filteredPerformances;
    const auto& axisRef = activeIndices.empty() ? mAxisDesignPerformances : activeAxisLabels;

    OptimizationOutput output{
        mMSDesigns,
        perfRef,
        axisRef,
        mSdTotalLoad,
        mExteriorSurfaceArea,
        mConfig.getModification(),
        mConfig.getWeightedModification(),
        mConfig.getObjectiveWeights(),
        mConfig.getDesignName(),
        mConfig.getDeleteMethod(),
        mConfig.getMaxIterationsInSequence(),
        mCurrentIteration,
        static_cast<unsigned int>(mConfig.getRngSeed())
    };
    
    mOutputManager.outputData(output);
}

void scdp_optimizer::initializeDesign()
{
    try
	{
		if (!mConfig.getInputFile().empty()) mMSDesigns.push_back(spatial_design::ms_N_building(mConfig.getInputFile()));
        mNrSpaces = mMSDesigns.back().getSpacePtrs().size();
        mFloorArea = mMSDesigns.back().getFloorArea();
	}
	catch (std::exception& e)
	{
		std::stringstream errorMessage;
		errorMessage << "Error, could not initialize MS building design using\n";
		if (!mConfig.getInputFile().empty())
			errorMessage << "the following MS_N input file: " << mConfig.getInputFile() << "\n";
		errorMessage << "Received the following error:\n" << e.what() << std::endl;
		
		throw std::invalid_argument(errorMessage.str());
	}
	mOutputManager.out("Initialized initial MS_N building model", true, true, mConfig.getVerbose());
}

void scdp_optimizer::getDesignResults()
{   
    try
	{
        // step 1, add object to store the performance of the current design
        // Four dimensions are used: sd, bp, dl (ADFbuilding), dl (score).
        // Initialize all entries with NaN; only active simulations will
        // overwrite their respective dimensions.
		mDesignPerformances.push_back(utilities::data_point(4));
        for (unsigned int idx = 0; idx < 4; ++idx)
        {
            mDesignPerformances.back()(idx) = std::numeric_limits<double>::quiet_NaN();
        }
        
        // step 2, generate SD and BP models and evaluate them.
        std::vector<spatial_design::cf_building> cf;
        if(mConfig.getRunDLSimulation()){cf.push_back(spatial_design::cf_building(mMSDesigns.back(), mConfig.getDLSettingsFile(), 1e-6));}
        else {cf.push_back(spatial_design::cf_building(mMSDesigns.back(),1e-6));}
        mOutputManager.out("Initialized conformal model",true,true,mConfig.getVerbose());

        // step 3, generate grammar
        grammar::grammar grm(cf.back());
        mOutputManager.out("Initialized grammar",true,true,mConfig.getVerbose());

        // step 4, get data for export and analysis
        std::vector<bso::utilities::geometry::quadrilateral> exteriorSurfaces = grm.getExteriorQuadrilaterals();
        double sumExteriorSurfaceArea = 0.0;
        for(const auto& surface: exteriorSurfaces){sumExteriorSurfaceArea += surface.getArea();}
        mExteriorSurfaceArea.push_back(sumExteriorSurfaceArea);
        mOutputManager.out("Got exterior wall Area data",true,true,mConfig.getVerbose());
        
        double sdResult = 0.0;
        double bpResult = 0.0;
        double dlResult1 = 0.0;
        double dlResult2 = 0.0;
        
        // CONDITIONAL: Only run DL simulation if needed
        // This is ran first as the windows created in the DL grammar turns 
        // the boolian of windows on or off in the rectangular_properties objects
        // Which is utilized in the SD and BP grammar to determine whether a rectangle is a window.
        if(mConfig.getRunDLSimulation())
        {
            // step 3, generate DL
            // Build an explicit folder path for this daylight run so
            // that all Radiance files live inside the SCDP result
            // directory, grouped per iteration.
            const std::string& resultDir = mOutputManager.getResultDir();
            std::string dlFolder = resultDir + "/Iteration_" + std::to_string(mCurrentIteration);
            mDLModels.push_back(grm.dl_grammar<grammar::DEFAULT_DL_GRAMMAR>(mConfig.getDLSettingsFile(), dlFolder));
            mOutputManager.out("Generated daylight model",true,true,mConfig.getVerbose());
            
            // step 4, evaluate DL and store results
            mDLModels.back().analyze();
            mOutputManager.out("Evaluated daylight model, ADFbuildingInverse: ",false,false,mConfig.getVerbose());
            dlResult1 = mDLModels.back().getTotalResults().mADFBuildingInverse; // optimization runs on inverse ADF, but we have the regular ADF as well for reference
            mOutputManager.out(dlResult1,false,false,mConfig.getVerbose());
            mOutputManager.out(", mBS-EN score: ",false,false,mConfig.getVerbose());
            dlResult2 = mDLModels.back().getTotalResults().mTotalScore;
            mOutputManager.out(dlResult2,true,true,mConfig.getVerbose());
            mDesignPerformances.back()(2) = dlResult1;
			mDesignPerformances.back()(3) = dlResult2;
        }
        else
        {
            // No additional DL-specific vectors to push (unlike SD which has mSdTotalLoad)
            // and keep DL dimensions as NaN in mDesignPerformances.
            mOutputManager.out("Skipped DL simulation (not in --sim list)",true,true,mConfig.getVerbose());
        }

        // CONDITIONAL: Only run SD simulation if needed
        if(mConfig.getRunSDSimulation())
        {
            // step 3, generate SD
            mSDModels.push_back(grm.sd_grammar<grammar::DEFAULT_SD_GRAMMAR>(mConfig.getSDSettingsFile()));
            mOutputManager.out("Generated structural model",true,true,mConfig.getVerbose());
            
            // step 4, evaluate SD and store results
            mSDModels.back().analyze();
            mOutputManager.out("Evaluated structural model: ",false,false,mConfig.getVerbose());
            sdResult = mSDModels.back().getTotalResults().mTotalStrainEnergy;
            mSdTotalLoad.push_back(mSDModels.back().getTotalLoad());
            mOutputManager.out(sdResult,true,true,mConfig.getVerbose());
            mDesignPerformances.back()(0) = sdResult;
        }
        else
        {
            // Push placeholder to keep vectors aligned, but leave SD
            // performance dimension as NaN.
            mSdTotalLoad.push_back(0.0);
            mOutputManager.out("Skipped SD simulation (not in --sim list)",true,true,mConfig.getVerbose());
        }
        
        // CONDITIONAL: Only run BP simulation if needed
        if(mConfig.getRunBPSimulation())
        {
            // step 5, generate BP
            mBPModels.push_back(grm.bp_grammar<grammar::DEFAULT_BP_GRAMMAR>(mConfig.getBPSettingsFile()));
            mOutputManager.out("Generated building physics model",true,true,mConfig.getVerbose());
            
            // step 6, evaluate BP and store results
            mBPModels.back().simulatePeriods("runge_kutta_dopri5",1e-3,1e-3);
            mOutputManager.out("Evaluated building physics model: ",false,false,mConfig.getVerbose());
            bpResult = mBPModels.back().getTotalResults().mTotalEnergy;
            mOutputManager.out(bpResult,true,true,mConfig.getVerbose());
            mDesignPerformances.back()(1) = bpResult;
        }
        else
        {
            // No additional BP-specific vectors to push (unlike SD which has mSdTotalLoad)
            // and keep BP performance dimension as NaN.
            mOutputManager.out("Skipped BP simulation (not in --sim list)",true,true,mConfig.getVerbose());
        }
        
        // step 4, export visualizations
        if (!mConfig.getVisualizations().empty()) for (const auto& visOption : mConfig.getVisualizations())
        {
            if (visOption == "ms") visualization::visualize(mMSDesigns.back(),"","ms_N_building",4.0);
            if (visOption == "sg") visualization::visualize(cf.back(),"space_at_ground");
            if (visOption == "de") visualization::visualize(cf.back(),"delaunay");
            if (visOption == "bo") visualization::visualize(cf.back(),"boundarylines");
            if (visOption == "tr") visualization::visualize(cf.back(),"trirec");
            if (visOption == "tp") visualization::visualize(cf.back(),"triangular_prism");
            if (visOption == "sd" && mConfig.getRunSDSimulation() && !mSDModels.empty()) visualization::visualize(mSDModels.back());
            if (visOption == "sdC" && mConfig.getRunSDSimulation() && !mSDModels.empty()) visualization::visualize(mSDModels.back(), "component");
            if (visOption == "sdD" && mConfig.getRunSDSimulation() && !mSDModels.empty()) visualization::visualize(mSDModels.back(), "density");
            if (visOption == "fe" && mConfig.getRunSDSimulation() && !mSDModels.empty()) visualization::visualize(mSDModels.back(), "element");
            if (visOption == "fe" && mConfig.getRunSDSimulation() && !mSDModels.empty()) visualization::visualize(mSDModels.back(), "strain_energy");
            if (visOption == "bp" && mConfig.getRunBPSimulation() && !mBPModels.empty()) visualization::visualize(mBPModels.back());
            if (visOption == "dl" && mConfig.getRunDLSimulation() && !mDLModels.empty()) visualization::visualize(mDLModels.back());
            if (visOption == "dlG" && mConfig.getRunDLSimulation() && !mDLModels.empty()) visualization::visualize(mDLModels.back(), "grid");
            if (visOption == "dlD" && mConfig.getRunDLSimulation() && !mDLModels.empty()) visualization::visualize(mDLModels.back(), "daylight_factor");
            if (visOption == "dlI" && mConfig.getRunDLSimulation() && !mDLModels.empty()) visualization::visualize(mDLModels.back(), "illuminance");

            if (visOption == "msw" && mCurrentIteration == mConfig.getIterations()) 
            { // Normally generated right before modification. However, in last iteration there is no modification, but we still want to visualize the final MSW with performance coloring.
                // Get performances per space
                std::vector<utilities::data_point> performances;
                structural_design::sd_model dummySD;
                building_physics::bp_model dummyBP;
                bso::daylight::dl_model dummyDL;
                structural_design::sd_model& sdRef = mConfig.getRunSDSimulation() && !mSDModels.empty() ? mSDModels.back() : dummySD;
                building_physics::bp_model& bpRef = mConfig.getRunBPSimulation() && !mBPModels.empty() ? mBPModels.back() : dummyBP;
                bso::daylight::dl_model& dlRef = mConfig.getRunDLSimulation() && !mDLModels.empty() ? mDLModels.back() : dummyDL;
                performance_evaluator::getPerformancePerSpace(performances, mMSDesigns.back(), sdRef, bpRef, dlRef, mConfig.getModification(), mConfig.getPerformanceDivision(), mOutputManager, mConfig.getVerbose(), mConfig.getRunSDSimulation(), mConfig.getRunBPSimulation(), mConfig.getRunDLSimulation());

                // step 3, normalize the data points
                utilities::data_point utopianPoint;
                utilities::data_point distopianPoint;
                performance_evaluator::normalizeData(performances, utopianPoint, distopianPoint, mOutputManager, mConfig.getVerbose());

                // Visualize the performance of spaces
                visualizeMsw(performances, mCurrentIteration, mConfig.getObjectiveWeights());
            }
        }
        mOutputManager.out("Sent models to visualization",true,true,mConfig.getVerbose());
    }
	catch(const std::exception& e)
	{
		// Log error message to export file 'log_error'
		mOutputManager.logError(e.what(), mCurrentIteration, mConfig.getVerbose());

		// Terminal message
		std::cout << "Error, could not finalize the run.\n"
				  << "Received the following error:\n" << e.what() << " \n"
				  << "start output generation despite not finishing: " << std::endl;

        // Start generating the requested outputs:
       outputResults();
       
        // After outputting the results, rethrow the exception to allow for debugging if needed
        throw;
	}
} // getDesignResults()

std::vector<utilities::data_point> scdp_optimizer::getIntermediateResults(spatial_design::ms_N_building& ms, const std::string& objective)
{
    std::vector<utilities::data_point> intermediateSpaceResults;
    try
    {
        // step 1, generate SD and BP models and evaluate them.
        // Use the same setup as in getDesignResults: when DL simulation
        // is enabled, construct the conformal model with the DL settings
        // file so that window definitions are available; otherwise use
        // the simpler constructor.
        std::vector<spatial_design::cf_building> cf;
        if (mConfig.getRunDLSimulation()){cf.push_back(spatial_design::cf_building(ms, mConfig.getDLSettingsFile(), 1e-6));}
        else{cf.push_back(spatial_design::cf_building(ms, 1e-6));}
        mOutputManager.out("Initialized conformal model",true,true,mConfig.getVerbose());

        // step 2, generate grammar
        grammar::grammar grm(cf.back());
        mOutputManager.out("Initialized grammar",true,true,mConfig.getVerbose());

        // get data to retrun for intermediate result not stored in member variables.        
        double sdResult = 0.0;
        double bpResult = 0.0;
        double dlResult = 0.0;

        // Create dummy models for when not all models are requested.
        bso::building_physics::bp_model dummyBP; 
        bso::structural_design::sd_model dummySD;
        bso::daylight::dl_model dummyDL; 

        bool sdAvailable = mConfig.getRunSDSimulation();
        bool bpAvailable = mConfig.getRunBPSimulation();
        bool dlAvailable = mConfig.getRunDLSimulation();

        if(dlAvailable && objective != "weighted")
        {
            // step 3, generate DL
            const std::string& resultDir = mOutputManager.getResultDir();
            std::string dlFolder = resultDir + "/Iteration_" + std::to_string(mCurrentIteration);
            bso::daylight::dl_model dl = grm.dl_grammar<grammar::DEFAULT_DL_GRAMMAR>(mConfig.getDLSettingsFile(), dlFolder);
            mOutputManager.out("Generated intermediate daylight model",true,true,mConfig.getVerbose());
            if(objective != "dl") mOutputManager.out("Note: this is to insert the window definition in sd or bp analysis",true,true,mConfig.getVerbose());
            if(objective == "dl")
            {
                // step 4, evaluate DL and store results
                dl.analyze();
                mOutputManager.out("Evaluated intermediate daylight model, ADFbuilding: ",false,false,mConfig.getVerbose());
                dlResult = dl.getTotalResults().mADFBuildingInverse;
                mOutputManager.out(dlResult,true,true,mConfig.getVerbose());
                performance_evaluator::getPerformancePerSpace(intermediateSpaceResults, ms, dummySD, dummyBP, dl, objective, mConfig.getPerformanceDivision(), mOutputManager, mConfig.getVerbose(), sdAvailable, bpAvailable, dlAvailable);
            }
        }
        if(objective == "sd" && sdAvailable)
        {
            // step 3, generate SD
            bso::structural_design::sd_model sd = grm.sd_grammar<grammar::DEFAULT_SD_GRAMMAR>(mConfig.getSDSettingsFile());
            mOutputManager.out("Generated intermediate structural model",true,true,mConfig.getVerbose());
            
            // step 4, evaluate SD and store results
            sd.analyze();
            mOutputManager.out("Evaluated intermediate structural model: ",false,false,mConfig.getVerbose());
            sdResult = sd.getTotalResults().mTotalStrainEnergy;
            mOutputManager.out(sdResult,true,true,mConfig.getVerbose());
            performance_evaluator::getPerformancePerSpace(intermediateSpaceResults, ms, sd, dummyBP, dummyDL, objective, mConfig.getPerformanceDivision(), mOutputManager, mConfig.getVerbose(), sdAvailable, bpAvailable, dlAvailable);
        }
        if(objective == "bp" && bpAvailable)
        {
            // step 3, generate BP
            bso::building_physics::bp_model bp = grm.bp_grammar<grammar::DEFAULT_BP_GRAMMAR>(mConfig.getBPSettingsFile());
            mOutputManager.out("Generated intermediate building physics model",true,true,mConfig.getVerbose());
            
            // step 4, evaluate BP and store results
            bp.simulatePeriods("runge_kutta_dopri5",1e-3,1e-3);
            mOutputManager.out("Evaluated intermediate building physics model: ",false,false,mConfig.getVerbose());
            bpResult = bp.getTotalResults().mTotalEnergy;
            mOutputManager.out(bpResult,true,true,mConfig.getVerbose());
            performance_evaluator::getPerformancePerSpace(intermediateSpaceResults, ms, dummySD, bp, dummyDL, objective, mConfig.getPerformanceDivision(), mOutputManager, mConfig.getVerbose(), sdAvailable, bpAvailable, dlAvailable);
        }
        if(objective == "weighted")
        {
            // step 3, generate DL
            const std::string& resultDir = mOutputManager.getResultDir();
            std::string dlFolder = resultDir + "/Iteration_" + std::to_string(mCurrentIteration);
            bso::daylight::dl_model dl = grm.dl_grammar<grammar::DEFAULT_DL_GRAMMAR>(mConfig.getDLSettingsFile(), dlFolder);
            mOutputManager.out("Generated intermediate daylight model",true,true,mConfig.getVerbose());
            bso::structural_design::sd_model sd = grm.sd_grammar<grammar::DEFAULT_SD_GRAMMAR>(mConfig.getSDSettingsFile());
            mOutputManager.out("Generated intermediate structural model",true,true,mConfig.getVerbose());
            bso::building_physics::bp_model bp = grm.bp_grammar<grammar::DEFAULT_BP_GRAMMAR>(mConfig.getBPSettingsFile());
            mOutputManager.out("Generated intermediate building physics model",true,true,mConfig.getVerbose());
            if(dlAvailable)
            {
                // step 4, evaluate DL and store results
                dl.analyze();
                mOutputManager.out("Evaluated intermediate daylight model, ADFbuilding: ",false,false,mConfig.getVerbose());
                dlResult = dl.getTotalResults().mADFBuildingInverse;
                mOutputManager.out(dlResult,true,true,mConfig.getVerbose());
            }
            if(sdAvailable)
            {
                // step 4, evaluate SD and store results
                sd.analyze();
                mOutputManager.out("Evaluated intermediate structural model: ",false,false,mConfig.getVerbose());
                sdResult = sd.getTotalResults().mTotalStrainEnergy;
                mOutputManager.out(sdResult,true,true,mConfig.getVerbose());
            }
            if(bpAvailable)
            {
                // step 4, evaluate BP and store results
                bp.simulatePeriods("runge_kutta_dopri5",1e-3,1e-3);
                mOutputManager.out("Evaluated intermediate building physics model: ",false,false,mConfig.getVerbose());
                bpResult = bp.getTotalResults().mTotalEnergy;
                mOutputManager.out(bpResult,true,true,mConfig.getVerbose());
            }

            // Store the weighted combination of the objectives in the building data point for this intermediate design
            performance_evaluator::getPerformancePerSpace(intermediateSpaceResults, ms, dummySD, bp, dummyDL, mConfig.getModification(), mConfig.getPerformanceDivision(), mOutputManager, mConfig.getVerbose(), sdAvailable, bpAvailable, dlAvailable);
        }
        else
        {
            mOutputManager.out("No simulation run requested.",true,true,mConfig.getVerbose());
        }  

        mOutputManager.out("Sent models to visualization",true,true,mConfig.getVerbose());
    }
	catch(const std::exception& e)
	{
		// Log error message to export file 'log_error'
		mOutputManager.logError(e.what(), mCurrentIteration, mConfig.getVerbose());

		// Terminal message
		std::cout << "Error, could not finalize the run.\n"
				  << "Received the following error:\n" << e.what() << " \n"
				  << "start output generation despite not finishing: " << std::endl;

        // Start generating the requested outputs:
       outputResults();

        // After outputting the results, rethrow the exception to allow for debugging if needed
        throw;
	}
    return intermediateSpaceResults;
} // getIntermediateResults()

void scdp_optimizer::runSplitDeleteIteration(const std::string& modificationType)
{
    try
	{
        mOutputManager.out("\nSplit-delete iteration: ",false,false,mConfig.getVerbose()); mOutputManager.out(mCurrentIteration,true,true,mConfig.getVerbose());
        // create a new MS model
        spatial_design::ms_N_building newMS = mMSDesigns.back();

        // step 2, get performances per space
        std::vector<utilities::data_point> performances;
        structural_design::sd_model dummySD;
        building_physics::bp_model dummyBP;
        bso::daylight::dl_model dummyDL;
        structural_design::sd_model& sdRef = mConfig.getRunSDSimulation() && !mSDModels.empty() ? mSDModels.back() : dummySD;
        building_physics::bp_model& bpRef = mConfig.getRunBPSimulation() && !mBPModels.empty() ? mBPModels.back() : dummyBP;
        bso::daylight::dl_model& dlRef = mConfig.getRunDLSimulation() && !mDLModels.empty() ? mDLModels.back() : dummyDL;
        mOutputManager.out("Timer start for getPerformancePerSpace. ",true,true,mConfig.getVerbose());
        performance_evaluator::getPerformancePerSpace(performances, newMS, sdRef, bpRef, dlRef, modificationType, mConfig.getPerformanceDivision(), mOutputManager, mConfig.getVerbose(), mConfig.getRunSDSimulation(), mConfig.getRunBPSimulation(), mConfig.getRunDLSimulation());

        // step 3, normalize the data points
        utilities::data_point utopianPoint;
        utilities::data_point distopianPoint;
        performance_evaluator::normalizeData(performances, utopianPoint, distopianPoint, mOutputManager, mConfig.getVerbose());

        // Visualize the performance of spaces
        visualizeMsw(performances, mCurrentIteration, mConfig.getObjectiveWeights());

        // Step 4, delete spaces
        deleteSpaces(newMS, performances, modificationType, distopianPoint);

        // step 6, Enforce constrains on BSD
        enforceConstraints(newMS);
        
        // step 7, scale spaces in x and y direction to recover initial conditions, i.e. floor area
        scaleDesign(newMS);

        // step 8, split spaces to recover initial number of spaces conditions,
        // (checking x first and only splitting in z-direction if the space is a minimum of 6000 in height)
        splitSpaces(newMS, performances, modificationType, mCurrentIteration, mConfig.getObjectiveWeights());
        
        // step 9, store the new design
        mMSDesigns.push_back(newMS);
        mOutputManager.out("Modified building spatial design into a new one",true,true,mConfig.getVerbose());
        mCurrentIteration++;
        
        // step 10, get results for the new design
        getDesignResults();
    }
    catch(const std::exception& e)
	{
		// Log error message to export file 'log_error'
		mOutputManager.logError(e.what(), mCurrentIteration, mConfig.getVerbose());

		// Terminal message
		std::cout << "Error, could not finalize the run.\n"
				  << "Received the following error:\n" << e.what() << " \n"
				  << "start output generation despite not finishing: " << std::endl;

        // Start generating the requested outputs:
        outputResults();

        // After outputting the results, rethrow the exception to allow for debugging if needed
        throw;
	}
} // runSplitDeleteIteration()

void scdp_optimizer::runModificationIteration(const std::string& modificationType)
{
    try
	{
        mOutputManager.out("\nmodification iteration: ",false,false,mConfig.getVerbose()); mOutputManager.out(mCurrentIteration,true,false,mConfig.getVerbose());
        
        // step 1, create a new MS model
        spatial_design::ms_N_building newMS = mMSDesigns.back();

        // step 2, get performances per space
        std::vector<utilities::data_point> performances;
        structural_design::sd_model dummySD;
        building_physics::bp_model dummyBP;
        bso::daylight::dl_model dummyDL2;
        structural_design::sd_model& sdRef = mConfig.getRunSDSimulation() && !mSDModels.empty() ? mSDModels.back() : dummySD;
        building_physics::bp_model& bpRef = mConfig.getRunBPSimulation() && !mBPModels.empty() ? mBPModels.back() : dummyBP;
        bso::daylight::dl_model& dlRef2 = mConfig.getRunDLSimulation() && !mDLModels.empty() ? mDLModels.back() : dummyDL2;
        performance_evaluator::getPerformancePerSpace(performances, newMS, sdRef, bpRef, dlRef2, modificationType, mConfig.getPerformanceDivision(), mOutputManager, mConfig.getVerbose(), mConfig.getRunSDSimulation(), mConfig.getRunBPSimulation(), mConfig.getRunDLSimulation());

        // step 3, get data for 'worstAdapt' method if selected, 
        // which determines the add method based on the location of the worst performing spaces.
        bso::utilities::geometry::vector worstSpacesDirection = {0,0,0};
        bso::utilities::geometry::vertex centreBuilding = {0,0,0};
        bso::utilities::geometry::vertex worstSpaceCentre = {0,0,0};
        if(mConfig.getAddMethod() == "worstAdapt")
        {
            // step 3a, sort performance data points from highest to lowest to determine where the worst is located for space removal later
            performance_evaluator::sortDataPointsHighestToLowest(performances, modificationType,
                    mConfig.getModificationSequence(), mCurrentIteration, mConfig.getObjectiveWeights(),
                    mConfig.getWeightedModification(), mOutputManager, mConfig.getVerbose());

            // step 3b, determine the amount of worst spaces on which the add-method will be determined
            int spaceAmountRemoval= round(mNrSpaces*mConfig.getSpaceModifyWorstRatio()); 
            if(spaceAmountRemoval < 1){spaceAmountRemoval = 1;}

            // step 3c, determine the centre of to worst spaces:
            for(long unsigned int k=performances.size()-1; k>=performances.size()-spaceAmountRemoval; k--)
            {
                worstSpaceCentre += performances[k].getMSpacePtrs().back()->getGeometry().getCenter(); 
            }
            worstSpaceCentre = worstSpaceCentre/spaceAmountRemoval;
            centreBuilding = newMS.getCentreBuilding();
            centreBuilding.z()=0; // centre of the building should always be defined on z=0

            // step 3d, determine the worst spaces direction, i.e. where the worst spaces are located in relation 
            // to the centre of the building for later use.
            worstSpacesDirection = worstSpaceCentre - centreBuilding;
        }

        // step 4, normalize the data points
        utilities::data_point utopianPoint;
        utilities::data_point distopianPoint;
        performance_evaluator::normalizeData(performances, utopianPoint, distopianPoint, mOutputManager, mConfig.getVerbose());

        // step 5, visualize the performance of spaces
        visualizeMsw(performances, mCurrentIteration, mConfig.getObjectiveWeights(), "_bases_Mod_decision");

        // Step 6, delete spaces
        deleteSpaces(newMS, performances, modificationType, distopianPoint);

        // step 7, Enforce constrains on BSD
        enforceConstraints(newMS);

        // step 8, perform the specified modification methods
        if(mConfig.getAddMethod() == "worstAdapt")
        {
            // step 8a, execute either addElevation or shiftFillPlaneCentre depending on the worst spaces direction,
            // i.e. if the worst spaces are located more in the horizontal direction, execute shift the fill plane centre,
            // and if they are located more in the vertical direction, execute add elevation.
            mOutputManager.out("Selected modification method in 'worstAdapt' Iteration, according to following parameters: ",false,false,mConfig.getVerbose());
            mOutputManager.out("\n\tworstSpaceCentre is: ",false,false,mConfig.getVerbose());mOutputManager.out(worstSpaceCentre,false,false,mConfig.getVerbose());
            mOutputManager.out("\n\tcentreBuilding is: ",false,false,mConfig.getVerbose());mOutputManager.out(centreBuilding,false,false,mConfig.getVerbose());
            mOutputManager.out("\n\tworstSpacesDirection.getHeading().second: ",false,false,mConfig.getVerbose());mOutputManager.out(worstSpacesDirection.getHeading().second,true,false,mConfig.getVerbose());

            if(worstSpacesDirection.getHeading().second < mConfig.getInclinationSwitch())
            {
                mOutputManager.out("Selected addElevation in 'worstAdapt' Iteration: ",false,false,mConfig.getVerbose()); mOutputManager.out(mCurrentIteration,true,false,mConfig.getVerbose());
                newMS.duplicateElevation();
                mOutputManager.out("addElevation done",true,true,mConfig.getVerbose()); 
            }
            else if(worstSpacesDirection.getHeading().second > mConfig.getInclinationSwitch())
            {
                mOutputManager.out("Selected shiftFillPlaneCentre in 'worstAdapt' iteration: ",false,false,mConfig.getVerbose()); mOutputManager.out(mCurrentIteration,true,false,mConfig.getVerbose());
                
                // Alternate the direction of the shift fill plane depending on mCurrentIteration
                bool shiftInMostHeadingDirection = false;
                if(mCurrentIteration == 0) {shiftInMostHeadingDirection = true;}
                else if(mCurrentIteration % 2) {shiftInMostHeadingDirection = true;}
                
                double averageWidthSpace = newMS.getAverageHorizontalRibSize();
                newMS.shiftFillPlaneCentre(averageWidthSpace, shiftInMostHeadingDirection, 6, mConfig.getVerbose());
                mOutputManager.out("shiftFillPlaneCentre done ",true,true,mConfig.getVerbose());
            }
        }
        else if(mConfig.getAddMethod() == "shiftfill")
        {
            mOutputManager.out("Start shiftFillPlaneCentre in 'shiftfill' iteration: ",false,false,mConfig.getVerbose()); mOutputManager.out(mCurrentIteration,true,false,mConfig.getVerbose());
            bool shiftInMostHeadingDirection = false;
            if(mCurrentIteration == 0) {shiftInMostHeadingDirection = true;}
            else if(mCurrentIteration % 2) {shiftInMostHeadingDirection = true;}
            
            double averageWidthSpace = newMS.getAverageHorizontalRibSize();
            newMS.shiftFillPlaneCentre(averageWidthSpace, shiftInMostHeadingDirection, 6, mConfig.getVerbose());
            mOutputManager.out("shiftFillPlaneCentre done ",true,true,mConfig.getVerbose());
        }
        else if(mConfig.getAddMethod() == "elevation")
        {
            mOutputManager.out("Start addElevation in 'elevation' Iteration: ",false,false,mConfig.getVerbose()); mOutputManager.out(mCurrentIteration,true,false,mConfig.getVerbose());
            newMS.duplicateElevation();
            mOutputManager.out("addElevation done ",true,true,mConfig.getVerbose());
        }

        // step 13, delete the spaces which are defined inside another space, 
            // as this is not allowed and may occur in the extrude or shiftfillplane
            // functions although carefully designed to not do this, but due to the
            // complexity of the geometry it can still occur. This is done at the end
            // of the iteration to not interfere with the other modification steps, 
            // but it is enforced so that at the end of the iteration, there are no spaces inside other spaces.
        mOutputManager.out("Checking for spaces inside other spaces.",true,true,mConfig.getVerbose());
        std::vector<bso::spatial_design::ms_space*> overlappingSpaces;
        std::vector<bso::spatial_design::ms_space*> notOverlappingSpaces;
        if(newMS.getOverlappingSpaces(overlappingSpaces, notOverlappingSpaces, 1e-3))
        {
            mOutputManager.out("\tFound " + std::to_string(overlappingSpaces.size()) + " overlapping spaces.",true,true,mConfig.getVerbose());

            for(const auto& space: overlappingSpaces) {space->highlightInVisualization4 = true;} // Highlight the overlapping spaces in visualization for clerification.

            // Export after_modification_ms visualizations
            if (!mConfig.getVisualizations().empty()) for (const auto& visOption : mConfig.getVisualizations())
            {
                if (visOption == "ms") visualization::visualize(newMS,"","red_overlapping_spaces",4.0);
            }

            for(const auto& space: overlappingSpaces)
            {
                newMS.deleteSpace(space);
            }
            mOutputManager.out("\tDeleted overlapping spaces.",true,true,mConfig.getVerbose());
        }
        
        // step 12,
        enforceConstraints(newMS);        

        // Export after_modification_ms visualizations
        if (!mConfig.getVisualizations().empty()) for (const auto& visOption : mConfig.getVisualizations())
        {
            if (visOption == "ms") visualization::visualize(newMS,"","After_modification_ms",4.0);
        }

        // step 14, recover the number of spaces if needed.
        // if the amount of spaces is higher than split spaces to recover initial number.
        // if lower, delete spaces to recover initial number.
        if(newMS.getSpacePtrs().size() > mNrSpaces)
        {
    			// Determine for which objective to get intermediate results
    			std::string activeObjective = getActiveObjective();

            // Get intermediate results for the new design to determine which spaces to delete to recover initial number of spaces.
            std::vector<utilities::data_point> intermediateResults = getIntermediateResults(newMS, activeObjective);
            
            // Normalize the data points
            performance_evaluator::normalizeData(intermediateResults, utopianPoint, distopianPoint, mOutputManager, mConfig.getVerbose());
            
            // Visualize the performance of spaces
            visualizeMsw(intermediateResults, mCurrentIteration, mConfig.getObjectiveWeights(), "_bases_delete_decision");
            
            // Delete spaces to recover initial number of spaces.
            deleteSpaces(newMS, intermediateResults, modificationType, distopianPoint);
        }
                if(newMS.getSpacePtrs().size() < mNrSpaces) // when using cluster delete method, it can happen that more spaces are deleted than intended, so we need to check if we need to split spaces to recover the initial number of spaces.
        {
            if(mConfig.getSplitMethod() == "performance")
            {
				// Determine for which objective to get intermediate results
				std::string activeObjective = getActiveObjective();
                
                // Get intermediate results for the new design to determine which spaces to delete to recover initial number of spaces.
                std::vector<utilities::data_point> intermediateResults = getIntermediateResults(newMS, activeObjective);
                
                // Normalize the data points
                performance_evaluator::normalizeData(intermediateResults, utopianPoint, distopianPoint, mOutputManager, mConfig.getVerbose());
                
                // Visualize the performance of spaces
                visualizeMsw(intermediateResults, mCurrentIteration, mConfig.getObjectiveWeights(), "_bases_split_decision");
                splitSpaces(newMS, intermediateResults, modificationType, mCurrentIteration, mConfig.getObjectiveWeights());
            }
            else // if split method is not performance, we can split spaces without getting intermediate results, as the performance of spaces will not be used for the split decision.
            {
               splitSpaces(newMS, performances, modificationType, mCurrentIteration, mConfig.getObjectiveWeights());
            }
        }

        // step 14, Enforce constrains on BSD
        enforceConstraints(newMS);

        // step 15, scale spaces in x and y direction to recover initial conditions, i.e. floor area
        scaleDesign(newMS);

        // step 16, store the new design
        mMSDesigns.push_back(newMS);
        mOutputManager.out("Modified building spatial design into a new one",true,true,mConfig.getVerbose());
        mCurrentIteration++;
        
        // step 17, get results for the new design
        getDesignResults();
    }
    catch(const std::exception& e)
	{
		// Log error message to export file 'log_error'
		mOutputManager.logError(e.what(), mCurrentIteration, mConfig.getVerbose());

		// Terminal message
		std::cout << "Error, could not finalize the run.\n"
				  << "Received the following error:\n" << e.what() << " \n"
				  << "start output generation despite not finishing: " << std::endl;

        // Start generating the requested outputs:
        outputResults();

        // After outputting the results, rethrow the exception to allow for debugging if needed
        throw;
	}
} // runModificationIteration()

void scdp_optimizer::deleteSpaces(spatial_design::ms_N_building& ms,
								  std::vector<utilities::data_point>& performances, 
                                  const std::string& modificationType,
                                  const utilities::data_point& distopianPoint)
{
	std::vector<utilities::data_point> performancesRemoveSpace = performances;
    if (mConfig.getDeleteMethod() == "")
    {
        std::stringstream errorMessage;
        errorMessage << "Error, expected a delete method to be specified.\n"
                    << "Please specify a delete method using the -d or --deleteMethod argument.\n"
                     << "use -h or --help for help\n";
        throw std::invalid_argument(errorMessage.str());
    }
    else if(mConfig.getDeleteMethod() == "cluster")
    {
        // step 4, cluster the data
        utilities::clustering clus(mConfig.getRngSeed(),performances,mConfig.getKMin(),mConfig.getKMax(),250);
        mOutputManager.out("Clustered space performances",true,true,mConfig.getVerbose());

        // Visualize the clustering if requested
        if (!mConfig.getVisualizations().empty()) for (const auto& visOption : mConfig.getVisualizations())
        {
            if (visOption == "msc")
            {
                std::string title = "ms_N_clustering" + std::to_string(mCurrentIteration);
                visualization::visualize(clus, title, 4.0);
                mOutputManager.out("Sent ms clustering models to visualization",true,true,mConfig.getVerbose());
            }
        }
        
        // step 5, select worst performing cluster and remove these spaces
        unsigned int nRetainedSpaces;
        if(mConfig.getAmountOfSpaceRemoval() > 0){nRetainedSpaces = mNrSpaces - mConfig.getAmountOfSpaceRemoval();} // getAmountOfSpaceRemoval() value cannot be below 1 as checked at input, so no need to check for that here.
        else 
        {
            nRetainedSpaces = std::floor(mNrSpaces*(1-mConfig.getSpaceRemoveRatio()));
            if(nRetainedSpaces == 0){nRetainedSpaces = 1;}
        }
        auto clusters = clus.getClusters();
        while (ms.getSpacePtrs().size() > nRetainedSpaces && clusters.size() > 1)
        {
            // find cluster with centroid closest to distopianPoint.
            unsigned int worstClusterIndex = 0;
            double smallestDistance = distopianPoint.calcDistanceTo(clusters[0].getCentroid());
            for (unsigned int j = 1; j < clusters.size(); ++j)
            {
                double distance = distopianPoint.calcDistanceTo(clusters[j].getCentroid());
                if (smallestDistance > distance)
                {
                    smallestDistance = distance;
                    worstClusterIndex = j;
                }
            }
            // remove spaces within that cluster.
            for (const auto& j : clusters[worstClusterIndex].getData())
            {
                std::string msSpaceLine = j->getLabels().back();
                ms.deleteSpace(j->getMSpacePtrs().back());
            }
            // remove that cluster from the vector containing all clusters
            clusters.erase(clusters.begin() + worstClusterIndex);
        }
        mOutputManager.out("Removed worst performing cluster of spaces in building spatial design",true,true,mConfig.getVerbose());
    }
    else if (mConfig.getDeleteMethod() == "worst")
    {
        // step 4, sort data and determine space removal amount
        performancesRemoveSpace = performances;
        performance_evaluator::sortDataPointsHighestToLowest(performancesRemoveSpace, mConfig.getModification(),
            mConfig.getModificationSequence(), mCurrentIteration, mConfig.getObjectiveWeights(),
            mConfig.getWeightedModification(), mOutputManager, mConfig.getVerbose());
        mOutputManager.out("Sort data points for space removal",true,true,mConfig.getVerbose());

        unsigned int spaceAmountRemoval;
        if(mConfig.getAmountOfSpaceRemoval() > 0){spaceAmountRemoval = mConfig.getAmountOfSpaceRemoval();} // getAmountOfSpaceRemoval() value cannot be below 1 as checked at input, so no need to check for that here.
        else 
        {
            spaceAmountRemoval = std::floor(performancesRemoveSpace.size()*mConfig.getSpaceRemoveRatio());
            if(spaceAmountRemoval < 1){spaceAmountRemoval = 1;}
        }

        // step 5, select worst performing spaces and remove these spaces
        bool removeMaxValue = false;
        if(removeMaxValue)
        {
            std::reverse(performancesRemoveSpace.begin(), performancesRemoveSpace.end());
            for(long unsigned int k=performancesRemoveSpace.size()-1; k>=performancesRemoveSpace.size()-spaceAmountRemoval; k--) ms.deleteSpace(performancesRemoveSpace[k].getMSpacePtrs().back());
        }
        else
        {
            for(long unsigned int k=performancesRemoveSpace.size()-1; k>=performancesRemoveSpace.size()-spaceAmountRemoval; k--) ms.deleteSpace(performancesRemoveSpace[k].getMSpacePtrs().back());
        }
        mOutputManager.out("Removed worst performing spaces in building spatial design",true,true,mConfig.getVerbose());
    }
    else if (mConfig.getDeleteMethod() == "off")
    {
        mOutputManager.out("No spaces removed in building spatial design",true,true,mConfig.getVerbose());
    }
    else 
    {
        std::stringstream errorMessage;
        errorMessage << "Error, the delete method specified is not recognized or defined: " << mConfig.getDeleteMethod() << "\n";
        throw std::invalid_argument(errorMessage.str());
    }
} // deleteSpaces

void scdp_optimizer::scaleDesign(spatial_design::ms_N_building& ms)
{
    // scale the dimensions in x and y direction
    double scaleFactor = sqrt(mFloorArea / ms.getFloorArea());
    ms.scale({{0,scaleFactor},{1,scaleFactor}});
    mOutputManager.out("Scaled design to initial floor area",true,true,mConfig.getVerbose());
} // scaleDesign

void scdp_optimizer::splitSpaces(spatial_design::ms_N_building& ms, const std::vector<utilities::data_point>& performances, const std::string& modification, const int i, const std::vector<double>& objectiveWeights)
{
	if(mConfig.getSplitMethod() == "")
    {
        std::stringstream errorMessage;
        errorMessage << "Error, expected a split method to be specified.\n"
                     << "use -h or --help for help\n";
        throw std::invalid_argument(errorMessage.str());
    }
    else if(mConfig.getSplitMethod() == "longest")
	{
        while (ms.getSpacePtrs().size() < mNrSpaces)
        {
            auto spaceWithLargestDimension = ms.getSpacePtrs().front();
            bso::utilities::geometry::line_segment LongestAxis;
            double longestLength = 0;

            for (const auto& j : ms.getSpacePtrs())
                    {
                        std::vector<bso::utilities::geometry::line_segment> lines = j->getGeometry().getLines();
                        for(const auto& k: lines)
                        {
                            if(k.getVector().isVertical())
                            {
                                if(k.getLength() < 6000) continue; // only consider horizontal lines which are long enough for elevation height purposes
                            } 
                            if(longestLength < k.getLength() && k.getLength() > 2000) // only split spaces wherefrom the longestLength is at least 2000 to avoid to narrow spaces
                            {
                                longestLength = k.getLength();
                                LongestAxis = k;
                                spaceWithLargestDimension = j;
                            }
                        }
                    }
                    int splitamount = 2;
                    if(LongestAxis.getVector().isVertical())
                    {
                        std::vector<std::pair<unsigned int, unsigned int> > splits = {{2,splitamount}};
                        ms.splitSpace(spaceWithLargestDimension, splits);
                    }
                    else
                    {
                        ms.splitSpace(spaceWithLargestDimension, splitamount , LongestAxis);
                    }
        }		
        mOutputManager.out("Split spaces with largest dimension to restore number of spaces",true,true,mConfig.getVerbose());
    }
    else if(mConfig.getSplitMethod() == "performance")
    {
        std::vector<utilities::data_point> performancesRemoveSpace = performances;
        performance_evaluator::sortDataPointsHighestToLowest(performancesRemoveSpace, modification,
            mConfig.getModificationSequence(), i, objectiveWeights, mConfig.getWeightedModification(),
            mOutputManager, mConfig.getVerbose());
        
        // split the spaces with the best performance
        int splitCount = 0;
        int splitamount = 2;
        while (ms.getSpacePtrs().size() < mNrSpaces)
        {				
            std::vector<bso::utilities::geometry::line_segment> lines = (performancesRemoveSpace[splitCount].getMSpacePtrs().back())->getGeometry().getLines();
            bso::utilities::geometry::line_segment LongestAxis;
            double longestLength = 0;
            bool found = false;
            for(const auto& k: lines)
            {
                if(k.getVector().isVertical())
                {
                    if(k.getLength() < 6000) continue; // only consider horizontal lines which are long enough for elevation height purposes
                } 
                if(longestLength < k.getLength() && k.getLength() > 4000)
                {
                    longestLength = k.getLength();
                    LongestAxis = k;
                    found = true;
                }
            }

            if(found){ms.splitSpace(performancesRemoveSpace[splitCount].getMSpacePtrs().back(), splitamount, LongestAxis);}
            splitCount++;
        }
        mOutputManager.out("Split spaces with best performance to restore number of spaces",true,true,mConfig.getVerbose());
    }
    else if(mConfig.getSplitMethod() == "off")
    {
        mOutputManager.out("No spaces split in building spatial design",true,true,mConfig.getVerbose());
    }
    else
    {
        std::stringstream errorMessage;
        errorMessage << "Error, the split method specified is not recognized or defined: " << mConfig.getSplitMethod() << "\n";
        throw std::invalid_argument(errorMessage.str());
    }
} // splitSpaces

// Function to enforce all constraints defined for a MS representation, 
// i.e. building start at ground floor, remove floating spaces and, if applied, smaller fragments.
void scdp_optimizer::enforceConstraints(spatial_design::ms_N_building& ms)
{
	ms.setZZero();
	std::vector<spatial_design::ms_space*> floatingSpaces;
	if (ms.hasFloatingSpaces(floatingSpaces))
	{
		for (auto& i : floatingSpaces) ms.deleteSpace(i);
	}
	if (mConfig.getRemoveSmallerFragments())
	{
		std::vector<spatial_design::ms_space*> spaceFragment;
		if (ms.isFragmented(spaceFragment, true))
		{
			for (auto& i : spaceFragment) ms.deleteSpace(i);
		}
	}
}

// Function to visualize a MS representation with the space colors defined
// by its performance data points from the modification type for this iteration
// while profiding an iteration number in the title.
// Keep in mind. The inserted performance data points should already be normalized.
void scdp_optimizer::visualizeMsw(
    const std::vector<bso::utilities::data_point>& performances, 
    const int interationNr, const std::vector<double>& objectiveWeights, std::string titleAddition)
{
    // Mapping of data_point dimensions used in performances:
    //  index 0 -> BP objective
    //  index 1 -> SD objective
    //  index 2 -> DL objective (e.g. ADFbuilding utilized for performance decisions)
    if (!mConfig.getVisualizations().empty()) for (const auto& visOption : mConfig.getVisualizations())
    {
        if (visOption == "msw")
        {
            float transparency = 1.0f;
            // Always create a weight vector that matches the 3 objectives (bp, sd, dl)
            std::vector<double> resultModificationFactor = {0.0, 0.0, 0.0};
            std::string titleBase = "ms_N_worst";
            std::string title = titleBase;

            // If weighted modification is enabled, combine objectives using the
            // provided weights (ordered as sd, bp, dl in the config).
            if (mConfig.getWeightedModification())
            {
                if (!objectiveWeights.empty())
                {
                    // objectiveWeights: [sd, bp, dl] -> resultModificationFactor: [bp, sd, dl]
                    if (objectiveWeights.size() >= 2)
                    {
                        resultModificationFactor[0] = objectiveWeights[1]; // bp
                        resultModificationFactor[1] = objectiveWeights[0]; // sd
                    }
                    if (objectiveWeights.size() >= 3)
                    {
                        resultModificationFactor[2] = objectiveWeights[2]; // dl
                    }
                }
                title = titleBase + "_weighted_" + std::to_string(interationNr) + titleAddition;
            }
            else
            {
                // Non-weighted: pick the active objective for the current iteration.
				std::string activeToken = getActiveObjective();

                if (activeToken == "sd")
                {
                    resultModificationFactor[1] = 1.0; // SD only
                    title = titleBase + "_sd_" + std::to_string(interationNr) + titleAddition;
                }
                else if (activeToken == "bp")
                {
                    resultModificationFactor[0] = 1.0; // BP only
                    title = titleBase + "_bp_" + std::to_string(interationNr) + titleAddition;
                }
                else if (activeToken == "dl")
                {
                    resultModificationFactor[2] = 1.0; // DL only
                    title = titleBase + "_dl_" + std::to_string(interationNr) + titleAddition;
                }
                else
                {
                    std::stringstream errorMessage;
                    errorMessage << "Error, active objective token not recognized: " << activeToken << "\n"
                                 << "(bso/optimizers/scdp/scdp_optimizer.cpp::visualizeMsw function)\n";
                    throw std::invalid_argument(errorMessage.str());
                }
            }
			
            // Optionally write a colour-settings file so that the same
            // performance visualization can be reconstructed later without
            // recomputing performances. The file name is derived from the
            // current title for traceability.
            if (mConfig.getMswWriteFile())
            {
                std::string safeTitle = title;
                // Replace spaces with underscores to avoid path issues.
                std::replace(safeTitle.begin(), safeTitle.end(), ' ', '_');
                std::string fileName = safeTitle + "_colors.txt";
                bso::visualization::write_msN_performance_visualization_to_file(fileName, performances, resultModificationFactor, transparency);
                mOutputManager.out("Wrote msw performance colour-settings file: " + fileName, true, true, mConfig.getVerbose());
            }

            bso::visualization::visualize(performances, resultModificationFactor, transparency, title, 4.0);
            mOutputManager.out("Sent ms performances models to visualization",true,true,mConfig.getVerbose());
            break;
        }
    }
} // visualizeMsw

void scdp_optimizer::runSplitDeleteOnlyIterations()
{
    while (mCurrentIteration < mConfig.getIterations())
    {
        runSplitDeleteIteration(mConfig.getModification());
        // incrementing the iteration number is done within the runSplitDeleteIteration function.
    }
    mOutputManager.out("\nCompleted all split-delete iterations", true, true, mConfig.getVerbose());
} // runSplitDeleteOnlyIterations()

void scdp_optimizer::runModificationOnlyIterations()
{
    while (mCurrentIteration < mConfig.getIterations())
    {
        runModificationIteration(mConfig.getModification());
        // incrementing the iteration number is done within the runModificationIteration function.
    } 
    mOutputManager.out("\nCompleted all modification iterations", true, true, mConfig.getVerbose());
} // runModificationOnlyIterations()

void scdp_optimizer::runAlternatingIterations()
{
    while (mCurrentIteration < mConfig.getIterations())
    {
        if (mCurrentIteration % 2 == 0)
        {
            runSplitDeleteIteration(mConfig.getModification());
            // incrementing the iteration number is done within the runSplitDeleteIteration function.
        }
        else
        {
            runModificationIteration(mConfig.getModification());
            // incrementing the iteration number is done within the runModificationIteration function.
        }
    }
    mOutputManager.out("\nCompleted all alternating iterations", true, true, mConfig.getVerbose());
} // runAlternatingIterations()

}}} // namespace bso::optimizers::scdp

#endif // SCDP_OPTIMIZER_CPP