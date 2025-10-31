#ifndef DATA_ANALYSIS_CPP
#define DATA_ANALYSIS_CPP

#include <vector>
#include <algorithm>
#include <cmath>
#include <sstream>

namespace bso { namespace utilities {

	template<typename T>
	static inline double Lerp(T v0, T v1, T t)
	{
		return (1 - t)*v0 + t*v1;
	}

	template<typename T>
	static inline std::vector<T> Quantile(const std::vector<T>& inData, const std::vector<T>& probs)
	{
		if (inData.empty())
		{
			return std::vector<T>();
		}

		if (1 == inData.size())
		{
			return std::vector<T>(1, inData[0]);
		}

		std::vector<T> data = inData;
		std::sort(data.begin(), data.end());
		std::vector<T> quantiles;

		for (size_t i = 0; i < probs.size(); ++i)
		{
			T poi = Lerp<T>(-0.5, data.size() - 0.5, probs[i]);

			size_t left = std::max(int64_t(std::floor(poi)), int64_t(0));
			size_t right = std::min(int64_t(std::ceil(poi)), int64_t(data.size() - 1));

			T datLeft = data.at(left);
			T datRight = data.at(right);

			T quantile = Lerp<T>(datLeft, datRight, poi - left);

			quantiles.push_back(quantile);
		}

		return quantiles;
	}

	template <typename T>
	std::vector<T> findOutliers(std::vector<T> data) 
	{
		if(data.empty())
		{
			std::stringstream errorMessage;
				errorMessage << "The dataSet is empty, so no outliers can be found" << std::endl
										<< "(bso/utilities/data_analysis.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
		}

		T Q1 = bso::utilities::Quantile(data, {0.25})[0];
		T Q3 = bso::utilities::Quantile(data, {0.75})[0];
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

