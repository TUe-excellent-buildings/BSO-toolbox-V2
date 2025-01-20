#ifndef GEOMETRY_FUNCTIONS_HPP
#define GEOMETRY_FUNCTIONS_HPP

#include <vector>
#include <bso/utilities/geometry.hpp>

namespace bso { namespace utilities {

	 /*
	 * The following function is made to take the points and orderedLinesSegments of a polygon_non_convex and return quads for this polygon_non_convex
	 * The Polygon_non_convex could not be used duo to 
	 */ 
	//void convertToQuads(const std::vector <utilities::geometry::vertex>& delaunayPoints, const std::vector <utilities::geometry::line_segment>& orderedLinesOfZone,
			//std::vector <utilities::geometry::quadrilateral>& quads, const double mTol = 1e-3, bool outputPs = false);
	
	/*

	*/
	// void splitLines(const std::vector<utilities::geometry::vertex>& intsecV, std::vector<utilities::geometry::line_segment>& intsecL, bool removeDuplicates = true, const double mTol = 1e-3); // used during the delaunay method to avoid the inserting of overlapping and duplicate constrain lines, therefore is this function for now defined in this class
	std::vector<utilities::geometry::line_segment> getOuterBounds2D(std::vector<utilities::geometry::line_segment> lines,  const double mTol = 1e-3);
	void sortLinesToChain(std::vector<utilities::geometry::line_segment>& lines,  const double mTol = 1e-3);

	/*

	*/
	bool hasCourtyard(std::vector<bso::utilities::geometry::quadrilateral> exteriorWalls, std::vector<std::vector<bso::utilities::geometry::line_segment>>& outerboundsExport, std::vector<bso::utilities::geometry::triangle>& triangleExport, std::vector<std::pair<double,std::vector<bso::utilities::geometry::polygon_non_convex>>>& courtyardsEnclosedPerElevation, std::vector<std::pair<double,std::vector<bso::utilities::geometry::polygon_non_convex>>>& courtyardsConnectedToAirPerElevation, const double mTol = 1e-3);
	bool comparisonFirstOfPair(std::pair<double,std::vector<bso::utilities::geometry::line_segment>> i, 
			std::pair<double,std::vector<bso::utilities::geometry::line_segment>> j){return (i.first < j.first);} // needed in function hasCourtyard

} // namespace utilities
} // namespace bso

#include <bso/utilities/geometry_functions.cpp>

#endif // GEOMETRY_FUNCTIONS_HPP

