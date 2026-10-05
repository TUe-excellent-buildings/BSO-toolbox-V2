#ifndef DISSIMILARITY_HPP
#define DISSIMILARITY_HPP

#include <bso/spatial_design/ms_N_building.hpp>

std::array<double, 4> calculate_dissimilarity(
	const bso::spatial_design::ms_N_building MS1,
	const bso::spatial_design::ms_N_building MS2
);

#include <bso\utilities\dissimilarity.cpp>

#endif // DISSIMILARITY_HPP
