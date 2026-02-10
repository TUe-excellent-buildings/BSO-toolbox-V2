#ifndef GEOMETRY_FUNCTIONS_HPP
#define GEOMETRY_FUNCTIONS_HPP

#include <vector>
#include <bso/utilities/geometry.hpp>
#include <bso/utilities/geometry/polygon_non_convex.hpp>

namespace bso { namespace utilities {
	
	std::vector<utilities::geometry::line_segment> getOuterBounds2D(std::vector<utilities::geometry::line_segment> lines,  const double mTol = 1e-3);
	void sortLinesToChain(std::vector<utilities::geometry::line_segment>& lines,  const double mTol = 1e-3);

	bool hasCourtyard(std::vector<bso::utilities::geometry::quadrilateral> exteriorWalls, std::vector<std::vector<bso::utilities::geometry::line_segment>>& outerboundsExport, std::vector<bso::utilities::geometry::triangle>& triangleExport, std::vector<std::pair<double,std::vector<bso::utilities::geometry::polygon_non_convex>>>& courtyardsEnclosedPerElevation, std::vector<std::pair<double,std::vector<bso::utilities::geometry::polygon_non_convex>>>& courtyardsConnectedToAirPerElevation, const double mTol = 1e-3);
	bool comparisonFirstOfPair(std::pair<double,std::vector<bso::utilities::geometry::line_segment>> i, 
			std::pair<double,std::vector<bso::utilities::geometry::line_segment>> j){return (i.first < j.first);} // needed in function hasCourtyard

} // namespace utilities
} // namespace bso

#include <bso/utilities/geometry_functions.cpp>

#endif // GEOMETRY_FUNCTIONS_HPP

