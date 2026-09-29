#ifndef BSO_EXPORT_HPP
#define BSO_EXPORT_HPP

#include <bso/utilities/data_point.hpp>
#include <vector>
#include <cstdlib>
#include <bso/spatial_design/ms_N_building.hpp>
#include <bso/structural_design/sd_model.hpp>

// To use the plot ability of this libary, you should either have:
// -  gnuPlot for 2D plots (http://www.gnuplot.info/; http://gnuplot.info/docs_6.0/loc6150.html)
// -  matplotlib for multiple y-axis plots(https://matplotlib.org/)

namespace bso { namespace utilities {
	
class data_export
{
private:
	// Data descriptions
	std::vector<std::shared_ptr<data_point> > mData;
	std::vector<std::string> mAxisDataTypes; // as stored from input and ID placement should correspond to the data in mData, e.g. "sd", "bp", "dl_ADFbuilding".
	std::string mDataName = "";
	std::map<std::string, bool> mIsMinimization = {{"sd", true}, {"bp", true}, {"dl_ADFbuilding", false}, {"dl_score", true}}; // whether the objective is a minimization or maximization objective, used for normalization and knee point detection.

	// Axis descriptions tranformed from mAxisDataType input.
	std::map<std::string, std::string> mAxisLabels = {{"sd", "Strain energy [Nmm]"}, {"bp", "Heating and cooling energy [kWh]"}, {"dl_ADFbuilding", "Average daylight factor [%]"}, {"dl_score", "BS EN 17037-2018 score [-]"}, {"iteration", "Iteration"}};

	// Utility functions to output data to indicated output stream; 
	void dataPointToStream(const utilities::data_point& data, std::ostringstream& oss);
	template <typename T>
	void vectorToStream(const std::vector<T>& data, unsigned int& iteration, const unsigned int& nrIterations, const int switchLineCount, const int lineCount, std::ostringstream& oss);
	template <typename T>
	void initVectorToFile(const std::vector<T>& data, const std::vector<std::string>& dataTypes, const int& nrIterations, std::ofstream& output);

	// Helper structures and functions to keep addToCombinedDataFile readable
	struct CombinedStats
	{
		int kneePointIteration = 0;
		std::vector<int> bestIterations;
		std::vector<utilities::data_point> bestDataPoints;
		std::vector<utilities::data_point> bestDataPointsNormalized;
		std::vector<double> bestImprovementPercentages;
		std::vector<double> lastImprovementPercentages;
		utilities::data_point kneePoint;
	};

	CombinedStats computeCombinedStats(const unsigned int nrIterations,
	                                  std::vector<utilities::data_point>& mDataNormalized) const;

	struct MsDesignData
	{
		std::vector<double> averageRecDif;
		std::vector<double> ratioBiggestSmallestFloorAreaSpace;
		std::vector<double> nrElevations;
		std::vector<double> median;
		std::vector<double> firstQuartile;
		std::vector<double> thirdQuartile;
		std::vector<double> average;
		std::vector<bool> isFragmented;
	};

	MsDesignData collectMsDesignData(const std::vector<bso::spatial_design::ms_N_building>& msDesigns) const;

	// Data file variables
	std::string mScatterData = "";
	std::string mParetoData = "";
	std::string mNonParetoData = "";
	std::string mVectorStyleData = "";
	std::string mMultiAxisData = "";
public:
	data_export();
	data_export(const std::vector<data_point>& data, std::vector<std::string> axisDataTypes, std::string plotName);
	data_export(utilities::data_export& rhs);
	~data_export();

	// Write data to file for GNUplot generation with Plot functions below.
	void writeDataToFile(const std::vector<std::shared_ptr<data_point> >& data, std::string fileName, std::string dataKind = "");
	void writeVectorStyleFile(const std::vector<std::shared_ptr<data_point> >& data, std::string fileName);
	void writeMultiAxisFile(const std::vector<std::shared_ptr<data_point> >& data, std::string fileName);

	// New helpers for matplotlib-based post-processing scripts:
	//  - writeObjective3DFile: 3D objective space (sd, bp, dl_ADFbuilding) + knee marker
	//  - writeImprovementCurvesFile: per-objective improvement [%] vs iteration + knee marker
	void writeObjective3DFile(const std::string& fileName) const;
	void writeImprovementCurvesFile(const std::string& fileName) const;

	// Write data to the same file in case of sequential runs to create an excel overview of severals runs
	void addToCombinedDataFile(const std::string& fileName, const std::string& runKind, const std::string& designName, const unsigned int& nrIterations, const std::vector<bso::spatial_design::ms_N_building>& msDesigns, const std::vector<double>& sd_total_load, const std::vector<double>& exteriorWallArea);
	// Parallel version: each run writes to its own file, to be combined later
	void addToCombinedDataFileParallel(const std::string& fileName, const std::string& runKind, const std::string& designName, const unsigned int& nrIterations, const std::vector<bso::spatial_design::ms_N_building>& msDesigns, const std::vector<double>& sd_total_load, const std::vector<double>& exteriorWallArea);
	// void addToOverviewCombinedDataFile(const std::string& fileName, const std::string& runKind, const std::string& designName, const unsigned int numberOfIterationsPerformed, const bso::spatial_design::ms_N_building& initialDesign); // function is not up to date yet with the additional objective possibilities in scdp_optimizer, so it is currently not used and not up to date with the current data collection, but it can be used as a basis for an overview file function in the future if desired.

	// Plot functions for mData using gnuPlot for two variables
	void plotScatter2D(std::string fileNameScatter = "scatterData", std::string pngName = "plotScatter.png", bool visualize = true, bool writeToPNG = true);
	void plotParetoFront2D(std::string fileNamePareto = "paretoData", std::string fileNameNonPareto = "nonParetoData", std::string pngName = "plotPareto.png", bool visualize = true, bool writeToPNG = true);
	void plotVector2D(std::string fileNameVector = "vectorStyleData", std::string pngName ="plotVector.png", bool visualize = true, bool writeToPNG = true);

	// Plot functions for mData using matplotlib for more variables where the .py code is placed in /utilities/plot directory
	void plotMultiAxis2D(std::string fileNameMultiAxis = "multiAxisData", std::string pngName = "plotMultiAxis.png", bool visualize = true, bool writeToPNG = true);
	// New matplotlib-based plots for multi-objective runs
	void plotObjectiveSpace3D(std::string fileNameObjective3D = "objective3DData", std::string pngName = "plotObjective3D.png", bool visualize = true, bool writeToPNG = true);
	void plotImprovementCurves(std::string fileNameImprovement = "improvementCurvesData", std::string pngName = "plotImprovementCurves.png", bool visualize = true, bool writeToPNG = true, bool showKnee = false, bool includeFourthObjective = false);

	// Get certain data
	void getParetoFrontData(std::vector<std::shared_ptr<data_point> >& data, std::vector<std::shared_ptr<data_point> >& dataPareto, std::vector<std::shared_ptr<data_point> >& dataNonPareto);
};
	
} // namespace utilities
} // namespace bso

#include <bso/utilities/data_export.cpp>

#endif // BSO_EXPORT_HPP