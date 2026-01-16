#include <iostream>

#include <bso/spatial_design/ms_building.hpp>
#include <bso/spatial_design/ms_N_building.hpp>
#include <bso/spatial_design/cf_building.hpp>
#include <bso/utilities/geometry.hpp>
#include <bso/utilities/data_point.hpp>
#include <bso/utilities/clustering.hpp>
#include <bso/utilities/data_plot.hpp>
#include <bso/utilities/histogram.hpp>
#include <bso/structural_design/sd_model.hpp>
#include <bso/building_physics/bp_model.hpp>
#include <bso/daylight/dl_model.hpp>
#include <bso/grammar/grammar.hpp>
#include <bso/visualization/visualization.hpp>

#include <boost/algorithm/string.hpp>

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <map>

void Organize(std::vector<bso::daylight::dl_model>& dlModels, bso::utilities::data_plot& dp, bool hist)
{ 
	//-------------------create main folder------------------------//
		// Get current time
		std::time_t t = std::time(nullptr);
		std::tm* now = std::localtime(&t);
		// Format date and time as ddmmyyyy_hhmm
		char buffer[16];
		std::strftime(buffer, sizeof(buffer), "%d%m%Y_%H%M", now);

		std::string RUNfolderName = "Run_" + std::string(buffer);
		std::string command = "mkdir -p \"" + RUNfolderName + "\""; // Use mkdir command to create directory

		int result = std::system(command.c_str());

		if (result != 0) 
		{   
			std::cerr << "Error creating directory: " << RUNfolderName << std::endl;
			std::cerr << "Command executed: " << command << std::endl;
		}

	//-------------------organize DL files------------------------//
		for (int i=0; i<dlModels.size(); ++i)
		{
			std::string DLfolderName = dlModels[i].getFolderName();
			std::string moveCommand = "mv \"" + DLfolderName + "\" \"" + RUNfolderName + "\""; // Use mv command to move directory
			int result = std::system(moveCommand.c_str());

			if (result != 0) 
			{   
				std::cerr << "Error moving folder: " << DLfolderName << std::endl;
				std::cerr << "Command executed: " << moveCommand << std::endl;
			}

				//-------------------organize histogram file------------------------//
		if (hist==true)
		{
			if (i != dlModels.size()-1 )
			{
				std::string histodatafileName = "histogram_data_iteration_" + std::to_string(i) +".txt";
				std::string moveCommand2 = "mv \"" + histodatafileName + "\" \"" + RUNfolderName + "\""; // Use mv command to move directory
				int result2 = std::system(moveCommand2.c_str());

				if (result2 != 0) 
				{   
					std::cerr << "Error moving file: " << histodatafileName << std::endl;
					std::cerr << "Command executed: " << moveCommand2 << std::endl;
				}
			}
		}
	}//hist==true
	//-------------------organize dataplot files------------------------//
	if (dp.getoutputDirectory() != "")
	{
		std::string dpFolderName = dp.getoutputDirectory();
		std::string moveCommand = "mv \"" + dpFolderName + "\" \"" + RUNfolderName + "\""; // Use mv command to move directory
        int result = std::system(moveCommand.c_str());

        if (result != 0) 
        {   
            std::cerr << "Error moving folder: " << dpFolderName << std::endl;
            std::cerr << "Command executed: " << moveCommand << std::endl;
        }
	}



} //Organize()

std::vector<std::string> VIS_OPTIONS = {"ms","msc","msw","sc","rc","cb","sd","fe","dl","dlo"};
std::vector<std::string> MOD_OPTIONS = {"dl","sd","sddl","dlsd"};
std::vector<std::string> OUT_OPTIONS = {"result_line", "verbose", "ms_files","best","all","plot", "hist"};
std::vector<std::string> DEL_OPTIONS = {"worst", "cluster"};

