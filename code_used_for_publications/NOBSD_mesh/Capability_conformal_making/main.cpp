#include <iostream>
#include <string>

#include <bso/spatial_design/ms_building.hpp>
#include <bso/spatial_design/ms_N_building.hpp>
#include <bso/spatial_design/cf_building.hpp>
#include <bso/structural_design/sd_model.hpp>
#include <bso/building_physics/bp_model.hpp>
#include <bso/grammar/grammar.hpp>
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
	std::cout.precision(17); //this precition only counts with cout not with the error message handeling used in the toolbox
	
	//Declarations
	std::string base = "design_";
	std::string integerDesign;
	
	//working variables
	std::vector <bso::spatial_design::ms_N_building*> ms_working; // the functioning desings to visualise
	std::vector <bso::spatial_design::cf_building*> cf_working; // the functioning designs to visualise
	std::vector <bso::grammar::grammar*> gram_working; // the functioning designs to visualise
	std::vector <bso::structural_design::sd_model*> sd_working; // the functioning designs to visualise
	std::vector <bso::building_physics::bp_model*> bp_working; // the functioning designs to visualise
	std::vector <std::string> MS_design_name; 
	std::vector <std::string> CF_design_name;
	std::vector <std::string> SD_design_name; 
	// error variables
	std::vector <bso::spatial_design::ms_N_building*> ms_of_cf_error;
	std::vector <bso::spatial_design::ms_N_building*> ms_of_gram_error;
	std::vector <bso::spatial_design::ms_N_building*> ms_of_sd_error;
	std::vector <bso::spatial_design::ms_N_building*> ms_of_bp_error;
	std::vector <bso::spatial_design::ms_N_building*> ms_of_vi_error;
	std::vector <std::string> error_design_MS; // the designs with an error in MS generation
	std::vector <std::string> error_design_MS_message;

	std::vector <std::string> error_design_CF; // the designs with an error in CF generation
	std::vector <std::string> error_design_CF_message;

	std::vector <std::string> error_design_gram; // the designs with an error in gram generation
	std::vector <std::string> error_design_gram_message;

	std::vector <std::string> error_design_SD; // the designs with an error in SD generation
	std::vector <std::string> error_design_SD_message;

	std::vector <std::string> error_design_SD_A; // the designs with an error in SD analysis
	std::vector <std::string> error_design_SD_A_message;

	std::vector <std::string> error_design_BP; // the designs with an error in BP generation
	std::vector <std::string> error_design_BP_message;

	std::vector <std::string> error_design_BP_A; // the designs with an error in BP analysis
	std::vector <std::string> error_design_BP_A_message;

	std::vector<bool> SD_analyse_sucses; // stores if the analysis of the SD model is skipped, this is neede to avoid a segfault at visualization!
	std::vector<bool> BP_analyse_sucses; // stores if the analysis of the SD model is skipped, this is neede to avoid a segfault at visualization!

	// count
	int count = 0;
	
	// Generate models
	for(int i=300; i <= 400; i++) //integerDesign++
	{	
		integerDesign = std::to_string(i);
		std::string design = base + integerDesign;
		
		try // skip the non-existing files
		{
			std::ifstream input;
			if (!design.empty()) input.open(design.c_str());
			
			if (design.empty() || !input.is_open())
			{
				std::stringstream errorMessage;
				errorMessage << "Could not initialize an MS building spatial design with the following input file:" << std::endl
										 << ((design.empty())? "no input file given" : design) << std::endl
										 << "(bso/spatial_design/ms_building.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
			}
		}
		catch(std::exception& e)
		{
			continue;
		}
		
		count++;
		std::cout << " " << std::endl;
		std::cout << "start " << design << std::endl;
		
		
		try // make MS-model
		{
			ms_working.push_back(new bso::spatial_design::ms_N_building (design));
			MS_design_name.push_back(design);
			out("Created an MS model",true,true,true);
		}
		catch(std::exception& e)
		{
			error_design_MS.push_back(design);
			error_design_MS_message.push_back(e.what());
			std::cout << e.what() << std::endl;
			std::cout << "This error occured during the generation of the ms_model generation of " << design << std::endl;
			continue;
		}
		
		
		try // make CF-model
		{
			cf_working.push_back(new bso::spatial_design::cf_building (*(ms_working.back()), 0.001));
			CF_design_name.push_back(design);
			out("Created an CF1 model",true,true,true);
		}
		catch(std::exception& e)
		{
			ms_of_cf_error.push_back(ms_working.back());
			error_design_CF.push_back(design);
			error_design_CF_message.push_back(e.what());
			std::cout << e.what() << std::endl;
			std::cout << "This error occured during the cf_model generation of " << design << std::endl;
			continue;
		}

		try // make gram-model
		{
			gram_working.push_back(new bso::grammar::grammar (*cf_working.back()));
			out("Created an gram model",true,true,true);
		}
		catch(std::exception& e)
		{
			ms_of_gram_error.push_back(ms_working.back());
			error_design_gram.push_back(design);
			error_design_gram_message.push_back(e.what());
			std::cout << e.what() << std::endl;
			std::cout << "This error occured during the gram_model generation of " << design << std::endl;
			continue;
		}

		bool SD_analyse_skip = false;
		try // make SD-model
		{
			sd_working.push_back(new bso::structural_design::sd_model((gram_working.back())->sd_grammar<bso::grammar::DEFAULT_SD_GRAMMAR>(std::string("settings/sd_settings.txt"))));
			out("Created an SD model",true,true,true);
		}
		catch(std::exception& e)
		{
			SD_analyse_skip = true;
			ms_of_sd_error.push_back(ms_working.back());
			error_design_SD.push_back(design);
			error_design_SD_message.push_back(e.what());
			std::cout << e.what() << std::endl;
			std::cout << "This error occured during the sd_model generation of " << design << std::endl;
			//continue;
		}

		if(SD_analyse_skip == false)
		{
			try // analyse SD-model
			{
				(sd_working.back())->analyze();
				SD_analyse_sucses.push_back(true);
				out("Analyzed SD model: ",false,false,true);
				out((sd_working.back())->getTotalResults().mTotalStrainEnergy,true,true,true);
			}
			catch(std::exception& e)
			{
				SD_analyse_sucses.push_back(false);
				ms_of_sd_error.push_back(ms_working.back());
				error_design_SD_A.push_back(design);
				error_design_SD_A_message.push_back(e.what());
				std::cout << e.what() << std::endl;
				std::cout << "This error occured during the sd_model analysis of " << design << std::endl;
				continue;
			}
		}

		try // make BP-model
		{
			bp_working.push_back(new bso::building_physics::bp_model ((gram_working.back())->bp_grammar<bso::grammar::DEFAULT_BP_GRAMMAR>(std::string("settings/bp_settings.txt"))));
			out("Created an BP model",true,true,true);
			
			(sd_working.back())->analyze();
			out("Analyzed SD model: ",false,false,true);
			out((sd_working.back())->getTotalResults().mTotalStrainEnergy,true,true,true);
		}
		catch(std::exception& e)
		{
			ms_of_bp_error.push_back(ms_working.back());
			error_design_BP.push_back(design);
			error_design_BP_message.push_back(e.what());
			std::cout << e.what() << std::endl;
			std::cout << "This error occured during the bp_model generation of " << design << std::endl;
			continue;
		}

		try // analyse BP-model
		{
			(bp_working.back())->simulatePeriods("runge_kutta_dopri5",1e-3,1e-3);
			BP_analyse_sucses.push_back(true);
			// output result of the simulation
			std::cout << "Total heating energy: " << (bp_working.back())->getTotalResults().mTotalHeatingEnergy << std::endl;
			std::cout << "Total cooling energy: " << (bp_working.back())->getTotalResults().mTotalCoolingEnergy << std::endl;
			out("Analysed an BP model",true,true,true);
		}
		catch(std::exception& e)
		{
			BP_analyse_sucses.push_back(false);
			ms_of_bp_error.push_back(ms_working.back());
			error_design_BP_A.push_back(design);
			error_design_BP_A_message.push_back(e.what());
			std::cout << e.what() << std::endl;
			std::cout << "This error occured during the bp_model analysis of " << design << std::endl;
			continue;
		}
	}
	
	// Send error messages to console
	std::cout << "  " << std::endl;
	std::cout << "All models generated " << std::endl;
	bool isdone = false;
	int b = 0;
	for(const auto errorDesign: error_design_MS)
	{
		if(isdone == false)
		{
			std::cout << "There where " << error_design_MS.size() << " errors found during MS generation" << std::endl;
			isdone = true;
		}
		std::cout << errorDesign << std::endl;
		std::cout << error_design_MS_message[b] << std::endl;
		b++;
	}
	isdone = false;
	b = 0;
	for(const auto errorDesign: error_design_CF)
	{
		if(isdone == false)
		{
			std::cout << "There where " << error_design_CF.size() << " errors found during CF generation" << std::endl;
			isdone = true;
		}
		std::cout << errorDesign << std::endl;
		std::cout << error_design_CF_message[b] << std::endl;
		b++;
	}
	isdone = false;
	b = 0;
	for(const auto errorDesign: error_design_gram)
	{
		if(isdone == false)
		{
			std::cout << "There where " << error_design_gram.size() << " errors found during gram generation" << std::endl;
			isdone = true;
		}
		std::cout << errorDesign << std::endl;
		std::cout << error_design_gram_message[b] << std::endl;
		b++;
	}
	isdone = false;
	b = 0;
	for(const auto errorDesign: error_design_SD)
	{
		if(isdone == false)
		{
			std::cout << "There where " << error_design_SD.size() << " errors found during SD generation" << std::endl;
			isdone = true;
		}
		std::cout << errorDesign << std::endl;
		std::cout << error_design_SD_message[b] << std::endl;
		b++;
	}
	isdone = false;
	b = 0;
	for(const auto errorDesign: error_design_SD_A)
	{
		if(isdone == false)
		{
			std::cout << "There where " << error_design_SD_A.size() << " errors found during SD analysis" << std::endl;
			isdone = true;
		}
		std::cout << errorDesign << std::endl;
		std::cout << error_design_SD_A_message[b] << std::endl;
		b++;
	}
	isdone = false;
	b = 0;
	for(const auto errorDesign: error_design_BP)
	{
		if(isdone == false)
		{
			std::cout << "There where " << error_design_BP.size() << " errors found during BP generation" << std::endl;
			isdone = true;
		}
		std::cout << errorDesign << std::endl;
		std::cout << error_design_BP_message[b] << std::endl;
		b++;
	}
	isdone = false;
	b = 0;
	for(const auto errorDesign: error_design_BP_A)
	{
		if(isdone == false)
		{
			std::cout << "There where " << error_design_BP_A.size() << " errors found during BP analysis" << std::endl;
			isdone = true;
		}
		std::cout << errorDesign << std::endl;
		std::cout << error_design_BP_A_message[b] << std::endl;
		b++;
	}


	std::cout << " " << std::endl;
	std::cout << " " << std::endl;
	std::cout << " " << std::endl;
	std::cout << " " << std::endl;
	std::cout << " " << std::endl;
	
	
	/*
	// Print error messages to console in a row
	std::cout << "  " << std::endl;
	std::cout << "All models generated " << std::endl;
	std::cout << "  " << std::endl;
	isdone = false;
	b = 0;
	for(const auto errorDesign: error_design_MS)
	{
		if(isdone == false)
		{
			std::cout << "There where " << error_design_MS.size() << " errors found during MS generation" << std::endl;
			isdone = true;
		}
		std::cout << errorDesign << std::endl;
		std::cout << error_design_MS_message[b] << std::endl;
		b++;
	}
	isdone = false;
	b = 0;
	for(const auto errorDesign: error_design_CF)
	{
		if(isdone == false)
		{
			std::cout << "There where " << error_design_CF.size() << " errors found during CF generation" << std::endl;
			isdone = true;
		}
		std::cout << errorDesign << std::endl;
		std::cout << error_design_CF_message[b] << std::endl;
		b++;
	}
	*/

	std::cout << "There are " << count << " designs tested" << std::endl;
	std::cout << "There where " << error_design_MS.size() << " errors found during MS generation" << std::endl;
	std::cout << "There where " << error_design_CF.size() << " errors found during CF generation" << std::endl;
	std::cout << "There where " << error_design_gram.size() << " errors found during gram generation" << std::endl;
	std::cout << "There where " << error_design_SD.size() << " errors found during SD generation" << std::endl;
	std::cout << "There where " << error_design_SD_A.size() << " errors found during SD analysis" << std::endl;
	std::cout << "There where " << error_design_BP.size() << " errors found during BP generation" << std::endl;
	std::cout << "There where " << error_design_BP_A.size() << " errors found during BP analysis" << std::endl;




	
	// Visualizations
	std::cout << "Started with the visualization: " << std::endl;
	bso::visualization::initVisualization(argc, argv);
	
	b=0;
	for(const auto CF: cf_working)
	{
		try
		{
			//bso::visualization::visualize(*CF,"triangular_prism", CF_design_name[b]);
		}
		catch(std::exception& e)
		{
			std::cout << " " << std::endl;
			std::cout << "The conformal model of " << CF_design_name[b] << " with triangular_prism was unable to be generated." << std::endl;
			std::cout << "The following error occured: " << std::endl;
			std::cout << e.what() << std::endl;
		}
		try
		{
			//bso::visualization::visualize(*CF,"triangle", CF_design_name[b]);
		}
		catch(std::exception& e)
		{
			std::cout << " " << std::endl;
			std::cout << "The conformal model of " << CF_design_name[b] << " with triangles was unable to be generated." << std::endl;
			std::cout << "The following error occured: " << std::endl;
			std::cout << e.what() << std::endl;
		}
		try
		{
			//bso::visualization::visualize(*CF,"rectangle", CF_design_name[b]);
			//bso::visualization::visualize(*CF,"triangle", CF_design_name[b]);
			//bso::visualization::visualize(*CF,"trirec", CF_design_name[b]);
			//bso::visualization::visualize(*CF,"cuboid", CF_design_name[b]);
			//bso::visualization::visualize(*CF,"space_at_ground", CF_design_name[b]);
			//bso::visualization::visualize(*CF,"delaunay", CF_design_name[b]);
		}
		catch(std::exception& e)
		{
			std::cout << " " << std::endl;
			std::cout << "The conformal model of " << CF_design_name[b] << " was unable to be generated." << std::endl;
			std::cout << "The following error occured: " << std::endl;
			std::cout << e.what() << std::endl;
		}
		
		b++;
	}

	b=0;
	for(const auto SD: sd_working)
	{
		try
		{
			bso::visualization::visualize(*SD,"component");
			if(SD_analyse_sucses[b] == true)
			{// Only visualize the analysis of the SD model if no errors occured
				//bso::visualization::visualize(*SD,"strain_energy");
				//bso::visualization::visualize(*SD,"element");
				//bso::visualization::visualize(*SD,"density");
			}
			else
			{
				std::cout << " " << std::endl;
				std::cout << "Skip SD model since the analysis went wrong and can therefore not be visualized." << std::endl;
			}
		}
		catch(const std::exception& e)
		{
			std::cout << " " << std::endl;
			std::cout << "A SD model was unable to be generated." << std::endl;
			std::cout << "The following error occured: " << std::endl;
			std::cout << e.what() << std::endl;
		}
		b++;
	}

	b=0;
	for(const auto BP: bp_working)
	{
		try
		{
			bso::visualization::visualize(*BP);
			if(BP_analyse_sucses[b] == true)
			{// Only visualize the analysis of the BP model if no errors occured

			}
		}
		catch(const std::exception& e)
		{
			std::cout << " " << std::endl;
			std::cout << "A bp model was unable to be generated." << std::endl;
			std::cout << "The following error occured: " << std::endl;
			std::cout << e.what() << std::endl;
		}
		b++;
	}




	for(const auto MS: ms_of_cf_error)
	{
		std::string error = "error_";
		std::string title = error + error_design_CF[b];
		bso::visualization::visualize(*MS);
		b++;
	}
	/*
	for(const auto MS: ms_working)
	{
		bso::visualization::visualize(*MS);
	}
	*/
	bso::visualization::endVisualization();
	

	return 0;
}