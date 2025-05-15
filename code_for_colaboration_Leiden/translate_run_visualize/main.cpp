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

std::vector<std::string> MULTI_VIS_OPTIONS = {"ms","msc","pnf","pn"};
std::vector<std::string> MOD_OPTIONS = {"d","s"};
std::vector<std::string> VIS_OPTIONS = {"ms","msbase","msc","pnf","rc","cb","tr","tp","tb","pn","sd","fe","bp"};
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
	std::string modifyAddition = "";
	std::string colorinputFile = "";
	std::string colourFileName = "";
	std::string PNinputFile = "";
	std::string PNFileName = "";
	std::vector<std::string> modification;
	std::vector<int> deleteSpacesIDs;
	std::vector<int> splitSpacesIDs;
	std::vector<std::string> visualizations;
	std::string kindMultiVisual = "";
	std::vector<std::string> multiVisualizations;
	bool result_file = false;
	bool ms_files = false;
	bool modify = false;
	bool verbose = false;
	bool bounded = false;
	bool pn_representation = false;
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
			std::cout << "-m\t" << "or --modification, to specify a modification made prior to analysis with -r option (if applied)\n"
								<<   "\t" << "to the MS input file. The modification can be the following: \n"
								<<   "\t" << "input: \'-m d,20,1,3 s,5,6,7\', means that we apply the 'd' standing for \n"
								<< 	 "\t" << "'delete' space option to spaces with index 20, 1, and 3.\n"
								<<	 "\t" << "additionally, the 's' stand for 'split' and splits the spaces 5, 6, and 7\n"
								<< 	 "\t" << "in the middle at their longest axis. (Note: any amount of delete and split can be defined as \n"
								<< 	 "\t" << "long as they are the same amount for each categoryspaces needs to be defined and you can not split \n"
								<<   "\t" << "and delete a space with the same ID value. Measures are taken to prevent floating space and/or  \n"
								<<	 "\t" << "a building not at ground level while still resulting in functioning buildings)\n";
			std::cout << "-mf\t"<< "or --modificationfileName, to turn store modified MS_file on and to specify an addition to the original file  \n "
								<<	 "\t" << "name to indicate modified MS_representation file (only has effect if -m is applied)\n";
			std::cout << "-v\t" << "or --visualization, to specify which models are\n"
								<<   "\t" << "visualized: \'ms\' for \'Movable Sizable\' either with modification if applied, \n"
								<<   "\t" << "visualized: \'msbase\' for \'Movable Sizable\' without modification showing the input file, \n"
								<<   "\t" << "\'msc\' for \'Movable Sizable\' view with alternate coloring according to input file, \n"
								<<   "\t" << "(note: insert the input file's name directly after the msc key, example: msc colorSetting -b etc.), \n"
								<<   "\t" << "\'pnf\' for \'PN representation\' view with alternate coloring according to input file, \n"
								<<   "\t" << "(note: insert the input file's name directly after the pnf key, example: pnf colorSetting -b etc.), \n"
								<<   "\t" << "\'rc\' for \'conformal model with Rectangles for R insertion method\', \n"
								<<   "\t" << "\'cb\' for \'conformal model with Cuboids for R insertion method\', \n"
								<<   "\t" << "\'tr\' for \'conformal model with Triangles and Rectangles for N insertion method\', \n"
								<<   "\t" << "\'tp\' for \'conformal model with Triangular Prism for N insertion method\', \n"
								<<   "\t" << "\'tb\' for \'conformal model with Triangular Prism for N insertion method and including bounding box \n"
								<<   "\t" << "(note: this visualization is only possible with the -b option included in the run statement) \n"
								<< 	 "\t" << "\'pn\' for \'visualization of the triangular prims used for the PN representation definition \n"
								<<   "\t" << "(note: this visualization is only possible with the -pn option included in the run statement) \n"
								<<   "\t" << "\'sd\' for'\'Structural Design\', \n"
								<<   "\t" << "\'fe\' for \'Finite Elements of the structural design model\',\n"
								<<   "\t" << "and \'bp\' for the \'Building Physics model\'.\n"
								<<   "\t" << "More than one can be specified.\n"
								<<   "\t" << "Note: for any visualization (apart from 'ms', 'msc', 'pnf', 'pn' and 'tb'), the model should be \n"
								<< 	 "\t" << "generated with the command -r, and thus needs to be specified.\n";
			std::cout << "-mv\t"  << "or --multipleVisuals, to specify multiple ms, msc or pn models from text files to be visualized in one visualization window.\n"
								<<   "\t" << "to do this, follow the -mv with first the kinds of models to be visualized (note only ms, msc , pnf or pn models at a time),\n"
								<<   "\t" << "and then the names of the text files to be visualized.\n"
								<<   "\t" << "Example: -mv ms MS1.txt MS2.txt MS3.txt\n";
			std::cout << "-b\t" << "or --bounded, to obtain to obtain a text file with PN representation (i.e. leiden representation) \n"
								<< 	 "\t" << "with subscore '_b' as originally performed resulting in conformal triangles.\n";
			std::cout << "-pn\t"<< "to obtain a text file with PN representation (i.e. leiden representation) with subscore '_PN' \n"
								<< 	 "\t" << "with a delaunay triangulation per elevation, i.e. less triangles per elevation as the -b option.\n";
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
		else if (*arg == "-m" || *arg == "--modification")
		{
			modify = true;
			while (++arg != args.end() && arg->operator[](0) != '-')
			{
				std::string modSpec = *arg;
				boost::trim(modSpec);

				if (std::find(MOD_OPTIONS.begin(),MOD_OPTIONS.end(),modSpec.substr(0,1)) == MOD_OPTIONS.end())
				{
					std::stringstream errorMessage;
					errorMessage << "\nError, did not recognize: \"" << modSpec << "\"\n"
											 << "as an option for -m or --modification as a specification\n"
											 << "for a modification technique, or the space definition might be incorrectly defined\n"
											 << "Was it possible that a space was placed between the spaces?\n"
											 << "use -h or --help for help\n";
					throw std::invalid_argument(errorMessage.str());
				}
				modification.push_back(modSpec);

				// tokenize the line
				boost::char_separator<char> sep(","); // defines what separates tokens in a string
				typedef boost::tokenizer< boost::char_separator<char> > t_tokenizer; // settings for the boost::tokenizer
				t_tokenizer tokens(modSpec, sep); // this is where the tokenized line will be stored
				t_tokenizer::iterator token = tokens.begin(); // set iterator to first token
				int number_of_tokens = std::distance( tokens.begin(), tokens.end()); // count the number of tokens int the line

				if(modSpec.substr(0,1) == "d")
				{
					for(int i=0; i<number_of_tokens-1; i++)
					{
						deleteSpacesIDs.push_back(utilities::trim_and_cast_int(*(++token)));
					}
				}
				else if(modSpec.substr(0,1) == "s")
				{
					for(int i=0; i<number_of_tokens-1; i++)
					{
						splitSpacesIDs.push_back(utilities::trim_and_cast_int(*(++token)));
					}
				}
			}
			if (deleteSpacesIDs.size() == 0 || splitSpacesIDs.size() == 0)
			{
				std::stringstream errorMessage;
				errorMessage << "\nError, expected at least one to delete space and one to split\n"
										 << " space to be specified after the specifiers 'd' and 's'\n"
										 << "-m or --modification, received nothing. Use -h or\n"
										 << "--help for help\n";
				throw std::invalid_argument(errorMessage.str());
			}
			if (deleteSpacesIDs.size() != splitSpacesIDs.size())
			{
				std::stringstream errorMessage;
				errorMessage << "\nError, expected the same amount of delete and split spaces\n"
										 << "to be specified to retain the original nr. of spaces.\n"
										 << "Use -h or --help for help\n";
				throw std::invalid_argument(errorMessage.str());
			}
			// check if there are the same ID values defined in the delete and split spaces vector, if so trow an error
			for (unsigned int i = 0; i < deleteSpacesIDs.size(); i++)
			{
				if (std::find(splitSpacesIDs.begin(),splitSpacesIDs.end(),deleteSpacesIDs[i]) != splitSpacesIDs.end())
				{
					std::stringstream errorMessage;
					errorMessage << "\nError, found the same ID values in the delete and split spaces vector\n"
											 << "this is not possible to execute and thus not allowed. \n"
											 << "the duplicate values found is: " << deleteSpacesIDs[i] << "\n"
											 << "Use -h or --help for help\n";
					throw std::invalid_argument(errorMessage.str());
				}
			}
		}
		else if (*arg == "-mf" || *arg == "--modificationfileName")
		{
			if (++arg == args.end() || arg->operator[](0) == '-')
			{
				std::stringstream errorMessage;
				errorMessage << "\nError, expected a addition to the input files name to be specified after -mf\n"
										 << "Use -h or --help for help" << std::endl;
				throw std::invalid_argument(errorMessage.str());
			}
			modifyAddition = *arg;
			boost::trim(modifyAddition);
			++arg;
		}
		else if (*arg == "-b" || *arg == "--bounded") 
		{
			bounded = true;
			++arg;
		}
		else if (*arg == "-pn") 
		{
			pn_representation = true;
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

				if(visualizations.back() == "pnf")
				{
					if(++arg == args.end() || arg->operator[](0) == '-')
					{
						std::stringstream errorMessage;
							errorMessage << "\nError, expected an colour input file to be specified after 'pnf'.\n"
													<< "use -h or --help for help\n";
							throw std::invalid_argument(errorMessage.str());
					}
						
					colorinputFile = *arg;
					boost::trim(colorinputFile);
					colourFileName = colorinputFile;

					if(colorinputFile == "")
					{
						std::stringstream errorMessage;
						errorMessage << "\nError, expected an colour input file to be specified after 'pnf'.\n"
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

					if(++arg == args.end() || arg->operator[](0) == '-')
					{
						std::stringstream errorMessage;
							errorMessage << "\nError, expected an PN input file to be specified after 'pnf'.\n"
													<< "use -h or --help for help\n";
							throw std::invalid_argument(errorMessage.str());
					}
						
					PNinputFile = *arg;
					boost::trim(PNinputFile);
					PNFileName = PNinputFile;

					if(PNinputFile == "")
					{
						std::stringstream errorMessage;
						errorMessage << "\nError, expected an colour input file to be specified after 'pnf'.\n"
												<< "use -h or --help for help\n";
						throw std::invalid_argument(errorMessage.str());
					}
					if (PNinputFile.empty())
					{
						std::stringstream errorMessage;
						errorMessage << "Error, can not visualize an PN representation using\n";
							errorMessage << "the following colour input file: " << PNinputFile << "\n";		
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
		else if (*arg == "-mv" || *arg == "--multipleVisuals")
		{
			if (++arg == args.end() || arg->operator[](0) == '-')
			{
				std::stringstream errorMessage;
				errorMessage << "\nError, expected the kind of model to be visualized after -mv or --multipleVisuals\n"
										 << "Use -h or --help for help" << std::endl;
				throw std::invalid_argument(errorMessage.str());
			}			
			std::string visSpec = *arg;
			boost::trim(visSpec);
			if (std::find(MULTI_VIS_OPTIONS.begin(),MULTI_VIS_OPTIONS.end(),visSpec) == MULTI_VIS_OPTIONS.end())
			{
				std::stringstream errorMessage;
				errorMessage << "\nError, did not recognize: \"" << visSpec << "\"\n"
											<< "as an option for -mv or --multipleVisuals as a specification\n"
											<< "of a model to visualize, use -h or --help for help\n";
				throw std::invalid_argument(errorMessage.str());
			}
			kindMultiVisual = visSpec;
		
			if(kindMultiVisual == "msc")
			{
				if (++arg == args.end() || arg->operator[](0) == '-')
				{
					std::stringstream errorMessage;
					errorMessage << "\nError, expected a colorsettings file to be defined next directly after -mv or --multipleVisuals\n"
											<< "Use -h or --help for help" << std::endl;
					throw std::invalid_argument(errorMessage.str());
				}
				colorinputFile = *arg;
				boost::trim(colorinputFile);
				colourFileName = colorinputFile;
				int count = 0;
				while (++arg != args.end() && arg->operator[](0) != '-')
				{
					std::string modelFile = *arg;
					boost::trim(modelFile);
					multiVisualizations.push_back(modelFile);
					count++;
				}
				if(count == 0)
				{
					std::stringstream errorMessage;
					errorMessage << "\nError, expected at least one fileName after colorinputFile\n"
											 << "-mv or --multipleVisuals, received nothing. Use -h or\n"
											 << "--help for help\n";
					throw std::invalid_argument(errorMessage.str());
				}
			}
			else if (kindMultiVisual == "pnf")
			{
				if (++arg == args.end() || arg->operator[](0) == '-')
				{
					std::stringstream errorMessage;
					errorMessage << "\nError, expected a colorSettings file to be defined next directly after -mv or --multipleVisuals\n"
											<< "Use -h or --help for help" << std::endl;
					throw std::invalid_argument(errorMessage.str());
				}
				colorinputFile = *arg;
				boost::trim(colorinputFile);
				colourFileName = colorinputFile;
				int count = 0;
				while (++arg != args.end() && arg->operator[](0) != '-')
				{
					std::string modelFile = *arg;
					boost::trim(modelFile);
					multiVisualizations.push_back(modelFile);
					count++;
				}
				if(count == 0)
				{
					std::stringstream errorMessage;
					errorMessage << "\nError, expected at least one fileName after colorinputFile\n"
											 << "-mv or --multipleVisuals, received nothing. Use -h or\n"
											 << "--help for help\n";
					throw std::invalid_argument(errorMessage.str());
				}
			}
			else if (kindMultiVisual == "ms")
			{
				int count = 0;
				while (++arg != args.end() && arg->operator[](0) != '-')
				{
					std::string modelFile = *arg;
					boost::trim(modelFile);
					multiVisualizations.push_back(modelFile);
					count++;
				}
				if(count == 0)
				{
					std::stringstream errorMessage;
					errorMessage << "\nError, expected at least one fileName after the ms kind specification\n"
											 << "-mv or --multipleVisuals, received nothing. Use -h or\n"
											 << "--help for help\n";
					throw std::invalid_argument(errorMessage.str());
				}
			}
			else if (kindMultiVisual == "pn")
			{
				int count = 0;
				while (++arg != args.end() && arg->operator[](0) != '-')
				{
					std::string modelFile = *arg;
					boost::trim(modelFile);
					multiVisualizations.push_back(modelFile);
					count++;
				}
				if(count == 0)
				{
					std::stringstream errorMessage;
					errorMessage << "\nError, expected at least one fileName after the pn kind specification\n"
											 << "-mv or --multipleVisuals, received nothing. Use -h or\n"
											 << "--help for help\n";
					throw std::invalid_argument(errorMessage.str());
				}
			}
			else
			{
				std::stringstream errorMessage;
				errorMessage << "\nError, did not recognize: \"" << kindMultiVisual << "\"\n"
											<< "as a option for -mv or --multipleVisuals as a specification\n"
											<< "of a kind of model to visualize, use -h or --help for help\n";
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
	if(!(colorinputFile == "") || !(kindMultiVisual == ""))
	{
		/*there is no need for an design inputFile if there 
		is a colorInputFile or kindMultiVisual defined, 
		however, it may still be defined*/
	}
	else 
	{
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
	}

	// start executing the options:
	
	// start visualization:
	if (!visualizations.empty() || !(kindMultiVisual == "")) 
	{
		visualization::initVisualization(argc,argv);
		out("Initialized visualization", true, true, verbose);
	}

	// Define the base MS building model for all following options
	bso::spatial_design::ms_N_building MS;
	bso::spatial_design::ms_N_building MSBaseLine;
	if(inputFile != "")
	{
		MS = bso::spatial_design::ms_N_building(inputFile);	
		MSBaseLine = MS;
		out("Created a Movable and Sizable (MS) building model", true, true, verbose);
	}

	// Apply any modification to the MS model prior to analysis
	if(modify)
	{
		double floorArea = MS.getFloorArea(); // Store original floor area
		unsigned int nSpaces = MS.getSpacePtrs().size(); // Store original number of spaces
		
		// Remove spaces with inserted indexes.
		for(const auto& i : deleteSpacesIDs)
		{
			bso::spatial_design::ms_space* ptrDelSpace =  nullptr;
			try
			{
				ptrDelSpace =  MS.getSpacePtrWithID(i);
			}
			catch(const std::exception& e)
			{
				std::stringstream errorMessage;
				errorMessage << "Error, can not find a space with ID value " << i << ", thus cannot be deleted\n";
					errorMessage << "received the following error: \n" << e.what() << "\n";
				throw std::invalid_argument(errorMessage.str());
			}
			
			MS.deleteSpace(ptrDelSpace);
		}
		out("Removed spaces in building spatial design according to their inserted indexes",true,true,verbose);
		
		// Split spaces with inserted indexes
		for(const auto& i : splitSpacesIDs)
		{
			bso::spatial_design::ms_space* ptrSplitSpace = nullptr;
			try
			{
				ptrSplitSpace =  MS.getSpacePtrWithID(i);
			}
			catch(const std::exception& e)
			{
				std::stringstream errorMessage;
				errorMessage << "Error, can not find a space with ID value " << i << ", thus cannot be split\n";
					errorMessage << "received the following error: \n" << e.what() << "\n";
				throw std::invalid_argument(errorMessage.str());
			}
			
			bso::utilities::geometry::line_segment LongestAxis;
			double longestLength = 0;
			std::vector<bso::utilities::geometry::line_segment> lines = ptrSplitSpace->getGeometry().getLines();
			for(const auto k: lines)
			{
				if(k.getVector().isVertical()) continue; // only consider horizontal lines for the axis
				if(longestLength < k.getLength())
				{
					longestLength = k.getLength();
					LongestAxis = k;
				}
			}
			int splitamount = 2;
			MS.splitSpace(ptrSplitSpace, splitamount , LongestAxis);
		}
		out("Split spaces according to their inserted indexes",true,true,verbose);

		// Apply constrains of no floating spaces and building should start at ground level
		MS.setZZero();
		std::vector<spatial_design::ms_space*> floatingSpaces;
		if (MS.hasFloatingSpaces(floatingSpaces))
		{
			for (auto& i : floatingSpaces) MS.deleteSpace(i);
		}
		out("Make sure building is at ground level and checked and removed floating spaces",true,true,verbose);
		
		if(MS.getSpacePtrs().size() < nSpaces)
		{
			// Split the spaces with the largest dimension in x or y (checking x first) 
			while (MS.getSpacePtrs().size() < nSpaces)
			{
				auto spaceWithLargestDimension = MS.getSpacePtrs().front();
				bso::utilities::geometry::line_segment LongestAxis;
				double longestLength = 0;

				for (const auto& j : MS.getSpacePtrs())
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
				MS.splitSpace(spaceWithLargestDimension, splitamount , LongestAxis);
			}
			out("Split additional spaces to keep the nr. of spaces constant. This is necessary duo to the deletion of floating spaces",true,true,verbose);
		}

		// Scale the dimensions in x and y direction to retrieve original floor area of the building
		double scaleFactor = sqrt(floorArea / MS.getFloorArea());
		MS.scale({{0,scaleFactor},{1,scaleFactor}});
		out("Scaled building to original floor area",true,true,verbose);

		if(modifyAddition != "")
		{
			std::string modifiedFileName = designName + modifyAddition;
			MS.writeToFile(modifiedFileName);
			out("Print modified MS_N_building to file", true, true, verbose);
		}
	}

	// Obtain a file containing the bounded conformal model for Leiden representation
	if (bounded)
	{
		// Generate the Movable and Sizable (MS) building model
		bso::spatial_design::ms_N_building MS_bounded = MS; // use a copy so the base model is not changed
		MS_bounded.addBoundary();
		out("Created a bounded Movable and Sizable (MS) building model", true, true, verbose);
		
		// Generate the conformal model based on MS
		bso::spatial_design::cf_building CM_bounded(MS_bounded, 0.001);
		out("Created a bounded Conformal Model (CM)",true,true,verbose);
		
		// Define file name
		std::stringstream fileNameStream;
			fileNameStream << designName << "_b";
			std::string fileName = fileNameStream.str();
		
		// Write to file
		std::ofstream output;
		if (!fileName.empty()) output.open(fileName.c_str());
		
		if (fileName.empty() || !output.is_open())
		{
			std::stringstream errorMessage;
			errorMessage << "Could not open the following file to write an PN building spatial design to:" << std::endl
									 << ((fileName.empty())? "no input file given" : designName) << std::endl
									 << "(main.cpp). " << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
		
		CM_bounded.writeToFile(fileName);
		out("Print b version of MS_N_building to file", true, true, verbose);

		// send visualization of bounded conformal model
		for (const auto& visOption : visualizations)
		{
			if (visOption == "tb")
			{
				visualization::visualize(CM_bounded,"triangular_prism_no_errors");
				out("Sent bounded conformal model to visualization",true,true,verbose);
			}
		}
	}

	// Obtain a file containing the PN representation of leiden in a text file
	if (pn_representation)
	{
		// Define file name
		std::stringstream fileNameStream;
			fileNameStream << designName << "_PN";
			std::string fileName = fileNameStream.str();
		
		// Generate the Movable and Sizable (MS) building model with added bounds and directly output in PN format
		bso::spatial_design::ms_N_building MS_PN = MS; // use a copy so the base model is not changed
		std::vector<bso::utilities::geometry::triangular_prism> PNPrism = MS_PN.convertMSToPN(fileName, 0.001);
		out("Created a PN text file representation of the movable and Sizable (MS) building model", true, true, verbose);

		// send visualization of bounded conformal model
		for (const auto& visOption : visualizations)
		{
			if (visOption == "pn")
			{
				visualization::visualize(PNPrism,"PN representations triangular prims");
				out("Sent PN representations triangular prims to visualization",true,true,verbose);
			}
		}
	}
	
	// Run and possibly print results to file
	if (run)
	{
		// Regular MS building model
		bso::spatial_design::cf_building CM(MS, 0.001);
		out("Created a Conformal Model (CM)",true,true,verbose);

		// Initialize the grammar to specify basic properties per geometry
		bso::grammar::grammar gram(CM);
		out("Initialized grammar (gram)",true,true,verbose);
		
		// Generate the SD model based on the grammar specified basic properties per geometry
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
										<< "(main.cpp). " << std::endl;
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
			if (visOption == "ms") bso::visualization::visualize(MS);
			if (visOption == "msbase") bso::visualization::visualize(MSBaseLine, "spaces", "ms_N_building_base");
			if (visOption == "msc" && !run)
			{				
				visualization::visualize(colourFileName,MS);
				out("Sent msc visualization option visualization",true,true,verbose);
			} 
			if (visOption == "pnf")
			{						
				visualization::visualize(colourFileName,PNFileName);
				out("Sent pnf visualization option visualization",true,true,verbose);
			} 
		}
	}
	
	if (kindMultiVisual == "ms")
	{
		for (const auto& modelFile : multiVisualizations)
		{
			bso::spatial_design::ms_N_building MS(modelFile);
			out("Created a Movable and Sizable (MS) building model", true, true, verbose);
			visualization::visualize(MS);
			out("Sent ms visualization option visualization",true,true,verbose);
		}
	}
	else if (kindMultiVisual == "pn")
	{
		for (const auto& modelFile : multiVisualizations)
		{
			visualization::visualize(modelFile);
			out("Sent pn visualization option visualization",true,true,verbose);
		}
	}
	else if (kindMultiVisual == "msc")
	{
		for (const auto& modelFile : multiVisualizations)
		{
			bso::spatial_design::ms_N_building MS(modelFile);
			out("Created a Movable and Sizable (MS) building model", true, true, verbose);
			try
			{
				bso::visualization::visualize(colourFileName,MS);
				out("Sent msc visualization option visualization",true,true,verbose);
			}
			catch(std::exception& e)
			{
				std::stringstream errorMessage;
				errorMessage << "Something when wrong while generating the msc visualization." << std::endl
										<< "Check if the first file after the msc key is the colorInput file." << std::endl						
										<< "(main.cpp)." << std::endl
										<< "Got the following error: " << std::endl
										<< e.what() << std::endl;
				throw std::invalid_argument(errorMessage.str());
			}
		}
	}
	else if (kindMultiVisual == "pnf")
	{
		for (const auto& modelFile : multiVisualizations)
		{
			try
			{
				visualization::visualize(colourFileName,modelFile);
				out("Sent pnf visualization option visualization",true,true,verbose);
			}
			catch(std::exception& e)
			{
				std::stringstream errorMessage;
				errorMessage << "Something when wrong while generating the pnf visualization." << std::endl
										<< "Check if the first file after the pnf key is the colorInput file." << std::endl						
										<< "(main.cpp)." << std::endl
										<< "Got the following error: " << std::endl
										<< e.what() << std::endl;
				throw std::invalid_argument(errorMessage.str());
			}
		}
	}
	
	// End the visualizations:
	if (!visualizations.empty() || !(kindMultiVisual == "")) visualization::endVisualization();

	return 0;
}
