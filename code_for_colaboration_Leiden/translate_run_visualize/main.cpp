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

std::vector<std::string> VIS_OPTIONS = {"ms","msc","rc","cb","tr","tp","tb", "sd","fe","bp"};
std::vector<std::string> OUT_OPTIONS = {"result_file", "verbose", "ms_files"};

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
	std::string inputFile = "";
	std::string designName = "";
	std::string colorinputFile = "";
	std::string colourFileName = "";
	std::string modification = "";
	std::vector<std::string> visualizations;
	bool result_file = false;
	bool ms_files = false;
	bool verbose = false;
	bool bounded = false;
	bool run = false;
	
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
			std::cout << "-o\t" << "or --output, to specify the different outputs:\n"
								<<   "\t" << "\'result_file\' output a file containing \n"
								<<   "\t" << "the file name, Structural Design model analysis results, \n"
								<<   "\t" << "and Building Physics model analysis results. \n"
								<<   "\t" << "Note option -r should be on to obtain the file. \n"
								<<   "\t" << "\'verbose\' prints a descriptive outline of the process \n"
								<<   "\t" << "to the console during running \n";
			std::cout << "-v\t" << "or --visualization, to specify which models are\n"
								<<   "\t" << "visualized: \'ms\' for \'Movable Sizable\', \n"
								<<   "\t" << "\'msc\' for \'Movable Sizable\' view with alternate coloring according to input file, \n"
								<<   "\t" << "(note: insert the input file's name directly after the msc key, example: msc colorSetting -b etc.), \n"
								<<   "\t" << "\'rc\' for \'conformal model with Rectangles for R insertion method\', \n"
								<<   "\t" << "\'cb\' for \'conformal model with Cuboids for R insertion method\', \n"
								<<   "\t" << "\'tr\' for \'conformal model with Triangles and Rectangles for N insertion method\', \n"
								<<   "\t" << "\'tp\' for \'conformal model with Triangular Prism for N insertion method\', \n"
								<<   "\t" << "\'tb\' for \'conformal model with Triangular Prism for N insertion method and including bounding box \n"
								<<   "\t" << "(note: for this to be able to be visualized -b should be included in the run statement)\', \n"
								<<   "\t" << "\'sd\' for'\'Structural Design\', \n"
								<<   "\t" << "\'fe\' for \'Finite Elements of the structural design model\',\n"
								<<   "\t" << "and \'bp\' for the \'Building Physics model\'.\n"
								<<   "\t" << "More than one can be specified.\n"
								<<   "\t" << "Note: for any visualization (apart from 'msc' and 'tb'), the model should be generated with the command -r needs to be specified.\n";
			std::cout << "-b\t" << "or --bounded, to obtain a model with boundaries.\n";
			std::cout << "-r\t" << "or --run, to run the sd and bps simulation of the building spatial design.\n";
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
		else if (*arg == "-b" || *arg == "--bounded") 
		{
			bounded = true;
			++arg;
		}
		else if (*arg == "-r" || *arg == "--run") 
		{
			run = true;
			++arg;
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

				if(visualizations.back() == "msc")
				{
					if(++arg == args.end() || arg->operator[](0) == '-')
					{
						std::stringstream errorMessage;
							errorMessage << "\nError, expected an colour input file to be specified after 'msc'.\n"
													<< "use -h or --help for help\n";
							throw std::invalid_argument(errorMessage.str());
					}
						
					colorinputFile = *arg;
					boost::trim(colorinputFile);
					colourFileName = colorinputFile;

					if(colorinputFile == "")
					{
						std::stringstream errorMessage;
						errorMessage << "\nError, expected an colour input file to be specified after 'msc'.\n"
												<< "use -h or --help for help\n";
						throw std::invalid_argument(errorMessage.str());
					}
					if (colorinputFile.empty())
					{
						std::stringstream errorMessage;
						errorMessage << "Error, can not visualize an MS_N_building design using\n";
							errorMessage << "the following colour input file: " << colorinputFile << "\n";		
						throw std::invalid_argument(errorMessage.str());
					}
				}
			}
			if (visualizations.size() == 0)
			{
				std::stringstream errorMessage;
				errorMessage << "\nError, expected at least one string after\n"
										 << "-v or --visualize, received nothing. Use -h or\n"
										 << "--help for help\n";
				throw std::invalid_argument(errorMessage.str());
			}
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
		errorMessage << "\nError, expected an input file to be specified after 'i'.\n"
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

	// start executing the options:
	
	// start visualization:
	if (!visualizations.empty()) 
	{
		visualization::initVisualization(argc,argv);
		out("Initialized visualization", true, true, verbose);
	}

	// Obtain a file containing the bounded conformal model for Leiden representation
	if (bounded)
	{
		// Generate the Movable and Sizable (MS) building model
		bso::spatial_design::ms_N_building MS_bounded(inputFile);
		MS_bounded.addBoundary();
		out("Created a bounded Movable and Sizable (MS) building model", true, true, verbose);
		
		// Generate the conformal model based on MS
		bso::spatial_design::cf_building CM_bounded(MS_bounded, 0.001);
		out("Created a bounded Conformal Model (CM)",true,true,verbose);
		
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

		// send visualization of bounded conformal model
		for (const auto& visOption : visualizations)
		{
			if (visOption == "tb") visualization::visualize(CM_bounded,"triangular_prism_no_errors");
		}
		if (!visualizations.empty()) out("Sent bounded conformal model to visualization",true,true,verbose);
	}
	
	// Run and possibly print results to file
	if (run)
	{
		// Regular MS building model
		bso::spatial_design::ms_N_building MS(inputFile);
		out("Created a Movable and Sizable (MS) building model", true, true, verbose);
		bso::spatial_design::cf_building CM(MS, 0.001);
		out("Created a Conformal Model (CM)",true,true,verbose);

		// Initialize the grammar to specify basic properties per geometry
		bso::grammar::grammar gram(CM);
		out("Initialized grammar (gram)",true,true,verbose);
		
		// Generate the SD model based on the grammer specified basic properties per geometry
		bso::structural_design::sd_model SD = gram.sd_grammar<bso::grammar::DEFAULT_SD_GRAMMAR>(std::string("settings/sd_settings.txt"));
		out("Generated Structural Design (SD) model ",true,true,verbose);
		
		// Analyze the SD model
		SD.analyze();
		out("Evaluated SD: ",false,false,verbose);
		double SDResult = SD.getTotalResults().mTotalStrainEnergy;
		out(SDResult,true,true,verbose);

		// Generate the BP model based on the grammar specified basic properties per geometry
		bso::building_physics::bp_model BP = gram.bp_grammar<bso::grammar::DEFAULT_BP_GRAMMAR>(std::string("settings/bp_settings.txt"));
		out("Created a Building Physics (BP) model",true,true,verbose);
		
		// Analyze the BP model
		BP.simulatePeriods("runge_kutta_dopri5",1e-3,1e-3);
		double BPResult = BP.getTotalResults().mTotalEnergy;
		out("Evaluated BP: ",false,false,verbose);
		out(BPResult,true,true,verbose);

		// Visualize the simulations:
		if (!visualizations.empty()) for (const auto& visOption : visualizations)
		{
			if (visOption == "ms") bso::visualization::visualize(MS);
			try
			{
				if (visOption == "msc")	bso::visualization::visualize(colourFileName,MS);
			}
			catch(std::exception& e)
			{
				std::stringstream errorMessage;
				errorMessage << "Something when wrong while generating the msc visualization" << std::endl
										<< "(main.cpp)." << std::endl
										<< "Got the following error: " << std::endl
										<< e.what() << std::endl;
				throw std::invalid_argument(errorMessage.str());
			}
			if (visOption == "rc") visualization::visualize(CM,"rectangle");
			if (visOption == "cb") visualization::visualize(CM,"cuboid");
			if (visOption == "tr") visualization::visualize(CM,"trirec");
			if (visOption == "tp") visualization::visualize(CM,"triangular_prism");
			if (visOption == "sd") visualization::visualize(SD);
			if (visOption == "fe") visualization::visualize(SD, "element");
			if (visOption == "bp") visualization::visualize(BP);
		}
		if (!visualizations.empty()) out("Sent models to visualization",true,true,verbose);
		
		if(result_file)
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
	}

	if (!visualizations.empty())
	{
		for (const auto& visOption : visualizations)
		{
			if (visOption == "msc" && !run)
			{				
				bso::spatial_design::ms_N_building MS(inputFile);
				out("Created a Movable and Sizable (MS) building model", true, true, verbose);
				
				visualization::visualize(colourFileName,MS);
			} 
		}
	}
	
	// End the visualizations:
	if (!visualizations.empty()) visualization::endVisualization();

	return 0;
}