int end, begin = 0;
template<class T>
void out(const T& t, const bool& e = false, const bool& i = false, const bool& verbose = false)
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
	std::string inputFile = "";
	std::string designName = "";
	std::string inputLine = "";
	std::string modification = "";
	std::vector<std::string> visualizations;
	bool result_line = false;
	bool ms_files = false;
	bool verbose = false;
	bool outputAll = false;
	bool outputBest = false;
	bool clusterDelete = true;
	unsigned int iterations = 0;
	unsigned int kMin = 2;
	unsigned int kMax = 10;
	unsigned int RNGSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	double spaceRemoveRatio = 0.1;
	int amountOfSpaceRemoval = 0;
	bool plot = false;
	bool hist = false;
	
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
								<<   "\t" << "in \'Movable Sizable\' format.\n";
			std::cout << "-l\t" << "or --line, to specify an input line for a\n"
								<<   "\t" << "supercube. Starts with design name, the a series\n"
								<<   "\t" << "integers and doubles that define the supercube\n";
			std::cout << "-k\t" << "or --kMinMax, to specify a kMin and kMax for a\n"
								<<   "\t" << "clustering, expects two integers larger than 1.\n";
			std::cout << "-r\t" << "or --ratio, to specify a minimum ratio of spaces\n"
								<<   "\t" << "that should be removed in every iteration.\n"
								<<	 "\t" << "Expects a number in the range [0,1]";
			std::cout << "-a\t" << "or --amount, to specify a amount of spaces\n"
								<<   "\t" << "that should be removed in every iteration.\n"
								<<	 "\t" << "Expects an integer number above 1.\n";
			std::cout << "-m\t" << "or --modification, to specify which modification\n"
								<<   "\t" << "will be applied: \'sd\' for the sd-modifications;\n"
								<<   "\t" << "\'dl\' for the dl_modifications; \'sddl\' to alternate\n"
								<<   "\t" << "between sd and dl, starting with sd; \'dlsd\', dito\n"
								<<   "\t" << "but now starting with dl. Only one option is possible.\n";
			std::cout << "-n\t" << "or --nModifications, to specify the number of modifications\n"
								<<   "\t" << "that will be applied. Expects only a positive integer\n";
			std::cout << "-d\t" << "or --deleteMethod, to specify how spaces are deleted:\n"
								<<   "\t" << "\'cluster\' for cluster deleting spaces\n"
								<<   "\t" << "\'worst\' for deleting spaces based on worst performance\n";
			std::cout << "-o\t" << "or --output, to specify the diferent outputs:\n"
								<<   "\t" << "\'result_line\' prints a line with the input\n"
								<<   "\t" << "parameters and the results; \'verbose\' prints\n"
								<<   "\t" << "a descriptive outline of the process;\n"
								<<   "\t" << "\'plot\' will make a data file (needed for plotting), \n"
								<<   "\t" << "visualize the plot to screen and the plot will be written to a png file.\n"
								<<   "\t" << "\'ms_files\' will write the ms designs to files\n"
								<<   "\t" << "\'all\' will write all the designs to the specified format\n"
								<<   "\t" << "\'best\' (default) will only write the best all the designs to\n"
								<<   "\t" << "the specified format. More than one option can be given,\n"
								<<   "\t" << "but \'all\' and \'best\' cannot be given simultaneously.\n";
			std::cout << "-v\t" << "or --visualization, to specify which models are\n"
								<<   "\t" << "visualized: \'ms\' for \'Movable Sizable\',\n"
								<<   "\t" << "\'msc\' for \'Movable Sizable with visualization of clustering for -d cluster option\',\n"
								<<   "\t" << "\'msw\' for \'Movable Sizable with visualization of worst spaces to less worst spaces for -d worst option\',\n"
								<<   "\t" << "\'sc\' for \'Super Cube\', \'rc\' for \'conformal model with\n"
								<<   "\t" << "ReCtangles\', \'cb\' for \'conformal model with CuBoids\'\n"
								<<   "\t" << "\'sd\' for'\'Structural Design\', \'fe\' for \'Finite\n"
								<<   "\t" << "Elements of the structural design model\',\n"
								<<   "\t" << "and \'dl\' for the \'Daylight model\'.\n"
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
		else if (*arg == "-l" || *arg == "--line")
		{
			std::stringstream scInputLine;
			bool first = true;
			if (++arg == args.end() || arg->operator[](0) == '-')
			{
				std::stringstream errorMessage;
				errorMessage << "\nError, expected a design name after -l or --line.\n";
				throw std::invalid_argument(errorMessage.str());
			}
			
			designName = *arg;
			boost::trim(designName);
			
			while (++arg != args.end() && arg->operator[](0) != '-')
			{
				std::string temp = *arg;
				boost::trim(temp);
				if (first) first = false;
				else scInputLine << "\t";
				scInputLine << temp;
			}
			if (first)
			{
				std::stringstream errorMessage;
				errorMessage << "\nError, expected a value for the input line\n"
										 << "specification after -l or --line.\n";
				throw std::invalid_argument(errorMessage.str());
			}
			inputLine = scInputLine.str();
		}
		else if (*arg == "-k" || *arg == "--kMinMax")
		{
			
			if (++arg == args.end() || arg->operator[](0) == '-')
			{
				std::stringstream errorMessage;
				errorMessage << "\nError, expected a value after -k or --kMinMax.\n";
				throw std::invalid_argument(errorMessage.str());
			}
			
			kMin = bso::utilities::trim_and_cast_uint(*arg);
			
			if (++arg == args.end() || arg->operator[](0) == '-')
			{
				std::stringstream errorMessage;
				errorMessage << "\nError, expected a second value after -k or --kMinMax.\n";
				throw std::invalid_argument(errorMessage.str());
			}
			
			kMax = bso::utilities::trim_and_cast_uint(*arg);
			
			if (kMax <= kMin)
			{
				std::stringstream errorMessage;
				errorMessage << "\nError, the second value after -k or --kMinMax \n"
										 << "must be larger than the first.\n";
				throw std::invalid_argument(errorMessage.str());
			}
			
			if (++arg != args.end() && arg->operator[](0) != '-')
			{
				std::stringstream errorMessage;
				errorMessage << "\nError, can only specify two values after -k or --kMinMax.\n";
				throw std::invalid_argument(errorMessage.str());
			}
			
		}
		else if (*arg == "-r" || *arg == "--ratio")
		{
			if (++arg == args.end() || arg->operator[](0) == '-')
			{
				std::stringstream errorMessage;
				errorMessage << "\nError, expected a value after -r or --ratio.\n";
				throw std::invalid_argument(errorMessage.str());
			}
			
			spaceRemoveRatio = bso::utilities::trim_and_cast_double(*arg);
			
			if (spaceRemoveRatio <= 0.0 || spaceRemoveRatio >= 1.0)
			{
				std::stringstream errorMessage;
				errorMessage << "\nError, the value after -r or --ratio \n"
										 << "should be within range [0,1], instead it is: "
										 << spaceRemoveRatio << std::endl;
				throw std::invalid_argument(errorMessage.str());
			}
			
			if (++arg != args.end() && arg->operator[](0) != '-')
			{
				std::stringstream errorMessage;
				errorMessage << "\nError, can only specify one value after -r or --ratio.\n";
				throw std::invalid_argument(errorMessage.str());
			}
			
		}
		else if (*arg == "-a" || *arg == "--amount")
		{
			if (++arg == args.end() || arg->operator[](0) == '-')
			{
				std::stringstream errorMessage;
				errorMessage << "\nError, expected a value after -a or --amount.\n";
				throw std::invalid_argument(errorMessage.str());
			}
			
			amountOfSpaceRemoval = bso::utilities::trim_and_cast_int(*arg);
			
			if (amountOfSpaceRemoval  <= 0)
			{
				std::stringstream errorMessage;
				errorMessage << "\nError, the value after -a or --amount \n"
										 << "should be an integer number larger then 1, instead it is: "
										 << amountOfSpaceRemoval << std::endl;
				throw std::invalid_argument(errorMessage.str());
			}
			
			if (++arg != args.end() && arg->operator[](0) != '-')
			{
				std::stringstream errorMessage;
				errorMessage << "\nError, can only specify one value after -a or --amount.\n";
				throw std::invalid_argument(errorMessage.str());
			}
			
		}
		else if (*arg == "-m" || *arg == "--modification")
		{
			if (++arg == args.end() || arg->operator[](0) == '-')
			{
				std::stringstream errorMessage;
				errorMessage << "\nError, expected a modification argument to be\n"
										 << "specified after -m or --modification. Use -h or\n"
										 << "--help for help\n";
				throw std::invalid_argument(errorMessage.str());
			}
			modification = *arg;
			boost::trim(modification);
			if (std::find(MOD_OPTIONS.begin(),MOD_OPTIONS.end(),modification) == MOD_OPTIONS.end())
			{
				std::stringstream errorMessage;
				errorMessage << "\nError, did not recognize: \"" << modification << "\"\n"
										 << "as an option for -m or --modification.\n"
										 << "use -h or --help for help\n";
				throw std::invalid_argument(errorMessage.str());
			}
			
			if (++arg != args.end() && arg->operator[](0) != '-')
			{
				std::stringstream errorMessage;
				errorMessage << "\nError, only one argument can be specified after\n"
										 << "-m or --modification. Use -h or -- help for help\n";
				throw std::invalid_argument(errorMessage.str());
			}				
		}
		else if (*arg == "-n" || *arg == "--nModifications")
		{
			if (++arg == args.end() || arg->operator[](0) == '-')
			{
				std::stringstream errorMessage;
				errorMessage << "\nError, expected a positive integer to be\n"
										 << "specified after -n or --nModifications. Use -h or\n"
										 << "--help for help\n";
				throw std::invalid_argument(errorMessage.str());
			}
			try
			{
				iterations = bso::utilities::trim_and_cast_uint(*arg);
			}
			catch (std::exception& e)
			{
				std::stringstream errorMessage;
				errorMessage << "\nError, could not parse argument specified after\n"
										 << "-n or --nModifications. received the following error:\n"
										 << e.what()
										 << "Use -h or -- help for help\n";
				throw std::invalid_argument(errorMessage.str());
			}
			
			if (++arg != args.end() && arg->operator[](0) != '-')
			{
				std::stringstream errorMessage;
				errorMessage << "\nError, only one argument can be specified after\n"
										 << "-n or --nModifications. Use -h or -- help for help\n";
				throw std::invalid_argument(errorMessage.str());
			}				
		}
		else if (*arg == "-d" || *arg == "--deleteMethod")
		{
			while (++arg != args.end() && arg->operator[](0) != '-')
			{
				std::string outSpec = *arg;
				boost::trim(outSpec);
				if (outSpec == "cluster") clusterDelete = true;
				else if(outSpec == "worst") clusterDelete = false;
				else
				{
					std::stringstream errorMessage;
					errorMessage << "\nError, did not recognize: " << outSpec << " as an\n"
												<< "argument for -d or --deleteMethod. Use -h or\n"
												<< "--help for help\n";
					throw std::invalid_argument(errorMessage.str());
				}
			}
		}
		else if (*arg == "-s" || *arg == "--seed")
		{
			if (++arg == args.end() || arg->operator[](0) == '-')
			{
				std::stringstream errorMessage;
				errorMessage << "\nError, expected a positive integer to be\n"
										 << "specified after -s or --seed. Use -h or\n"
										 << "--help for help\n";
				throw std::invalid_argument(errorMessage.str());
			}
			try
			{
				RNGSeed = bso::utilities::trim_and_cast_uint(*arg);
			}
			catch (std::exception& e)
			{
				std::stringstream errorMessage;
				errorMessage << "\nError, could not parse argument specified after\n"
										 << "-s or --seed. received the following error:\n"
										 << e.what()
										 << "Use -h or -- help for help\n";
				throw std::invalid_argument(errorMessage.str());
			}
			
			if (++arg != args.end() && arg->operator[](0) != '-')
			{
				std::stringstream errorMessage;
				errorMessage << "\nError, only one argument can be specified after\n"
										 << "-s or --seed. Use -h or -- help for help\n";
				throw std::invalid_argument(errorMessage.str());
			}				
		}
		else if (*arg == "-o" || *arg == "--output")
		{
			while (++arg != args.end() && arg->operator[](0) != '-')
			{
				std::string outSpec = *arg;
				boost::trim(outSpec);
				if 			(outSpec == "result_line") result_line = true;
				else if (outSpec == "verbose") verbose = true;
				else if (outSpec == "ms_files") ms_files = true;
				else if (outSpec == "all") outputAll = true;
				else if (outSpec == "best") outputBest = true;
				else if (outSpec == "plot") plot=true;
				else if (outSpec == "hist") hist=true;
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
	if (inputFile == "" && inputLine == "")
	{
		std::stringstream errorMessage;
		errorMessage << "\nError, expected an input file or line to be specified.\n"
								 << "use -h or --help for help\n";
		throw std::invalid_argument(errorMessage.str());
	}
	if ((ms_files || result_line) && (outputAll && outputBest))
	{
		std::stringstream errorMessage;
		errorMessage << "Cannot specify both \'all\'  and \'best\' for output.\n"
								 << "use -h or --help for help\n";
		throw std::invalid_argument(errorMessage.str());
	}
	if (!outputAll && !outputBest) outputBest = true;
	out("Parsed program arguments", true, true, verbose);
	if (!visualizations.empty()) visualization::initVisualization(argc,argv);
	out("Initialized visualization", true, true, verbose);
	
	std::vector<spatial_design::ms_building> msDesigns;
	std::vector<spatial_design::ms_building> msDesignsTemp;
	try
	{
		if (!inputFile.empty()) msDesigns.push_back(spatial_design::ms_building(inputFile));
		else
		{
			spatial_design::sc_building sc(inputLine);
			msDesigns.push_back(spatial_design::ms_building(sc));
		}
	}
	catch (std::exception& e)
	{
		std::stringstream errorMessage;
		errorMessage << "Error, could not initialize MS building design using\n";
		if (!inputFile.empty())
			errorMessage << "the following MS input file: " << inputFile << "\n";
		else
			errorMessage << "an SC model initialized by the following line:\n" << inputLine;
		errorMessage << "Received the following error:\n" << e.what() << std::endl;
		
		throw std::invalid_argument(errorMessage.str());
	}
	out("Initialized initial MS building model", true, true, verbose);
	
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------//

	unsigned int nSpaces = msDesigns.back().getSpacePtrs().size();
	double floorArea = msDesigns.back().getFloorArea();
	std::vector<spatial_design::sc_building> scDesigns;
	std::vector<structural_design::sd_model> sdModels;
	//std::vector<building_physics::bp_model> bpModels;
	std::vector<daylight::dl_model> dlModels;
	std::vector<utilities::data_point> designPerformances;
	std::vector<utilities::data_point> DLdesignPerformances;
	std::vector<std::string> axisDesingPerformances = {"sd", "dl"};

	for (unsigned int i = 0; i <= iterations; ++i)
	{
		out("\nIteration: ",false,false,verbose); out(i,true,false,verbose);
		if (std::find(visualizations.begin(),visualizations.end(),"sc") != visualizations.end() ||
				result_line)
		{
			scDesigns.push_back(spatial_design::sc_building(msDesigns.back()));
			out("initialized sc building",true,true,verbose);
		}
		designPerformances.push_back(utilities::data_point(2));
		
		// step 1, generate SD and DL models and evaluate them.
		spatial_design::cf_building cf(msDesigns.back(), "settings/dl_settings.txt"/*, 1e-6*/);
		out("Initialized conformal model",true,true,verbose);

		grammar::grammar grm(cf);
		out("Initialized grammar",true,true,verbose);
		
		dlModels.push_back(grm.dl_grammar <bso::grammar::DEFAULT_DL_GRAMMAR>(std::string("settings/dl_settings.txt")));
		out("Generated daylight model",true,true,verbose);

		dlModels.back().analyse();
		out("Evaluated daylight model (B_target): ",false,false,verbose);
		//mADFbuilding = 1/ADF , mTotalScore = acc to buildingguidelines (0-5-10)
		double dlResultADF = dlModels.back().getTotalResults().mADFbuilding;
		double dlResultNEN = dlModels.back().getTotalResults().mTotalScore;
		out(dlResultNEN,true,true,verbose);
		//out(dlResultNEN,true,true,verbose);
		out("Evaluated daylight model (B_ADF): ",false,false,verbose);
		out(dlResultADF,true,true,verbose);
		designPerformances.back()(1) = dlResultNEN;

		// bpModels.push_back(grm.bp_grammar<grammar::DEFAULT_BP_GRAMMAR>(std::string("settings/bp_settings.txt")));
		// out("Generated building physics model",true,true,verbose);
		// bpModels.back().simulatePeriods("runge_kutta_dopri5",1e-3,1e-3);
		// out("Evaluated building physics model: ",false,false,verbose);
		// double bpResult = bpModels.back().getTotalResults().mTotalEnergy;
		// out(bpResult,true,true,verbose);
		// designPerformances.back()(1) = bpResult;

		sdModels.push_back(grm.sd_grammar<grammar::DEFAULT_SD_GRAMMAR>(std::string("settings/sd_settingsQH.txt")));
		out("Generated structural model",true,true,verbose);
		
		sdModels.back().analyze();
		out("Evaluated structural model: ",false,false,verbose);
		double sdResult = sdModels.back().getTotalResults().mTotalStrainEnergy;
		out(sdResult,true,true,verbose);
		designPerformances.back()(0) = sdResult;
		
		if (!visualizations.empty()) for (const auto& visOption : visualizations)
		{
			if (visOption == "ms" && i != 0) visualization::visualize(msDesignsTemp.back(),"","ms_building",4.0);
			if (visOption == "ms") visualization::visualize(msDesigns.back(),"","ms_building",4.0);
			if (visOption == "sc") visualization::visualize(scDesigns.back());
			if (visOption == "rc") visualization::visualize(cf,"rectangle");
			if (visOption == "cb") visualization::visualize(cf,"cuboid");
			if (visOption == "sd") visualization::visualize(sdModels.back());
			if (visOption == "fe") visualization::visualize(sdModels.back(), "element");
			if (visOption == "fe") visualization::visualize(sdModels.back(), "strain_energy");
			if (visOption == "dl") visualization::visualize(dlModels.back(), "");
		}
		
		out("Sent models to visualization",true,true,verbose);
		if (i >= iterations) break;
		
		// create a new MS model
		spatial_design::ms_building newMS = msDesigns.back();
		
		// step 2, get performances per space
		std::vector<utilities::data_point> performances;
		std::vector<std::vector<double>> roomInfo;
		for (const auto& j : newMS.getSpacePtrs())
		{
			std::vector<double> ADF_ID(3);
			utilities::data_point tempDataPoint({0,0});
			std::stringstream spaceString;
			spaceString << *j;
			tempDataPoint.addLabel(spaceString.str());
			tempDataPoint.addSpacePtr(j);

			auto spaceGeom = j->getGeometry();
			auto spaceID = j->getID();
			//if (modification != "bp") tempDataPoint(1) = sdModels.back().getPartialResults(&spaceGeom).mTotalStrainEnergy;
			if (modification != "dl") tempDataPoint(1) = sdModels.back().getPartialResults(&spaceGeom).mTotalStrainEnergy;
			if (modification != "sd") 
				{
					tempDataPoint(0) = dlModels.back().getPartialResults(spaceID).mADFbuilding; //run optimization on inverse ADF

					//collect data for dl histogram
					ADF_ID[0]=dlModels.back().getPartialResults(spaceID).mTotalScore;
					ADF_ID[1]=static_cast<double>(spaceID);
					ADF_ID[2]=dlModels.back().getPartialResults(spaceID).mADFroom; //returns ADF
					roomInfo.push_back(ADF_ID);
				}
			tempDataPoint /= j->getFloorArea();
			performances.push_back(tempDataPoint);
		}
		out("Collected performances for each space",true,true,verbose);

		if(hist==true)
		{
			utilities::histogram histogramDL(roomInfo, dlModels.back(),std::to_string(i));
		}

		// step 3, normalize the data points
		utilities::data_point utopianPoint;
		utopianPoint.setToUtopia(performances);
		utilities::data_point distopianPoint;
		distopianPoint.setToDistopia(performances);
		std::swap(utopianPoint(1),distopianPoint(1));
		for (auto& j : performances)
		{
			j.normalize(utopianPoint,distopianPoint,{1});
		}
		utopianPoint.setToUtopia(performances);
		distopianPoint.setToDistopia(performances);
		out("Normalized space performances",true,true,verbose);
		
		// Visualize the performance of spaces
		if (!visualizations.empty()) for (const auto& visOption : visualizations)
		{
			if (visOption == "msw")
			{
				std::vector<double> resultModificationFactor = {0,0};
				std::string title = "ms_worst";

				if (modification == "sd"){resultModificationFactor[1] = 1; title = "ms_worst_sd_" + std::to_string(i);}
				else if (modification == "dl"){resultModificationFactor[0] = 1; title = "ms_worst_dl_" + std::to_string(i);}
				else if (modification == "sddl")
				{
					if(i % 2){resultModificationFactor[0] = 1; title = "ms_worst_dl_" + std::to_string(i);}
					else{resultModificationFactor[1] = 1; title = "ms_worst_sd_" + std::to_string(i);}
				}
				else if (modification == "dlsd")
				{
					if(i % 2){resultModificationFactor[1] = 1; title = "ms_worst_sd_" + std::to_string(i);}
					else{resultModificationFactor[0] = 1; title = "ms_worst_dl_" + std::to_string(i);}
				}	
				
				float transparency = 1.0;
				//out("resultModificationFactor is: "<< resultModificationFactor[0] << " ; " << resultModificationFactor[1],true,true,verbose)
				std::cout << "resultModificationFactor is: " << resultModificationFactor[0] << " ; " << resultModificationFactor[1] << std::endl;
				visualization::visualize(performances, resultModificationFactor, transparency, title, 4.0);
				out("Sent ms performances models to visualization",true,true,verbose);
			}

			//conduct building orientation advice, based on performances
			//because it is only an advise and not considered in the loop, shown here
			if (visOption == "dlo") 
			{
				std::string ADFSide = dlModels.back().BuildingOrientationAdvice();
				out("Buliding Orientation Advice conducted",true,true,verbose);
				out(ADFSide,true,true,verbose);
				visualization::visualize(dlModels.back(),"orientation");
			}
		}

		// Step 4, delete spaces
		if(clusterDelete)
		{
			// step 4, cluster the data
			utilities::clustering clus(RNGSeed,performances,kMin,kMax,250);
			out("Clustered space performances",true,true,verbose);

			// Visualize the clustering if requested
			if (!visualizations.empty()) for (const auto& visOption : visualizations)
			{
				if (visOption == "msc")
				{
					std::string title = "ms_clustering" + std::to_string(i);
					visualization::visualize(clus, title, 4.0);
					out("Sent ms clustering models to visualization",true,true,verbose);
				}
			}
			
			// step 5, select worst performing cluster and remove these spaces
			unsigned int nRetainedSpaces = std::floor(nSpaces*(1-spaceRemoveRatio));
			auto clusters = clus.getClusters();
			while (newMS.getSpacePtrs().size() > nRetainedSpaces && clusters.size() > 1)
			{
				// find cluster with centroid closest to distopianPoint.
				unsigned int worstClusterIndex = 0;
				double smallestDistance = distopianPoint.calcDistanceTo(clusters[0].getCentroid());
				for (unsigned int j = 1; j < clusters.size(); ++j)
				{
					double distance = distopianPoint.calcDistanceTo(clusters[j].getCentroid());
					if (smallestDistance > distance)
					{
						smallestDistance = distance;
						worstClusterIndex = j;
					}
				}
				
				// remove spaces within that cluster.
				for (const auto& j : clusters[worstClusterIndex].getData())
				{
					std::string msSpaceLine = j->getLabels().back();
					bso::spatial_design::ms_space temp_space(msSpaceLine);
					newMS.deleteSpace(temp_space);
				}
				
				// remove that cluster from the vector containing all clusters
				clusters.erase(clusters.begin() + worstClusterIndex);
			}
			newMS.setZZero();
			std::vector<spatial_design::ms_space*> floatingSpaces;
			if (newMS.hasFloatingSpaces(floatingSpaces))
			{
				for (auto& i : floatingSpaces) newMS.deleteSpace(i);
			}
			out("Removed worst performing cluster of spaces in building spatial design",true,true,verbose);
		}
		else //delete per space
		{
			std::vector<utilities::data_point> performancesRemoveSpace = performances;
			if (modification == "sd" || modification == "dl")
			{
				std::sort(performancesRemoveSpace.begin(), performancesRemoveSpace.end());
			}
			else
			{ 
				// remove performance data which is not used so for deleting spaces so the following sorting algorithm works.
				int index = 0;
				if(modification == "sddl")
				{
					if((i+1) % 2){index = 0;}
					else {index = 1;}
				}
				else if(modification == "dlsd")
				{
					if((i+1) % 2){index = 1;}
					else {index = 0;}
				}
				for(auto &k: performancesRemoveSpace){k(index) = 0;} 
				std::sort(performancesRemoveSpace.begin(), performancesRemoveSpace.end());
			}	

			// Determine space removal amount
			int spaceAmountRemoval;
			if(amountOfSpaceRemoval > 0) {spaceAmountRemoval = amountOfSpaceRemoval;}
			else 
			{
				spaceAmountRemoval = nSpaces*spaceRemoveRatio;
				if(spaceAmountRemoval == 0){spaceAmountRemoval == 1;}
			}

			// Remove spaces
			for(int k=performancesRemoveSpace.size()-1; k>=performancesRemoveSpace.size()-spaceAmountRemoval; k--)
			{
				newMS.deleteSpace(performancesRemoveSpace[k].getMSpacePtrs().back());
			}
			out("Removed worst performing spaces in building spatial design",true,true,verbose);

			newMS.setZZero();
			std::vector<spatial_design::ms_space*> floatingSpaces;
			if (newMS.hasFloatingSpaces(floatingSpaces))
			{
				for (auto& i : floatingSpaces) newMS.deleteSpace(i);
			}
			out("Removed floating spaces in building spatial design",true,true,verbose);

		}
		msDesignsTemp.push_back(newMS);
		
		// step 6, scale and split spaces to recover initial conditions
		// scale the dimensions in x and y direction
		double scaleFactor = sqrt(floorArea / newMS.getFloorArea());
		newMS.scale({{0,scaleFactor},{1,scaleFactor}});

		// split the spaces with the largest dimension in x or y direction (checking x first)
		while (newMS.getSpacePtrs().size() < nSpaces)
		{
			auto spaceWithLargestDimension = newMS.getSpacePtrs().front();
			std::pair<unsigned int, double> largestDimension;
			largestDimension.first = 0;
			largestDimension.second = spaceWithLargestDimension->getDimensions()(0);
			bool firstCheck = true;
			for (const auto& j : newMS.getSpacePtrs())
			{
				for (unsigned int k = 0; k < 3; ++k)
				{
					double dimension = j->getDimensions()(k);
					if (k == 2 && dimension < 6000) continue;
					if (largestDimension.second < dimension || firstCheck)
					{
						largestDimension.first = k;
						largestDimension.second = dimension;
						spaceWithLargestDimension = j;
						firstCheck = false;
					}
					else if (largestDimension.second == dimension)
					{ // if they both are the largest, check which one has a smaller z-coordinate
						if (spaceWithLargestDimension->getCoordinates()(2) > j->getCoordinates()(2))
						{
							spaceWithLargestDimension = j;
							largestDimension.first = k;
						}
					}
				}
			}
			newMS.splitSpace(spaceWithLargestDimension,{{largestDimension.first,2}});
		}
		out("Split spaces to restore number of spaces",true,true,verbose);
		
		// in case the lower spaces were removed reset minimum z coordinate to Z
		newMS.setZZero();
		out("Scaled design to initial volume",true,true,verbose);
		
		msDesigns.push_back(newMS);
		out("Modified building spatial design into a new one",true,true,verbose);
	} // i of iterations

	out("finished SCDP process",true,true,verbose);
	
	std::vector<unsigned int> outputSpaces;
	// if best, find which designs are best;
	if (outputAll)
	{
		for (unsigned int i = 0; i < msDesigns.size(); ++i)
		{
			outputSpaces.push_back(i);
		}
	}
	
	if(plot==true)
	{
		utilities::data_plot dp(designPerformances, axisDesingPerformances, "Design Iteration", "plotFiles");
		std::string fileScatter = "scatterData_" + inputFile + "_" + modification;
		std::string fileParetoFront = "paretoFrontData_" + inputFile + "_" + modification;
		std::string fileNonParetoFront = "paretoNonFrontData_" + inputFile + "_" + modification;
		std::string fileVector = "vectorData_" + inputFile + "_" + modification;

		dp.plotScatter2D(fileScatter);
		dp.plotParetoFront2D(fileParetoFront, fileNonParetoFront);
		dp.plotVector2D(fileVector); 

		Organize(dlModels, dp, hist);
	}
	else if(plot==false)
	{
		utilities::data_plot dp2;
		Organize(dlModels, dp2, hist);
	}

	for (const auto& i : outputSpaces)
	{
		if (result_line)
		{
			std::cout << designName << "\t"
								<< modification << "\t"
								<< i << "\t"
								<< designPerformances[i](0) << "\t"
								<< designPerformances[i](1) << "\t"
								<< RNGSeed << "\t"
								<< scDesigns[i] << std::endl;
		}
		if (ms_files)
		{
			std::stringstream fileNameStream;
			fileNameStream << designName << "_"
										 << modification << "_"
										 << i << "_"
										 << designPerformances[i](0) << "_"
										 << designPerformances[i](1) << "_"
										 << RNGSeed;
			std::string fileName = fileNameStream.str();
			msDesigns[i].writeToFile(fileName);
		}
	}
	
	if (!visualizations.empty()) visualization::endVisualization();
	
	return 0;
}
