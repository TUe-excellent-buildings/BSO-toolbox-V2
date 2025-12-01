#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE data_analysis_test
#endif

#include <bso/utilities/data_analysis.hpp>

#include <boost/test/included/unit_test.hpp>

/*
BOOST_TEST()
BOOST_REQUIRE_THROW(function, std::domain_error)
BOOST_REQUIRE_NO_THROW(function)
BOOST_REQUIRE(!s[8].dominates(s[9]) && !s[9].dominates(s[8]))
BOOST_CHECK_EQUAL_COLLECTIONS(a.begin(), a.end(), b.begin(), b.end());
*/

namespace utilities_test {
using namespace bso::utilities;

BOOST_AUTO_TEST_SUITE( data_analysis_tests )

	BOOST_AUTO_TEST_CASE( findQuartiles_simple )
	{
		std::vector<double> data = {1,2,3,4,5,6,7,8,9,10};
		std::vector<double> quartiles = Quantile(data, {0.25, 0.5, 0.75});
		BOOST_CHECK_EQUAL(quartiles.size(), 3);
		BOOST_CHECK_EQUAL(quartiles[0], 3);
		BOOST_CHECK_EQUAL(quartiles[1], 5.5);
		BOOST_CHECK_EQUAL(quartiles[2], 8);
	}

    BOOST_AUTO_TEST_CASE( findQuartiles_advanced )
	{
		std::vector<double> data = {5, 8, 15, 18, 20, 25, 30, 40};
		std::vector<double> quartiles = Quantile(data, {0.25, 0.5, 0.75});
		BOOST_CHECK_EQUAL(quartiles.size(), 3);
		BOOST_CHECK_EQUAL(quartiles[0], 11.5);
		BOOST_CHECK_EQUAL(quartiles[1], 19);
		BOOST_CHECK_EQUAL(quartiles[2], 27.5);
	}

	BOOST_AUTO_TEST_CASE( findOutliers_simple )
	{
		std::vector<double> data = {1,2,3,4,5,100};
		std::vector<double> outliers = findOutliers(data);
		BOOST_CHECK_EQUAL(outliers.size(), 1);
		BOOST_CHECK_EQUAL(outliers[0], 100);
	}
	

BOOST_AUTO_TEST_SUITE_END()
} // namespace utilities_test