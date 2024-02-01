#include <iostream>

#include <bso/spatial_design/ms_N_building.hpp>
#include <bso/spatial_design/cf_building.hpp>
#include <bso/visualization/visualization.hpp>

#include <boost/algorithm/string.hpp>
#include <vector>
#include <sstream>

std::vector<std::string> VIS_OPTIONS = {"ms","tr","tp"};
std::vector<std::string> OUT_OPTIONS = {"PN_file", "verbose", "ms_files"};

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
	std::string inputFile = "";
	std::string designName = "";
	std::string modification = "";
	std::vector<std::string> visualizations;
	bool PN_file = false;
	bool ms_files = false;
	bool verbose = false;
	
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
								<<   "\t" << "\'PN_file\' output a file containing \n"
								<<   "\t" << "the PN definition of the input MS_N_building definition \n"
								<<   "\t" << "\'verbose\' prints a descriptive outline of the process \n"
								<<   "\t" << "to the console during running \n";
			std::cout << "-v\t" << "or --visualization, to specify which models are\n"
								<<   "\t" << "visualized: \'ms\' for \'Movable Sizable\', \n"
								<<   "\t" << "\'tr\' for \'conformal model with Triangles and Rectangles for N insertion method\', \n"
								<<   "\t" << "\'tp\' for \'conformal model with Triangular Prism for N insertion method\', \n"
								<<   "\t" << "More than one can be specified.\n";
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
				if 			(outSpec == "PN_file") PN_file = true;
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
	bso::spatial_design::ms_N_building MS(inputFile); 	
	out("Created a Movable and Sizable (MS) building model", true, true, verbose);
	
	// Generate the conformal model based on MS
	bso::spatial_design::cf_building CM(MS, 0.001);
	out("Created a Conformal Model (CM)",true,true,verbose);

	// Print results to file
	if (PN_file)
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
		
		CM.writeToFile(fileName);
		out("Print PN version of MS_N_building to file", true, true, verbose);
	}
	
	if (!visualizations.empty()) 
	{
		bso::visualization::initVisualization(argc,argv);
		out("Initialized visualization", true, true, verbose);
	}
	
	if (!visualizations.empty()) for (const auto& visOption : visualizations)
	{
		if (visOption == "ms") bso::visualization::visualize(MS);
		if (visOption == "tr") bso::visualization::visualize(CM,"trirec");
		if (visOption == "tp") bso::visualization::visualize(CM,"triangular_prism");
	}
	if (!visualizations.empty()) out("Sent models to visualization",true,true,verbose);
	if (!visualizations.empty()) bso::visualization::endVisualization();

	return 0;
}