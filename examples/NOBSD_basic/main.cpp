#include <bso/spatial_design/ms_N_building.hpp>
#include <bso/spatial_design/cf_building.hpp>
#include <bso/structural_design/sd_model.hpp>
#include <bso/building_physics/bp_model.hpp>
#include <bso/daylight/dl_model.hpp>
#include <bso/grammar/grammar.hpp>
#include <bso/visualization/visualization.hpp>

int main(int argc, char* argv[])
{
 // initialize a movable sizable model named MS
 bso::spatial_design::ms_N_building MS("ms_N_input_file.txt");

 // convert MS to a conformal model name CF.
 // Optional: utilizing a default window geometry grammar
 // to include the window geometry into the conformal model.
 // later, at the grammar level, it can be assigned to be a window.
 bso::spatial_design::cf_building CF(MS, "settings/window_settings.txt",
	 bso::spatial_design::conformal::window_geometry_grammar_tag<bso::spatial_design::conformal::DEFAULT_WINDOW_GEOMETRY_GRAMMAR>(), 1e-3);

 // create an instance of the grammar base class named gram using CF
 bso::grammar::grammar gram(CF);

 // Optionally apply a window grammar to identify which rectangles are windows.
 // Users can skip this if window detection is not needed.
 // Supported grammars: DEFAULT_WINDOW_GRAMMAR (or custom grammars if defined)
 gram.applyWindowGrammar<bso::grammar::DEFAULT_WINDOW_GRAMMAR>();

 // use the default daylight design grammar to create a
 // daylight model named DL, using settings defined in
 // settings/dl_settings.txt
 // Note, the generation of the DL model will assign 
 // which rectangles are windows or not and this is 
 // accounted for in the structural design grammar 
 // and building physics grammar as well, so it is important 
 // to generate the DL model before the SD and BP model.
 bso::daylight::dl_model DL = gram.dl_grammar <bso::grammar::DEFAULT_DL_GRAMMAR >(std::string("settings/dl_settings.txt"));

 // analyze the daylight model
 DL.analyze();

 // output result of the analysis
 std::cout << "ADF building score: " << DL.getTotalResults().mADFBuilding << std::endl;
 std::cout << "NEN building score: " << DL.getTotalResults().mTotalScore << std::endl;

 // use the default structural design grammar to create a structural
 // design named SD, using settings defined in
 // settings/sd_settings.txt
 bso::structural_design::sd_model SD = gram.sd_grammar <bso::grammar::DEFAULT_SD_GRAMMAR >(std::string("settings/sd_settings.txt"));

 // analyze the structural model
 SD.analyze();

 // output result of the analysis
 std::cout << "Structural compliance: " << SD.getTotalResults().mTotalStrainEnergy << std::endl;

 // use the default building physics design grammar to create a
 // building physics model named BP, using settings defined in
 // settings/bp_settings.txt
 bso::building_physics::bp_model BP = gram.bp_grammar <bso::grammar::DEFAULT_BP_GRAMMAR >(std::string("settings/bp_settings.txt"));

 // simulate the periods defined in bp_settings.txt
 // using the runge_kutta_dopri5 solver with both a
 // relative and an absolute error of 1e-3
 BP.simulatePeriods("runge_kutta_dopri5",1e-3,1e-3);

 // output result of the simulation
 std::cout << "Total heating energy: " << BP.getTotalResults().mTotalHeatingEnergy << std::endl;

 std::cout << "Total cooling energy: " << BP.getTotalResults().mTotalCoolingEnergy << std::endl;

 // visualize the different models
 bso::visualization::initVisualization(argc, argv);
 bso::visualization::visualize(MS);
 bso::visualization::visualize(CF,"trirec");
 //bso::visualization::visualize(CF,"triangular_prism");
 bso::visualization::visualize(SD,"component");
 bso::visualization::visualize(SD,"element");
 bso::visualization::visualize(BP);
 bso::visualization::visualize(BP, "rc_model", "rc_model", false);
 bso::visualization::visualize(DL, "");
 bso::visualization::visualize(DL, "grid");
 bso::visualization::visualize(DL, "DF");
 bso::visualization::visualize(DL, "Horizontal_Illuminance");
 bso::visualization::visualize(DL, "NEN");
 bso::visualization::visualize(DL, "ADF");
 bso::visualization::endVisualization();

 return 0;
}