#include <iostream>

#include <bso/spatial_design/ms_building.hpp>
#include <bso/spatial_design/ms_N_building.hpp>
#include <bso/spatial_design/cf_building.hpp>
#include <bso/structural_design/sd_model.hpp>
#include <bso/building_physics/bp_model.hpp>
#include <bso/grammar/grammar.hpp>
#include <bso/visualization/visualization.hpp>

#include <boost/algorithm/string.hpp>
#include <vector>
#include <sstream>

std::vector<std::string> VIS_OPTIONS = {"ms","rc","cb","tr","tp","tb", "sd","fe","bp"};
std::vector<std::string> OUT_OPTIONS = {"PN_file", "result_file", "verbose", "ms_files"};

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

// void deleteInnerBoundaryPrisms(bso::spatial_design::conformal::cf_building_model building)
// { 
// 	std::vector<bso::spatial_design::conformal::cf_triPrism> prism_to_deletion;
// 	//vector<utilities::geometry::quad_hexahedron> cudoid_to_deletion;
// 	for (auto i : building.cfSpaces())
// 	{
// 		int space = i->getSpaceID();
// 		if(space == 0){
// 			for(auto j: i->cfTriPrism())
// 			{
// 				prism_to_deletion.push_back(*j);
// 			}
// 			// for(auto j: i->cfCuboids())
// 			// {
// 			// 	cuboid_to_deletion.push_back(j);
// 			// }
// 		} else {
// 			for(auto j: i->cfTriPrism())
// 			{
// 				auto index_prism = std::find(prism_to_deletion.begin(), prism_to_deletion.end(), *j);
// 				if(*index_prism != *prism_to_deletion.end()){
// 					prism_to_deletion.erase(index_prism);
// 					std::cout << "found" << std::endl;
// 				}
// 			}
// 			// for(auto j: i->cfCuboids())
// 			// {
// 			// }					cuboid_to_deletion.push_back(j);
// 		}
// 	}
// 	// for_each(prism_to_deletion.begin(),prism_to_deletion.end(),[](const auto& elem) {
// 	// 	delete elem;
// 	// });
	
// } // deleteInnerBoundaryPrisms

