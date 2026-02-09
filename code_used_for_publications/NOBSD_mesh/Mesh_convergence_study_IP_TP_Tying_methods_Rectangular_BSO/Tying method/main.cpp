//#include <iostream>

#include <bso/spatial_design/ms_building.hpp>
#include <bso/spatial_design/cf_building.hpp>
#include <bso/structural_design/sd_model.hpp>
#include <bso/building_physics/bp_model.hpp>
#include <bso/grammar/grammar.hpp>
#include <bso/visualization/visualization.hpp>
#include <time.h>
#include <string>

int end, begin = 0;
template<class T>
void out(const T& t, const bool& e = false,const bool& i = false, const bool& verbose = false)
{
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
	//freopen("out.txt","w",stdout);
	clock_t tStart = clock();
	
	// initialize a movable sizable model named MS
	std::cout << "Please insert the character for the design to analyze either A, B or C: ";
	char design;
	std::cin >> design;
	std::string design_str(1,design);
	std::string file_name = "design_" + design_str;

	bso::spatial_design::ms_building MS(file_name);
	out("Created an MS model",true,true,true);
	
	// convert MS to a conformal model name CF
	bso::spatial_design::cf_building CF(MS);
	out("Created an CF model",true,true,true);
	
	// create an instance of the grammar base class named gram using CF
	bso::grammar::grammar gram(CF);
	out("Created an gram model",true,true,true);
	
	// use the delault structural design grammar to create a structural 
	// design named SD, using settings defined in settings/sd_settings.txt
	bso::structural_design::sd_model SD = gram.sd_grammar<bso::grammar::DEFAULT_SD_GRAMMAR>(std::string("settings/sd_settings.txt"));
	out("Created an SD model",true,true,true);
	
	// analyse the structural model
	SD.analyze();
	out("Analyzed SD model and time taken to analyze and mesh model ",true,true,true);
	
	// output result of the analysis
	std::cout << "Structural compliance: " << SD.getTotalResults().mTotalStrainEnergy << std::endl;
	std::cout << "Time taken: " << ((clock() - tStart)/CLOCKS_PER_SEC) << "sec." << std::endl;
	
	// visualize the different models
	bso::visualization::initVisualization(argc, argv);
	bso::visualization::visualize(MS);
	bso::visualization::visualize(CF,"rectangle");
	bso::visualization::visualize(CF,"cuboid");
	bso::visualization::visualize(SD,"strain_energy");
	bso::visualization::visualize(SD,"element");
	//bso::visualization::visualize(BP);
	bso::visualization::endVisualization();
	
	return 0;
}
