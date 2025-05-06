#ifndef DATA_ANALYSIS_HPP
#define DATA_ANALYSIS_HPP

#include <vector>
#include <algorithm>
namespace bso { namespace utilities {

	/*
	 * The functions in this file are used to analyze a list of data
	 */
	 
	template <typename T>
	T calculateMedian(std::vector<T> data);

	// This functions calculates a quartile of a data set and return the data which is at that quartile.
	// 'data' is the data set of T value's and quartile is ratio at which the quartile is requested.
	// i.e. 0.25 is the first quartile, 0.75 is the second quartile.
	// Note: 0.5 is the median of the data set.
	template <typename T>
	T calculateQuartile(std::vector<T> data, double quartile);

	
	template <typename T>
	std::vector<T> findOutliers(std::vector<T> data);

} // namespace utilities
} // namespace bso

#include <bso/utilities/data_analysis.cpp>

#endif // DATA_ANALYSIS_HPP

