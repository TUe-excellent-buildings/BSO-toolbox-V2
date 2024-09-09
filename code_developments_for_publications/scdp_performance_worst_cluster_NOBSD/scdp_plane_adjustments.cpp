#include <iostream>

#include <bso/spatial_design/ms_N_building.hpp>
#include <bso/spatial_design/cf_building.hpp>
#include <bso/utilities/geometry.hpp>
#include <bso/utilities/data_point.hpp>
#include <bso/utilities/clustering.hpp>
#include <bso/utilities/data_plot.hpp>
#include <bso/structural_design/sd_model.hpp>
#include <bso/building_physics/bp_model.hpp>
#include <bso/grammar/grammar.hpp>
#include <bso/visualization/visualization.hpp>

#include <boost/algorithm/string.hpp>

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <map>
#include <filesystem>
#include <chrono>

std::vector<std::string> VIS_OPTIONS = {"ms","msc","msw","tp","tr","de","bo","sd","fe","bp"};
std::vector<std::string> MOD_OPTIONS = {"bp","sd","sdbp","bpsd"};
std::vector<std::string> OUT_OPTIONS = {"result_line", "verbose", "combine_data", "ms_files","plot","best","all","recDif"};
std::vector<std::string> DEL_OPTIONS = {"worst", "cluster"};

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

std::string getCurrentDateTime() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d_%H-%M-%S");
    return ss.str();
}

