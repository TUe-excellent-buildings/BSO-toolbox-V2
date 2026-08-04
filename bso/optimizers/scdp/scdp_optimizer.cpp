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
	if(mConfig.getLogMemoryUsage()){mOutputManager.log_memory_usage("after initializeDesign");}
}

void scdp_optimizer::getDesignResults()
{   
    if(mConfig.getLogMemoryUsage()){mOutputManager.log_memory_usage("start getDesignResults");}
    try
	{
        // Step 1, add object to store the performance of the current design
        // Four dimensions are used: sd, bp, dl (ADFbuilding), dl (score).
        // Initialize all entries with NaN; only active simulations will
        // overwrite their respective dimensions.
		mDesignPerformances.push_back(utilities::data_point(4));
        for (unsigned int idx = 0; idx < 4; ++idx)
        {
            mDesignPerformances.back()(idx) = std::numeric_limits<double>::quiet_NaN();
        }
        
        // Step 2, generate SD and BP models and evaluate them.
        std::vector<spatial_design::cf_building> cf;
        if(mConfig.getRunDLSimulation()){cf.push_back(spatial_design::cf_building(mMSDesigns.back(), mConfig.getDLSettingsFile(), 1e-6));}
        else {cf.push_back(spatial_design::cf_building(mMSDesigns.back(),1e-6));}
        mOutputManager.out("Initialized conformal model",true,true,mConfig.getVerbose());

        // Step 3, generate grammar
        grammar::grammar grm(cf.back());
        mOutputManager.out("Initialized grammar",true,true,mConfig.getVerbose());

        // Step 4, get data for export and analysis
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
            // Step 3, generate DL
            // Build an explicit folder path for this daylight run so
            // that all Radiance files live inside the SCDP result
            // directory, grouped per iteration.
            const std::string& resultDir = mOutputManager.getResultDir();
            std::string dlFolder = resultDir + "/Iteration_" + std::to_string(mCurrentIteration);
            mDLModelLast.clear(); // Clear the vector to free memory from the previous iteration's model before generating a new one, since DL models can be quite large and we only need to keep the latest one for the next iteration's modifications.
            mDLModelLast.push_back(bso::daylight::dl_model(grm.dl_grammar<grammar::DEFAULT_DL_GRAMMAR>(mConfig.getDLSettingsFile(), dlFolder)));
            mOutputManager.out("Generated daylight model",true,true,mConfig.getVerbose(), true);
            
            // Step 4, evaluate DL and store results
            if(mConfig.getLogMemoryUsage()){mOutputManager.log_memory_usage("pre DL analysis");}
            mDLModelLast.back().analyze();
            if(mConfig.getLogMemoryUsage()){mOutputManager.log_memory_usage("post SD analysis");}
            mOutputManager.out("Evaluated daylight model, ADFbuilding: ",false,false,mConfig.getVerbose());
            dlResult1 = mDLModelLast.back().getTotalResults().mADFBuilding; 
            mOutputManager.out(dlResult1,false,false,mConfig.getVerbose());
            mOutputManager.out(", mBS-EN score: ",true,true,mConfig.getVerbose(), true);
            dlResult2 = mDLModelLast.back().getTotalResults().mTotalScore;
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
            // Step 3, generate SD
            mSDModelLast.clear(); // Clear the vector to free memory from the previous iteration's model before generating a new one, since SD models can be quite large and we only need to keep the latest one for the next iteration's modifications.
            mSDModelLast.push_back(bso::structural_design::sd_model(grm.sd_grammar<grammar::DEFAULT_SD_GRAMMAR>(mConfig.getSDSettingsFile())));
            mOutputManager.out("Generated structural model",true,true,mConfig.getVerbose());
            
            // Step 4, evaluate SD and store results
            if(mConfig.getLogMemoryUsage()){mOutputManager.log_memory_usage("pre SD analysis");}
            mSDModelLast.back().analyze();
            if(mConfig.getLogMemoryUsage()){mOutputManager.log_memory_usage("post SD analysis");}
            mOutputManager.out("Evaluated structural model: ",false,false,mConfig.getVerbose());
            sdResult = mSDModelLast.back().getTotalResults().mTotalStrainEnergy;
            mSdTotalLoad.push_back(mSDModelLast.back().getTotalLoad());
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
            // Step 5, generate BP
            mBPModelLast.clear(); // Clear the vector to free memory from the previous iteration's model before generating a new one, since BP models can be quite large and we only need to keep the latest one for the next iteration's modifications.
            mBPModelLast.push_back(bso::building_physics::bp_model(grm.bp_grammar<grammar::DEFAULT_BP_GRAMMAR>(mConfig.getBPSettingsFile())));
            mOutputManager.out("Generated building physics model",true,true,mConfig.getVerbose());
            
            // Step 6, evaluate BP and store results
            if(mConfig.getLogMemoryUsage()){mOutputManager.log_memory_usage("pre BP analysis");}
            mBPModelLast.back().simulatePeriods("runge_kutta_dopri5",1e-3,1e-3);
            if(mConfig.getLogMemoryUsage()){mOutputManager.log_memory_usage("post BP analysis");}
            mOutputManager.out("Evaluated building physics model: ",false,false,mConfig.getVerbose());
            bpResult = mBPModelLast.back().getTotalResults().mTotalEnergy;
            mOutputManager.out(bpResult,true,true,mConfig.getVerbose());
            mDesignPerformances.back()(1) = bpResult;
        }
        else
        {
            // No additional BP-specific vectors to push (unlike SD which has mSdTotalLoad)
            // and keep BP performance dimension as NaN.
            mOutputManager.out("Skipped BP simulation (not in --sim list)",true,true,mConfig.getVerbose());
        }
        
        // Step 4, export visualizations and visualization text files if specified in config
        bool mswVisualized = false;
        // flags to ensure visualization text file is only written once per iteration.
        bool dlVisualAlreadyWritten = false; 
        bool sdVisualAlreadyWritten = false; // flag to ensure SD visualization text file is only written once per iteration, even if multiple SD-related visualizations are specified in the config
        bool bpVisualAlreadyWritten = false; // flag to ensure BP visualization text file is only written once per iteration, even if multiple BP-related visualizations are specified in the config
        if (!mConfig.getVisualizations().empty()) for (const auto& visOption : mConfig.getVisualizations())
        {
            if (visOption == "ms") visualization::visualize(mMSDesigns.back(),"","ms_N_building",4.0);
            if (visOption == "sg") visualization::visualize(cf.back(),"space_at_ground");
            if (visOption == "de") visualization::visualize(cf.back(),"delaunay");
            if (visOption == "bo") visualization::visualize(cf.back(),"boundarylines");
            if (visOption == "tr") visualization::visualize(cf.back(),"trirec");
            if (visOption == "tp") visualization::visualize(cf.back(),"triangular_prism");
            if (visOption == "sd" && mConfig.getRunSDSimulation()) 
            {
                visualization::visualize(mSDModelLast.back());
                if(mConfig.getSdVisualWriteFile()) sdVisualAlreadyWritten = true;
            }
            if (visOption == "sdC" && mConfig.getRunSDSimulation()) 
            {
                visualization::visualize(mSDModelLast.back(), "component");
                if(mConfig.getSdVisualWriteFile()) sdVisualAlreadyWritten = true;
            }
            if (visOption == "fe" && mConfig.getRunSDSimulation()) 
            {
                visualization::visualize(mSDModelLast.back(), "element");
                if(mConfig.getSdVisualWriteFile()) sdVisualAlreadyWritten = true;
            }
            if (visOption == "fe" && mConfig.getRunSDSimulation())
            {
                visualization::visualize(mSDModelLast.back(), "strain_energy", true);
                if(mConfig.getSdVisualWriteFile()) sdVisualAlreadyWritten = true;
            }
            if (visOption == "bp" && mConfig.getRunBPSimulation()) 
            {
                visualization::visualize(mBPModelLast.back());
                if(mConfig.getBpVisualWriteFile()) bpVisualAlreadyWritten = true;
            }
            if (visOption == "dl" && mConfig.getRunDLSimulation())
            {
                visualization::visualize(mDLModelLast.back(), "", "dl_model", mConfig.getDlVisualWriteFile(), mOutputManager.getVisualizationSubDir() + "/dl_iteration_" + std::to_string(mCurrentIteration) + ".txt");
                if (mConfig.getDlVisualWriteFile()) dlVisualAlreadyWritten = true;
            }
            if (visOption == "dlG" && mConfig.getRunDLSimulation())
            {
                visualization::visualize(mDLModelLast.back(), "grid", "dl_model", mConfig.getDlVisualWriteFile(), mOutputManager.getVisualizationSubDir() + "/dl_iteration_" + std::to_string(mCurrentIteration) + ".txt");
                if (mConfig.getDlVisualWriteFile()) dlVisualAlreadyWritten = true;
            }
            if (visOption == "dlD" && mConfig.getRunDLSimulation()) visualization::visualize(mDLModelLast.back(), "DF");
            if (visOption == "dlI" && mConfig.getRunDLSimulation()) visualization::visualize(mDLModelLast.back(), "Horizontal_Illuminance");
            if (visOption == "dlADF" && mConfig.getRunDLSimulation()) visualization::visualize(mDLModelLast.back(), "ADF");
            if (visOption == "dlNEN" && mConfig.getRunDLSimulation()) visualization::visualize(mDLModelLast.back(), "NEN");

            if (visOption == "msw" && mCurrentIteration == mConfig.getIterations()) mswVisualized = true; // set a flag to indicate that the final MSW visualization has been done, so that it is not generated again in the outputResults() function after the optimization process is finished.
        }
        if(mCurrentIteration == mConfig.getIterations())
        {
            if (mswVisualized || mConfig.getMswVisualWriteFile()) 
            { // Normally generated right before modification. However, in last iteration there is no modification, but we still want to visualize the final MSW with performance coloring.
                // Get performances per space
                std::vector<utilities::data_point> performances;
                structural_design::sd_model dummySD;
                building_physics::bp_model dummyBP;
                bso::daylight::dl_model dummyDL;
                structural_design::sd_model& sdRef = mConfig.getRunSDSimulation() ? mSDModelLast.back() : dummySD;
                building_physics::bp_model& bpRef = mConfig.getRunBPSimulation() ? mBPModelLast.back() : dummyBP;
                bso::daylight::dl_model& dlRef = mConfig.getRunDLSimulation() ? mDLModelLast.back() : dummyDL;
                performance_evaluator::getPerformancePerSpace(performances, mMSDesigns.back(), sdRef, bpRef, dlRef, mConfig.getModification(), mConfig.getPerformanceDivision(), mOutputManager, mConfig.getVerbose(), mConfig.getRunSDSimulation(), mConfig.getRunBPSimulation(), mConfig.getRunDLSimulation());

                // Step 3, normalize the data points
                utilities::data_point utopianPoint;
                utilities::data_point distopianPoint;
                performance_evaluator::normalizeData(performances, utopianPoint, distopianPoint, mOutputManager, mConfig.getVerbose());

                // Visualize the performance of spaces
                visualizeMsw(mMSDesigns.back(), performances, mCurrentIteration, mConfig.getObjectiveWeights());
            }
        }
        if (mConfig.getDlVisualWriteFile() && mConfig.getRunDLSimulation() && !dlVisualAlreadyWritten)
        {
            const std::string dlVisFileName = mOutputManager.getVisualizationSubDir() + "/dl_iteration_" + std::to_string(mCurrentIteration) + ".txt";
            bso::visualization::DL_Model dlVisualFileModel(mDLModelLast.back(), "", "dl_model");
            dlVisualFileModel.writeToFile(dlVisFileName);
            bso::visualization::DL_Model dlVisualGridFileModel(mDLModelLast.back(), "grid", "dl_model");
            dlVisualGridFileModel.writeToFile(dlVisFileName);
            mOutputManager.out("Wrote DL visualization text file (without sending DL viewport)", true, true, mConfig.getVerbose());
        }
        if (mConfig.getSdVisualWriteFile() && mConfig.getRunSDSimulation() && !sdVisualAlreadyWritten)
        {
            // const std::string sdVisFileName = mOutputManager.getVisualizationSubDir() + "/sd_iteration_" + std::to_string(mCurrentIteration) + ".txt";
            // bso::visualization::SD_Model sdVisualFileModel(mSDModelLast.back(), "", "sd_model");
            // sdVisualFileModel.writeToFile(sdVisFileName);
            // mOutputManager.out("Wrote SD visualization text file (without sending SD viewport)", true, true, mConfig.getVerbose());
        }
        if (mConfig.getBpVisualWriteFile() && mConfig.getRunBPSimulation() && !bpVisualAlreadyWritten)
        {
            // const std::string bpVisFileName = mOutputManager.getVisualizationSubDir() + "/bp_iteration_" + std::to_string(mCurrentIteration) + ".txt";
            // bso::visualization::BP_Model bpVisualFileModel(mBPModelLast.back(), "", "bp_model");
            // bpVisualFileModel.writeToFile(bpVisFileName);
            // mOutputManager.out("Wrote BP visualization text file (without sending BP viewport)", true, true, mConfig.getVerbose());
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
    if(mConfig.getLogMemoryUsage()){mOutputManager.log_memory_usage("after getDesignResults");}
} // getDesignResults()

std::vector<utilities::data_point> scdp_optimizer::getIntermediateResults(spatial_design::ms_N_building& ms, const std::string& objective)
{
    std::vector<utilities::data_point> intermediateSpaceResults;
    try
    {
        // Step 1, generate SD and BP models and evaluate them.
        // Use the same setup as in getDesignResults: when DL simulation
        // is enabled, construct the conformal model with the DL settings
        // file so that window definitions are available; otherwise use
        // the simpler constructor.
        std::vector<spatial_design::cf_building> cf;
        if (mConfig.getRunDLSimulation()){cf.push_back(spatial_design::cf_building(ms, mConfig.getDLSettingsFile(), 1e-6));}
        else{cf.push_back(spatial_design::cf_building(ms, 1e-6));}
        mOutputManager.out("Initialized conformal model",true,true,mConfig.getVerbose());

        // Step 2, generate grammar
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
            // Step 3, generate DL
            const std::string& resultDir = mOutputManager.getResultDir();
            std::string dlFolder = resultDir + "/Iteration_" + std::to_string(mCurrentIteration);
            bso::daylight::dl_model dl = grm.dl_grammar<grammar::DEFAULT_DL_GRAMMAR>(mConfig.getDLSettingsFile(), dlFolder);
            mOutputManager.out("Generated intermediate daylight model",true,true,mConfig.getVerbose());
            if(objective != "dl") mOutputManager.out("Note: this is to insert the window definition in sd or bp analysis",true,true,mConfig.getVerbose());
            if(objective == "dl")
            {
                // Step 4, evaluate DL and store results
                dl.analyze();
                mOutputManager.out("Evaluated intermediate daylight model, ADFbuilding: ",false,false,mConfig.getVerbose());
                dlResult = dl.getTotalResults().mADFBuilding;
                mOutputManager.out(dlResult,true,true,mConfig.getVerbose());
                performance_evaluator::getPerformancePerSpace(intermediateSpaceResults, ms, dummySD, dummyBP, dl, objective, mConfig.getPerformanceDivision(), mOutputManager, mConfig.getVerbose(), sdAvailable, bpAvailable, dlAvailable);
            }
        }
        if(objective == "sd" && sdAvailable)
        {
            // Step 3, generate SD
            bso::structural_design::sd_model sd = grm.sd_grammar<grammar::DEFAULT_SD_GRAMMAR>(mConfig.getSDSettingsFile());
            mOutputManager.out("Generated intermediate structural model",true,true,mConfig.getVerbose());
            
            // Step 4, evaluate SD and store results
            sd.analyze();
            mOutputManager.out("Evaluated intermediate structural model: ",false,false,mConfig.getVerbose());
            sdResult = sd.getTotalResults().mTotalStrainEnergy;
            mOutputManager.out(sdResult,true,true,mConfig.getVerbose());
            performance_evaluator::getPerformancePerSpace(intermediateSpaceResults, ms, sd, dummyBP, dummyDL, objective, mConfig.getPerformanceDivision(), mOutputManager, mConfig.getVerbose(), sdAvailable, bpAvailable, dlAvailable);
        }
        if(objective == "bp" && bpAvailable)
        {
            // Step 3, generate BP
            bso::building_physics::bp_model bp = grm.bp_grammar<grammar::DEFAULT_BP_GRAMMAR>(mConfig.getBPSettingsFile());
            mOutputManager.out("Generated intermediate building physics model",true,true,mConfig.getVerbose());
            
            // Step 4, evaluate BP and store results
            bp.simulatePeriods("runge_kutta_dopri5",1e-3,1e-3);
            mOutputManager.out("Evaluated intermediate building physics model: ",false,false,mConfig.getVerbose());
            bpResult = bp.getTotalResults().mTotalEnergy;
            mOutputManager.out(bpResult,true,true,mConfig.getVerbose());
            performance_evaluator::getPerformancePerSpace(intermediateSpaceResults, ms, dummySD, bp, dummyDL, objective, mConfig.getPerformanceDivision(), mOutputManager, mConfig.getVerbose(), sdAvailable, bpAvailable, dlAvailable);
        }
        if(objective == "weighted")
        {
            // Step 3, generate DL
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
                // Step 4, evaluate DL and store results
                dl.analyze();
                mOutputManager.out("Evaluated intermediate daylight model, ADFbuilding: ",false,false,mConfig.getVerbose());
                dlResult = dl.getTotalResults().mADFBuilding;
                mOutputManager.out(dlResult,true,true,mConfig.getVerbose());
            }
            if(sdAvailable)
            {
                // Step 4, evaluate SD and store results
                sd.analyze();
                mOutputManager.out("Evaluated intermediate structural model: ",false,false,mConfig.getVerbose());
                sdResult = sd.getTotalResults().mTotalStrainEnergy;
                mOutputManager.out(sdResult,true,true,mConfig.getVerbose());
            }
            if(bpAvailable)
            {
                // Step 4, evaluate BP and store results
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

        // Step 2, get performances per space
        std::vector<utilities::data_point> performances;
        structural_design::sd_model dummySD;
        building_physics::bp_model dummyBP;
        bso::daylight::dl_model dummyDL;
        structural_design::sd_model& sdRef = mConfig.getRunSDSimulation() ? mSDModelLast.back() : dummySD;
        building_physics::bp_model& bpRef = mConfig.getRunBPSimulation() ? mBPModelLast.back() : dummyBP;
        bso::daylight::dl_model& dlRef = mConfig.getRunDLSimulation() ? mDLModelLast.back() : dummyDL;
        performance_evaluator::getPerformancePerSpace(performances, newMS, sdRef, bpRef, dlRef, modificationType, mConfig.getPerformanceDivision(), mOutputManager, mConfig.getVerbose(), mConfig.getRunSDSimulation(), mConfig.getRunBPSimulation(), mConfig.getRunDLSimulation());

        // Step 3, normalize the data points, and in doing so ensure that all objectives are oriented in the same direction (i.e. lower is better)
        utilities::data_point utopianPoint;
        utilities::data_point distopianPoint;
        performance_evaluator::normalizeData(performances, utopianPoint, distopianPoint, mOutputManager, mConfig.getVerbose());

        // Visualize the performance of spaces
        visualizeMsw(newMS, performances, mCurrentIteration, mConfig.getObjectiveWeights());

        // Step 4, delete spaces
        deleteSpaces(newMS, performances, modificationType, distopianPoint);

        // Step 6, Enforce constrains on BSD
        // Note: Can only reduce nr. of spaces, so recoverNrSpaces is not needed here,
        // as splitSpaces will be called later to recover the initial number of spaces.
        enforceConstraints(newMS); 
        
        // Step 7, scale spaces in x and y direction to recover initial conditions, i.e. floor area
        scaleDesign(newMS);

        // Step 8, split spaces to recover initial number of spaces conditions,
        // (checking x first and only splitting in z-direction if the space is a minimum of 6000 in height)
        splitSpaces(newMS, performances, modificationType, mCurrentIteration, mConfig.getObjectiveWeights());
        
        // Step 9, store the new design
        mMSDesigns.push_back(newMS);
        mOutputManager.out("Modified building spatial design into a new one",true,true,mConfig.getVerbose());
        mCurrentIteration++;
        
        // Step 10, get results for the new design
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
        
        // Step 1, create a new MS model
        spatial_design::ms_N_building newMS = mMSDesigns.back();

        // Step 2, get performances per space
        std::vector<utilities::data_point> performances;
        structural_design::sd_model dummySD;
        building_physics::bp_model dummyBP;
        bso::daylight::dl_model dummyDL2;
        structural_design::sd_model& sdRef = mConfig.getRunSDSimulation() ? mSDModelLast.back() : dummySD;
        building_physics::bp_model& bpRef = mConfig.getRunBPSimulation() ? mBPModelLast.back() : dummyBP;
        bso::daylight::dl_model& dlRef2 = mConfig.getRunDLSimulation() ? mDLModelLast.back() : dummyDL2;
        performance_evaluator::getPerformancePerSpace(performances, newMS, sdRef, bpRef, dlRef2, modificationType, mConfig.getPerformanceDivision(), mOutputManager, mConfig.getVerbose(), mConfig.getRunSDSimulation(), mConfig.getRunBPSimulation(), mConfig.getRunDLSimulation());

        // Step 3, normalize the data points, and in doing so ensure that all objectives are oriented in the same direction (i.e. lower is better)
        utilities::data_point utopianPoint;
        utilities::data_point distopianPoint;
        performance_evaluator::normalizeData(performances, utopianPoint, distopianPoint, mOutputManager, mConfig.getVerbose());

        // Step 4, get data for 'worstAdapt' method if selected, 
        // which determines the add method based on the location of the worst performing spaces.
        bso::utilities::geometry::vector worstSpacesDirection = {0,0,0};
        bso::utilities::geometry::vertex centreBuilding = {0,0,0};
        bso::utilities::geometry::vertex worstSpaceCentre = {0,0,0};
        if(mConfig.getAddMethod() == "worstAdapt")
        {
            // Step 4a, sort performance data points from highest to lowest to determine where the worst is located for space removal later
            performance_evaluator::sortDataPointsLowestToHighest(performances, modificationType,
                    mConfig.getModificationSequence(), mCurrentIteration, mConfig.getObjectiveWeights(),
                    mConfig.getWeightedModification(), mOutputManager, mConfig.getVerbose());

            // Step 4b, determine the amount of worst spaces on which the add-method will be determined
            std::size_t spaceAmountRemoval= round(mNrSpaces*mConfig.getSpaceModifyWorstRatio()); 
            if(spaceAmountRemoval < 1){spaceAmountRemoval = 1;}

            // Step 4c, determine the centre of to worst spaces:
            const std::size_t safeDeleteGuard = std::min(spaceAmountRemoval, performances.size());
            const std::size_t startIdx = performances.size() - safeDeleteGuard;
            for (std::size_t idx = startIdx; idx < performances.size(); ++idx)
            {
                worstSpaceCentre += performances[idx].getMSpacePtrs().back()->getGeometry().getCenter(); 
            }
            worstSpaceCentre = worstSpaceCentre/spaceAmountRemoval;
            centreBuilding = newMS.getCentreBuilding();
            centreBuilding.z()=0; // centre of the building should always be defined on z=0

            // Step 4d, determine the worst spaces direction, i.e. where the worst spaces are located in relation 
            // to the centre of the building for later use.
            worstSpacesDirection = worstSpaceCentre - centreBuilding;
        }

        // Step 5, visualize the performance of spaces
        visualizeMsw(newMS, performances, mCurrentIteration, mConfig.getObjectiveWeights(), "_bases_Del_Mod_decision");
        
        // Step 6, delete spaces
        deleteSpaces(newMS, performances, modificationType, distopianPoint);
        
        // Step 7, Enforce constrains on BSD
        enforceConstraints(newMS);

        // Step 8, perform the specified modification methods
        if(mConfig.getAddMethod() == "worstAdapt")
        {
            // Step 8a, execute either addElevation or shiftFillPlaneCentre depending on the worst spaces direction,
            // i.e. if the worst spaces are located more in the horizontal direction, execute shift the fill plane centre,
            // and if they are located more in the vertical direction, execute add elevation.
            mOutputManager.out("Selected modification method in 'worstAdapt' Iteration, according to following parameters: ",false,false,mConfig.getVerbose());
            mOutputManager.out("\n\tworstSpaceCentre is: ",false,false,mConfig.getVerbose());mOutputManager.out(worstSpaceCentre,false,false,mConfig.getVerbose());
            mOutputManager.out("\n\tcentreBuilding is: ",false,false,mConfig.getVerbose());mOutputManager.out(centreBuilding,false,false,mConfig.getVerbose());
            mOutputManager.out("\n\tworstSpacesDirection.getHeading().second: ",false,false,mConfig.getVerbose());mOutputManager.out(worstSpacesDirection.getHeading().second,true,false,mConfig.getVerbose());

            // Step 8b, execute addElevation
            if(worstSpacesDirection.getHeading().second < mConfig.getInclinationSwitch())
            {
                mOutputManager.out("Selected addElevation in 'worstAdapt' Iteration: ",false,false,mConfig.getVerbose()); mOutputManager.out(mCurrentIteration,true,false,mConfig.getVerbose());
                newMS.duplicateElevation();
                mOutputManager.out("addElevation done",true,true,mConfig.getVerbose()); 
            }
            // Step 8b, execute shiftFillPlaneCentre
            else if(worstSpacesDirection.getHeading().second > mConfig.getInclinationSwitch())
            {
                mOutputManager.out("Selected shiftFillPlaneCentre in 'worstAdapt' iteration: ",false,false,mConfig.getVerbose()); mOutputManager.out(mCurrentIteration,true,false,mConfig.getVerbose());
                
                // Alternate the direction of the shift fill plane depending on mCurrentIteration
                bool shiftInMostHeadingDirection = false;
                if(mCurrentIteration == 0) {shiftInMostHeadingDirection = true;}
                else if(mCurrentIteration % 2) {shiftInMostHeadingDirection = true;}
                
                double averageWidthSpace = newMS.getAverageHorizontalRibSize();
                try
                {
                    newMS.shiftFillPlaneCentre(averageWidthSpace, shiftInMostHeadingDirection, 6, mConfig.getVerbose());
                }
                catch(const std::exception& e)                
                {
                    mOutputManager.out("Error during shiftFillPlaneCentre: ",false,false,true);
                    mOutputManager.out(e.what(),true,true,mConfig.getVerbose());
                    // Continue run as the nr. of spaces will be corrected later in this method.
                    // Also, later iterations may still be successful and we want to gather as much data as possible,
                    // but log the error for debugging and future improvement of the method.
                }
                mOutputManager.out("shiftFillPlaneCentre done ",true,true,mConfig.getVerbose());
            }
        }
        else if(mConfig.getAddMethod() == "shiftfill")
        {
            // Step 8a, execute shiftFillPlaneCentre
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
            // Step 8a, execute addElevation
            mOutputManager.out("Start addElevation in 'elevation' Iteration: ",false,false,mConfig.getVerbose()); mOutputManager.out(mCurrentIteration,true,false,mConfig.getVerbose());
            newMS.duplicateElevation();
            mOutputManager.out("addElevation done ",true,true,mConfig.getVerbose());
        }

        // Step 13, delete the spaces which are defined inside another space, 
            // as this is not allowed and may occur in the extrude or shiftfillplane
            // functions although carefully designed to not do this, but due to the
            // complexity of the geometry it can still occur. This is done at the end
            // of the iteration to not interfere with the other modification Steps, 
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
            if (mConfig.getMsFiles())
            {
                std::stringstream fileNameStream;
                fileNameStream << mOutputManager.getVisualizationSubDir() << "ms_"
                                << mCurrentIteration << "_red_overlapping_spaces_before_removal";
                std::string fileName = fileNameStream.str();
                newMS.writeToFile(fileName);
            }

            for(const auto& space: overlappingSpaces)
            {
                newMS.deleteSpace(space);
            }
            mOutputManager.out("\tDeleted overlapping spaces.",true,true,mConfig.getVerbose());
        }     
   
        // Step 9, Enforce constrains on BSD
        enforceConstraints(newMS);  

        // Step 10, Export after_modification_ms visualizations
        if (!mConfig.getVisualizations().empty()) for (const auto& visOption : mConfig.getVisualizations())
        {
            if (visOption == "ms") visualization::visualize(newMS,"","After_del_Mod_ms",4.0);
        }
        if (mConfig.getMsFiles())
        {
            std::stringstream fileNameStream;
            fileNameStream << mOutputManager.getVisualizationSubDir() << "ms_"
                            << mCurrentIteration << "_After_deleteSpace&addModification_ms";
            std::string fileName = fileNameStream.str();
            newMS.writeToFile(fileName);
        }

        // Step 11, recover number of spaces to match the initial design.
        recoverNrSpaces(newMS, performances, mCurrentIteration, mConfig.getObjectiveWeights(), modificationType);

        // Step 16, scale spaces in x and y direction to recover initial conditions, i.e. floor area
        scaleDesign(newMS);

        // Step 12, store the new design
        mMSDesigns.push_back(newMS);
        mOutputManager.out("Modified building spatial design into a new one",true,true,mConfig.getVerbose());
        mCurrentIteration++;
        
        // Step 13, get results for the new design
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

// Delete spaces from the building spatial design based on the specified delete method.
// Note: the space data needs to be normalized before calling this function.
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
        // Step 4, cluster the data
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
        
        // Step 5, select worst performing cluster and remove these spaces
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
        // Step 4, sort data
        performancesRemoveSpace = performances;
        performance_evaluator::sortDataPointsLowestToHighest(performancesRemoveSpace, mConfig.getModification(),
            mConfig.getModificationSequence(), mCurrentIteration, mConfig.getObjectiveWeights(),
            mConfig.getWeightedModification(), mOutputManager, mConfig.getVerbose());
        mOutputManager.out("Sort data points for space removal",true,true,mConfig.getVerbose());

        // Step 5, determine space removal amount
        std::size_t spaceAmountRemoval;
        if(mConfig.getAmountOfSpaceRemoval() > 0){spaceAmountRemoval = mConfig.getAmountOfSpaceRemoval();} 
        else 
        {
            spaceAmountRemoval = std::floor(performancesRemoveSpace.size()*mConfig.getSpaceRemoveRatio());
            if(spaceAmountRemoval < 1){spaceAmountRemoval = 1;}
        }

        // Step 6, select worst performing spaces and remove these spaces
        const std::size_t safeDeleteGuard = std::min(spaceAmountRemoval, performancesRemoveSpace.size());
        const std::size_t startIdx = performancesRemoveSpace.size() - safeDeleteGuard;
        for (std::size_t idx = startIdx; idx < performancesRemoveSpace.size(); ++idx)
        {
            ms.deleteSpace(performancesRemoveSpace[idx].getMSpacePtrs().back());
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

void scdp_optimizer::recoverNrSpaces(spatial_design::ms_N_building& ms, 
                      std::vector<utilities::data_point>& performances, 
                      const int iteration, 
                      const std::vector<double>& objectiveWeights,
                      const std::string& modificationType)
{
    // If already at target size there is nothing to recover.
    if(ms.getSpacePtrs().size() == mNrSpaces) return;

    mOutputManager.out("Start recovering number of spaces",true,true,mConfig.getVerbose());
    // If more spaces are present than target, remove exactly the required amount.
    // Intermediate results are computed only once because this is an expensive call.
    if(ms.getSpacePtrs().size() > mNrSpaces)
    {
        const std::size_t sizeBeforeDelete = ms.getSpacePtrs().size();
        const std::size_t spacesToRemove = sizeBeforeDelete - mNrSpaces;

        // Only generate intermediate results for the objective guiding the deletion, 
        // as this is the only relevant objective for the deletion and simulation are expensive to run.
        std::string activeObjective = getActiveObjective();
        std::vector<utilities::data_point> intermediateResults = getIntermediateResults(ms, activeObjective);

        if (intermediateResults.empty())
        {
            std::stringstream errorMessage;
            errorMessage << "Error, recoverNrSpaces could not compute intermediate space performances while deleting to target.\n"
                         << "(bso/optimizers/scdp/scdp_optimizer.cpp)." << std::endl;
            throw std::runtime_error(errorMessage.str());
        }

        utilities::data_point utopianPoint;
        utilities::data_point distopianPointIntermediate;
        performance_evaluator::normalizeData(intermediateResults, utopianPoint, distopianPointIntermediate, mOutputManager, mConfig.getVerbose());
        visualizeMsw(ms, intermediateResults, iteration, objectiveWeights, "_retrieve_nr_space_delete");

        // Recovery deletion intentionally always uses the worst-performance strategy,
        // independent of the configured delete method.
        performance_evaluator::sortDataPointsLowestToHighest(intermediateResults, modificationType,
            mConfig.getModificationSequence(), iteration, objectiveWeights,
            mConfig.getWeightedModification(), mOutputManager, mConfig.getVerbose());

        const std::size_t removable = std::min(spacesToRemove, intermediateResults.size());
        const std::size_t startIdx = intermediateResults.size() - removable;
        for (std::size_t idx = startIdx; idx < intermediateResults.size(); ++idx)
        {
            ms.deleteSpace(intermediateResults[idx].getMSpacePtrs().back());
        }
        mOutputManager.out("Removed worst performing spaces in building spatial design",true,true,mConfig.getVerbose());

        if(ms.getSpacePtrs().size() > mNrSpaces)
        {
            std::stringstream errorMessage;
            errorMessage << "Error, recoverNrSpaces could not reduce number of spaces from " << sizeBeforeDelete
                         << " to target " << mNrSpaces << " in a single recovery pass.\n"
                         << "Please adjust delete settings (e.g. delete method/ratio/amount).\n"
                         << "(bso/optimizers/scdp/scdp_optimizer.cpp)." << std::endl;
            throw std::runtime_error(errorMessage.str());
        }

        if(ms.getSpacePtrs().size() < mNrSpaces)
        {
            std::stringstream errorMessage;
            errorMessage << "Error, recoverNrSpaces deleted too many spaces while recovering from " << sizeBeforeDelete
                         << " to target " << mNrSpaces << ".\n"
                         << "(bso/optimizers/scdp/scdp_optimizer.cpp)." << std::endl;
            throw std::runtime_error(errorMessage.str());
        }

        // If the number of spaces is now correct, then we still need to enforce constraints, as the deletion may have violated them.
        enforceConstraints(ms);
        // NOTE: As a result of 'enforceConstraints(ms);', more spaces, may have been deleted in the enforceConstraints method
        // But this will be picked up by the following if statement, which will then recover the number of spaces again, until the target is reached.
    }

    // If fewer spaces are present than target, split until we reach target.
    if(ms.getSpacePtrs().size() < mNrSpaces)
    {
        const std::size_t sizeBeforeSplit = ms.getSpacePtrs().size();

        // If split method is not performance, use already available performances, as these value's will not be used in the split method anyway.
        splitSpaces(ms, performances, modificationType, iteration, objectiveWeights);

        if(ms.getSpacePtrs().size() <= sizeBeforeSplit && ms.getSpacePtrs().size() < mNrSpaces)
        {
            std::stringstream errorMessage;
            errorMessage << "Error, recoverNrSpaces could not increase number of spaces from " << sizeBeforeSplit
                         << " while target is " << mNrSpaces << ".\n"
                         << "Please adjust split settings (e.g. split method).\n"
                         << "(bso/optimizers/scdp/scdp_optimizer.cpp)." << std::endl;
            throw std::runtime_error(errorMessage.str());
        }
    }
    mOutputManager.out("Finish recovering number of spaces",true,true,mConfig.getVerbose());
} // recoverNrSpaces

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
        performance_evaluator::sortDataPointsLowestToHighest(performancesRemoveSpace, modification,
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
void scdp_optimizer::visualizeMsw(const spatial_design::ms_N_building& ms,
    const std::vector<bso::utilities::data_point>& performances, 
    const int interationNr, const std::vector<double>& objectiveWeights, std::string titleAddition)
{
    // Mapping of data_point dimensions used in performances:
    //  index 0 -> BP objective
    //  index 1 -> SD objective
    //  index 2 -> DL objective (e.g. ADFbuilding utilized for performance decisions)

    bool visualizeMsw = std::find(mConfig.getVisualizations().begin(), mConfig.getVisualizations().end(), "msw") != mConfig.getVisualizations().end();
    
    if (visualizeMsw || mConfig.getMswVisualWriteFile())
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
        if (mConfig.getMswVisualWriteFile())
        {
            std::string safeTitle = title;
            // Replace spaces with underscores to avoid path issues.
            std::replace(safeTitle.begin(), safeTitle.end(), ' ', '_');
            std::string fileName = mOutputManager.getVisualizationSubDir() + safeTitle + "_colors.txt";
            // Write color file:
            bso::visualization::write_msN_performance_visualization_to_file(fileName, performances, resultModificationFactor, transparency);
            // Write accomanied ms_file:
            std::string msFileName = mOutputManager.getVisualizationSubDir() + safeTitle + "_ms.txt";
            ms.writeToFile(msFileName);
            mOutputManager.out("Wrote msw performance colour-settings file: " + fileName, true, true, mConfig.getVerbose());
        }
        if (visualizeMsw)
        {
            bso::visualization::visualize(performances, resultModificationFactor, transparency, title, 4.0);
            mOutputManager.out("Sent ms performances models to visualization",true,true,mConfig.getVerbose());
        }
    }
} // visualizeMsw

void scdp_optimizer::runSplitDeleteOnlyIterations()
{
    mOutputManager.startTimer("total_loop");
    while (mCurrentIteration < mConfig.getIterations())
    {
        mOutputManager.startTimer("iteration");
        runSplitDeleteIteration(mConfig.getModification());
        // incrementing the iteration number is done within the runSplitDeleteIteration function.
        mOutputManager.endTimer("iteration");
    }
    mOutputManager.endTimer("total_loop");
    mOutputManager.out("\nCompleted all split-delete iterations", true, true, mConfig.getVerbose());
} // runSplitDeleteOnlyIterations()

void scdp_optimizer::runModificationOnlyIterations()
{
    mOutputManager.startTimer("total_loop");
    while (mCurrentIteration < mConfig.getIterations())
    {
        mOutputManager.startTimer("iteration");
        runModificationIteration(mConfig.getModification());
        // incrementing the iteration number is done within the runModificationIteration function.
        mOutputManager.endTimer("iteration");
    }
    mOutputManager.endTimer("total_loop");
    mOutputManager.out("\nCompleted all modification iterations", true, true, mConfig.getVerbose());
} // runModificationOnlyIterations()

void scdp_optimizer::runAlternatingIterations()
{
    mOutputManager.startTimer("total_loop");
    while (mCurrentIteration < mConfig.getIterations())
    {
        mOutputManager.startTimer("iteration");
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
        mOutputManager.endTimer("iteration");
    }
    mOutputManager.endTimer("total_loop");
    mOutputManager.out("\nCompleted all alternating iterations", true, true, mConfig.getVerbose());
} // runAlternatingIterations()

}}} // namespace bso::optimizers::scdp

#endif // SCDP_OPTIMIZER_CPP