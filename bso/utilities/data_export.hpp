#ifndef BSO_EXPORT_HPP
#define BSO_EXPORT_HPP

#include <bso/utilities/data_point.hpp>
#include <vector>
#include <cstdlib>

// To use this libary, gnuPlot should be installed on the computer (http://www.gnuplot.info/; http://gnuplot.info/docs_6.0/loc6150.html)

namespace bso { namespace utilities {
	
class data_export
{
private:
	// Data descriptions
	std::vector<std::shared_ptr<data_point> > mData;
	std::vector<std::string> axisDataTypes;
	std::string dataName = "";

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
	void addToCombinedDataFile(const std::string& fileName, const std::string& runKind, const std::string& designName, const unsigned int& nrIterations);
	void addToOverviewCombinedDataFile(const std::string& fileName, const std::string& runKind, const std::string& designName, const unsigned int numberOfIterationsPerformed);

	// Plot functions for mData
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