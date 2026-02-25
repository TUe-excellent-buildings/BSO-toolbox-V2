#ifndef BSO_EXPORT_HPP
#define BSO_EXPORT_HPP

#include <bso/utilities/data_point.hpp>
#include <vector>
#include <cstdlib>
#include <bso/spatial_design/ms_N_building.hpp>
#include <bso/structural_design/sd_model.hpp>

// To use the plot ability of this libary, gnuPlot should be installed on the computer (http://www.gnuplot.info/; http://gnuplot.info/docs_6.0/loc6150.html)

namespace bso { namespace utilities {
	
class data_export
{
private:
	// Data descriptions
	std::vector<std::shared_ptr<data_point> > mData;
	std::vector<std::string> axisDataTypes;
	std::string dataName = "";

	// Utility functions to output data to indicated output stream; 
	void dataPointToStream(const utilities::data_point& data, std::ostringstream& oss);
	template <typename T>
	void vectorToStream(const std::vector<T>& data, unsigned int& iteration, const unsigned int& nrIterations, const int switchLineCount, const int lineCount, std::ostringstream& oss);
	template <typename T>
	void initVectorToFile(const std::vector<T>& data, const int& nrIterations, std::ofstream& output);

	// Data file variables
	std::string scatterData = "";
	std::string paretoData = "";
	std::string nonParetoData = "";
	std::string vectorStyleData = "";
public:
	data_export();
	data_export(const std::vector<data_point>& data, std::vector<std::string> axisDataTypes, std::string plotName);
	data_export(utilities::data_export& rhs);
	~data_export();
	
	// Write data to file
	void writeDataToFile(const std::vector<std::shared_ptr<data_point> >& data, std::string fileName, std::string dataKind = "");
	void writeVectorStyleFile(const std::vector<std::shared_ptr<data_point> >& data, std::string fileName);

	// Write data to the same file in case of sequential runs to create an excel overview of severals runs
	void addToCombinedDataFile(const std::string& fileName, const std::string& runKind, const std::string& designName, const unsigned int& nrIterations, const std::vector<bso::spatial_design::ms_N_building>& msDesigns, const std::vector<double>& sd_total_load, const std::vector<double>& exteriorWallArea);
	void addToOverviewCombinedDataFile(const std::string& fileName, const std::string& runKind, const std::string& designName, const unsigned int numberOfIterationsPerformed, const bso::spatial_design::ms_N_building& initialDesign);

	// Plot functions for mData using gnuPlot
	void plotScatter2D(std::string fileNameScatter = "scatterData", std::string pngName = "plotScatter.png", bool visualize = true, bool writeToPNG = true);
	void plotParetoFront2D(std::string fileNamePareto = "paretoData", std::string fileNameNonPareto = "nonParetoData", std::string pngName = "plotPareto.png", bool visualize = true, bool writeToPNG = true);
	void plotVector2D(std::string fileNameVector = "vectorStyleData", std::string pngName ="plotVector.png", bool visualize = true, bool writeToPNG = true);

	// Get certain data
	void getParetoFrontData(std::vector<std::shared_ptr<data_point> >& data, std::vector<std::shared_ptr<data_point> >& dataPareto, std::vector<std::shared_ptr<data_point> >& dataNonPareto);
};
	
} // namespace utilities
} // namespace bso

#include <bso/utilities/data_export.cpp>

#endif // BSO_EXPORT_HPP