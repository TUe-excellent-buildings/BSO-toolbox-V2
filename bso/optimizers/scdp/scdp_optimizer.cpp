#ifndef SCDP_OPTIMIZER_CPP
#define SCDP_OPTIMIZER_CPP

#include <algorithm>
#include <cmath>
#include <chrono>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <bso/spatial_design/ms_N_building.hpp>
#include <bso/spatial_design/cf_building.hpp>
#include <bso/utilities/geometry.hpp>
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
    if (!mConfig.getVisualizations().empty()) visualization::initVisualization(mConfig.getArgc(), mConfig.getArgv());
	output_manager::out("Initialized visualization", true, true, mConfig.getVerbose());
    initializeDesign();
    getDesignResults();
}

void scdp_optimizer::runOptimization()
{
    // Run the optimization process for the specified number of iterations
    unsigned int iterations = mConfig.getIterations();
    if (iterations == 0)
    {
        std::stringstream errorMessage;
        errorMessage << "Error, expected a positive number of iterations, received 0.\n"
                     << "Please specify the number of iterations using the -i or --iterations argument.\n";
        throw std::invalid_argument(errorMessage.str());
    }

    if(mConfig.getResultDir())
    {
        std::string dir = "";
        mOutputManager.getSubResultDir(dir, mConfig.getInputFile(), mConfig.getModification(), mConfig.getMaxIterationsInSequence(), mConfig.getVerbose());
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
    OptimizationOutput output{
        mConfig,
        mMSDesigns,
        mDesignPerformances,
        axisDesignPerformances,
        sd_total_load,
        exteriorSurfaceArea,
        mConfig.getModification(),
        mConfig.getDesignName(),
        mConfig.getDeleteMethod(),
        mConfig.getMaxIterationsInSequence(),
        currentIteration,
        static_cast<unsigned int>(mConfig.getRngSeed())
    };
    
    mOutputManager.outputData(output);
}

void scdp_optimizer::initializeDesign()
{
    try
	{
		if (!mConfig.getInputFile().empty()) mMSDesigns.push_back(spatial_design::ms_N_building(mConfig.getInputFile()));
        nSpaces = mMSDesigns.back().getSpacePtrs().size();
        floorArea = mMSDesigns.back().getFloorArea();
        averageWidthSpace = mMSDesigns.back().getAverageHorizontalRibSize();
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
	output_manager::out("Initialized initial MS_N building model", true, true, mConfig.getVerbose());
}

void scdp_optimizer::getDesignResults()
{   
    try
	{
        if(mConfig.getAverageSpace())
        {
            mMSDesigns.back().enforceAverageSpaceSize();
            output_manager::out("Enforce averageSpaceSize", true, true, mConfig.getVerbose());
        }

        // add object to Store the performance of the current design
        designPerformances.push_back(utilities::data_point(2));
        
        // step 1, generate SD and BP models and evaluate them.
        spatial_design::cf_building cf(mMSDesigns.back(),1e-6);
        output_manager::output_manager::out("Initialized conformal model",true,true,mConfig.getVerbose());

        // step 2, generate grammar
        grammar::grammar grm(cf);
        output_manager::output_manager::out("Initialized grammar",true,true,mConfig.getVerbose());

        // get data for export and analysis
        std::vector<bso::utilities::geometry::quadrilateral> exteriorSurfaces = grm.getExteriorQuadrilaterals();
        double sumExteriorSurfaceArea = 0.0;
        for(const auto& surface: exteriorSurfaces){sumExteriorSurfaceArea += surface.getArea();}
        exteriorSurfaceArea.push_back(sumExteriorSurfaceArea);
        output_manager::output_manager::out("Got exterior wall Area",true,true,mConfig.getVerbose());
        
        // step 3, generate SD
        mSDModels.push_back(grm.sd_grammar<grammar::DEFAULT_SD_GRAMMAR>(std::string("settings/sd_settings.txt")));
        output_manager::output_manager::out("Generated structural model",true,true,mConfig.getVerbose());
        
        // step 4, evaluate SD and store results
        mSDModels.back().analyze();
        output_manager::output_manager::out("Evaluated structural model: ",false,false,mConfig.getVerbose());
        double sdResult = mSDModels.back().getTotalResults().mTotalStrainEnergy;
        sd_total_load.push_back(mSDModels.back().getTotalLoad());
        output_manager::output_manager::out(sdResult,true,true,mConfig.getVerbose());
        designPerformances.back()(0) = sdResult;
        
        // step 5, generate BP
        mBPModels.push_back(grm.bp_grammar<grammar::DEFAULT_BP_GRAMMAR>(std::string("settings/bp_settings.txt")));
        output_manager::output_manager::out("Generated building physics model",true,true,mConfig.getVerbose());
        
        // step 6, evaluate BP and store results
        mBPModels.back().simulatePeriods("runge_kutta_dopri5",1e-3,1e-3);
        output_manager::output_manager::out("Evaluated building physics model: ",false,false,mConfig.getVerbose());
        double bpResult = mBPModels.back().getTotalResults().mTotalEnergy;
        output_manager::output_manager::out(bpResult,true,true,mConfig.getVerbose());
        designPerformances.back()(1) = bpResult;
        
        // step 4, export visualizations
        if (!mConfig.getVisualizations().empty()) for (const auto& visOption : mConfig.getVisualizations())
        {
            if (visOption == "ms") visualization::visualize(mMSDesigns.back(),"","ms_N_building",4.0);
            if (visOption == "sg") visualization::visualize(cf,"space_at_ground");
            if (visOption == "de") visualization::visualize(cf,"delaunay");
            if (visOption == "bo") visualization::visualize(cf,"boundarylines");
            if (visOption == "tr") visualization::visualize(cf,"trirec");
            if (visOption == "tp") visualization::visualize(cf,"triangular_prism");
            if (visOption == "sd") visualization::visualize(mSDModels.back());
            if (visOption == "sdC") visualization::visualize(mSDModels.back(), "component");
            if (visOption == "sdD") visualization::visualize(mSDModels.back(), "density");
            if (visOption == "fe") visualization::visualize(mSDModels.back(), "element");
            if (visOption == "fe") visualization::visualize(mSDModels.back(), "strain_energy");
            if (visOption == "bp") visualization::visualize(mBPModels.back());
        }
        output_manager::out("Sent models to visualization",true,true,mConfig.getVerbose());
    }
	catch(const std::exception& e)
	{
		// Log error message to export file 'log_error'
		output_manager::logError(e.what(), mConfig.getInputFile(), mConfig.getModification(), mConfig.getIterations(), currentIteration);

		// Terminal message
		std::cout << "Error, could not finalize the run.\n"
				  << "Received the following error:\n" << e.what() << " \n"
				  << "start output generation despite not finishing: " << std::endl;

        // Start generating the requested outputs:
       outputResults();
	}
}

void scdp_optimizer::runSplitDeleteIteration(const std::string& modificationType)
{
    try
	{
        output_manager::out("\nSplit-delete iteration: ",false,false,mConfig.getVerbose()); output_manager::out(currentIteration,true,false,mConfig.getVerbose());
        // create a new MS model
        spatial_design::ms_N_building newMS = mMSDesigns.back();

        // step 2, get performances per space
        std::vector<utilities::data_point> performances;
        performance_evaluator::getPerformancePerSpace(performances, newMS, mSDModels.back(),  mBPModels.back(), modificationType, mConfig.getPerformanceDivision(), mConfig.getVerbose());

        // step 3, normalize the data points
        utilities::data_point utopianPoint;
        utilities::data_point distopianPoint;
        performance_evaluator::normalizeData(performances, utopianPoint, distopianPoint, mConfig.getVerbose());

        // Visualize the performance of spaces
        visualizeMsw(performances, currentIteration, modificationType, mConfig.getObjectiveWeights());

        // Step 4, delete spaces
        deleteSpaces(newMS, performances, modificationType, distopianPoint);

        // step 6, Enforce constrains on BSD
        enforceConstraints(newMS);
        
        // step 7, scale spaces in x and y direction to recover initial conditions, i.e. floor area
        scaleDesign(newMS);

        // step 8, split spaces to recover initial number of spaces conditions,
        // (checking x first and only splitting in z-direction if the space is a minimum of 6000 in height)
        splitSpaces(newMS, performances, modificationType, currentIteration, mConfig.getObjectiveWeights());
        
        // step 9, store the new design
        mMSDesigns.push_back(newMS);
        output_manager::out("Modified building spatial design into a new one",true,true,mConfig.getVerbose());
        currentIteration++;
        
        // step 10, get results for the new design
        getDesignResults();
    }
    catch(const std::exception& e)
	{
		// Log error message to export file 'log_error'
		output_manager::logError(e.what(), mConfig.getInputFile(), mConfig.getModification(), mConfig.getIterations(), currentIteration);

		// Terminal message
		std::cout << "Error, could not finalize the run.\n"
				  << "Received the following error:\n" << e.what() << " \n"
				  << "start output generation despite not finishing: " << std::endl;

        // Start generating the requested outputs:
        outputResults();
	}
}

void scdp_optimizer::runModificationIteration(const std::string& modificationType)
{
    try
	{
        output_manager::out("\nmodification iteration: ",false,false,mConfig.getVerbose()); output_manager::out(currentIteration,true,false,mConfig.getVerbose());
        
        // create a new MS model
        spatial_design::ms_N_building newMS = mMSDesigns.back();

        // step 1, get performances per space
        std::vector<utilities::data_point> performances;
        performance_evaluator::getPerformancePerSpace(performances, newMS, mSDModels.back(),  mBPModels.back(), modificationType, mConfig.getPerformanceDivision(), mConfig.getVerbose());

        bso::utilities::geometry::vector worstSpacesDirection = {0,0,0};
        if(mConfig.getAddMethod() == "delAdapt")
        {
            // step 2, sort performance data points from highest to lowest to determine where the worst is located for space removal later
            performance_evaluator::sortDataPointsHighestToLowest(performances, modificationType, currentIteration, mConfig.getObjectiveWeights(), mConfig.getVerbose());

            // step 3, determine the amount of worst spaces on which the add-method will be determined
            int spaceAmountRemoval= round(nSpaces*mConfig.getSpaceModifyWorstRatio()); 
            if(spaceAmountRemoval < 1){spaceAmountRemoval = 1;}

            // step 4, determine the centre of to worst spaces:
            bso::utilities::geometry::vertex worstSpaceCentre = {0,0,0};
            for(long unsigned int k=performances.size()-1; k>=performances.size()-spaceAmountRemoval; k--)
            {
                worstSpaceCentre += performances[k].getMSpacePtrs().back()->getGeometry().getCenter(); 
            }
            worstSpaceCentre = worstSpaceCentre/spaceAmountRemoval;
            bso::utilities::geometry::vertex centreBuilding = newMS.getCentreBuilding();
            centreBuilding.z()=0; // centre of the building should always be defined on z=0

            // step 5, determine the worst spaces direction, i.e. where the worst spaces are located in relation 
            // to the centre of the building for later use.
            worstSpacesDirection = worstSpaceCentre - centreBuilding;

            if(mConfig.getVerbose())
            {
                std::cout << "worstSpaceCentre is: " << worstSpaceCentre << std::endl;
                std::cout << "centreBuilding is: " << centreBuilding << std::endl;
                std::cout << "worstSpacesDirection.getHeading().second: " << worstSpacesDirection.getHeading().second << std::endl;
            }
        }

        // step 8, normalize the data points
        utilities::data_point utopianPoint;
        utilities::data_point distopianPoint;
        performance_evaluator::normalizeData(performances, utopianPoint, distopianPoint, mConfig.getVerbose());

        // Visualize the performance of spaces
        visualizeMsw(performances, currentIteration, modificationType, mConfig.getObjectiveWeights());

        // Step 9, delete spaces
        deleteSpaces(newMS, performances, modificationType, distopianPoint);

        // step 10, Enforce constrains on BSD
        enforceConstraints(newMS);

        if(mConfig.getAddMethod() == "delAdapt")
        {
            // step 11, execute either addElevation or shiftFillPlaneCentre depending on the worst spaces direction,
            // i.e. if the worst spaces are located more in the horizontal direction, execute shift the fill plane centre,
            // and if they are located more in the vertical direction, execute add elevation.
            if(worstSpacesDirection.getHeading().second < mConfig.getInclinationSwitch())
            {
                output_manager::out("\nStart addElevation in Iteration: ",false,false,mConfig.getVerbose()); output_manager::out(currentIteration,true,false,mConfig.getVerbose());
                newMS.duplicateElevation();
                output_manager::out("\naddElevation in Iteration: ",false,false,mConfig.getVerbose()); output_manager::out(currentIteration,true,false,mConfig.getVerbose());
            }
            else if(worstSpacesDirection.getHeading().second > mConfig.getInclinationSwitch())
            {
                output_manager::out("\nStart shiftFillPlaneCentre in iteration: ",false,false,mConfig.getVerbose()); output_manager::out(currentIteration,true,false,mConfig.getVerbose());
                
                // Alternate the direction of the shift of the fill plane
                bool shiftInMostHeadingDirection = false;
                if(currentIteration == 0) {shiftInMostHeadingDirection = true;}
                else if(currentIteration % 2) {shiftInMostHeadingDirection = true;}
                
                averageWidthSpace = newMS.getAverageHorizontalRibSize();
                newMS.shiftFillPlaneCentre(averageWidthSpace, shiftInMostHeadingDirection);
                output_manager::out("shiftFillPlaneCentre done: ",false,false,mConfig.getVerbose()); output_manager::out(currentIteration,true,false,mConfig.getVerbose());
            }
        }
        else if(mConfig.getAddMethod() == "shiftfill")
        {
            output_manager::out("\nStart shiftFillPlaneCentre in iteration: ",false,false,mConfig.getVerbose()); output_manager::out(currentIteration,true,false,mConfig.getVerbose());
            bool shiftInMostHeadingDirection = false;
            if(currentIteration == 0) {shiftInMostHeadingDirection = true;}
            else if(currentIteration % 2) {shiftInMostHeadingDirection = true;}
            
            averageWidthSpace = newMS.getAverageHorizontalRibSize();
            newMS.shiftFillPlaneCentre(averageWidthSpace, shiftInMostHeadingDirection);
            output_manager::out("shiftFillPlaneCentre done: ",false,false,mConfig.getVerbose()); output_manager::out(currentIteration,true,false,mConfig.getVerbose());
        }
        else if(mConfig.getAddMethod() == "elevation")
        {
            output_manager::out("\nStart addElevation in Iteration: ",false,false,mConfig.getVerbose()); output_manager::out(currentIteration,true,false,mConfig.getVerbose());
            newMS.duplicateElevation();
            output_manager::out("\naddElevation in Iteration: ",false,false,mConfig.getVerbose()); output_manager::out(currentIteration,true,false,mConfig.getVerbose());
        }

        // step 12,
        enforceConstraints(newMS);

        // step 13, recover the number of spaces if needed.
        // if the amount of spaces is higher than split spaces to recover initial number.
        // if lower, delete spaces to recover initial number.
        if(newMS.getSpacePtrs().size() > nSpaces)
        {
            splitSpaces(newMS, performances, modificationType, currentIteration, mConfig.getObjectiveWeights());
        }
        else if(newMS.getSpacePtrs().size() < nSpaces)
        {
            deleteSpaces(newMS, performances, modificationType, distopianPoint);
        }

        // step 14, Enforce constrains on BSD
        enforceConstraints(newMS);

        // step 15, scale spaces in x and y direction to recover initial conditions, i.e. floor area
        scaleDesign(newMS);

        // step 16, store the new design
        mMSDesigns.push_back(newMS);
        output_manager::out("Modified building spatial design into a new one",true,true,mConfig.getVerbose());
        currentIteration++;
        
        // step 17, get results for the new design
        getDesignResults();
    }
    catch(const std::exception& e)
	{
		// Log error message to export file 'log_error'
		output_manager::logError(e.what(), mConfig.getInputFile(), mConfig.getModification(), mConfig.getIterations(), currentIteration);

		// Terminal message
		std::cout << "Error, could not finalize the run.\n"
				  << "Received the following error:\n" << e.what() << " \n"
				  << "start output generation despite not finishing: " << std::endl;

        // Start generating the requested outputs:
        outputResults();
	}
}

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
        output_manager::out("Clustered space performances",true,true,mConfig.getVerbose());

        // Visualize the clustering if requested
        if (!mConfig.getVisualizations().empty()) for (const auto& visOption : mConfig.getVisualizations())
        {
            if (visOption == "msc")
            {
                std::string title = "ms_N_clustering" + std::to_string(currentIteration);
                visualization::visualize(clus, title, 4.0);
                output_manager::out("Sent ms clustering models to visualization",true,true,mConfig.getVerbose());
            }
        }
        
        // step 5, select worst performing cluster and remove these spaces
        unsigned int nRetainedSpaces;
        if(mConfig.getAmountOfSpaceRemoval() > 0){nRetainedSpaces = nSpaces - mConfig.getAmountOfSpaceRemoval();} // getAmountOfSpaceRemoval() value cannot be below 1 as checked at input, so no need to check for that here.
        else 
        {
            nRetainedSpaces = std::floor(nSpaces*(1-mConfig.getSpaceRemoveRatio()));
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
        output_manager::out("Removed worst performing cluster of spaces in building spatial design",true,true,mConfig.getVerbose());
    }
    else if (mConfig.getDeleteMethod() == "worst")
    {
        // step 4, sort data and determine space removal amount
        performancesRemoveSpace = performances;
        performance_evaluator::sortDataPointsHighestToLowest(performancesRemoveSpace, mConfig.getModification(), currentIteration, mConfig.getObjectiveWeights(), mConfig.getVerbose());
        output_manager::out("Sort data points for space removal",true,true,mConfig.getVerbose());

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
        output_manager::out("Removed worst performing spaces in building spatial design",true,true,mConfig.getVerbose());
    }
    else if (mConfig.getDeleteMethod() == "off")
    {
        output_manager::out("No spaces removed in building spatial design",true,true,mConfig.getVerbose());
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
    double scaleFactor = sqrt(floorArea / ms.getFloorArea());
    ms.scale({{0,scaleFactor},{1,scaleFactor}});
    output_manager::out("Scaled design to initial floor area",true,true,mConfig.getVerbose());
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
        while (ms.getSpacePtrs().size() < nSpaces)
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
        output_manager::out("Split spaces with largest dimension to restore number of spaces",true,true,mConfig.getVerbose());
    }
    else if(mConfig.getSplitMethod() == "performance")
    {
        std::vector<utilities::data_point> performancesRemoveSpace = performances;
		performance_evaluator::sortDataPointsHighestToLowest(performancesRemoveSpace, modification, i, objectiveWeights, mConfig.getVerbose());
        
        // split the spaces with the best performance
        int splitCount = 0;
        int splitamount = 2;
        while (ms.getSpacePtrs().size() < nSpaces)
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
        output_manager::out("Split spaces with best performance to restore number of spaces",true,true,mConfig.getVerbose());
    }
    else if(mConfig.getSplitMethod() == "off")
    {
        output_manager::out("No spaces split in building spatial design",true,true,mConfig.getVerbose());
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
void scdp_optimizer::visualizeMsw(
    const std::vector<bso::utilities::data_point>& performances, 
    const int interationNr, const std::string& modificationType, 
    const std::vector<double>& objectiveWeights)
{
	if (!mConfig.getVisualizations().empty()) for (const auto& visOption : mConfig.getVisualizations())
    {
        if (visOption == "msw")
        {
            float transparency = 1.0;
            std::vector<double> resultModificationFactor = {0,0};
            std::string title = "ms_N_worst";

            if (modificationType == "sd"){resultModificationFactor[1] = 1; title = "ms_N_worst_sd_" + std::to_string(interationNr);}
            else if (modificationType == "bp"){resultModificationFactor[0] = 1; title = "ms_N_worst_bp_" + std::to_string(interationNr);}
            else if (modificationType == "sdbp")
            {
                if(interationNr % 2){resultModificationFactor[0] = 1; title = "ms_N_worst_bp_" + std::to_string(interationNr);}
                else{resultModificationFactor[1] = 1; title = "ms_N_worst_sd_" + std::to_string(interationNr);}
            }
            else if (modificationType == "bpsd")
            {
                if(interationNr % 2){resultModificationFactor[1] = 1; title = "ms_N_worst_sd_" + std::to_string(interationNr);}
                else{resultModificationFactor[0] = 1; title = "ms_N_worst_bp_" + std::to_string(interationNr);}
            }
            else if(modificationType == "weighted")
            {
                resultModificationFactor[0] = objectiveWeights[0];
                resultModificationFactor[1] = objectiveWeights[1];
                title = "ms_N_worst_weighted_" + std::to_string(interationNr);
            }
            
            bso::visualization::visualize(performances, resultModificationFactor, transparency, title, 4.0);
            output_manager::out("Sent ms performances models to visualization",true,true,mConfig.getVerbose());
            break;
        }
    }
}

void scdp_optimizer::runSplitDeleteOnlyIterations()
{
    unsigned int iterations = mConfig.getIterations();
    for (unsigned int i = 0; i < iterations; ++i)
    {
        runSplitDeleteIteration(mConfig.getModification());
    }
    output_manager::out("\nCompleted all split-delete iterations", true, true, mConfig.getVerbose());
}

void scdp_optimizer::runModificationOnlyIterations()
{
    unsigned int iterations = mConfig.getIterations();
    for (unsigned int i = 0; i < iterations; ++i)
    {
        runModificationIteration(mConfig.getModification());
    }
    output_manager::out("\nCompleted all modification iterations", true, true, mConfig.getVerbose());
}

void scdp_optimizer::runAlternatingIterations()
{
    unsigned int iterations = mConfig.getIterations();
    for (unsigned int i = 0; i < iterations; ++i)
    {
        if (i % 2 == 0)
        {
            runSplitDeleteIteration(mConfig.getModification());
        }
        else
        {
            runModificationIteration(mConfig.getModification());
        }
    }
    output_manager::out("\nCompleted all alternating iterations", true, true, mConfig.getVerbose());
}

}}} // namespace bso::optimizers::scdp

#endif // SCDP_OPTIMIZER_CPP