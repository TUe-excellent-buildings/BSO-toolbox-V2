#include <iostream>
#include <string>

#include <bso/spatial_design/ms_N_building.hpp>
#include <bso/visualization/visualization.hpp>

int end, begin = 0;
template<class T>
void out(const T& t, const bool& e = false,
				 const bool& i = false, const bool& verbose = false){
	if (!verbose) return;
	std::cout << t;
	end = clock();
	if (i) std::cout << " (" << 1000*(end-begin)/CLOCKS_PER_SEC << " ms)";
	if (e) std::cout << std::endl;
	begin = end;
} // out()

using namespace bso;
int main(int argc, char* argv[])
{
	//std::cout.precision(17); //this precision only counts with cout not with the error message handling used in the toolbox
	int amountIterations = 10;

	bso::visualization::initVisualization(argc, argv);

	// initialize a movable sizable model named MS
	std::vector<spatial_design::ms_N_building> msDesigns;
	std::string inputFile = "design_N_2";
	msDesigns.push_back(bso::spatial_design::ms_N_building(inputFile));
	bso::visualization::visualize(msDesigns.back());

	for(int i=0; i<amountIterations; i++)
	{
		spatial_design::ms_N_building newMS = msDesigns.back();
		
		auto spaceWithLargestDimension = newMS.getSpacePtrs().front();
		bso::utilities::geometry::line_segment LongestAxis;
		double longestLength = 0;

		for (const auto& j : newMS.getSpacePtrs())
		{
			std::vector<bso::utilities::geometry::line_segment> lines = j->getGeometry().getLines();
			for(const auto k: lines)
			{
				if(k.getVector().isVertical()) continue; // only consider horizontal lines for the axis
				if(longestLength < k.getLength())
				{
					longestLength = k.getLength();
					LongestAxis = k;
					spaceWithLargestDimension = j;
				}
			}
		}
		int splitamount = 2;

		// pre-split value
		bso::utilities::geometry::quad_hexahedron spaceGeomo = spaceWithLargestDimension->getGeometry();
		std::vector<bso::utilities::geometry::quadrilateral> spaceQuadso = spaceGeomo.getQuadrilateral();
		double RecDif = (spaceQuadso[5]).getDifRectangle();
		std::cout << "spaceWithLargestDimension recDif: " << RecDif << std::endl;

		// split
		newMS.splitSpace(spaceWithLargestDimension, splitamount , LongestAxis);

		// post split value
		int id = newMS.getSpacePtrs().size()-2;
		bso::utilities::geometry::quad_hexahedron spaceGeomo1 = (newMS.getSpacePtrs())[id]->getGeometry();
		std::vector<bso::utilities::geometry::quadrilateral> spaceQuadso1 = spaceGeomo1.getQuadrilateral();
		double RecDif1 = (spaceQuadso1[5]).getDifRectangle();
		std::cout << "result split 1 recDif: " << RecDif1 << std::endl;
		
		// post split value
		id++;
		bso::utilities::geometry::quad_hexahedron spaceGeomo2 = (newMS.getSpacePtrs())[id]->getGeometry();
		std::vector<bso::utilities::geometry::quadrilateral> spaceQuadso2 = spaceGeomo2.getQuadrilateral();
		double RecDif2 = (spaceQuadso2[5]).getDifRectangle();
		std::cout << "result split 2 recDif: " << RecDif2 << std::endl;

		std::cout << "avaride recDif of both splits: " << (RecDif1+RecDif2)/2 << std::endl;

		msDesigns.push_back(newMS);
		bso::visualization::visualize(msDesigns.back());

		// get recDif
		int spaceCount = msDesigns.back().getSpacePtrs().size();
		double averageRecDif = 0;
		for(const auto i: msDesigns.back().getSpacePtrs())
		{
			bso::utilities::geometry::quad_hexahedron spaceGeom = i->getGeometry();
			std::vector<bso::utilities::geometry::quadrilateral> spaceQuads = spaceGeom.getQuadrilateral();
			averageRecDif = averageRecDif + (spaceQuads[5]).getDifRectangle();
		}
		averageRecDif = averageRecDif/spaceCount;
		std::cout <<"averageRecDif: " << averageRecDif << std::endl;
	}

 	bso::visualization::endVisualization();

	return 0;
}