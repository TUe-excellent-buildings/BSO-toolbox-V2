#ifndef DELAUNAY_HPP
#define DELAUNAY_HPP

#include <vector>
#include <bso/utilities/geometry.hpp>

namespace bso { namespace utilities {

	/*
	 * The following functions in this file are used to perform constrained delaunay triangulations
	 * The zoned variant only return the triangles within the defined zone, 
	 * otherwise the convex hull of the inserted delaunayPoints are returned.
	 * For the zoned variant, it is required that the zone's lines are defined in sequencial order and form a closed loop.
	 * 
	 */
	 
	std::vector<utilities::geometry::triangle> delaunay(const std::vector <utilities::geometry::vertex> delaunayPoints, 
			const std::vector <utilities::geometry::line_segment> delaunayLines, const double mTol = 1e-3, bool outputPs = false); // Constrained Delaunay triangulation performed with the help of the FADE_2D libary
	std::vector<utilities::geometry::triangle> delaunayZoned(const std::vector <utilities::geometry::vertex> delaunayPoints,
			const std::vector <utilities::geometry::line_segment> orderedLinesOfZone, const double mTol = 1e-3, bool outputPs = false); // Constrained Delaunay triangulation performed with the help of the FADE_2D libary
	std::vector<std::vector<utilities::geometry::triangle>> delaunayMultipleZoned(const std::vector <utilities::geometry::vertex> delaunayPoints,
			const std::vector<std::vector<utilities::geometry::line_segment>> orderedLinesOfZone, const std::vector <utilities::geometry::line_segment> delaunayLines,
			bool getTriOutsideZones = true, bool getBothInAndOutZones = false, const double mTol = 1e-3, bool outputPs = false);
	void splitLines(const std::vector<utilities::geometry::vertex>& intsecV, std::vector<utilities::geometry::line_segment>& intsecL, bool removeDuplicates = true, const double mTol = 1e-3); // used during the delaunay method to avoid the inserting of overlapping and duplicate constrain lines


} // namespace utilities
} // namespace bso

#include <bso/utilities/delaunay.cpp>

#endif // DELAUNAY_HPP

