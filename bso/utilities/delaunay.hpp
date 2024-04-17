#ifndef DELAUNAY_HPP
#define DELAUNAY_HPP

namespace bso { namespace utilities {

	/*
	 * The functions in this file are used to transform a building spatial design representation
	 * to a 2D representation in triangles of the building.
	 */
	 
	std::vector<utilities::geometry::triangle> delaunay(const std::vector <utilities::geometry::vertex> delaunayPoints, const std::vector <utilities::geometry::line_segment> delaunayLines); // Constrained Delaunay triangulation performed with the help of the FADE_2D libary
	std::vector<utilities::geometry::triangle> delaunay(const std::vector <utilities::geometry::vertex> delaunayPoints); // Delaunay triangulation performed with the help of the FADE_2D libary
	void splitLines(const std::vector<utilities::geometry::vertex>& intsecV, std::vector<utilities::geometry::line_segment>& intsecL); // used during the delaunay method to avoid the inserting of overlapping and duplicate constrain lines

} // namespace utilities
} // namespace bso

#endif // DELAUNAY_HPP

