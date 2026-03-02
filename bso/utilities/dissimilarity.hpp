#ifndef DISSIMILARITY_HPP
#define DISSIMILARITY_HPP

#include <bso/spatial_design/ms_N_building.hpp>

double calculate_dissimilarity(
	const bso::spatial_design::ms_N_building MS1,
	const bso::spatial_design::ms_N_building MS2
);

double calculate_dissimilarity_weighted(
	const bso::spatial_design::ms_N_building MS1,
	const bso::spatial_design::ms_N_building MS2,
	int rot = 3, double w1 = 1, double w2 = 1, double w3 = 1
);

std::array<double, 3> calculate_dissimilarity_components(
	const bso::spatial_design::ms_N_building MS1,
	const bso::spatial_design::ms_N_building MS2
);

#include <bso\utilities\dissimilarity.cpp>

#endif // DISSIMILARITY_HPP