using namespace bso;
int main(int argc, char* argv[])
{
	// settings for iteration variables
	std::string inputFile = "";
	std::string designName = "";
	std::string inputLine = "";
	std::string modification = "";
	std::vector<std::string> visualizations;
	bool result_line = false;
	bool ms_files = false;
	bool plot = false;
	bool verbose = false;
	bool outputAll = false;
	bool outputBest = false;
	bool recDif = false;
	bool clusterDelete = false;
	bool resultDir = false;
	bool combineData = false;
	unsigned int iterations;
	unsigned int kMin = 2;
	unsigned int kMax = 10;
	unsigned int RNGSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	double spaceRemoveRatio = 0.1;
	int amountOfSpaceRemoval = 0;

	// Go trough the input argument and set the settings correctly for the iterations
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
			std::cout << "-k\t" << "or --kMinMax, to specify a kMin and kMax for a\n"
								<<   "\t" << "clustering, expects two integers larger than 1.\n";
			std::cout << "-r\t" << "or --ratio, to specify a minimum ratio(in case of clustering)/a ratio of spaces\n"
								<<   "\t" << "that should be removed in every iteration.\n"
								<<	 "\t" << "Expects a number in the range [0,1].\n";
			std::cout << "-a\t" << "or --amount, to specify a amount of spaces\n"
								<<   "\t" << "that should be removed in every iteration.\n"
								<<	 "\t" << "Expects an integer number above 1.\n";
			std::cout << "-m\t" << "or --modification, to specify which modification\n"
								<<   "\t" << "will be applied: \'sd\' for the sd-modifications;\n"
								<<   "\t" << "\'bp\' for the bp_modifications; \'sdbp\' to alternate\n"
								<<   "\t" << "between sd and bp, starting with sd; \'bpsd\', dito\n"
								<<   "\t" << "but now starting with bp. Only one option is possible.\n";
			std::cout << "-n\t" << "or --nModifications, to specify the number of modifications\n"
								<<   "\t" << "that will be applied. Expects only a positive integer\n";
			std::cout << "-d\t" << "or --deleteMethod, to specify how spaces are deleted:\n"
								<<   "\t" << "\'cluster\' for cluster deleting spaces\n"
								<<   "\t" << "\'worst\' for deleting spaces based on worst performance\n";
			std::cout << "-o\t" << "or --output, to specify the different outputs:\n"
								<<   "\t" << "\'result_line\' prints a line with the input\n"
								<<   "\t" << "parameters and the results; \'verbose\' prints\n"
								<<   "\t" << "a descriptive outline of the process;\n"
								<<   "\t" << "\'plot\' will make a data file (needed for plotting), \n"
								<< 	 "\t" << "visualize the plot to screen and the plot will be written to a png file\n"
								<<   "\t" << "\'combine_data\' will make a data file where saporate runs of this, \n"
								<< 	 "\t" << "executable can be copy pasted behind each other. \n"
								<< 	 "\t" << "This file can be loaded into excel to compare individual runs of this executable.\n"
								<<   "\t" << "\'ms_files\' will write the ms designs to files\n"
								<<   "\t" << "\'recdif\' will output the average divergence from a rectangle\n"
								<<   "\t" << "\'all\' will write all the designs to the specified format\n"
								<<   "\t" << "\'best\' (default) will only write the best all the designs to\n"
								<<   "\t" << "the specified format. More than one option can be given,\n"
								<<   "\t" << "but \'all\' and \'best\' cannot be given simultaneously.\n";
			std::cout << "-v\t" << "or --visualization, to specify which models are visualized: \n"
								<<   "\t" << "\'ms\' for \'Movable Sizable\',\n"
								<<   "\t" << "\'msc\' for \'Movable Sizable with visualization of clustering for -d cluster option\',\n"
								<<   "\t" << "\'msw\' for \'Movable Sizable with visualization of worst spaces to less worst spaces for -d worst option\',\n"
								<<   "\t" << "\'tp\' for \'conformal model with triangular_prism\', \n"
								<<   "\t" << "\'tr\' for \'conformal model with ReCtangles and triangles\', \n"
								<<   "\t" << "\'de\' for \'2D delaunay view used as basis for conformal model\'\n"
								<<   "\t" << "\'bo\' for \'2D view of boundary lines used for constrained delaunay\',\n"
								<<   "\t" << "\'sd\' for'\'Structural Design\',\n"
								<< 	 "\t" << "\'fe\' for \'Finite Elements of the structural design model\' and \n"
								<<   "\t" << "\'bp\' for the \'Building Physics model\'.\n"
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
										 << "must belarger than the first.\n";
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
				else if (outSpec == "combine_data") combineData = true;
				else if (outSpec == "plot")
				{
					plot = true;
					resultDir = true;
				}
				else if (outSpec == "ms_files")
				{
					ms_files = true;
					resultDir = true;
				} 
				else if (outSpec == "all") outputAll = true;
				else if (outSpec == "best") outputBest = true;
				else if (outSpec == "recdif") recDif = true;
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
	

























	// Initialize initial design:
	std::vector<spatial_design::ms_N_building> msDesigns;
	std::vector<spatial_design::ms_N_building> msDesignsTemp;
	try
	{
		if (!inputFile.empty()) msDesigns.push_back(spatial_design::ms_N_building(inputFile));
	}
	catch (std::exception& e)
	{
		std::stringstream errorMessage;
		errorMessage << "Error, could not initialize MS building design using\n";
		if (!inputFile.empty())
			errorMessage << "the following MS_N input file: " << inputFile << "\n";
		errorMessage << "Received the following error:\n" << e.what() << std::endl;
		
		throw std::invalid_argument(errorMessage.str());
	}
	out("Initialized initial MS_N building model", true, true, verbose);

	if(recDif)
	{
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
	












	// Start with co-evolutionary design proccess:
	unsigned int nSpaces = msDesigns.back().getSpacePtrs().size();
	double floorArea = msDesigns.back().getFloorArea();
	std::vector<structural_design::sd_model> sdModels;
	std::vector<building_physics::bp_model> bpModels;
	std::vector<utilities::data_point> designPerformances;
	std::vector<std::string> axisDesignPerformances = {"sd","bp"};
	for (unsigned int i = 0; i <= iterations; ++i)
	{
		out("\nIteration: ",false,false,verbose); out(i,true,false,verbose);
		
		designPerformances.push_back(utilities::data_point(2));

		msDesigns.back().writeToFile("lastGenDesign");
		
		// step 1, generate SD and BP models and evaluate them.
		spatial_design::cf_building cf(msDesigns.back(),1e-6);
		out("Initialized conformal model",true,true,verbose);

		grammar::grammar grm(cf);
		out("Initialized grammar",true,true,verbose);
		
		sdModels.push_back(grm.sd_grammar<grammar::DEFAULT_SD_GRAMMAR>(std::string("settings/sd_setting_only_liveload.txt"))); 
		out("Generated structural model",true,true,verbose);
		
		sdModels.back().analyze();
		out("Evaluated structural model: ",false,false,verbose);
		double sdResult = sdModels.back().getTotalResults().mTotalStrainEnergy;
		out(sdResult,true,true,verbose);
		designPerformances.back()(0) = sdResult;
		
		bpModels.push_back(grm.bp_grammar<grammar::DEFAULT_BP_GRAMMAR>(std::string("settings/bp_settings.txt")));
		out("Generated building physics model",true,true,verbose);
		
		bpModels.back().simulatePeriods("runge_kutta_dopri5",1e-3,1e-3);
		out("Evaluated building physics model: ",false,false,verbose);
		double bpResult = bpModels.back().getTotalResults().mTotalEnergy;
		out(bpResult,true,true,verbose);
		designPerformances.back()(1) = bpResult;
		
		if (!visualizations.empty()) for (const auto& visOption : visualizations)
		{
			if (visOption == "ms" && i != 0) visualization::visualize(msDesignsTemp.back(),"","ms_N_building",4.0);
			if (visOption == "ms") visualization::visualize(msDesigns.back(),"","ms_N_building",4.0);
			if (visOption == "sg") visualization::visualize(cf,"space_at_ground");
			if (visOption == "de") visualization::visualize(cf,"delaunay");
			if (visOption == "bo") visualization::visualize(cf,"boundarylines");
			if (visOption == "tr") visualization::visualize(cf,"trirec");
			if (visOption == "tp") visualization::visualize(cf,"triangular_prism");
			if (visOption == "sd") visualization::visualize(sdModels.back());
			if (visOption == "sdC") visualization::visualize(sdModels.back(), "component");
			if (visOption == "sdD") visualization::visualize(sdModels.back(), "density");
			if (visOption == "fe") visualization::visualize(sdModels.back(), "element");
			if (visOption == "fe") visualization::visualize(sdModels.back(), "strain_energy");
			if (visOption == "bp") visualization::visualize(bpModels.back());
			out("Sent models to visualization",true,true,verbose);
		}
		if (i >= iterations) break;
		
		// Create a new MS model so the original can not be adapted and adapt this version
		spatial_design::ms_N_building newMS = msDesigns.back();

		// Step 2, get performances per space
		std::vector<utilities::data_point> performances;
		std::vector<std::string> axisDataTypes = {"bp","sd"};
		bool floorAreaDivision = false;
		bool SurfaceAreaDivision = true;
		for (const auto& j : newMS.getSpacePtrs())
		{
			utilities::data_point tempDataPoint({0,0});
			std::stringstream spaceString;
			spaceString << *j;
			tempDataPoint.addLabel(spaceString.str());
			tempDataPoint.addSpacePtr(j);

			auto spaceGeom = j->getGeometry();
			if (modification != "bp") tempDataPoint(1) = sdModels.back().getPartialResults(&spaceGeom).mTotalStrainEnergy;
			if (modification != "sd") tempDataPoint(0) = bpModels.back().getPartialResults(&spaceGeom).mTotalEnergy;

			if(floorAreaDivision){tempDataPoint /= j->getFloorArea();}
			else if(SurfaceAreaDivision){tempDataPoint /= j->getSurfaceArea();}	

			performances.push_back(tempDataPoint);
		}
		out("Collected performances for each space",true,true,verbose);	

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
				std::vector<int> resultModificationFactor = {0,0};
				std::string title = "ms_N_worst";

				if (modification == "sd"){resultModificationFactor[1] = 1; title = "ms_N_worst_sd_" + std::to_string(i);}
				else if (modification == "bp"){resultModificationFactor[0] = 1; title = "ms_N_worst_bp_" + std::to_string(i);}
				else if (modification == "sdbp")
				{
					if(i % 2){resultModificationFactor[0] = 1; title = "ms_N_worst_bp_" + std::to_string(i);}
					else{resultModificationFactor[1] = 1; title = "ms_N_worst_sd_" + std::to_string(i);}
				}
				else if (modification == "bpsd")
				{
					if(i % 2){resultModificationFactor[1] = 1; title = "ms_N_worst_sd_" + std::to_string(i);}
					else{resultModificationFactor[0] = 1; title = "ms_N_worst_bp_" + std::to_string(i);}
				}	
				
				float transparency = 1.0;
				visualization::visualize(performances, resultModificationFactor, transparency, title, 4.0);
				out("Sent ms performances models to visualization",true,true,verbose);
			}
		}

		// Step 4, delete spaces
		std::vector<utilities::data_point> performancesRemoveSpace = performances;
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
					std::string title = "ms_N_clustering" + std::to_string(i);
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
					newMS.deleteSpace(j->getMSpacePtrs().back());
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
		else
		{
			//std::vector<utilities::data_point> performancesRemoveSpace = performances;
			if (modification == "sd" || modification == "bp")
			{
				std::sort(performancesRemoveSpace.begin(), performancesRemoveSpace.end());
			}
			else
			{ 
				// remove performance data which is not used so for deleting spaces so the following sorting algorithm works.
				int index = 0;
				if(modification == "sdbp")
				{
					if((i+1) % 2){index = 0;}
					else {index = 1;}
				}
				else if(modification == "bpsd")
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
				spaceAmountRemoval = round(nSpaces*spaceRemoveRatio);
				if(spaceAmountRemoval < 1){spaceAmountRemoval = 1;}
			}

			// Remove spaces
			for(int k=performancesRemoveSpace.size()-1; k>=performancesRemoveSpace.size()-spaceAmountRemoval; k--)
			{
				newMS.deleteSpace(performancesRemoveSpace[k].getMSpacePtrs().back());
			}

			newMS.setZZero();
			std::vector<spatial_design::ms_space*> floatingSpaces;
			if (newMS.hasFloatingSpaces(floatingSpaces))
			{
				for (auto& i : floatingSpaces) newMS.deleteSpace(i);
			}

			out("Removed worst performing spaces in building spatial design",true,true,verbose);
		}
		msDesignsTemp.push_back(newMS);
		


		// step 6, scale and split spaces to recover initial conditions
		// scale the dimensions in x and y direction
		double scaleFactor = sqrt(floorArea / newMS.getFloorArea());
		newMS.scale({{0,scaleFactor},{1,scaleFactor}});

		bool longestRib = false;
		bool performanceSplit = false;
		if(longestRib)
		{
			// split the spaces with the longest horizontal rib
			while (newMS.getSpacePtrs().size() < nSpaces)
			{
				auto spaceWithLargestDimension = newMS.getSpacePtrs().front();
				bso::utilities::geometry::line_segment LongestAxis;
				double longestLength = 0;

				for (const auto& j : newMS.getSpacePtrs())
				{
					std::vector<bso::utilities::geometry::line_segment> lines = j->getGeometry().getLines();
					for(const auto k: lines)
					{
						if(k.getVector().isVertical())
						{
							if(k.getLength() < 6000) continue; // only consider horizontal lines which are long enough for elevation height purposes
						} 
						if(longestLength < k.getLength())
						{
							longestLength = k.getLength();
							LongestAxis = k;
							spaceWithLargestDimension = j;
						}
					}
				}
				int splitamount = 2;
				if(LongestAxis.getVector().isVertical())
				{
					std::vector<std::pair<unsigned int, unsigned int> > splits = {{2,splitamount}};
					newMS.splitSpace(spaceWithLargestDimension, splits);
				}
				else
				{
					newMS.splitSpace(spaceWithLargestDimension, splitamount , LongestAxis);
				}
			}
		}
		else if(performanceSplit)
		{
			int splitCount = 0;
			int splitamount = 2;
			while (newMS.getSpacePtrs().size() < nSpaces)
			{				
				std::vector<bso::utilities::geometry::line_segment> lines = (performancesRemoveSpace[splitCount].getMSpacePtrs().back())->getGeometry().getLines();
				bso::utilities::geometry::line_segment LongestAxis;
				double longestLength = 0;
				bool found = false;
				for(const auto k: lines)
				{
					if(k.getVector().isVertical())
					{
						if(k.getLength() < 6000) continue; // only consider horizontal lines which are long enough for elevation height purposes
					} 
					if(longestLength < k.getLength() && k.getLength() > 4000)
					{
						longestLength = k.getLength();
						LongestAxis = k;
						found = true;
					}
				}

				if(found){newMS.splitSpace(performancesRemoveSpace[splitCount].getMSpacePtrs().back(), splitamount, LongestAxis);}
				splitCount++;
			}
		}
		// scaleFactor = sqrt(floorArea / newMS.getFloorArea());
		// newMS.scale({{0,scaleFactor},{1,scaleFactor}});
		out("Split spaces to restore number of spaces",true,true,verbose);
		
		// in case the lower spaces were removed reset minimum z coordinate to Z
		newMS.setZZero();
		out("Scaled design to initial volume",true,true,verbose);
		msDesigns.push_back(newMS);
		out("Modified building spatial design into a new one",true,true,verbose);

		if(recDif)
		{
			int spaceCount = msDesigns.back().getSpacePtrs().size();
			double averageRecDif = 0;
			for(const auto i: msDesigns.back().getSpacePtrs())
			{
				bso::utilities::geometry::quad_hexahedron spaceGeom = i->getGeometry();
				std::vector<bso::utilities::geometry::quadrilateral> spaceQuads = spaceGeom.getQuadrilateral();
				averageRecDif = averageRecDif + (spaceQuads[5]).getDifRectangle();
			}
			averageRecDif = averageRecDif/spaceCount;
			std::cout << "averageRecDif: " << averageRecDif << std::endl;
		}
	}
	out("finished SCDP process",true,true,verbose);
	















	// Output the requested outputs:
	std::vector<unsigned int> outputSpaces;
	// if best, find which designs are best;
	if (outputAll)
	{
		for (unsigned int i = 0; i < msDesigns.size(); ++i)
		{
			outputSpaces.push_back(i);
		}
	}


	std::string dir = "";
	if(resultDir)
	{
		// create resultDir if not existing yet
		bool resultDirGen = false;
		std::string path = "results";
		if (!std::filesystem::exists(path)) 
		{
			if (std::filesystem::create_directory(path)) 
			{
				resultDirGen = true;
				dir = path + "/";
				out("Directory created successfully.",true,true,verbose);
			} 
			else 
			{
				out("Failed to create directory.",true,true,verbose);
			}
		} 
		else 
		{
			resultDirGen = true;
			dir = path + "/";
			out("Directory already exists.",true,true,verbose);
		}

		if(resultDirGen)
		{
			std::string path2 = dir + inputFile + "_" + modification + "_-n=" + std::to_string(iterations) + "_" + getCurrentDateTime();
			if (!std::filesystem::exists(path2)) 
			{
				if (std::filesystem::create_directory(path2)) 
				{
					dir = path2 + "/";
					out("Sub-directory of results created successfully.",true,true,verbose);
				} 
				else 
				{
					out("Sub-directory of results failed to create directory.",true,true,verbose);
				}
			} 
			else 
			{
				dir = path2 + "/";
				out("Sub-directory of results directory already exists.",true,true,verbose);
			}
		}
	}

	if(plot)
	{
		utilities::data_plot dp(designPerformances, axisDesignPerformances, "Design Iteration");
		std::string fileScatter = dir+"scatterData_" + inputFile + "_" + modification;
		std::string fileParetoFront = dir+"paretoFrontData_" + inputFile + "_" + modification;
		std::string fileNonParetoFront = dir+"paretoNonFrontData_" + inputFile + "_" + modification;
		std::string fileVector = dir+"vectorData_" + inputFile + "_" + modification;
		std::string plotScatter = dir+"scatterPlot_" + inputFile + "_" + modification +".png";
		std::string plotParetoFront = dir+"paretoFrontPlot_" + inputFile + "_" + modification +".png";
		std::string plotVector = dir+"vectorPlot_" + inputFile + "_" + modification +".png";
		dp.plotScatter2D(fileScatter, plotScatter, false, true);
		dp.plotParetoFront2D(fileParetoFront, fileNonParetoFront, plotParetoFront, false, true);
		dp.plotVector2D(fileVector, plotVector, true, true);
	}
	if(combineData)
	{		
		utilities::data_plot dp(designPerformances, axisDesignPerformances, "Design Iteration");
		dp.addToCombinedDataFile("combined_data.csv", modification, inputFile);
	}
	
	for (const auto& i : outputSpaces)
	{
		if (result_line)
		{
			std::cout << designName << "\t"
								<< modification << "\t"
								<< i << "\t" << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10)
								<< designPerformances[i](0) << "\t"
								<< designPerformances[i](1) << "\t" << std::endl;
								if(clusterDelete)
								{
									std::cout << designName << RNGSeed << std::endl;
								}
		}
		if (ms_files)
		{
			std::stringstream fileNameStream;
			fileNameStream << dir << designName << "_"
										 << modification << "_"
										 << i << "_" << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10)
										 << designPerformances[i](0) << "_"
										 << designPerformances[i](1) << "_";
										 if(clusterDelete)
										 {
											fileNameStream << RNGSeed;
										 }
			std::string fileName = fileNameStream.str();
			msDesigns[i].writeToFile(fileName);
		}
	}
	
	if (!visualizations.empty()) visualization::endVisualization();
	
	return 0;
}
