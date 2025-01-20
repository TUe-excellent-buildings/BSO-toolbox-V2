#ifndef DATA_ANALYSIS_CPP
#define DATA_ANALYSIS_CPP

#include <vector>
#include <algorithm>

namespace bso { namespace utilities {

	template <typename T>
	T calculateMedian(std::vector<T> data) 
	{
		if(data.empty())
		{
			std::stringstream errorMessage;
				errorMessage << "The dataSet is empty, so no outliers can be fount" << std::endl
										<< "(bso/utilities/data_analysis.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
		}
		
		std::sort(data.begin(), data.end());
		size_t size = data.size();

		if (size % 2 == 0) {
			return (data[size / 2 - 1] + data[size / 2]) / static_cast<T>(2);
		} else {
			return data[size / 2];
		}
	}

	template <typename T>
	T calculateQuartile(std::vector<T> data, double quartile) 
	{
		if(data.empty())
		{
			std::stringstream errorMessage;
				errorMessage << "The dataSet is empty, so no outliers can be fount" << std::endl
										<< "(bso/utilities/data_analysis.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
		}

		std::sort(data.begin(), data.end());
		double pos = (data.size() + 1) * quartile;
		size_t intPos = static_cast<size_t>(pos);
		double frac = pos - intPos;

		if (intPos >= data.size()) {
			return data.back();
		} else if (intPos == 0) {
			return data.front();
		} else {
			return data[intPos - 1] + frac * (data[intPos] - data[intPos - 1]);
		}
	}

	template <typename T>
	std::vector<T> findOutliers(std::vector<T> data) 
	{
		if(data.empty())
		{
			std::stringstream errorMessage;
				errorMessage << "The dataSet is empty, so no outliers can be fount" << std::endl
										<< "(bso/utilities/data_analysis.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
		}
		
		T Q1 = calculateQuartile(data, 0.25);
		T Q3 = calculateQuartile(data, 0.75);
		T IQR = Q3 - Q1;
		T lowerBound = Q1 - 1.5 * IQR;
		T upperBound = Q3 + 1.5 * IQR;

		std::vector<T> outliers;
		for (const T& value : data) {
			if (value < lowerBound || value > upperBound) {
				outliers.push_back(value);
			}
		}
		return outliers;
	}

} // namespace utilities
} // namespace bso

#endif // DATA_ANALYSIS_CPP

