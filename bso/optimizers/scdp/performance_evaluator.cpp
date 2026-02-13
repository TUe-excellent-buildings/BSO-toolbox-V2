#include <bso/optimizers/scdp/performance_evaluator.hpp>

#include <algorithm>
#include <sstream>
#include <utility>

namespace bso { namespace optimizers { namespace scdp {

// Function to retrieve performance data for each space from the objective value models, i.e. bp and sd.
// based on the modification type, i.e. run for bp or sd, and performance division type, i.e. floor area, surface area or volume area.
void performance_evaluator::getPerformancePerSpace(
    std::vector<bso::utilities::data_point>& performances, 
    bso::spatial_design::ms_N_building& ms, 
    bso::structural_design::sd_model& sd, 
	bso::building_physics::bp_model& bp, 
    const std::string& modificationType, 
    const std::string& performanceDivisionType,
	bool verbose)
{
	std::vector<std::string> axisDataTypes = {"bp","sd"};
	for (const auto& j : ms.getSpacePtrs())
	{
		bso::utilities::data_point tempDataPoint({0,0});
		std::stringstream spaceString;
		spaceString << *j;
		tempDataPoint.addLabel(spaceString.str());
		tempDataPoint.addSpacePtr(j);

		auto spaceGeom = j->getGeometry();
		if (modificationType != "bp"){tempDataPoint(1) = sd.getPartialResults(&spaceGeom).mTotalStrainEnergy;}
		if (modificationType != "sd"){tempDataPoint(0) = bp.getPartialResults(&spaceGeom).mTotalEnergy;} 

		if(performanceDivisionType == "floorArea"){tempDataPoint /= j->getFloorArea();}
		else if(performanceDivisionType == "surfaceArea"){tempDataPoint /= j->getSurfaceArea();}	
		else if(performanceDivisionType == "volumeArea"){tempDataPoint /= j->getVolume();}

		performances.push_back(tempDataPoint);
	}
    output_manager::out("Collected performances for each space",true,true,verbose);
} // getPerformancePerSpace

// Function to normalize the performance data points based on the utopian and distopian points.
void performance_evaluator::normalizeData(
    std::vector<bso::utilities::data_point>& performances, 
    bso::utilities::data_point& utopianPoint, 
    bso::utilities::data_point& distopianPoint,
	bool verbose)
{
	utopianPoint.setToUtopia(performances);
	distopianPoint.setToDistopia(performances);
	std::swap(utopianPoint(1),distopianPoint(1));
	for (auto& j : performances)
	{
		j.normalize(utopianPoint,distopianPoint,{1});
	}
	utopianPoint.setToUtopia(performances);
	distopianPoint.setToDistopia(performances);
    output_manager::out("Normalized space performances",true,true,verbose);
}

// Function to sort the performance data points according to highest to lowest for example space removal later.
// The sorting is based on the modification type, i.e. sd, bp, sdbp, bpsd or weighted.
void performance_evaluator::sortDataPointsHighestToLowest(
	std::vector<bso::utilities::data_point>& performances, 
	const std::string& modificationType, 
	const int iterationNr, 
	const std::vector<double>& objectiveWeights,
	bool verbose)
{
	// Sort data for space removal later
	if (modificationType == "sd" || modificationType == "bp"){std::sort(performances.begin(), performances.end());}
	else
	{ 
		// Adjust data such that the following sorting algorithm works.
		int index = 0;
		if(modificationType == "sdbp")
		{
			if((iterationNr+1) % 2){index = 0;}
			else {index = 1;}
		}
		else if(modificationType == "bpsd")
		{
			if((iterationNr+1) % 2){index = 1;}
			else {index = 0;}
		}
		else if(modificationType == "weighted")
		{
			// Add both objectiveWeights * normalized performances up for a weighted performance per space
			for(auto &k: performances)
			{
				k(0) = k(0) * objectiveWeights[0] + k(1) * objectiveWeights[1];
			} 
			index = 1;
		}
		for(auto &k: performances){k(index) = 0;} 
		std::sort(performances.begin(), performances.end());
	}	
	output_manager::out("Sorted space performances from highest to lowest",true,true,verbose);
} // sortDataPointsHighestToLowest

}}} // namespace bso::optimizers::scdp
