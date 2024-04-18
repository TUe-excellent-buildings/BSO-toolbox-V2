#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE trim_and_cast
#endif

#include <boost/test/included/unit_test.hpp>

#include <bso/spatial_design/ms_N_building.hpp>
#include <bso/utilities/geometry.hpp>
#include <bso/utilities/delaunay.hpp>

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

BOOST_AUTO_TEST_SUITE( delaunay )

	BOOST_AUTO_TEST_CASE( getNOBSDAtGround_1 ) 
	{
		bso::spatial_design::ms_N_building ms;
		bso::spatial_design::ms_space s1 = bso::spatial_design::ms_space("N,1,	0,0,0,	3000,0,0,	0,3000,0,	3000,3000,0,	0,0,3000,	3000,0,3000,	0,3000,3000,	3000,3000,3000");
		bso::spatial_design::ms_space s2 = bso::spatial_design::ms_space("N,2,	3000,0,0,	6000,0,0,	3000,3000,0,	6000,3000,0,	3000,0,3000,	6000,0,3000,	3000,3000,3000,	6000,3000,3000");
		bso::spatial_design::ms_space s3 = bso::spatial_design::ms_space("N,3,	-1500,0,3000,	1500,0,3000,	-1500,3000,3000,	1500,3000,3000,	-1500,0,6000,	1500,0,6000,	-1500,3000,6000,	1500,3000,6000");

		ms.addSpace(s1);
		ms.addSpace(s2);
		ms.addSpace(s3);

		std::vector<std::pair<bso::spatial_design::ms_space*, bso::utilities::geometry::quad_hexahedron>> spaceAtGroundV;
		std::vector<std::pair<bso::spatial_design::ms_space*, bso::utilities::geometry::quad_hexahedron>> spaceAtGroundV2;
		std::vector<double> allLevels;

		// These functions get the above defined information
		bso::utilities::getNOBSDAtGround(ms, spaceAtGroundV, allLevels);
		bso::utilities::getNOBSDAtGround(ms, spaceAtGroundV2);

		BOOST_REQUIRE(spaceAtGroundV.size() == 3);
		BOOST_REQUIRE(allLevels.size() == 3);
		BOOST_REQUIRE(allLevels[0] == 0);
		BOOST_REQUIRE(allLevels[1] == 3000);
		BOOST_REQUIRE(allLevels[2] == 6000);
		BOOST_REQUIRE(*(spaceAtGroundV[0].first) == s1);
		BOOST_REQUIRE(*(spaceAtGroundV[1].first) == s2);
		BOOST_REQUIRE(*(spaceAtGroundV[2].first) == s3);

		BOOST_REQUIRE(spaceAtGroundV2.size() == 3);
		BOOST_REQUIRE(*(spaceAtGroundV2[0].first) == s1);
		BOOST_REQUIRE(*(spaceAtGroundV2[1].first) == s2);
		BOOST_REQUIRE(*(spaceAtGroundV2[2].first) == s3);

		bso::utilities::geometry::quad_hexahedron QH1 = {{0,0,0},{3000,0,0,},{0,3000,0},{3000,3000,0},
														 {0,0,3000},{3000,0,3000},{0,3000,3000},{3000,3000,3000}};
		bso::utilities::geometry::quad_hexahedron QH2 = {{3000,0,0},{6000,0,0,},{3000,3000,0},{6000,3000,0},
														 {3000,0,3000},{6000,0,3000},{3000,3000,3000},{6000,3000,3000}};
		bso::utilities::geometry::quad_hexahedron QH3 = {{-1500,0,0},{1500,0,0,},{-1500,3000,0},{1500,3000,0},
														 {-1500,0,3000},{1500,0,3000},{-1500,3000,3000},{1500,3000,3000}};

		BOOST_REQUIRE(spaceAtGroundV[0].second.isSameAs(QH1));
		BOOST_REQUIRE(spaceAtGroundV[1].second.isSameAs(QH2));
		BOOST_REQUIRE(spaceAtGroundV[2].second.isSameAs(QH3));	
		BOOST_REQUIRE(spaceAtGroundV2[0].second.isSameAs(QH1));
		BOOST_REQUIRE(spaceAtGroundV2[1].second.isSameAs(QH2));
		BOOST_REQUIRE(spaceAtGroundV2[2].second.isSameAs(QH3));										 
	}

	BOOST_AUTO_TEST_CASE( getDelaunayVariables_1 )
	{
		bso::spatial_design::ms_N_building ms;
		bso::spatial_design::ms_space s1 = bso::spatial_design::ms_space("N,1,	0,0,0,	3000,0,0,	0,3000,0,	3000,3000,0,	0,0,3000,	3000,0,3000,	0,3000,3000,	3000,3000,3000");
		bso::spatial_design::ms_space s2 = bso::spatial_design::ms_space("N,2,	3000,0,0,	6000,0,0,	3000,3000,0,	6000,3000,0,	3000,0,3000,	6000,0,3000,	3000,3000,3000,	6000,3000,3000");
		bso::spatial_design::ms_space s3 = bso::spatial_design::ms_space("N,3,	-1500,-1500,3000,	1500,-1500,3000,	-1500,1500,3000,	1500,1500,3000,	-1500,-1500,6000,	1500,-1500,6000,	-1500,1500,6000,	1500,1500,6000");

		ms.addSpace(s1);
		ms.addSpace(s2);
		ms.addSpace(s3);

		std::vector<std::pair<bso::spatial_design::ms_space*, bso::utilities::geometry::quad_hexahedron>> spaceAtGroundV;
		std::vector <bso::utilities::geometry::vertex> delaunayPoints;
		std::vector <bso::utilities::geometry::line_segment> delaunayLines;
		bso::utilities::getNOBSDAtGround(ms, spaceAtGroundV);
		bso::utilities::getDelaunayVariables(delaunayPoints, delaunayLines, spaceAtGroundV);

		std::vector <bso::utilities::geometry::vertex> checkDelaunayV = {{0,0,0},{3000,0,0},{0,3000,0},{3000,3000,0},{6000,0,0},
																		{6000,3000,0},{1500,1500,0},{-1500,1500,0},{-1500,-1500,0},
																		{1500,-1500,0},{1500,0,0},{0,1500,0}};

		BOOST_REQUIRE(delaunayPoints.size() == 12);
		bool anyNotFound = false;
		for(const bso::utilities::geometry::vertex i: checkDelaunayV)
		{
			bool found = false;
			for(const bso::utilities::geometry::vertex j: delaunayPoints)
			{
				if(i.isSameAs(j)) 
				{
					found = true;
					break;
				}
			}
			if(!found)
			{
				anyNotFound = true;
				break;
			}
		}
		BOOST_REQUIRE(!anyNotFound);

		std::vector <bso::utilities::geometry::line_segment> checkDelaunayL = {{{0,0,0},{1500,0,0}},{{0,0,0},{0,1500,0}},{{1500,0,0},{1500,1500,0}},{{0,1500,0},{1500,1500,0}},
																		{{1500,0,0},{3000,0,0}},{{0,1500,0},{0,3000,0}},{{3000,0,0},{3000,3000,0}},{{0,3000,0},{3000,3000,0}},
																		{{3000,3000,0},{6000,3000,0}},{{6000,3000,0},{6000,0,0}},{{3000,0,0},{6000,0,0}},{{1500,0,0},{1500,-1500,0}},
																		{{1500,-1500,0},{-1500,-1500,0}},{{-1500,-1500,0,},{-1500,1500,0}},{{-1500,1500,0},{0,1500,0}}};	
			
		BOOST_REQUIRE(delaunayLines.size() == 15);
		anyNotFound = false;
		for(const bso::utilities::geometry::line_segment i: checkDelaunayL)
		{
			bool found = false;
			for(const bso::utilities::geometry::line_segment j: delaunayLines)
			{
				if(i.isSameAs(j)) 
				{
					found = true;
					break;
				}
			}
			if(!found)
			{
				anyNotFound = true;
				break;
			}
		}
		BOOST_REQUIRE(!anyNotFound);
	}

	BOOST_AUTO_TEST_CASE( splitLines_1 ) 
	{
		std::vector <bso::utilities::geometry::vertex> delaunayVertices = {{0,0,0},{3000,0,0},{0,3000,0},{3000,3000,0},{6000,0,0},
																		{6000,3000,0},{1500,1500,0},{-1500,-1500,0},
																		{1500,-1500,0},{0,1500,0}};

		std::vector <bso::utilities::geometry::line_segment> delaunayLines = {{{0,0,0},{3000,0,0}},{{0,0,0},{0,3000,0}},{{1500,1500,0},{1500,-1500,0}},{{1500,1500,0},{0,1500,0}},
																		{{-1500,-1500,0},{1500,-1500,0}},{{3000,0,0},{3000,3000,0}},{{6000,0,0},{6000,6000,0}}};	

		bso::utilities::splitLines(delaunayVertices, delaunayLines);

		std::vector <bso::utilities::geometry::line_segment> checkLines = {{{0,0,0},{1500,0,0}},{{0,0,0},{0,1500,0}},{{1500,0,0},{1500,1500,0}},{{0,1500,0},{1500,1500,0}},
																		{{1500,0,0},{3000,0,0}},{{0,1500,0},{0,3000,0}},{{3000,0,0},{3000,3000,0}},{{6000,0,0},{6000,6000,0}},
																		{{-1500,-1500,0},{1500,-1500,0}}};

		std::cout << delaunayLines.size() << std::endl;
		for(const auto i: delaunayLines)
		{
			std::cout << i << std::endl;
		}
		BOOST_REQUIRE(delaunayLines.size() == 9);
		bool anyNotFound = false;
		for(const bso::utilities::geometry::line_segment i: checkLines)
		{
			bool found = false;
			for(const bso::utilities::geometry::line_segment j: delaunayLines)
			{
				if(i.isSameAs(j)) 
				{
					found = true;
					break;
				}
			}
			if(!found)
			{
				anyNotFound = true;
				break;
			}
		}
		BOOST_REQUIRE(!anyNotFound);
	}

	BOOST_AUTO_TEST_CASE( getQuadAtZ0_1 ) 
	{
		

	}

	BOOST_AUTO_TEST_CASE( getLinesAtZ0_1 ) 
	{
		

	}

	BOOST_AUTO_TEST_CASE( performDelaunay_1 )
	{


	}

BOOST_AUTO_TEST_SUITE_END()
} // namespace utilities_test