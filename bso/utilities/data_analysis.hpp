#ifndef BSO_DATA_ANALYSIS_HPP
#define BSO_DATA_ANALYSIS_HPP

#include <bso/utilities/data_point.hpp>
#include <bso/utilities/cluster.hpp>
#include <vector>

// include libary gnuPlot and make plots with this libary for data visualization (http://www.gnuplot.info/)

namespace bso { namespace utilities {
	
class data_analysis
{
private:
	std::vector<std::shared_ptr<data_point> > mData;
	std::vector<cluster> mClusters;
public:
	data_analysis();
	~data_analysis();

	// void plot()
	// void sortData(const CONTAINER& dataCollection)
};
	
} // namespace utilities
} // namespace bso

#include <bso/utilities/data_analysis.cpp>

#endif // BSO__DATA_ANALYSIS_HPP