#include <iostream>
#include <cmath>

//#include <bso/spatial_design/ms_building.hpp>
#include <bso/spatial_design/ms_N_building.hpp>
//#include <bso/spatial_design/cf_building.hpp>
//#include <bso/structural_design/sd_model.hpp>
//#include <bso/building_physics/bp_model.hpp>
//#include <bso/grammar/grammar.hpp>
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

int main(int argc, char* argv[])
{   
    bso::visualization::initVisualization(argc, argv);
	out("Started visualizations",true,true,true);
    
	bool design_327 = true;
	bool design_x1 = true;
	bool design_x2 = true;
	bool design_x3 = true;
	bool design_x4 = true;
	bool design_x5 = true;
	bool design_x6 = true;
	bool design_x7 = true;
	bool design_x8 = true;
	
	if(design_327)
	{
		bso::spatial_design::ms_N_building ms("design_327");
		bso::visualization::visualize(ms);

		bso::utilities::geometry::vertex location = {7000,0,0};
		bso::utilities::geometry::vector direction = {1,0,0};
		double shift = 3000;
		ms.shiftFillPlane(location, direction, shift);
		ms.writeToFile("design_327_Linear_shift");
		bso::visualization::visualize(ms);
	}
	if(design_x1)
	{
		bso::spatial_design::ms_N_building ms("design_325");
		bso::visualization::visualize(ms);

		bso::utilities::geometry::vertex location = {3000,0,0};
		bso::utilities::geometry::vector direction = {1,-1,0}; //{1,0,0};
		double shift = sqrt(2) * 3000;
		ms.shiftFillPlane(location, direction, shift);
		ms.writeToFile("design_325_cornerDiagonalShift");
		std::cout << "shiftFillPlane() is done" << std::endl;
		bso::visualization::visualize(ms);
	}
	if(design_x2)
	{
		bso::spatial_design::ms_N_building ms("design_325");
		bso::visualization::visualize(ms);

		bso::utilities::geometry::vertex location = {3000,0,0};
		bso::utilities::geometry::vector direction = {1,0,0};
		double shift = sqrt(2) * 3000;
		ms.shiftFillPlane(location, direction, shift);
		ms.writeToFile("design_325_cornerFullShift");
		std::cout << "shiftFillPlane() is done" << std::endl;
		bso::visualization::visualize(ms);
	}
	if(design_x3)
	{
		bso::spatial_design::ms_N_building ms("design_325");
		bso::visualization::visualize(ms);

		bso::utilities::geometry::vertex location = {3000,0,0};
		bso::utilities::geometry::vector direction = {1,0,0};
		double shift = 3000;
		ms.shiftFillPlane(location, direction, shift);
		ms.writeToFile("design_325_cornerMatchShift");
		std::cout << "shiftFillPlane() is done" << std::endl;
		bso::visualization::visualize(ms);
	}
	if(design_x4)
	{
		bso::spatial_design::ms_N_building ms("design_325");
		bso::visualization::visualize(ms);

		bso::utilities::geometry::vertex location = {3000,0,0};
		bso::utilities::geometry::vector direction = {1,0,0};
		double shift = 1500;
		ms.shiftFillPlane(location, direction, shift);
		ms.writeToFile("design_325_cornerOverlapShift");
		std::cout << "shiftFillPlane() is done" << std::endl;
		bso::visualization::visualize(ms);
	}
	if(design_x5)
	{
		bso::spatial_design::ms_N_building ms("design_325");
		bso::visualization::visualize(ms);

		bso::utilities::geometry::vertex location = {0,6000,0};
		bso::utilities::geometry::vector direction = {0,1,0};
		double shift = 3000;
		ms.shiftFillPlane(location, direction, shift);
		ms.writeToFile("design_325_shiftSlantedSide");
		std::cout << "shiftFillPlane() is done" << std::endl;
		bso::visualization::visualize(ms);
	}
	if(design_x6)
	{
		bso::spatial_design::ms_N_building ms("design_308");
		bso::visualization::visualize(ms);

		bso::utilities::geometry::vertex location = {0,15000,0};
		bso::utilities::geometry::vector direction = {0,1,0};
		double shift = 6000;
		ms.shiftFillPlane(location, direction, shift);
		ms.writeToFile("design_308_splitLevelAndHorizontalOverlapShift");
		std::cout << "shiftFillPlane() is done" << std::endl;
		bso::visualization::visualize(ms);
	}
	if(design_x7)
	{
		bso::spatial_design::ms_N_building ms("design_308");
		bso::visualization::visualize(ms);

		bso::utilities::geometry::vertex location = {0,15000,0};
		bso::utilities::geometry::vector direction = {0,1,0};
		double shift = 6000;
		ms.duplicateElevation(3000);
		bso::visualization::visualize(ms);
		ms.duplicateElevation();
		bso::visualization::visualize(ms);
		ms.writeToFile("design_308_dupElevation");
		ms.shiftFillPlane(location, direction, shift);
		std::cout << "shiftFillPlane() is done" << std::endl;
		bso::visualization::visualize(ms);
	}
	if(design_x8)
	{
		bso::spatial_design::ms_N_building ms("design_308");
		bso::visualization::visualize(ms);
		ms.shiftFillPlaneCentre(5000);
		ms.writeToFile("design_308_centreShiftFillPlane");
		std::cout << "shiftFillPlaneCentre() is done" << std::endl;
		bso::visualization::visualize(ms);

	}




    bso::visualization::endVisualization();
	out("End visualizations",true,true,true);

 return 0;
}