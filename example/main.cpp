#include <bso/spatial_design/ms_N_building.hpp>
#include <bso/utilities/dissimilarity.hpp>
#include <bso/utilities/graph_edit_distance.hpp>

int main(int argc, char* argv[]) {
    // Example usage
    bso::spatial_design::ms_N_building MS1("./test_files/example_building_1.txt");
    bso::spatial_design::ms_N_building MS2("./test_files/example_building_3.txt");

    std::map<int, SpaceInfo> buildingA = getBuildingData(MS1);
	std::map<int, SpaceInfo> buildingB = getBuildingData(MS2);

    GEDResult result = optimized_GED_poly(buildingA, buildingB);

    std::cout << "GED cost: " << result.cost << std::endl;
    std::cout << "Number of nodes: " << result.num_nodes << std::endl;
    std::cout << "Number of edges: " << result.num_edges << std::endl;
    
    return 0;
}