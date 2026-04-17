#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE "grammar_dl_rectangle_rule_generate_grid_test"
#endif

#include <bso/grammar/rule_set/dl_rule_set/dl_rectangle_rule.hpp>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>

namespace grammar_dl_rule_set_test {
using namespace bso::grammar::rule_set;

// TEST ARE NOT READY YET, AS THE CHECK VALUES are not correct yet!

namespace {
// Only holds for axis alinged grid definitions!
std::vector<double> getUniqueSortedCoordinates(
	const std::vector<bso::utilities::geometry::vertex>& points,
	const int axis,
	const double tol = 1e-6)
{
	std::vector<double> values;
	for (const auto& p : points) values.push_back(p[axis]);
	std::sort(values.begin(), values.end());

	std::vector<double> uniqueValues;
	for (const auto& v : values)
	{
		if (uniqueValues.empty() || std::abs(v - uniqueValues.back()) > tol)
		{
			uniqueValues.push_back(v);
		}
	}
	return uniqueValues;
}

double stepBetweenFirstTwo(const std::vector<double>& coords)
{
	if (coords.size() < 2) return 0.0;
	return std::abs(coords[1] - coords[0]);
}
} // namespace

BOOST_AUTO_TEST_SUITE( grammar_dl_rectangle_rule_generate_grid_test )

// BOOST_AUTO_TEST_CASE( generateGridNEN_square_surface_obeys_margin_and_cell_ratio )
// {// 
// 	bso::spatial_design::ms_N_building ms;

// 	// Square space thus floor surface
// 	ms.addSpace(bso::spatial_design::ms_space("N,1, 0,0,0, 5000,0,0, 0,5000,0, 5000,5000,0, 0,0,5000, 5000,0,5000, 0,5000,5000, 5000,5000,5000"));
// 	bso::spatial_design::cf_building cf(ms);

// 	auto quadrilateral = bso::utilities::geometry::quadrilateral({{0,0,0},{5000,0,0},{0,5000,0},{5000,5000,0}});
// 	bso::spatial_design::conformal::cf_surface* surfPtr = nullptr;
// 	for (bso::spatial_design::conformal::cf_surface* sur : cf.cfSurfaces())
// 	{
// 		if (sur != nullptr && quadrilateral.isSameAs(*sur))
// 		{
// 			surfPtr = sur;
// 			break;
// 		}
// 	}
// 	BOOST_REQUIRE(surfPtr != nullptr); // check if the expected rectangle surface was found in the geometry model

// 	std::vector<cf::cf_surface> surfaces = {*surfPtr};
// 	std::vector<unsigned int> spaceID = {99}; // value doesn't matter for this test as long as it's consistent
// 	std::vector<double> dfSettings = {14400, 850.0, 500.0, 10.0}; // {horizontal_outdoor_illuminance[lx], reference_height[mm], space_margin[mm], minGridSize[m]}
// 	std::vector<bso::utilities::geometry::vertex> gridPoints;

// 	auto gridPath = std::filesystem::temp_directory_path() / "bso_generateGridNEN_square_grid.txt";
// 	std::ofstream gridFile(gridPath.string(), std::ios::trunc);
// 	BOOST_REQUIRE(gridFile.is_open());

// 	auto result = dl_rule_set::generateGridNEN(surfaces, gridFile, spaceID, dfSettings, gridPoints); // auto is tuple std::vector<std::tuple<Space ID, amount of gridpoints>>
// 	gridFile.close();
// 	std::filesystem::remove(gridPath);

// 	BOOST_REQUIRE_EQUAL(result.size(), 1u); // Only 1-surface thus 1 tuple in result vector
// 	BOOST_REQUIRE_EQUAL(std::get<0>(result[0]), 99u); // Check spaceID in result tuple
// 	BOOST_REQUIRE_EQUAL(std::get<1>(result[0]), 9); // check amount of gridpoints
// 	BOOST_REQUIRE_EQUAL(gridPoints.size(), 9u); // check amount of gridpoints

// 	// d=5 m thus p=0.5*5^log10(5)=0.5*5^0.69897=1.54 m
// 	// gridPointamount in either x or y axis is: 
// 	// rouned up((surface_length - 2*margin) / gridSize) = rouned up((5000 - 2*500) / 1540) = 4
// 	// Distance between grid points is then: (surface_length - 2*margin) / (gridPointAmount - 1) = (5000 - 2*500) / (4-1) = 1333.33 mm

// 	// Check grid point amount
// 	auto xCoords = getUniqueSortedCoordinates(gridPoints, 0);
// 	auto yCoords = getUniqueSortedCoordinates(gridPoints, 1);
// 	bool expectedPointCounts = (xCoords.size() == 4u && yCoords.size() == 4u);
// 	BOOST_REQUIRE(expectedPointCounts);

// 	// Only check this for first case and in case space is not at ground level,
// 	// as it is assumed that the remainder cases will also be at the correct 
// 	// reference height if this is correct for the first
// 	// case, as the same code is used for all cases. 
// 	for (const auto& p : gridPoints){ BOOST_REQUIRE_SMALL(std::abs(p[2] - 850.0), 1e-6);} // Check if all z-coordinates are at the reference height of 850.0 mm

// 	// Check distance between grid points and cell ratio
// 	double dx = stepBetweenFirstTwo(xCoords);
// 	double dy = stepBetweenFirstTwo(yCoords);

// 	BOOST_REQUIRE_SMALL(std::abs(dx - 1333.33), 0.001);
// 	BOOST_REQUIRE_SMALL(std::abs(dy - 1333.33), 0.001);
// 	double ratio = std::max(dx, dy) / std::min(dx, dy);
// 	BOOST_REQUIRE(ratio <= 2.0);
// }

// BOOST_AUTO_TEST_CASE( generateGridNEN_rectangle_surface_ratio_2_1_just_treshold )
// {// 
// 	bso::spatial_design::ms_N_building ms;

// 	// Rectangle space thus floor surface, with aspect ratio of 2:1, 
// 	// thus shortest edge of 5000mm should be used for p calculation, 
// 	// but grid should still be generated for entire rectangle and 
// 	// thus have more points in the longer dimension
// 	ms.addSpace(bso::spatial_design::ms_space("N,1, 0,0,1000, 5000,0,1000, 0,10000,1000, 5000,10000,1000, 0,0,5000, 5000,0,5000, 0,10000,5000, 5000,10000,5000"));
// 	bso::spatial_design::cf_building cf(ms);
	
// 	auto quadrilateral = bso::utilities::geometry::quadrilateral({{0,0,1000},{5000,0,1000},{0,10000,1000},{5000,10000,1000}});
// 	bso::spatial_design::conformal::cf_surface* surfPtr = nullptr;
// 	for (bso::spatial_design::conformal::cf_surface* sur : cf.cfSurfaces())
// 	{
// 		if (sur != nullptr && quadrilateral.isSameAs(*sur))
// 		{
// 			surfPtr = sur;
// 			break;
// 		}
// 	}
// 	BOOST_REQUIRE(surfPtr != nullptr); // check if the expected rectangle surface was found in the geometry model

// 	std::vector<cf::cf_surface> surfaces = {*surfPtr};
// 	std::vector<unsigned int> spaceID = {99}; // value doesn't matter for this test as long as it's consistent
// 	std::vector<double> dfSettings = {14400, 850.0, 500.0, 10.0}; // {horizontal_outdoor_illuminance[lx], reference_height[mm], space_margin[mm], minGridSize[m]}
// 	std::vector<bso::utilities::geometry::vertex> gridPoints;

// 	auto gridPath = std::filesystem::temp_directory_path() / "bso_generateGridNEN_rectangle_grid.txt";
// 	std::ofstream gridFile(gridPath.string(), std::ios::trunc);
// 	BOOST_REQUIRE(gridFile.is_open());

// 	auto result = dl_rule_set::generateGridNEN(surfaces, gridFile, spaceID, dfSettings, gridPoints); // auto is tuple std::vector<std::tuple<Space ID, amount of gridpoints>>
// 	gridFile.close();
// 	std::filesystem::remove(gridPath);

// 	BOOST_REQUIRE_EQUAL(result.size(), 1u); // Only 1-surface thus 1 tuple in result vector
// 	BOOST_REQUIRE_EQUAL(std::get<0>(result[0]), 99u); // Check spaceID in result tuple
// 	BOOST_REQUIRE_EQUAL(std::get<1>(result[0]), 18); // check amount of gridpoints
// 	BOOST_REQUIRE_EQUAL(gridPoints.size(), 18u); // check amount of gridpoints

// 	// d=5m at the longest edge is 2 times the shortest edge, thus p=0.5*5^log10(5)=0.5*5^0.69897=1.54 m
// 	// gridPointamount in x axis is: 
// 	// rouned up((surface_length - 2*margin) / gridSize) = rouned up((5000 - 2*500) / 1540) = 4
// 	// Distance between grid points is then: (surface_length - 2*margin) / (gridPointAmount - 1) = (5000 - 2*500) / (4-1) = 1333.33 mm
// 	// gridPointamount in y axis is: 
// 	// rouned up((surface_length - 2*margin) / gridSize) = rouned to nearest integer((10000 - 2*500) / 1333.33) = 7
// 	// Distance between grid points is then: (surface_length - 2*margin) / (gridPointAmount - 1) = (10000 - 2*500) / (7-1) = 1500 mm

// 	// Check grid point amount
// 	auto xCoords = getUniqueSortedCoordinates(gridPoints, 0);
// 	auto yCoords = getUniqueSortedCoordinates(gridPoints, 1);
// 	bool expectedPointCounts = (xCoords.size() == 4u && yCoords.size() == 7u);
// 	BOOST_REQUIRE(expectedPointCounts);

// 	// Only check this for first case and in case space is not at ground level,
// 	// as it is assumed that the remainder cases will also be at the correct 
// 	// reference height if this is correct for the first
// 	// case, as the same code is used for all cases. 
// 	for (const auto& p : gridPoints){ BOOST_REQUIRE_SMALL(std::abs(p[2] - 1850.0), 1e-6);} // Check if all z-coordinates are at the reference height of 850.0 mm

// 	// Check distance between grid points and cell ratio
// 	double dx = stepBetweenFirstTwo(xCoords);
// 	double dy = stepBetweenFirstTwo(yCoords);

// 	BOOST_REQUIRE_SMALL(std::abs(dx - 1333.33), 0.001);
// 	BOOST_REQUIRE_SMALL(std::abs(dy - 1500.0), 0.001);
// 	double ratio = std::max(dx, dy) / std::min(dx, dy);
// 	BOOST_REQUIRE(ratio <= 2.0);
// }

// BOOST_AUTO_TEST_CASE( generateGridNEN_rectangle_surface_ratio_2_1_below_treshold )
// {// 
// 	bso::spatial_design::ms_N_building ms;

// 	// Rectangle space thus floor surface, with aspect ratio smaller then 2:1, 
// 	// thus longest edge of 7500mm should be used for p calculation, 
// 	// but grid should still be generated for entire rectangle and 
// 	// thus have more points in the longer dimension
// 	ms.addSpace(bso::spatial_design::ms_space("N,1, 0,0,-1000, 5000,0,-1000, 0,7500,-1000, 5000,7500,-1000, 0,0,5000, 5000,0,5000, 0,7500,5000, 5000,7500,5000"));
// 	bso::spatial_design::cf_building cf(ms);
	
// 	auto quadrilateral = bso::utilities::geometry::quadrilateral({{0,0,-1000},{5000,0,-1000},{0,7500,-1000},{5000,7500,-1000}});
// 	bso::spatial_design::conformal::cf_surface* surfPtr = nullptr;
// 	for (bso::spatial_design::conformal::cf_surface* sur : cf.cfSurfaces())
// 	{
// 		if (sur != nullptr && quadrilateral.isSameAs(*sur))
// 		{
// 			surfPtr = sur;
// 			break;
// 		}
// 	}
// 	BOOST_REQUIRE(surfPtr != nullptr); // check if the expected rectangle surface was found in the geometry model

// 	std::vector<cf::cf_surface> surfaces = {*surfPtr};
// 	std::vector<unsigned int> spaceID = {99}; // value doesn't matter for this test as long as it's consistent
// 	std::vector<double> dfSettings = {14400, 850.0, 500.0, 10.0}; // {horizontal_outdoor_illuminance[lx], reference_height[mm], space_margin[mm], minGridSize[m]}
// 	std::vector<bso::utilities::geometry::vertex> gridPoints;

// 	auto gridPath = std::filesystem::temp_directory_path() / "bso_generateGridNEN_rectangle_grid.txt";
// 	std::ofstream gridFile(gridPath.string(), std::ios::trunc);
// 	BOOST_REQUIRE(gridFile.is_open());

// 	auto result = dl_rule_set::generateGridNEN(surfaces, gridFile, spaceID, dfSettings, gridPoints); // auto is tuple std::vector<std::tuple<Space ID, amount of gridpoints>>
// 	gridFile.close();
// 	std::filesystem::remove(gridPath);

// 	BOOST_REQUIRE_EQUAL(result.size(), 1u); // Only 1-surface thus 1 tuple in result vector
// 	BOOST_REQUIRE_EQUAL(std::get<0>(result[0]), 99u); // Check spaceID in result tuple
// 	BOOST_REQUIRE_EQUAL(std::get<1>(result[0]), 15); // check amount of gridpoints
// 	BOOST_REQUIRE_EQUAL(gridPoints.size(), 15u); // check amount of gridpoints

// 	// d=7.5m , thus p=0.5*5^log10(7.5)=2.045 m
// 	// gridPointamount in x axis is: 
// 	// rouned up((surface_length - 2*margin) / gridSize) = rouned up((7500 - 2*500) / 2.045) = 4
// 	// Distance between grid points is then: (surface_length - 2*margin) / (gridPointAmount - 1) = (7500 - 2*500) / (4-1) = 2166.6666666666665 mm
// 	// gridPointamount in y axis is: 
// 	// rouned up((surface_length - 2*margin) / gridSize) = rouned to nearest integer((5000 - 2*500) / 2166.6666666666665) = 2
// 	// Distance between grid points is then: (surface_length - 2*margin) / (gridPointAmount - 1) = (5000 - 2*500) / (2-1) = 4000 mm


// 	// d=7.5m, thus p=0.5*5^log10(7.5)=2.04 m
// 	// gridPointamount in x axis is: 
// 	// rouned up((surface_length - 2*margin) / gridSize) = rouned up((5000 - 2*500) / 2045) = 2
// 	// Distance between grid points is then: (surface_length - 2*margin) / (gridPointAmount - 1) = (5000 - 2*500) / (3-1) = 4000 mm
// 	// gridPointamount in y axis is: 
// 	// rouned up((surface_length - 2*margin) / gridSize) = rouned up((7500 - 2*500) / 2045) = 4
// 	// Distance between grid points is then: (surface_length - 2*margin) / (gridPointAmount - 1) = (7500 - 2*500) / (4-1) = 2166 mm

// 	// Check grid point amount
// 	auto xCoords = getUniqueSortedCoordinates(gridPoints, 0);
// 	auto yCoords = getUniqueSortedCoordinates(gridPoints, 1);
// 	bool expectedPointCounts = (xCoords.size() ==4u && yCoords.size() == 2u);
// 	BOOST_REQUIRE(expectedPointCounts);

// 	// Only check this for first case and in case space is not at ground level,
// 	// as it is assumed that the remainder cases will also be at the correct 
// 	// reference height if this is correct for the first
// 	// case, as the same code is used for all cases. 
// 	for (const auto& p : gridPoints){ BOOST_REQUIRE_SMALL(std::abs(p[2] - 1850.0), 1e-6);} // Check if all z-coordinates are at the reference height of 850.0 mm

// 	// Check distance between grid points and cell ratio
// 	double dx = stepBetweenFirstTwo(xCoords);
// 	double dy = stepBetweenFirstTwo(yCoords);

// 	BOOST_REQUIRE_SMALL(std::abs(dx - 2166.6666666666665), 0.001);
// 	BOOST_REQUIRE_SMALL(std::abs(dy - 4000), 0.001);
// 	double ratio = std::max(dx, dy) / std::min(dx, dy);
// 	BOOST_REQUIRE(ratio <= 2.0);
// }

// BOOST_AUTO_TEST_CASE( generateGridNEN_rectangle_surface_ratio_2_1_Large_above_treshold )
// {// 
// 	bso::spatial_design::ms_N_building ms;

// 	// Rectangle space thus floor surface, with aspect ratio smaller then 2:1, 
// 	// thus longest edge of 7500mm should be used for p calculation, 
// 	// but grid should still be generated for entire rectangle and 
// 	// thus have more points in the longer dimension
// 	ms.addSpace(bso::spatial_design::ms_space("N,1, 0,0,0, 5000,0,0, 0,20000,0, 5000,20000,0, 0,0,5000, 5000,0,5000, 0,20000,5000, 5000,20000,5000"));
// 	bso::spatial_design::cf_building cf(ms);
	
// 	auto quadrilateral = bso::utilities::geometry::quadrilateral({{0,0,0},{5000,0,0},{0,20000,0},{5000,20000,0}});
// 	bso::spatial_design::conformal::cf_surface* surfPtr = nullptr;
// 	for (bso::spatial_design::conformal::cf_surface* sur : cf.cfSurfaces())
// 	{
// 		if (sur != nullptr && quadrilateral.isSameAs(*sur))
// 		{
// 			surfPtr = sur;
// 			break;
// 		}
// 	}
// 	BOOST_REQUIRE(surfPtr != nullptr); // check if the expected rectangle surface was found in the geometry model

// 	std::vector<cf::cf_surface> surfaces = {*surfPtr};
// 	std::vector<unsigned int> spaceID = {99}; // value doesn't matter for this test as long as it's consistent
// 	std::vector<double> dfSettings = {14400, 850.0, 500.0, 10.0}; // {horizontal_outdoor_illuminance[lx], reference_height[mm], space_margin[mm], minGridSize[m]}
// 	std::vector<bso::utilities::geometry::vertex> gridPoints;

// 	auto gridPath = std::filesystem::temp_directory_path() / "bso_generateGridNEN_rectangle_grid.txt";
// 	std::ofstream gridFile(gridPath.string(), std::ios::trunc);
// 	BOOST_REQUIRE(gridFile.is_open());

// 	auto result = dl_rule_set::generateGridNEN(surfaces, gridFile, spaceID, dfSettings, gridPoints); // auto is tuple std::vector<std::tuple<Space ID, amount of gridpoints>>
// 	gridFile.close();
// 	std::filesystem::remove(gridPath);

// 	BOOST_REQUIRE_EQUAL(result.size(), 1u); // Only 1-surface thus 1 tuple in result vector
// 	BOOST_REQUIRE_EQUAL(std::get<0>(result[0]), 99u); // Check spaceID in result tuple
// 	BOOST_REQUIRE_EQUAL(std::get<1>(result[0]), 39); // check amount of gridpoints
// 	BOOST_REQUIRE_EQUAL(gridPoints.size(), 39u); // check amount of gridpoints

// 	// d=5m as the longest edge is 2 times the shortest edge, thus p=0.5*5^log10(5)=0.5*5^0.69897=1.54 m
// 	// gridPointamount in x axis is: 
// 	// rouned up((surface_length - 2*margin) / gridSize) = rouned up((5000 - 2*500) / 1540) = 5
// 	// Distance between grid points is then: (surface_length - 2*margin) / (gridPointAmount - 1) = (5000 - 2*500) / (5-1) = 1000 mm
// 	// gridPointamount in y axis is: 
// 	// rouned up((surface_length - 2*margin) / gridSize) = rouned up((20000 - 2*500) / 1540) = 13
// 	// Distance between grid points is then: (surface_length - 2*margin) / (gridPointAmount - 1) = (20000 - 2*500) / (13-1) = 1583.333333333333 mm

// 	// Check grid point amount
// 	auto xCoords = getUniqueSortedCoordinates(gridPoints, 0);
// 	auto yCoords = getUniqueSortedCoordinates(gridPoints, 1);
// 	bool expectedPointCounts = (xCoords.size() == 3u && yCoords.size() == 13u);
// 	BOOST_REQUIRE(expectedPointCounts);

// 	// Check distance between grid points and cell ratio
// 	double dx = stepBetweenFirstTwo(xCoords);
// 	double dy = stepBetweenFirstTwo(yCoords);

// 	BOOST_REQUIRE_SMALL(std::abs(dx - 2000.0), 0.001);
// 	BOOST_REQUIRE_SMALL(std::abs(dy - 1583.333333333333), 0.001);
// 	double ratio = std::max(dx, dy) / std::min(dx, dy);
// 	BOOST_REQUIRE(ratio <= 2.0);
// }

// BOOST_AUTO_TEST_CASE( generateGridNEN_square_surface_too_narrow_for_margin )
// {// 
// 	bso::spatial_design::ms_N_building ms;

// 	// Rectangle space thus floor surface, with aspect ratio smaller then 2:1, 
// 	// thus longest edge of 7500mm should be used for p calculation, 
// 	// but grid should still be generated for entire rectangle and 
// 	// thus have more points in the longer dimension
// 	ms.addSpace(bso::spatial_design::ms_space("N,1, 0,0,0, 800,0,0, 0,800,0, 800,800,0, 0,0,1000, 800,0,1000, 0,800,1000, 800,800,1000"));
// 	bso::spatial_design::cf_building cf(ms);
	
// 	auto quadrilateral = bso::utilities::geometry::quadrilateral({{0,0,0},{800,0,0},{0,800,0},{800,800,0}});
// 	bso::spatial_design::conformal::cf_surface* surfPtr = nullptr;
// 	for (bso::spatial_design::conformal::cf_surface* sur : cf.cfSurfaces())
// 	{
// 		if (sur != nullptr && quadrilateral.isSameAs(*sur))
// 		{
// 			surfPtr = sur;
// 			break;
// 		}
// 	}
// 	BOOST_REQUIRE(surfPtr != nullptr); // check if the expected rectangle surface was found in the geometry model

// 	std::vector<cf::cf_surface> surfaces = {*surfPtr};
// 	std::vector<unsigned int> spaceID = {99}; // value doesn't matter for this test as long as it's consistent
// 	std::vector<double> dfSettings = {14400, 850.0, 500.0, 10.0}; // {horizontal_outdoor_illuminance[lx], reference_height[mm], space_margin[mm], minGridSize[m]}
// 	std::vector<bso::utilities::geometry::vertex> gridPoints;

// 	auto gridPath = std::filesystem::temp_directory_path() / "bso_generateGridNEN_rectangle_grid.txt";
// 	std::ofstream gridFile(gridPath.string(), std::ios::trunc);
// 	BOOST_REQUIRE(gridFile.is_open());

// 	auto result = dl_rule_set::generateGridNEN(surfaces, gridFile, spaceID, dfSettings, gridPoints); // auto is tuple std::vector<std::tuple<Space ID, amount of gridpoints>>
// 	gridFile.close();
// 	std::filesystem::remove(gridPath);

// 	BOOST_REQUIRE_EQUAL(result.size(), 1u); // Only 1-surface thus 1 tuple in result vector
// 	BOOST_REQUIRE_EQUAL(std::get<0>(result[0]), 99u); // Check spaceID in result tuple
// 	BOOST_REQUIRE_EQUAL(std::get<1>(result[0]), 39); // check amount of gridpoints
// 	BOOST_REQUIRE_EQUAL(gridPoints.size(), 39u); // check amount of gridpoints

// 	// d=0.8m at the longest edge is 2 times the shortest edge, thus p=0.5*5^log10(0.8)=0.428 m
// 	// gridPointamount in x and y axis is: 
// 	// rouned up((surface_length - 2*margin) / gridSize) = rouned up((800 - 2*500) / 1540) = 3
// 	// Distance between grid points is then: (surface_length - 2*margin) / (gridPointAmount - 1) = (800 - 2*500) / (3-1) = 2000 mm

// 	// Check grid point amount
// 	auto xCoords = getUniqueSortedCoordinates(gridPoints, 0);
// 	auto yCoords = getUniqueSortedCoordinates(gridPoints, 1);
// 	bool expectedPointCounts = (xCoords.size() == 3u && yCoords.size() == 13u);
// 	BOOST_REQUIRE(expectedPointCounts);

// 	// Check distance between grid points and cell ratio
// 	double dx = stepBetweenFirstTwo(xCoords);
// 	double dy = stepBetweenFirstTwo(yCoords);

// 	BOOST_REQUIRE_SMALL(std::abs(dx - 2000.0), 0.001);
// 	BOOST_REQUIRE_SMALL(std::abs(dy - 1583.333333333333), 0.001);
// 	double ratio = std::max(dx, dy) / std::min(dx, dy);
// 	BOOST_REQUIRE(ratio <= 2.0);
// }

// BOOST_AUTO_TEST_CASE( generateGridNEN_rectangle_surface_too_narrow_for_margin )
// {// 
// 	bso::spatial_design::ms_N_building ms;

// 	// Rectangle space thus floor surface, with aspect ratio smaller then 2:1, 
// 	// thus longest edge of 7500mm should be used for p calculation, 
// 	// but grid should still be generated for entire rectangle and 
// 	// thus have more points in the longer dimension
// 	ms.addSpace(bso::spatial_design::ms_space("N,1, 0,0,0, 800,0,0, 0,4000,0, 800,4000,0, 0,0,1000, 800,0,1000, 0,4000,1000, 800,4000,1000"));
// 	bso::spatial_design::cf_building cf(ms);

// 	auto quadrilateral = bso::utilities::geometry::quadrilateral({{0,0,0},{800,0,0},{0,4000,0},{800,4000,0}});
// 	bso::spatial_design::conformal::cf_surface* surfPtr = nullptr;
// 	for (bso::spatial_design::conformal::cf_surface* sur : cf.cfSurfaces())
// 	{
// 		if (sur != nullptr && quadrilateral.isSameAs(*sur))
// 		{
// 			surfPtr = sur;
// 			break;
// 		}
// 	}
// 	BOOST_REQUIRE(surfPtr != nullptr); // check if the expected rectangle surface was found in the geometry model

// 	std::vector<cf::cf_surface> surfaces = {*surfPtr};
// 	std::vector<unsigned int> spaceID = {99}; // value doesn't matter for this test as long as it's consistent
// 	std::vector<double> dfSettings = {14400, 850.0, 500.0, 10.0}; // {horizontal_outdoor_illuminance[lx], reference_height[mm], space_margin[mm], minGridSize[m]}
// 	std::vector<bso::utilities::geometry::vertex> gridPoints;

// 	auto gridPath = std::filesystem::temp_directory_path() / "bso_generateGridNEN_rectangle_grid.txt";
// 	std::ofstream gridFile(gridPath.string(), std::ios::trunc);
// 	BOOST_REQUIRE(gridFile.is_open());

// 	auto result = dl_rule_set::generateGridNEN(surfaces, gridFile, spaceID, dfSettings, gridPoints); // auto is tuple std::vector<std::tuple<Space ID, amount of gridpoints>>
// 	gridFile.close();
// 	std::filesystem::remove(gridPath);

// 	BOOST_REQUIRE_EQUAL(result.size(), 1u); // Only 1-surface thus 1 tuple in result vector
// 	BOOST_REQUIRE_EQUAL(std::get<0>(result[0]), 99u); // Check spaceID in result tuple
// 	BOOST_REQUIRE_EQUAL(std::get<1>(result[0]), 39); // check amount of gridpoints
// 	BOOST_REQUIRE_EQUAL(gridPoints.size(), 39u); // check amount of gridpoints

// 	// d=0.8m at the longest edge is 2 times the shortest edge, thus p=0.5*5^log10(0.8)=0.428 m
// 	// gridPointamount in x axis is: 
// 	// rouned up((surface_length - 2*margin) / gridSize) = rouned up((800 - 2*500) / 1540) = 3
// 	// Distance between grid points is then: (surface_length - 2*margin) / (gridPointAmount - 1) = (800 - 2*500) / (3-1) = 2000 mm
// 	// gridPointamount in y axis is: 
// 	// rouned up((surface_length - 2*margin) / gridSize) = rouned up((4000 - 2*500) / 1540) = 13
// 	// Distance between grid points is then: (surface_length - 2*margin) / (gridPointAmount - 1) = (4000 - 2*500) / (13-1) = 1583.333333333333 mm

// 	// Check grid point amount
// 	auto xCoords = getUniqueSortedCoordinates(gridPoints, 0);
// 	auto yCoords = getUniqueSortedCoordinates(gridPoints, 1);
// 	bool expectedPointCounts = (xCoords.size() == 3u && yCoords.size() == 13u);
// 	BOOST_REQUIRE(expectedPointCounts);

// 	// Check distance between grid points and cell ratio
// 	double dx = stepBetweenFirstTwo(xCoords);
// 	double dy = stepBetweenFirstTwo(yCoords);

// 	BOOST_REQUIRE_SMALL(std::abs(dx - 2000.0), 0.001);
// 	BOOST_REQUIRE_SMALL(std::abs(dy - 1583.333333333333), 0.001);
// 	double ratio = std::max(dx, dy) / std::min(dx, dy);
// 	BOOST_REQUIRE(ratio <= 2.0);
// }

// BOOST_AUTO_TEST_CASE( generateGridNEN_non_square_surface_obeys_margin_and_cell_ratio )
// {// 
// 	bso::spatial_design::ms_N_building ms;

// 	// Square space thus floor surface
// 	ms.addSpace(bso::spatial_design::ms_space("N,1, 0,0,0, 2000,0,0, 0,3500,0, 5000,3500,0, 0,0,5000, 2000,0,5000, 0,3500,5000, 5000,3500,5000"));
// 	bso::spatial_design::cf_building cf(ms);

// 	auto quadrilateral = bso::utilities::geometry::quadrilateral({{0,0,0},{2000,0,0},{0,3500,0},{5000,3500,0}});
// 	bso::spatial_design::conformal::cf_surface* surfPtr = nullptr;
// 	for (bso::spatial_design::conformal::cf_surface* sur : cf.cfSurfaces())
// 	{
// 		if (sur != nullptr && quadrilateral.isSameAs(*sur))
// 		{
// 			surfPtr = sur;
// 			break;
// 		}
// 	}
// 	BOOST_REQUIRE(surfPtr != nullptr); // check if the expected rectangle surface was found in the geometry model

// 	std::vector<cf::cf_surface> surfaces = {*surfPtr};
// 	std::vector<unsigned int> spaceID = {99}; // value doesn't matter for this test as long as it's consistent
// 	std::vector<double> dfSettings = {14400, 850.0, 500.0, 10.0}; // {horizontal_outdoor_illuminance[lx], reference_height[mm], space_margin[mm], minGridSize[m]}
// 	std::vector<bso::utilities::geometry::vertex> gridPoints;

// 	auto gridPath = std::filesystem::temp_directory_path() / "bso_generateGridNEN_grid.txt";
// 	std::ofstream gridFile(gridPath.string(), std::ios::trunc);
// 	BOOST_REQUIRE(gridFile.is_open());

// 	auto result = dl_rule_set::generateGridNEN(surfaces, gridFile, spaceID, dfSettings, gridPoints); // auto is tuple std::vector<std::tuple<Space ID, amount of gridpoints>>
// 	gridFile.close();
// 	std::filesystem::remove(gridPath);

// 	BOOST_REQUIRE_EQUAL(result.size(), 1u); // Only 1-surface thus 1 tuple in result vector
// 	BOOST_REQUIRE_EQUAL(std::get<0>(result[0]), 99u); // Check spaceID in result tuple
// 	BOOST_REQUIRE_EQUAL(std::get<1>(result[0]), 9); // check amount of gridpoints
// 	BOOST_REQUIRE_EQUAL(gridPoints.size(), 9u); // check amount of gridpoints

// 	// d=2000 m thus p=0.5*5^log10(2)=0.8117 m
// 	// gridPointamount in either x or y axis is: 
// 	// rouned up((surface_length - 2*margin) / gridSize) = rouned up((2000 - 2*500) / 811.7) = 2
// 	// Distance between grid points is then: (surface_length - 2*margin) / (gridPointAmount - 1) = (2000 - 2*500) / (2-1) = 1000 mm

// 	// Check grid point amount
// 	auto xCoords = getUniqueSortedCoordinates(gridPoints, 0);
// 	auto yCoords = getUniqueSortedCoordinates(gridPoints, 1);
// 	bool expectedPointCounts = (xCoords.size() == 4u && yCoords.size() == 4u);
// 	BOOST_REQUIRE(expectedPointCounts);

// 	// Only check this for first case and in case space is not at ground level,
// 	// as it is assumed that the remainder cases will also be at the correct 
// 	// reference height if this is correct for the first
// 	// case, as the same code is used for all cases. 
// 	for (const auto& p : gridPoints){ BOOST_REQUIRE_SMALL(std::abs(p[2] - 850.0), 1e-6);} // Check if all z-coordinates are at the reference height of 850.0 mm

// 	// Check distance between grid points and cell ratio
// 	double dx = stepBetweenFirstTwo(xCoords);
// 	double dy = stepBetweenFirstTwo(yCoords);

// 	BOOST_REQUIRE_SMALL(std::abs(dx - 1333.33), 0.001);
// 	BOOST_REQUIRE_SMALL(std::abs(dy - 1333.33), 0.001);
// 	double ratio = std::max(dx, dy) / std::min(dx, dy);
// 	BOOST_REQUIRE(ratio <= 2.0);
// }


BOOST_AUTO_TEST_SUITE_END()
} // namespace grammar_dl_rule_set_test
