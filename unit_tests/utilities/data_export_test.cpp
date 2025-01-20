#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE data_export
#endif

#include <boost/test/included/unit_test.hpp>

#include <bso/utilities/data_export.hpp>

#include <vector>
#include <stdexcept>
#include <iostream>

/*
BOOST_TEST()
BOOST_REQUIRE_THROW(function, std::domain_error)
BOOST_REQUIRE(!s[8].dominates(s[9]) && !s[9].dominates(s[8]))
BOOST_CHECK_EQUAL_COLLECTIONS(a.begin(), a.end(), b.begin(), b.end());
*/

namespace utilities_test {
using namespace bso::utilities;

BOOST_AUTO_TEST_SUITE( initialization )

	BOOST_AUTO_TEST_CASE( initialization_1_empty ) 
	{
		data_export dp;
	}

	BOOST_AUTO_TEST_CASE( initialization_2 )
	{
		// Generate performances
		std::vector<data_point> performances;
		std::vector<std::string> axisDataTypes = {"bp","sd"};
		for (double j=0; j<6; j++)
		{
			data_point tempDataPoint({j,j});
			performances.push_back(tempDataPoint);
		}
		
		data_export dp(performances, axisDataTypes, "random");

		dp.plotParetoFront2D("utilities/results_data_export_test/paretoPlot", "utilities/results_data_export_test/paretoPlot", "utilities/results_data_export_test/paretoPlot.png", false, true);
		dp.plotScatter2D("utilities/results_data_export_test/scatterPlot", "utilities/results_data_export_test/scatterPlot.png", false, true);
		dp.plotVector2D("utilities/results_data_export_test/VectorPlot", "utilities/results_data_export_test/VectorPlot.png", false, true);
	}

	BOOST_AUTO_TEST_CASE( error_handling)
	{
		data_export dp;
		BOOST_REQUIRE_THROW(dp.plotScatter2D("utilities/results_data_export_test/scatterPlot", "utilities/results_data_export_test/scatterPlot.png", false, true), std::invalid_argument);

		// Generate performances
		std::vector<data_point> performances;
		std::vector<std::string> axisDataTypes = {"bp","sd"};
		for (double j=0; j<6; j++)
		{
			data_point tempDataPoint({j,j});
			performances.push_back(tempDataPoint);
		}
		
		dp = data_export(performances, axisDataTypes, "random");

		BOOST_REQUIRE_THROW(dp.plotScatter2D("utilities/results_data_export_test/scatterPlot", "utilities/results_data_export_test/scatterPlot.png", false, false), std::invalid_argument);
		BOOST_REQUIRE_THROW(dp.plotScatter2D("utilities/results_data_export_test/vectorPlot", "utilities/results_data_export_test/vectorPlot.png", false, false), std::invalid_argument);
		BOOST_REQUIRE_THROW(dp.plotParetoFront2D("utilities/results_data_export_test/paretoPlot", "utilities/results_data_export_test/paretoPlot", "utilities/results_data_export_test/paretoPlot.png", false, false), std::invalid_argument);
	}

BOOST_AUTO_TEST_SUITE_END()
} // namespace utilities_test