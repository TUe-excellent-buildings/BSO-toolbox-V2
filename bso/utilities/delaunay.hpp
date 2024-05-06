#ifndef DELAUNAY_HPP
#define DELAUNAY_HPP

#include <vector>
#include <bso/utilities/geometry.hpp>
#include <bso/spatial_design/ms_space.hpp>

namespace bso { namespace utilities {

	/*
	 * The following functions in this file are used to perform delaunay triangulations
	   transform a building spatial design representation
	 * to a 2D representation in triangles of the building at z=0.
	 */
	 
	std::vector<utilities::geometry::triangle> delaunay(const std::vector <utilities::geometry::vertex> delaunayPoints, const std::vector <utilities::geometry::line_segment> delaunayLines, const double mTol = 1e-3, bool outputPs = false); // Constrained Delaunay triangulation performed with the help of the FADE_2D libary
	std::vector<utilities::geometry::triangle> delaunayZoned(const std::vector <utilities::geometry::vertex> delaunayPoints, const std::vector <utilities::geometry::line_segment> orderedLinesOfZone, const double mTol = 1e-3, bool outputPs = false); // Constrained Delaunay triangulation performed with the help of the FADE_2D libary
	void splitLines(const std::vector<utilities::geometry::vertex>& intsecV, std::vector<utilities::geometry::line_segment>& intsecL,  const double mTol = 1e-3); // used during the delaunay method to avoid the inserting of overlapping and duplicate constrain lines


	/*
	The following function is made to take the points and orderedLinesSegments of a polygon_non_convex and return quads for this polygon_non_convex
	The Polygon_non_convex could not be used duo to 
	*/

	void convertToQuads(const std::vector <utilities::geometry::vertex>& delaunayPoints, const std::vector <utilities::geometry::line_segment>& orderedLinesOfZone, std::vector <utilities::geometry::quadrilateral>& quads, const double mTol = 1e-3, bool outputPs = false);

} // namespace utilities
} // namespace bso

#include <bso/utilities/delaunay.cpp>

#endif // DELAUNAY_HPP

