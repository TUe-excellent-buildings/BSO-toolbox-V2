#ifndef HISTOGRAM_HPP
#define HISTOGRAM_HPP

#include <bso/utilities/data_point.hpp>
#include <bso/daylight/dl_model.hpp>
#include <iostream>
#include <vector>
#include <cmath>    // for sqrt()
#include <algorithm> // for min/max
#include <iomanip>  // for setw()
#include <fstream>  // for file operations
#include <cstdlib>  // for system()

namespace bso { namespace utilities {
	
class histogram
{
private:
	std::vector<double> data;
	std::vector<std::vector<double>> FinalRoomInfo;

public:
	histogram();
	histogram(std::vector<double>& mdata, const std::string& pngName);
	histogram(std::vector<std::vector<double>>& roomInfo, const bso::daylight::dl_model& dlModel, const std::string& i);
	~histogram();

	double computeMean();
    double computeVariance(double mean);
	void exportToCSV(const std::vector<std::vector<double>>& finalRoomInfo, const std::string& filename);
	
}; //class histogram
	
} // namespace utilities
} // namespace bso

#include <bso/utilities/histogram.cpp>

#endif // HISTOGRAM_HPP