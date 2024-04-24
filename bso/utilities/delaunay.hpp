#ifndef DELAUNAY_HPP
#define DELAUNAY_HPP

#include <vector>
#include <bso/utilities/geometry.hpp>
#include <bso/spatial_design/ms_N_building.hpp>
#include <bso/spatial_design/ms_space.hpp>

namespace bso { namespace utilities {

	/*
	 * The functions in this file are used to transform a building spatial design representation
	 * to a 2D representation in triangles of the building at z=0.
	 */
	 
	std::vector<utilities::geometry::triangle> delaunay(const std::vector <utilities::geometry::vertex> delaunayPoints, const std::vector <utilities::geometry::line_segment> delaunayLines, const double mTol = 1e-3, bool outputPs = false); // Constrained Delaunay triangulation performed with the help of the FADE_2D libary
	// std::vector<utilities::geometry::triangle> delaunayZoned(const std::vector <utilities::geometry::vertex> delaunayPoints, const utilities::geometry::polygon_non_convex zone, const double mTol = 1e-3, bool outputPs = false); // Constrained Delaunay triangulation performed with the help of the FADE_2D libary
	void splitLines(const std::vector<utilities::geometry::vertex>& intsecV, std::vector<utilities::geometry::line_segment>& intsecL,  const double mTol = 1e-3); // used during the delaunay method to avoid the inserting of overlapping and duplicate constrain lines
	void getNOBSDAtGround(const bso::spatial_design::ms_N_building& msNModel, std::vector<std::pair<bso::spatial_design::ms_space*, utilities::geometry::quad_hexahedron>>& spacePair, std::vector<double>& allLevels, const double mTol = 1e-3);
	void getNOBSDAtGround(const bso::spatial_design::ms_N_building& msNModel, std::vector<std::pair<bso::spatial_design::ms_space*, utilities::geometry::quad_hexahedron>>& spaceAtGroundV, const double mTol = 1e-3);
	std::vector <bso::utilities::geometry::quadrilateral> getQuadAtZ0(std::vector<std::pair<bso::spatial_design::ms_space*, utilities::geometry::quad_hexahedron>>& spacePair, const double mTol = 1e-3);
	std::vector <bso::utilities::geometry::line_segment> getLinesAtZ0(std::vector<std::pair<bso::spatial_design::ms_space*, utilities::geometry::quad_hexahedron>>& spacePair, const double mTol = 1e-3);
	void getDelaunayVariables(std::vector <utilities::geometry::vertex>& delaunayPoints, std::vector <utilities::geometry::line_segment>& delaunayLines, std::vector<std::pair<bso::spatial_design::ms_space*, utilities::geometry::quad_hexahedron>>& spacePair, const double mTol = 1e-3);

} // namespace utilities
} // namespace bso

#include <bso/utilities/delaunay.cpp>

#endif // DELAUNAY_HPP

