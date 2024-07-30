#ifndef BSO_DATA_PLOT_HPP
#define BSO_DATA_PLOT_HPP

#include <bso/utilities/data_point.hpp>
#include <vector>
#include <cstdlib>

// To use this libary, gnuPlot should be installed on the computer (http://www.gnuplot.info/; http://gnuplot.info/docs_6.0/loc6150.html)

namespace bso { namespace utilities {
	
class data_plot
{
private:
	std::vector<std::shared_ptr<data_point> > mData;
	std::vector<std::string> axisDataTypes;
	std::string dataName = "";
	std::string dataFileName = "";
public:
	data_plot();
	data_plot(std::string fileNameForData, const std::vector<data_point>& data, std::vector<std::string> axisDataTypes, std::string plotName);
	~data_plot();
	
	// Plot functions for mData
	void writeMDataToFile(std::string fileName);
	void writeDataToFile(const std::vector<std::shared_ptr<data_point> >& data, std::string fileName);
	void writeVectorStyleFile(const std::vector<std::shared_ptr<data_point> >& data, std::string fileName);

	void plotScatter2D(std::string pngName = "plotScatter.png", bool visualize = true, bool writeToPNG = true);
	void plotParetoFront2D(std::string fileNamePareto = "Pareto", std::string fileNameNonPareto = "NonPareto", std::string pngName = "plotPareto.png", bool visualize = true, bool writeToPNG = true);
	void plotVector2D(std::string fileNameVector = "vectorStyleData", std::string pngName ="plotVector.png", bool visualize = true, bool writeToPNG = true);

	void getParetoFrontData(std::vector<std::shared_ptr<data_point> >& dataPareto, std::vector<std::shared_ptr<data_point> >& dataNonPareto);
};
	
} // namespace utilities
} // namespace bso

#include <bso/utilities/data_plot.cpp>

#endif // BSO__DATA_PLOT_HPP