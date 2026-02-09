#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE delaunay
#endif

#include <boost/test/included/unit_test.hpp>

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

	BOOST_AUTO_TEST_CASE( performDelaunay_1 )
	{
		// delaunayVertices and delaunayLines represent ms from first test getNOBSDAtGround_1.
		// the addition of vertex {6000.00001,0,0} should result in an FADE_2D triangle which is 
		// unvalid for BSO-toolbox and therefore filtered out before returning it from the delaunay function
		std::vector <bso::utilities::geometry::vertex> delaunayVertices = {{0,0,0},{3000,0,0},{0,3000,0},{3000,3000,0},{6000,0,0},
																		{6000,3000,0},{1500,1500,0},{-1500,1500,0},{-1500,-1500,0},
																		{1500,-1500,0},{1500,0,0},{0,1500,0},{6000.00001,0,0}}; //
		std::vector <bso::utilities::geometry::line_segment> delaunayLines = {{{0,0,0},{1500,0,0}},{{0,0,0},{0,1500,0}},{{1500,0,0},{1500,1500,0}},{{0,1500,0},{1500,1500,0}},
																		{{1500,0,0},{3000,0,0}},{{0,1500,0},{0,3000,0}},{{3000,0,0},{3000,3000,0}},{{0,3000,0},{3000,3000,0}},
																		{{3000,3000,0},{6000,3000,0}},{{6000,3000,0},{6000,0,0}},{{3000,0,0},{6000,0,0}},{{1500,0,0},{1500,-1500,0}},
																		{{1500,-1500,0},{-1500,-1500,0}},{{-1500,-1500,0,},{-1500,1500,0}},{{-1500,1500,0},{0,1500,0}}};
		
		std::vector<bso::utilities::geometry::triangle> delaunayResults = bso::utilities::delaunay(delaunayVertices, delaunayLines);
		std::vector<bso::utilities::geometry::triangle> checkTriangles = {{{1500,1500,0},{3000,3000,0},{0,3000,0}},{{1500,1500,0},{3000,0,0},{3000,3000,0}},
																		 {{6000,0,0},{3000,3000,0},{3000,0,0}},{{6000,3000,0},{3000,3000,0},{6000,0,0}},
																		 {{0,1500,0},{0,0,0},{1500,1500,0}},{{1500,0,0},{1500,-1500,0},{3000,0,0}},
																		 {{0,1500,0},{1500,1500,0},{0,3000,0}},{{-1500,-1500,0},{0,0,0},{-1500,1500,0}},
																		 {{1500,-1500,0},{0,0,0},{-1500,-1500,0}},{{1500,0,0},{0,0,0},{1500,-1500,0}},
																		 {{1500,-1500,0},{6000,0,0},{3000,0,0}}, {{1500,0,0},{3000,0,0},{1500,1500,0}},
																		 {{1500,0,0},{1500,1500,0},{0,0,0}},{{0,1500,0},{0,3000,0},{-1500,1500,0}},
																		 {{0,1500,0},{-1500,1500,0},{0,0,0}}
																		 };
		
		
		BOOST_REQUIRE(delaunayResults.size() == 15);
		bool anyNotFound = false;
		for(const bso::utilities::geometry::triangle i: checkTriangles)
		{
			bool found = false;
			for(const bso::utilities::geometry::triangle j: delaunayResults)
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

	BOOST_AUTO_TEST_CASE( performGetOuterBounds_1 )
	{
		std::vector <bso::utilities::geometry::line_segment> lines = { 
		{{3000,-4000,0},{21000,-4000,0}}, {{21000,-4000,0},{21000,22000,0}},
		{{21000,22000,0},{3000,22000,0}}, {{3000,22000,0},{3000,-4000,0}},
		{{13000,-2000,0},{13000,20000,0}}, {{13000,20000,0},{3000,20000,0}},
		{{3000,20000,0},{3000,-2000,0}}, {{3000,-2000,0},{13000,-2000,0}},
		{{13000,-4000,0},{13000,22000,0}}, {{13000,22000,0},{3000,22000,0}},
		{{3000,-4000,0},{13000,-4000,0}}, {{13000,-2000,0},{16000,-2000,0}},
		{{16000,-2000,0},{16000,20000,0}}, {{16000,20000,0},{13000,20000,0}},
		{{16000,-2000,0},{21000,-2000,0}}, {{21000,-2000,0},{21000,20000,0}},
		{{21000,20000,0},{16000,20000,0}}, {{16000,-4000,0},{21000,-4000,0}},
		{{21000,22000,0},{16000,22000,0}}, {{16000,22000,0},{16000,-4000,0}},
		{{3000,0,0},{18000,0,0}}, {{18000,0,0},{18000,15000,0}},
		{{18000,15000,0},{3000,15000,0}}, {{3000,15000,0},{3000,0,0}}};
		
		std::vector <bso::utilities::geometry::line_segment> linesAfterSplit = { 
		{{13000,-2000,0},{13000,20000,0}}, {{13000,20000,0},{3000,20000,0}},
		{{3000,-2000,0},{13000,-2000,0}},{{13000,22000,0},{3000,22000,0}},
		{{3000,-4000,0},{13000,-4000,0}},{{13000,-2000,0},{16000,-2000,0}},
		{{16000,-2000,0},{16000,20000,0}},{{16000,20000,0},{13000,20000,0}},
		{{16000,-2000,0},{21000,-2000,0}},{{21000,-2000,0},{21000,20000,0}},
		{{21000,20000,0},{16000,20000,0}},{{16000,-4000,0},{21000,-4000,0}},
		{{21000,22000,0},{16000,22000,0}},{{3000,0,0},{18000,0,0}},
		{{18000,0,0},{18000,15000,0}},{{18000,15000,0},{3000,15000,0}},
		{{3000,15000,0},{3000,0,0}},{{21000,-4000,0},{21000,-2000,0}},
		{{3000,22000,0},{3000,20000,0}},{{3000,-2000,0},{3000,0,0}},
		{{13000,-4000,0},{13000,-2000,0}},{{16000,-4000,0},{16000,-2000,0}},
		{{13000,-4000,0},{16000,-4000,0}},{{21000,22000,0},{21000,20000,0}},
		{{13000,22000,0},{16000,22000,0}},{{3000,-4000,0},{3000,-2000,0}},
		{{3000,20000,0},{3000,15000,0}},{{13000,22000,0},{13000,20000,0}},
		{{16000,22000,0},{16000,20000,0}}};

		std::cout << "lines after split: " << std::endl;
		for(const auto i: lines)
		{
			std::cout << i << std::endl;
		}

		std::vector <bso::utilities::geometry::line_segment> checkLines = { 
		{{3000,-4000,0},{13000,-4000,0}}, {{13000,-4000,0},{16000,-4000,0}},
		{{16000,-4000,0},{21000,-4000,0}}, {{21000,-4000,0},{21000,-2000,0}},
		{{21000,-2000,0},{21000,20000,0}}, {{21000,20000,0},{21000,22000,0}},
		{{21000,22000,0},{16000,22000,0}}, {{16000,22000,0},{13000,22000,0}},
		{{13000,22000,0},{3000,22000,0}}, {{3000,22000,0},{3000,20000,0}},
		{{3000,20000,0},{3000,15000,0}}, {{3000,15000,0},{3000,0,0}},
		{{3000,0,0},{3000,-2000,0}}, {{3000,-2000,0},{3000,-4000,0}}};

		std::vector <bso::utilities::geometry::line_segment> boundsLines = bso::utilities::getOuterBounds2D(lines, 0.001);

		BOOST_REQUIRE(boundsLines.size() == 14);
		for(int i=0; i<14; i++)
		{
			BOOST_REQUIRE(boundsLines[i] == checkLines[i]);
		}
	}

BOOST_AUTO_TEST_SUITE_END()
} // namespace utilities_test