int main(int argc, char* argv[])
{
	std::string inputFile = "";
	std::string designName = "";
	std::string modification = "";
	std::vector<std::string> visualizations;
	bool result_file = false;
	bool PN_file = true;
	bool ms_files = false;
	bool verbose = false;
	bool bounded = false;
	bool save = false;
	
	// Handle the given input arguments
	std::vector<std::string> args(argv+1, argv+argc);
	auto arg = args.begin();
	if (arg == args.end())
	{
		std::stringstream errorMessage;
		errorMessage << "\nError, expected arguments, received nothing.\n"
								 << "use -h or --help for help." << std::endl;
		throw std::invalid_argument(errorMessage.str());
	}
	while (arg != args.end())
	{
		if (*arg == "-h" || *arg == "--help")
		{
			std::cout << "This program ...." << std::endl;
			std::cout << std::endl;
			std::cout << "-i\t" << "or --input, to specify an input file,\n"
								<<   "\t" << "expects a string containing the file name\n"
								<<   "\t" << "of the input file. The input file should be\n"
								<<   "\t" << "in \'Movable Sizable\' format.\n"
								<<   "\t" << "and the filename should have no spaces.\n";
			std::cout << "-o\t" << "or --output, to specify the diferent outputs:\n"
								<<   "\t" << "\'result_file\' output a file containing \n"
								<<   "\t" << "the file name, Structural Design model analysis results, \n"
								<<   "\t" << "and Building Physics model analysis results \n"
								<<   "\t" << "\'verbose\' prints a descriptive outline of the process \n"
								<<   "\t" << "to the console during running \n";
			std::cout << "-v\t" << "or --visualization, to specify which models are\n"
								<<   "\t" << "visualized: \'ms\' for \'Movable Sizable\', \n"
								<<   "\t" << "\'rc\' for \'conformal model with Rectangles for R insertion method\', \n"
								<<   "\t" << "\'cb\' for \'conformal model with Cuboids for R insertion method\', \n"
								<<   "\t" << "\'tr\' for \'conformal model with Triangles and Rectangles for N insertion method\', \n"
								<<   "\t" << "\'tp\' for \'conformal model with Triangular Prism for N insertion method\', \n"
								<<   "\t" << "\'tb\' for \'conformal model with Triangular Prism for N insertion method and including bounding box\', \n"
								<<   "\t" << "\'sd\' for'\'Structural Design\', \n"
								<<   "\t" << "\'fe\' for \'Finite Elements of the structural design model\',\n"
								<<   "\t" << "and \'bp\' for the \'Building Physics model\'.\n"
								<<   "\t" << "More than one can be specified.\n";
			std::cout << "-b\t" << "or --bounded, to obtain a model with boundaries.\n";
			std::cout << "-s\t" << "or --save, to save the output from triangulation.\n";
			std::cout << "\n\n" << "Press ENTER to continue..." << std::endl;
			std::cin.get();
			return 0;
		}
		else if (*arg == "-i" || *arg == "--input")
		{
			if (++arg == args.end() || arg->operator[](0) == '-')
			{
				std::stringstream errorMessage;
				errorMessage << "\nError, expected the name of the input file after -i or --input\n"
										 << "Use -h or --help for help" << std::endl;
				throw std::invalid_argument(errorMessage.str());
			}
			inputFile = *arg;
			boost::trim(inputFile);
			designName = inputFile;
			++arg;
		}
		else if (*arg == "-o" || *arg == "--output")
		{
			while (++arg != args.end() && arg->operator[](0) != '-')
			{
				std::string outSpec = *arg;
				boost::trim(outSpec);
				if 			(outSpec == "result_file") result_file = true;
				else if (outSpec == "verbose") verbose = true;
				else
				{
					std::stringstream errorMessage;
					errorMessage << "\nError, did not recognize: " << outSpec << " as an\n"
											 << "argument for -o or --output. Use -h or\n"
											 << "--help for help\n";
					throw std::invalid_argument(errorMessage.str());
				}
			}
		}
		else if (*arg == "-v" || *arg == "--visualize")
		{
			while (++arg != args.end() && arg->operator[](0) != '-')
			{
				std::string visSpec = *arg;
				boost::trim(visSpec);
				if (std::find(VIS_OPTIONS.begin(),VIS_OPTIONS.end(),visSpec) == VIS_OPTIONS.end())
				{
					std::stringstream errorMessage;
					errorMessage << "\nError, did not recognize: \"" << visSpec << "\"\n"
											 << "as an option for -v or --visualization as a specification\n"
											 << "of a model to visualize, use -h or --help for help\n";
					throw std::invalid_argument(errorMessage.str());
				}
				visualizations.push_back(visSpec);
			}
			if (visualizations.size() == 0)
			{
				std::stringstream errorMessage;
				errorMessage << "\nError, expected at least one string after\n"
										 << "-v or --visualize, received nothing. Use -h or\n"
										 << "--help for help\n";
				throw std::invalid_argument(errorMessage.str());
			}
		} else if (*arg == "-b" || *arg == "--bounded") {
			bounded = true;
			++arg;
		} else if (*arg == "-s" || *arg == "--save") {
			save = true;
			++arg;
		}
		else
		{
			std::stringstream errorMessage;
			errorMessage << "\nError, did not recognize argument: " << *arg << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
	}
	if (inputFile == "")
	{
		std::stringstream errorMessage;
		errorMessage << "\nError, expected an input file to be specified.\n"
								 << "use -h or --help for help\n";
		throw std::invalid_argument(errorMessage.str());
	}
	if (inputFile.empty())
	{
		std::stringstream errorMessage;
		errorMessage << "Error, can not initialize MS building design using\n";
			errorMessage << "the following MS input file: " << inputFile << "\n";		
		throw std::invalid_argument(errorMessage.str());
	}
	

	// Generate the Movable and Sizable (MS) building model
	bso::spatial_design::ms_N_building MS_bounded(inputFile);
	

	MS_bounded.addBoundary();
	
	// Generate the conformal model based on MS
	bso::spatial_design::cf_building CM_bounded(MS_bounded, 0.001);
	

	// Regular MS building model
	bso::spatial_design::ms_N_building MS(inputFile);
	out("Created a Movable and Sizable (MS) building model", true, true, verbose);
	bso::spatial_design::cf_building CM(MS, 0.001);
	out("Created a Conformal Model (CM)",true,true,verbose);

	// Initialize the grammer to specify basic properties per geometry
	bso::grammar::grammar gram(CM);
	out("Initialized grammar (gram)",true,true,verbose);
	
	// Generate the SD model based on the grammer specified basic properties per geometry
	bso::structural_design::sd_model SD = gram.sd_grammar<bso::grammar::DEFAULT_SD_GRAMMAR>(std::string("settings/sd_settings.txt"));
	out("Generated Structural Design (SD) model ",true,true,verbose);
	
	// Analyse the SD model
	SD.analyze();
	out("Evaluated SD: ",false,false,verbose);
	double SDResult = SD.getTotalResults().mTotalStrainEnergy;
	out(SDResult,true,true,verbose);

	std::cout << "SDResult: " << SDResult << std::endl;

	// Generate the BP model based on the grammer specified basic properties per geometry
	bso::building_physics::bp_model BP = gram.bp_grammar<bso::grammar::DEFAULT_BP_GRAMMAR>(std::string("settings/bp_settings.txt"));
	out("Created a Building Physics (BP) model",true,true,verbose);
	
	// Analyse the BP model
	BP.simulatePeriods("runge_kutta_dopri5",1e-3,1e-3);
	double BPResult = BP.getTotalResults().mTotalEnergy;
	out("Evaluated BP: ",false,false,verbose);
	out(BPResult,true,true,verbose);

	std::cout << "BPResult: " << BPResult << std::endl;


	if (bounded)
	{
		// Define file name
		std::stringstream fileNameStream;
			fileNameStream << designName << "_PN";
			std::string fileName = fileNameStream.str();
		
		// Write to file
		std::ofstream output;
		if (!fileName.empty()) output.open(fileName.c_str());
		
		if (fileName.empty() || !output.is_open())
		{
			std::stringstream errorMessage;
			errorMessage << "Could not open the following file to write an PN building spatial design to:" << std::endl
									 << ((fileName.empty())? "no input file given" : designName) << std::endl
									 << "(bso/spatial_design/ms_building.cpp). " << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
		
		CM_bounded.writeToFile(fileName);
		out("Print PN version of MS_N_building to file", true, true, verbose);
	}
	
	// Print results to file
	if (result_file)
	{
		// Define file name
		std::stringstream fileNameStream;
			fileNameStream << designName << "_results";
			std::string fileName = fileNameStream.str();
		
		// Write to file
		std::ofstream output;
		if (!fileName.empty()) output.open(fileName.c_str());
		
		if (fileName.empty() || !output.is_open())
		{
			std::stringstream errorMessage;
			errorMessage << "Could not open the following file to write an MS building spatial design to:" << std::endl
									 << ((fileName.empty())? "no input file given" : designName) << std::endl
									 << "(bso/spatial_design/ms_building.cpp). " << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
		
		output << designName << std::endl;
		output << "SDResult, " << SDResult << std::endl;
		output << "BPResult, " << BPResult << std::endl;
	}
	
	if(save) {
		for (const auto& i : CM.cfTriPrism())
		{
			std::vector<bso::utilities::geometry::vertex> triPrismVertices;
			for (const auto& j : *i)
			{
				triPrismVertices.push_back(i->getCenter() + (j - i->getCenter()));
			}
			bso::utilities::geometry::triangular_prism triPrismGeometry(triPrismVertices);
			// Check for intersections
			bool intersects = false;
			for(const auto j : CM.cfVertices())
			{
				bool cornerVertex = false;
				for(const auto k : (i->cfVertices()))
				{
					if(j->isSameAs(*k, CM.tolerance()))
					{
						cornerVertex = true;
						break;
					}
				}
				
				if(i->isInsideOrOn(*j, CM.tolerance()) && cornerVertex == false)
				{
					intersects = true;
					break;
				}
			}
		}
	}

	if (!visualizations.empty()) 
	{
		visualization::initVisualization(argc,argv);
		out("Initialized visualization", true, true, verbose);
	}
	
	if (!visualizations.empty()) for (const auto& visOption : visualizations)
	{
		if (visOption == "ms") bso::visualization::visualize(MS);
		if (visOption == "rc") visualization::visualize(CM,"rectangle");
		if (visOption == "cb") visualization::visualize(CM,"cuboid");
		if (visOption == "tr") visualization::visualize(CM,"trirec");
		if (visOption == "tp") visualization::visualize(CM,"triangular_prism");
		if (visOption == "sd") visualization::visualize(SD);
		if (visOption == "fe") visualization::visualize(SD, "element");
		if (visOption == "bp") visualization::visualize(BP);

		if (bounded && visOption == "tb") visualization::visualize(CM_bounded,"triangular_prism");
	}
	if (!visualizations.empty()) out("Sent models to visualization",true,true,verbose);
	if (!visualizations.empty()) visualization::endVisualization();

	return 0;
}
