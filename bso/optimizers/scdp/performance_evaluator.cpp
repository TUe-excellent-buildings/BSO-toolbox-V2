#include <bso/optimizers/scdp/performance_evaluator.hpp>
#include <bso/optimizers/scdp/output_manager.hpp>

#include <algorithm>
#include <sstream>
#include <utility>
#include <chrono>

namespace bso { namespace optimizers { namespace scdp {

// Function to retrieve performance data for each space from the objective value models, i.e. bp and sd.
// based on the modification type, i.e. run for bp or sd, and performance division type, i.e. floor area, surface area or volume area.
void performance_evaluator::getPerformancePerSpace(
    std::vector<bso::utilities::data_point>& performances, 
    bso::spatial_design::ms_N_building& ms, 
    bso::structural_design::sd_model& sd, 
	bso::building_physics::bp_model& bp, 
	bso::daylight::dl_model& dl,
    const std::string& modificationType, 
    const std::string& performanceDivisionType,
    output_manager& outputMgr,
	bool verbose,
	bool sdAvailable,
	bool bpAvailable,
	bool dlAvailable)
{
	std::vector<std::string> axisDataTypes = {"bp","sd","dl"};

    // Determine which objectives are requested by the modificationType string.
    // The modificationType is a concatenation of 2-letter tokens: "sd", "bp", "dl".
    // Here we only need to know whether SD,BP and/or DL are part of the modification;
    const bool usesSD = (modificationType.find("sd") != std::string::npos);
    const bool usesBP = (modificationType.find("bp") != std::string::npos);
    const bool usesDL = (modificationType.find("dl") != std::string::npos);

    using clock = std::chrono::high_resolution_clock;
    double totalSD = 0.0, totalBP = 0.0, totalDL = 0.0;
    int countSD = 0, countBP = 0, countDL = 0;

    for (const auto& j : ms.getSpacePtrs())
    {
        bso::utilities::data_point tempDataPoint({0,0,0});
        std::stringstream spaceString;
        spaceString << *j;
        tempDataPoint.addLabel(spaceString.str());
        tempDataPoint.addSpacePtr(j);

        auto spaceGeom = j->getGeometry();
        unsigned int spaceID = j->getID();
        // Only collect SD/BP performance data when the corresponding objective is
        // both available and requested by the modificationType.
        if (sdAvailable && usesSD) {
            auto t0 = clock::now();
            tempDataPoint(1) = sd.getPartialResults(&spaceGeom).mTotalStrainEnergy;
            auto t1 = clock::now();
            totalSD += std::chrono::duration<double>(t1 - t0).count();
            ++countSD;
        }
        if (bpAvailable && usesBP) {
            auto t0 = clock::now();
            tempDataPoint(0) = bp.getPartialResults(&spaceGeom).mTotalEnergy;
            auto t1 = clock::now();
            totalBP += std::chrono::duration<double>(t1 - t0).count();
            ++countBP;
        }
        if (dlAvailable && usesDL) {
            auto t0 = clock::now();
            tempDataPoint(2) = dl.getPartialResults(spaceID).mADFBuilding; 
            auto t1 = clock::now();
            totalDL += std::chrono::duration<double>(t1 - t0).count();
            ++countDL;
        }

        if(performanceDivisionType == "floorArea"){tempDataPoint /= j->getFloorArea();}
        else if(performanceDivisionType == "surfaceArea"){tempDataPoint /= j->getSurfaceArea();}	
        else if(performanceDivisionType == "volumeArea"){tempDataPoint /= j->getVolume();}

        performances.push_back(tempDataPoint);
    }
	if(verbose)
	{
		if (countSD > 0) std::cout << "SD performance retrieval: " + std::to_string(totalSD) + " s for " + std::to_string(countSD) + " spaces (" + std::to_string(totalSD/countSD) + " s/space)" << std::endl;
		if (countBP > 0) std::cout << "BP performance retrieval: " + std::to_string(totalBP) + " s for " + std::to_string(countBP) + " spaces (" + std::to_string(totalBP/countBP) + " s/space)" << std::endl;
		if (countDL > 0) std::cout << "DL performance retrieval (Note wall clock will take longer): " + std::to_string(totalDL) + " s for " + std::to_string(countDL) + " spaces (" + std::to_string(totalDL/countDL) + " s/space)" << std::endl;
	}
    outputMgr.out("Collected performances for each space",true,true,verbose);
} // getPerformancePerSpace

// Function to normalize the performance data points based on the utopian and distopian points.
// Note: sd is a minimization objective and bp is a maximization objective,
// However, by swapping the utopian and distopian points for sd, we can ensure 
// that the normalized values are in the range [0,1]
// where 0 is the worst performance and 1 is the best performance for both objectives.
void performance_evaluator::normalizeData(
    std::vector<bso::utilities::data_point>& performances, 
    bso::utilities::data_point& utopianPoint, 
    bso::utilities::data_point& distopianPoint,
    output_manager& outputMgr,
	bool verbose)
{
	utopianPoint.setToUtopia(performances); // contains all minimum values for each objective across all spaces
	distopianPoint.setToDistopia(performances); // contains all maximum values for each objective across all spaces

	// For both sd and dl a low performance is bad, 
	// so swap the utopian and distopian point
	// to make sure that the dispopian value is the lowest and
	// consequently normalization is done correctly.
	std::swap(utopianPoint(1),distopianPoint(1)); // sd
	std::swap(utopianPoint(2),distopianPoint(2)); // dl

	// Normalize each performance data point based on the utopian and distopian points.
	for (auto& j : performances)
	{
		j.normalize(utopianPoint,distopianPoint,{1});
	}

	utopianPoint.setToUtopia(performances); // minimum values which are now all 0 after normalization
	distopianPoint.setToDistopia(performances); // maximum values which are now all 1 after normalization
    outputMgr.out("Normalized space performances",true,true,verbose);
}

namespace {

// Map a base token to the corresponding index in data_point:
//  bp -> 0, sd -> 1, dl -> 2.
static int tokenToIndex(const std::string& token)
{
	if (token == "bp") return 0;
	if (token == "sd") return 1;
	if (token == "dl") return 2;
	return -1;
}

} // unnamed namespace

// Function to sort the performance data points from lowest to highest, for example for space removal later.
// The sorting is based on the modification type (any concatenation of sd, bp, dl)
// and can optionally use weighted objectives when weightedModification is true.
void performance_evaluator::sortDataPointsLowestToHighest(
	std::vector<bso::utilities::data_point>& performances, 
	const std::string& modificationType, 
	const std::vector<std::string>& modificationSequence,
	const int iterationNr, 
	const std::vector<double>& objectiveWeights,
	bool weightedModification,
	output_manager& outputMgr,
	bool verbose)
{
	if (performances.empty())
	{
		std::stringstream errorMessage;
		errorMessage << "Error, no performance data available for sorting." << std::endl
								 << "(bso/optimizers/scdp/performance_evaluator.cpp). " << std::endl;
		throw std::invalid_argument(errorMessage.str());
	}

	const unsigned int nDims = performances.front().size();
	// Use the already-parsed modification sequence provided by scdp_config.
	const std::vector<std::string>& modSequence = modificationSequence;

	// Weighted mode: combine active objectives using objectiveWeights
	// (ordered as sd, bp, dl in scdp_config) into a single score.
	if (weightedModification)
	{
		double w_sd = 0.0, w_bp = 0.0, w_dl = 0.0;

		if(objectiveWeights.empty())
		{
			std::stringstream errorMessage;
			errorMessage << "Error, weighted modification is enabled but no objective weights provided." << std::endl
								 << "at this stage, it would render a segmentationfault, as such an error is \n "
								 << "provided to prevent that and to make sure the user is aware of the issue. \n"
								 << "However, the issue should be fixed in the scdp_config class, \n"
								 << "as that is where the values should be set. " << std::endl
								 << "(bso/optimizers/scdp/performance_evaluator.cpp). " << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}

		w_sd = objectiveWeights[0]; // sd weight
		w_bp = objectiveWeights[1]; // bp weight
		w_dl = objectiveWeights[2]; // dl weight

		for (auto& p : performances)
		{
			double combined = 0.0;
			if (nDims > 1) combined += p(1) * w_sd; // sd at index 1
			if (nDims > 0) combined += p(0) * w_bp; // bp at index 0
			if (nDims > 2) combined += p(2) * w_dl; // dl at index 2

			// Store combined score in first dimension and zero the rest
			if (nDims > 0) p(0) = combined;
			for (unsigned int d = 1; d < nDims; ++d)
			{
				p(d) = 0.0;
			}
		}

		std::sort(performances.begin(), performances.end());
	}
	else
	{
		// Non-weighted mode: select one active objective for this iteration.
		int activeIndex = -1;

		if (modSequence.size() == 1)
		{
			// Single-objective modification (sd, bp, or dl)
			activeIndex = tokenToIndex(modSequence.front());
		}
		else
		{
			// General case: cycle through all tokens in declared order.
			std::size_t pos = 0;
			if (!modSequence.empty())
			{
				pos = static_cast<std::size_t>(iterationNr) % modSequence.size();
			}
			activeIndex = tokenToIndex(modSequence[pos]);
		}

		if (activeIndex < 0 || static_cast<unsigned int>(activeIndex) >= nDims)
		{
			std::stringstream errorMessage;
			errorMessage << "Error, invalid active index for performance sorting found." << std::endl
									<< "(bso/optimizers/scdp/performance_evaluator.cpp). " << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
		else
		{
			// Zero out all other dimensions so that the sort is effectively
			// based only on the active objective.
			for (auto& p : performances)
			{
				for (unsigned int d = 0; d < nDims; ++d)
				{
					if (static_cast<int>(d) != activeIndex)
					{
						p(d) = 0.0;
					}
				}
			}
			std::sort(performances.begin(), performances.end());
		}
	}	
	outputMgr.out("Sorted space performances from highest to lowest",true,true,verbose);
}

}}} // namespace bso::optimizers::scdp
