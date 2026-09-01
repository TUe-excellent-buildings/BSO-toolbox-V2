#include <bso/optimizers/scdp/scdp_config.hpp>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <sstream>
#include <stdexcept>

#include <boost/algorithm/string.hpp>

#include <bso/utilities/trim_and_cast.hpp>

namespace bso { namespace optimizers { namespace scdp {

// Helper: parse a modification string (e.g. "sd", "sdbp", "bpsddl")
// into a sequence of base tokens using the provided token set. Throws if invalid.
static std::vector<std::string> parseModificationString(
    const std::string& mod,
    const std::vector<std::string>& baseTokens)
{
    std::string s = mod;
    boost::trim(s);

    std::vector<std::string> seq;

    while (!s.empty())
    {
        bool matched = false;
        for (const auto& token : baseTokens)
        {
            if (s.rfind(token, 0) == 0) // s starts with token
            {
                seq.push_back(token);
                s.erase(0, token.size());
                matched = true;
                break;
            }
        }

        if (!matched)
        {
            std::stringstream errorMessage;
            errorMessage << "\nError, did not recognize: \"" << mod << "\"\n"
                         << "as an option for -m or --modification.\n"
                         << "Valid pattern is a sequence composed of: ";
            for (std::size_t i = 0; i < baseTokens.size(); ++i)
            {
                errorMessage << baseTokens[i];
                if (i + 1 < baseTokens.size()) errorMessage << ", ";
            }
            errorMessage << ".\n"
                         << "use -h or --help for help\n"
                         << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
            throw std::invalid_argument(errorMessage.str());
        }
    }

    return seq;
}

scdp_config::scdp_config()
{

}

scdp_config::scdp_config(int argc, char* argv[])
    : argc(argc), argv(argv)
{
    parseArguments(argc, argv);
    validate();
}

void scdp_config::parseArguments(int argc, char* argv[])
{
    std::vector<std::string> args(argv + 1, argv + argc);
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
            std::stringstream helpMessage;
            helpMessage << "This program ...." << std::endl << std::endl
                    << "-i\t" << "or --input, to specify an input file,\n"
								<<   "\t" << "expects a string containing the file name\n"
								<<   "\t" << "of the input file. \n"
			             "-k\t" << "or --kMinMax, to specify a kMin and kMax for a\n"
								<<   "\t" << "clustering, expects two integers larger than 1.\n"
			             "-r\t" << "or --ratio, to specify a minimum ratio of spaces\n"
								<<   "\t" << "that should be removed in every iteration when \n "
                                <<   "\t" << "utilizing the delete space(s) method.\n"
								<<	 "\t" << "Expects a number in the range [0,1].\n"
                                <<   "\t" << "The default is 0.1. This ratio is overuled if -a is set.\n"
			             "-a\t" << "or --amount, to specify a amount of spaces\n"
								<<   "\t" << "that should be removed in every iteration.\n"
								<<	 "\t" << "Expects an integer number above 1.\n"
                         "-m\t" << "or --modification, to specify which modification\n"
                                <<   "\t" << "will be applied: \'sd\' for the sd-modifications,\n"
                                <<   "\t" << "\'bp\' for the bp_modifications, and \'dl\' for the dl-modifications.\n"
                                <<   "\t" << "Any concatenation of these tokens is allowed, e.g. 'sdbp', 'bpdl', 'sdbpdl'.\n"
                                <<   "\t" << "The string is parsed as a sequence of the 2-letter codes sd, bp, dl.\n"
                                <<   "\t" << "In case of weighted option (i.e. -w): the sequence determine which objectives are \n"
                                <<   "\t" << "included in the alteration based on weighter performance.\n "
                                <<   "\t" << "In case of non-weighted option: the sequence determines the \n"
                                <<   "\t" << "order in which the building design is altered for that single objective.\n"
                         "-w\t" << "or --weights, to enable weighted objectives and optionally set weights.\n"
                                <<   "\t" << "If no numbers follow, default weights 1/3, 1/3, 1/3 are used for (sd, bp, dl).\n"
                                <<   "\t" << "If three numbers follow, they give the weights for (sd, bp, dl) in that order.\n"
                         "-n\t" << "or --nModifications, to specify the number of modifications\n"
								<<   "\t" << "that will be applied. Expects only a positive integer\n"
			             "-d\t" << "or --deleteMethod, to specify how spaces are deleted:\n"
								<<   "\t" << "\'cluster\' for cluster deleting spaces\n"
								<<   "\t" << "\'worst\' for deleting spaces based on worst performance\n"
                                <<   "\t" << "\'off\' no deleting of spaces. The default is \'worst\'.\n"
			             "-so\t" << "or --splitOption, to specify which spaces are split:\n"
								<<   "\t" << "\'longest\' for cluster deleting spaces\n"
								<<   "\t" << "\'performance\' for deleting spaces based on worst performance\n"
								<<   "\t" << "\'off\' no splitting of spaces. The default is \'longest\'.\n"
			             "-f\t" << "or --deleteFragments, to specify what to do with spaces which are not connected\n"
								<<	 "\t" << "as a full building. Option are \'true\' or \'false\'.\n"
                                <<   "\t" << "If \'true\' these smaller fragments will be removed. The default is \'false\'.\n"
			             "-p\t" << "or --performanceDivision, to specify per objective by which the performances \n"
								<<	 "\t" << "of the spaces should be divided. Option are \'floorArea\', \'surfaceArea\',\n"
								<<   "\t" << "\'volumeArea\' or \'nothing\'. To specify, use -p,\n"
                                <<   "\t" << "followed by a specifier: 'sd', 'bp', or 'dl', with directly\n"
                                <<   "\t" << "behind the specifier the division value. Example: '-p sd floorArea bp surfaceArea'.\n"
                                <<   "\t" << "The default is sd = \'floorArea\', bp = \'floorArea\', dl = \'nothing\'.\n"
			            "-ad\t" << "or --addMethod, to specify an adding spaces method, the choices are: \n"
								<<   "\t" << "\'elevation\' for adding an elevation to the building\n"
								<<   "\t" << "\'shiftfill\' for shifting a section of the \n"
								<<   "\t" << "building and filling it with spaces\n"
								<<   "\t" << "\'worstAdapt\' for using either add elevation or shiftfillplane method \n"
								<<   "\t" << "depending on where the worst performing spaces are located. \n"
								<<   "\t" << "this is determined by finding the centre of to worst spaces\n"
								<<   "\t" << "and the centre of the building. The vector centre centre building -> worst spaces\n"
								<<   "\t" << "has an inclination. If inclination is bellow x then addElevation is applied otherwise shiftFillPlane\n"
								<<   "\t" << "The value x needs to be set behind the statement \'worstAdapt\'. \n"
			             "-o\t" << "or --output, to specify the different outputs:\n"
								<<   "\t" << "\'result_line\' prints a line with the input\n"
								<<   "\t" << "parameters and the results; \'verbose\' prints\n"
								<<   "\t" << "a descriptive outline of the process;\n"
                                <<   "\t" << "\'memory_usage\' will log memory usage at key points during the optimization process;\n"
								<<   "\t" << "\'plot\' will make a data file (needed for plotting), \n"
								<< 	 "\t" << "visualize the plot to screen and the plot will be written to a png file\n"
								<<	 "\t" << "\'combine_data\' will make a data file where separate runs of this, \n"
								<< 	 "\t" << "executable can be copy pasted behind each other. \n"
								<< 	 "\t" << "For this, the maximum amount of iterations for all runs in the sequence should be set. \n"
								<< 	 "\t" << "For the formatting of the excel file. As such, \'combine_data\' key should be followed by \n"
								<< 	 "\t" << "by an unsigned integer value of the maximum iterations. \n"
								<< 	 "\t" << "This file can be loaded into excel to compare individual runs of this executable.\n"
								<<   "\t" << "\'ms_files\' will write the ms designs to files\n"
                                <<   "\t" << "\'msw_file\' will write the msw designs to files, so designed colored for performance\n"
                                <<   "\t" << "The bellow VisualWriteFile options do not work yet, but are implemented in the input arguments.\n"
                                <<   "\t" << "\'dlVisualWriteFile\' will write the dl visualization to files which can be visualized directly\n"
                                <<   "\t" << "\'sdVisualWriteFile\' will write the sd visualization to files which can be visualized directly \n"
                                <<   "\t" << "\'bpVisualWriteFile\' will write the bp visualization to files which can be visualized directly\n"
								<<   "\t" << "\'all\' will write all the designs to the specified format\n"
								<<   "\t" << "\'best\' (default) will only write the best all the designs to (does not work yet)\n"
								<<   "\t" << "the specified format. More than one option can be given,\n"
								<<   "\t" << "but \'all\' and \'best\' cannot be given simultaneously.\n"
                     << "-rd\t" << "or --resultDir, controls where results are written.\n "
                                <<   "\t" << "By default, results are written in a 'results' directory in the current working directory.\n"
                                <<   "\t" << "Usage 1 (backwards compatible): -rd <name>\n"
                                <<   "\t" << "    <name> is a sub-directory name (or 'shortStandard') that will be created inside 'results'.\n"
                                <<   "\t" << "    By default the sub-directory will consist of: \n"
                                <<   "\t" << "    inputFileName_modification_NrOfIterations_currentDateAndTime.\n"
                                <<   "\t" << "    If <name> is given directly behind -rd, then the sub-directory will be named with this string instead.\n"
                                <<   "\t" << "    Use argument 'shortStandard' to make a sub-directory with a shorter name: \n "
                                <<   "\t" << "    inputFileName_modification_NrOfIterations(_weightmodification in case of weighted run).\n"
                                <<   "\t" << "Usage 2: -rd <baseDir> <name>\n"
                                <<   "\t" << "    <baseDir> specifies the main results directory to create/use (e.g. './result', '/tmp/result').\n"
                                <<   "\t" << "    <name> is interpreted as in Usage 1 (sub-directory name or 'shortStandard').\n"
                                <<   "\t" << "Usage 3: -rd <baseDirWithPath>\n"
                                <<   "\t" << "    If the single argument contains '/' or '\\', it is interpreted as the base results directory;\n"
                                <<   "\t" << "    a standard sub-directory name will then be created inside this base directory (as in default behavior).\n"
			          << "-v\t" << "or --visualization, to specify which models are visualized: \n"
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
                                <<   "\t" << "\'rc\' for the \'RC Model\'.\n"
                                <<   "\t" << "\'dl\' for the \'Daylight model\'.\n"
                                <<   "\t" << "\'dlG\' for the \'Daylight model with grid visualization\'.\n"
                                <<   "\t" << "\'dlD\' for the \'Daylight model with daylight factor visualization\'.\n"
                                <<   "\t" << "\'dlI\' for the \'Daylight model with illuminance visualization\'.\n"
								<<   "\t" << "\'dlADF\' for the \'Daylight model with average daylight factor per-space visualization\'.\n"
								<<   "\t" << "\'dlNEN\' for the \'Daylight model with NEN score per-space visualization\'.\n"
    							<<   "\t" << "More than one can be specified.\n"
			         << "-is\t" << "or --iterationStrategy, to specify which iteration strategy to use: \n"
								<<   "\t" << "\'split_delete\' runs only split-delete iterations (default),\n"
								<<   "\t" << "\'modification\' runs only modification iterations,\n"
								<<   "\t" << "\'alternating\' alternates between split-delete and modification iterations,\n"
								<<   "\t" << "starting with split-delete. Only one strategy is possible.\n"
                    << "-sim\t" << "or --simulations, to specify which simulations to run: \n"
                                <<   "\t" << "\'sd\' for structural design simulation,\n"
                                <<   "\t" << "\'bp\' for building physics simulation,\n"
                                <<   "\t" << "\'dl\' for daylighting simulation.\n"
                                <<   "\t" << "More than one can be specified. Default is all (sd bp dl).\n"
                                <<   "\t" << "Example: -sim sd       (only structural design)\n"
                                <<   "\t" << "Example: -sim bp       (only building physics)\n"
                                <<   "\t" << "Example: -sim dl       (only daylighting)\n"
                                <<   "\t" << "Example: -sim sd bp   (sd and bp)\n"
                                <<   "\t" << "Example: -sim sd bp dl (all three)\n"
                    << "-sds\t" << "or --sdSettings, to specify the structural design settings file path.\n"
                                <<   "\t" << "Expects a string path; default is settings/sd_settings.txt.\n"
                    << "-bps\t" << "or --bpSettings, to specify the building physics settings file path.\n"
                                <<   "\t" << "Expects a string path; default is settings/bp_settings.txt.\n"
                    << "-dls\t" << "or --dlSettings, to specify the daylight settings file path.\n"
                                <<   "\t" << "Expects a string path; default is settings/dl_settings_A.txt.\n"
                             << std::endl;
            std::cout << helpMessage.str();
            return;
        }
        else if (*arg == "-i" || *arg == "--input")
        {
            if (++arg == args.end() || arg->operator[](0) == '-')
            {
                std::stringstream errorMessage;
                errorMessage << "\nError, expected the name of the input file after -i or --input\n"
                             << "Use -h or --help for help" << std::endl
                             << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
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
                errorMessage << "\nError, expected a value after -k or --kMinMax.\n"
                << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                throw std::invalid_argument(errorMessage.str());
            }

            kMin = bso::utilities::trim_and_cast_uint(*arg);

            if (++arg == args.end() || arg->operator[](0) == '-')
            {
                std::stringstream errorMessage;
                errorMessage << "\nError, expected a second value after -k or --kMinMax.\n"
                             << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                throw std::invalid_argument(errorMessage.str());
            }

            kMax = bso::utilities::trim_and_cast_uint(*arg);

            if (kMax <= kMin)
            {
                std::stringstream errorMessage;
                errorMessage << "\nError, the second value after -k or --kMinMax \n"
                             << "must be larger than the first.\n"
                             << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                throw std::invalid_argument(errorMessage.str());
            }

            if (++arg != args.end() && arg->operator[](0) != '-')
            {
                std::stringstream errorMessage;
                errorMessage << "\nError, can only specify two values after -k or --kMinMax.\n"
                             << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                throw std::invalid_argument(errorMessage.str());
            }

        }
        else if (*arg == "-r" || *arg == "--ratio")
        {
            if (++arg == args.end() || arg->operator[](0) == '-')
            {
                std::stringstream errorMessage;
                errorMessage << "\nError, expected a value after -r or --ratio.\n"
                             << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                throw std::invalid_argument(errorMessage.str());
            }

            spaceRemoveRatio = bso::utilities::trim_and_cast_double(*arg);

            if (spaceRemoveRatio <= 0.0 - 0.00001 || spaceRemoveRatio >= 1.0 + 0.00001)
            {
                std::stringstream errorMessage;
                errorMessage << "\nError, the value after -r or --ratio \n"
                             << "should be within range [0,1], instead it is: "
                             << spaceRemoveRatio << std::endl
                             << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                throw std::invalid_argument(errorMessage.str());
            }

            if (++arg != args.end() && arg->operator[](0) != '-')
            {
                std::stringstream errorMessage;
                errorMessage << "\nError, can only specify one value after -r or --ratio.\n"
                             << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                throw std::invalid_argument(errorMessage.str());
            }

            ratioSet = true;
        }
        else if (*arg == "-a" || *arg == "--amount")
        {
            if (++arg == args.end() || arg->operator[](0) == '-')
            {
                std::stringstream errorMessage;
                errorMessage << "\nError, expected a value after -a or --amount.\n"
                             << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                throw std::invalid_argument(errorMessage.str());
            }

            amountOfSpaceRemoval = bso::utilities::trim_and_cast_int(*arg);

            if (amountOfSpaceRemoval <= 0)
            {
                std::stringstream errorMessage;
                errorMessage << "\nError, the value after -a or --amount \n"
                             << "should be an integer number larger then 1, instead it is: "
                             << amountOfSpaceRemoval << std::endl
                             << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                throw std::invalid_argument(errorMessage.str());
            }

            if (++arg != args.end() && arg->operator[](0) != '-')
            {
                std::stringstream errorMessage;
                errorMessage << "\nError, can only specify one value after -a or --amount.\n"
                             << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                throw std::invalid_argument(errorMessage.str());
            }

            amountOfSpaceRemovalSet = true;
        }
        else if (*arg == "-m" || *arg == "--modification")
        {
            if (++arg == args.end() || arg->operator[](0) == '-')
            {
                std::stringstream errorMessage;
                errorMessage << "\nError, expected a modification argument to be\n"
                             << "specified after -m or --modification. Use -h or\n"
                             << "--help for help\n"
                             << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                throw std::invalid_argument(errorMessage.str());
            }
            modification = *arg;
            boost::trim(modification);
            // Validate and decode the modification string as a sequence of base tokens.
            // This allows arbitrary combinations like "sd", "bpdl", "sdbpdl", etc.
            // Any error will be thrown from parseModificationString.
            modificationSequence = parseModificationString(modification, kModBaseOptions);

            if (++arg != args.end() && arg->operator[](0) != '-')
            {
                std::stringstream errorMessage;
                errorMessage << "\nError, only one argument can be specified after\n"
                             << "-m or --modification. Use -h or -- help for help\n"
                             << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                throw std::invalid_argument(errorMessage.str());
            }
        }
        else if (*arg == "-w" || *arg == "--weights")
        {
            // Enable weighted objective; optionally override default weights.
            mWeightedModification = true;

            std::vector<double> weights;
            while (++arg != args.end() && arg->operator[](0) != '-')
            {
                try
                {
                    double w = bso::utilities::trim_and_cast_double(*arg);
                    weights.push_back(w);
                }
                catch (const std::exception& e)
                {
                    std::stringstream errorMessage;
                    errorMessage << "\nError, could not parse weight specified after -w or --weights.\n"
                                 << "Received error: " << e.what() << "\n"
                                 << "use -h or --help for help\n"
                                 << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                    throw std::invalid_argument(errorMessage.str());
                }
            }

            if (weights.size() != 3 && !weights.empty())
            {
                // If weights are empty, we will use equal weights, 
                // however, they can only be set when all the modification types are set. 
                // So end of this function will initialize the default.
                
                std::stringstream errorMessage;
                errorMessage << "\nError, expected either zero or three weights after -w or --weights,\n"
                                << "corresponding to (sd, bp, dl). Received " << weights.size() << " value(s).\n"
                                << "use -h or --help for help\n"
                                << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                throw std::invalid_argument(errorMessage.str());
            }
            mObjectiveWeights = weights;
        }
        else if (*arg == "-n" || *arg == "--nModifications")
        {
            if (++arg == args.end() || arg->operator[](0) == '-')
            {
                std::stringstream errorMessage;
                errorMessage << "\nError, expected a positive integer to be\n"
                             << "specified after -n or --nModifications. Use -h or\n"
                             << "--help for help\n"
                             << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
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
                             << "Use -h or -- help for help\n"
                             << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                throw std::invalid_argument(errorMessage.str());
            }

            if (++arg != args.end() && arg->operator[](0) != '-')
            {
                std::stringstream errorMessage;
                errorMessage << "\nError, only one argument can be specified after\n"
                             << "-n or --nModifications. Use -h or -- help for help\n"
                             << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                throw std::invalid_argument(errorMessage.str());
            }
        }
        else if (*arg == "-d" || *arg == "--deleteMethod")
        {
            while (++arg != args.end() && arg->operator[](0) != '-')
            {
                deleteMethod = *arg;
                boost::trim(deleteMethod);
                if (std::find(kDelOptions.begin(), kDelOptions.end(), deleteMethod) == kDelOptions.end())
                {
                    std::stringstream errorMessage;
                    errorMessage << "\nError, did not recognize: " << deleteMethod << " as an\n"
                                 << "argument for -d or --deleteMethod. Use -h or\n"
                                 << "--help for help\n"
                                 << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                    throw std::invalid_argument(errorMessage.str());
                }
            }
        }
        else if (*arg == "-so" || *arg == "--splitOption")
        {
            while (++arg != args.end() && arg->operator[](0) != '-')
            {
                splitMethod = *arg;
                boost::trim(splitMethod);
                if (std::find(kSplitOptions.begin(), kSplitOptions.end(), splitMethod) == kSplitOptions.end())
                {
                    std::stringstream errorMessage;
                    errorMessage << "\nError, did not recognize: " << splitMethod << " as an\n"
                                 << "argument for -so or --splitOption. Use -h or\n"
                                 << "--help for help\n"
                                 << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                    throw std::invalid_argument(errorMessage.str());
                }
            }
        }
        else if (*arg == "-f" || *arg == "--deleteFragments")
        {
            while (++arg != args.end() && arg->operator[](0) != '-')
            {
                std::string outSpec = *arg;
                boost::trim(outSpec);
                if (outSpec == "true") removeSmallerFragments = true;
                else if (outSpec == "false") removeSmallerFragments = false;
                else
                {
                    std::stringstream errorMessage;
                    errorMessage << "\nError, did not recognize: " << outSpec << " as an\n"
                                 << "argument for -f or --deleteFragments. Use -h or\n"
                                 << "--help for help\n"
                                 << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                    throw std::invalid_argument(errorMessage.str());
                }
            }
        }
        else if (*arg == "-p" || *arg == "--performanceDivision")
        {
            unsigned int argumentCount = 0;
            while (++arg != args.end() && arg->operator[](0) != '-')
            {
                std::string type = *arg;
                if(type != "sd" && type != "bp" && type != "dl")
                {
                    std::stringstream errorMessage;
                    errorMessage << "\nError, did not recognize: \"" << type << "\"\n"
                                << "as an specifier for -p or --performanceDivision.\n"
                                << "use -h or --help for help\n"
                                << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                    throw std::invalid_argument(errorMessage.str());
                }
                if (++arg != args.end() || arg->operator[](0) == '-')
                {
                    std::string performanceDivisionKey = *arg;
                    if (std::find(kPerOptions.begin(), kPerOptions.end(), performanceDivisionKey) == kPerOptions.end())
                    {
                        std::stringstream errorMessage;
                        errorMessage << "\nError, did not recognize: " << performanceDivisionKey << " as an\n"
                                    << "argument for -p or --performanceDivision. Use -h or\n"
                                    << "--help for help\n"
                                    << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                        throw std::invalid_argument(errorMessage.str());
                    }
                    performanceDivision[type] = performanceDivisionKey;
                }
                else
                {
                    std::stringstream errorMessage;
                    errorMessage << "\nError, expected a value behind the specifier sd, bp, or dl but got nothing or \n"
                                    << "there where not multiple arguments as was expected as an"
                                    << "argument for -p or --performanceDivision. Use -h or\n"
                                    << "--help for help\n"
                                    << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                    throw std::invalid_argument(errorMessage.str());
                }
                argumentCount++;
            }
            if(argumentCount <= 0)
            {
                std::stringstream errorMessage;
                errorMessage << "\nError, expected a specifier sd, bp, or dl but got nothing or \n"
                                << "there where not multiple arguments as was expected as an"
                                << "argument for -p or --performanceDivision. Use -h or\n"
                                << "--help for help\n"
                                << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                throw std::invalid_argument(errorMessage.str());
            }
        }
        else if (*arg == "-ad" || *arg == "--addMethod")
        {
            while (++arg != args.end() && arg->operator[](0) != '-')
            {
                addMethod = *arg;
                boost::trim(addMethod);
                if (std::find(kAddOptions.begin(), kAddOptions.end(), addMethod) != kAddOptions.end())
                {
                    if (addMethod == "worstAdapt")
                    {
                        if (++arg != args.end())
                        {
                            if (arg->operator[](0) == '-') {--arg;}
                            else
                            {
                                inclinationSwitch = bso::utilities::trim_and_cast_double(*arg);
                                if (++arg != args.end())
                                {
                                    if (arg->operator[](0) == '-') {--arg;}
                                    else
                                    {
                                        spaceModifyWorstRatio = bso::utilities::trim_and_cast_double(*arg);
                                    }
                                }
                            }
                        }
                        else
                        {
                            std::stringstream errorMessage;
                            errorMessage << "\nError, expected a value behind the worstAdapt argument but got nothing or \n"
                                         << "there where not multiple arguments as was expected as an"
                                         << "argument for -ad or --addMethod. Use -h or\n"
                                         << "--help for help\n"
                                         << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                            throw std::invalid_argument(errorMessage.str());
                        }
                    }
                }
                else
                {
                    std::stringstream errorMessage;
                    errorMessage << "\nError, did not recognize: " << addMethod << " as an\n"
                                 << "argument for -ad or --addMethod. Use -h or\n"
                                 << "--help for help\n"
                                 << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
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
                             << "--help for help\n"
                             << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                throw std::invalid_argument(errorMessage.str());
            }
            try
            {
                rngSeed = bso::utilities::trim_and_cast_uint(*arg);
            }
            catch (std::exception& e)
            {
                std::stringstream errorMessage;
                errorMessage << "\nError, could not parse argument specified after\n"
                             << "-s or --seed. received the following error:\n"
                             << e.what()
                             << "Use -h or -- help for help\n"
                             << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                throw std::invalid_argument(errorMessage.str());
            }

            if (++arg != args.end() && arg->operator[](0) != '-')
            {
                std::stringstream errorMessage;
                errorMessage << "\nError, only one argument can be specified after\n"
                             << "-s or --seed. Use -h or -- help for help\n"
                             << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                throw std::invalid_argument(errorMessage.str());
            }
        }
        else if (*arg == "-o" || *arg == "--output")
        {
            // Parse one or more output specifiers following -o/--output. We avoid
            // incrementing 'arg' to args.end() inside the loop body to prevent
            // undefined behaviour when the while-condition pre-increments it.
            while (true)
            {
                ++arg;
                if (arg == args.end() || arg->operator[](0) == '-')
                {
                    break; // no more -o options
                }

                std::string outSpec = *arg;
                boost::trim(outSpec);
                if (outSpec == "result_line") resultLine = true;
                else if (outSpec == "verbose") verbose = true;
                else if (outSpec == "memory_usage") logMemoryUsage = true;
                else if (outSpec == "plot")
                {
                    plot = true;
                    resultDir = true;

                    // Optional extra flag immediately after 'plot' to control
                    // visualization: 'true' or 'false'. Use a look-ahead iterator
                    // so that we don't move 'arg' past args.end().
                    auto next = std::next(arg);
                    if (next != args.end())
                    {
                        std::string visSpec = *next;
                        boost::trim(visSpec);
                        if (visSpec == "true")
                        {
                            visualizePlot = true;
                            arg = next; // consume the extra token
                        }
                        else if (visSpec == "false")
                        {
                            visualizePlot = false;
                            arg = next; // consume the extra token
                        }
                        // Any other token is left for the next loop iteration
                    }
                    continue;
                }
                else if (outSpec == "ms_files")
                {
                    msFiles = true;
                    resultDir = true;
                }
                else if (outSpec == "msw_file")
                {
                    // When requested, write a colour-settings file for the
                    // msw (Movable Sizable Worst) performance visualization.
                    mswVisualWriteFile = true;
                }
                else if (outSpec == "all") outputAll = true;
                else if (outSpec == "best") outputBest = true;
                else if (outSpec == "combine_data")
                {
                    combineData = true;
                    // Expect an unsigned integer for maxIterationsInSequence.
                    auto next = std::next(arg);
                    if (next == args.end() || next->operator[](0) == '-')
                    {
                        std::stringstream errorMessage;
                        errorMessage << "\nError, expected an unsigned integer value to be defined to \n"
                                     << "state the maximum nr. of iteration in the sequence. \n"
                                     << "argument for -o or --output. Use -h or\n"
                                     << "--help for help\n"
                                     << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                        throw std::invalid_argument(errorMessage.str());
                    }
                    try
                    {
                        maxIterationsInSequence = bso::utilities::trim_and_cast_int(*next);
                        arg = next; // consume the integer token
                    }
                    catch (const std::exception& e)
                    {
                        std::stringstream errorMessage;
                        errorMessage << "\nError, expected an unsigned integer value to be defined to \n"
                                     << "state the maximum nr. of iteration in the sequence. \n"
                                     << "however, during trim anc cast integer got the following error: "
                                     << e.what()
                                     << "argument for -o or --output. Use -h or\n"
                                     << "--help for help\n"
                                     << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                        throw std::invalid_argument(errorMessage.str());
                    }
                    continue;
                }
                else if (outSpec == "dlVisualWriteFile") 
                {
                    dlVisualWriteFile = true;
                }
                else if (outSpec == "sdVisualWriteFile") 
                {
                    sdVisualWriteFile = true;
                }
                else if (outSpec == "bpVisualWriteFile") 
                {
                    bpVisualWriteFile = true;
                }
                else
                {
                    std::stringstream errorMessage;
                    errorMessage << "\nError, did not recognize: " << outSpec << " as an\n"
                                 << "argument for -o or --output. Use -h or\n"
                                 << "--help for help\n"
                                 << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                    throw std::invalid_argument(errorMessage.str());
                }
            }
        }
        else if (*arg == "-rd" || *arg == "--resultDir")
        {
            if (++arg == args.end() || arg->operator[](0) == '-')
            {
                std::stringstream errorMessage;
                errorMessage << "\nError, expected at least one string after -rd or --resultDir to specify\n"
                             << " a name for the sub-directory and/or the base results directory.\n"
                             << "Use -h or --help for help\n"
                             << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                throw std::invalid_argument(errorMessage.str()); 
            }

            // At this point *arg is the first token after -rd.
            resultDir = true;
            std::string firstToken = *arg;
            boost::trim(firstToken);

            // Look ahead: if there is a second non-flag token, interpret the
            // first as the base directory and the second as the sub-directory
            // name or keyword (e.g. "shortStandard").
            auto next = arg;
            ++next;
            if (next != args.end() && next->operator[](0) != '-')
            {
                // Two-argument form: -rd <baseDir> <subDirOrKeyword>
                resultBaseDir = firstToken;
                setSubDirName = *next;
                boost::trim(resultBaseDir);
                boost::trim(setSubDirName);
                arg = next + 1;
            }
            else
            {
                // Single-argument form. To remain backwards compatible while still
                // allowing the user to override the base directory, we interpret
                // tokens containing a path separator as a base directory, and other
                // tokens (including 'shortStandard') as a pure sub-directory name.
                if (firstToken.find('/') != std::string::npos || firstToken.find('\\') != std::string::npos)
                {
                    // Treat as base directory; keep subDirName empty so that the
                    // standard naming scheme is used inside this base directory.
                    resultBaseDir = firstToken;
                    boost::trim(resultBaseDir);
                }
                else
                {
                    // Backwards compatible: only sub-directory name or keyword.
                    setSubDirName = firstToken;
                    boost::trim(setSubDirName);
                }
                ++arg;
            }
        }
        else if (*arg == "-pl" || *arg == "--parallel")
        {
            parallel = true;
            ++arg;
            continue;
        }        
        else if (*arg == "-v" || *arg == "--visualize")
        {
            while (++arg != args.end() && arg->operator[](0) != '-')
            {
                std::string visSpec = *arg;
                boost::trim(visSpec);
                if (std::find(kVisOptions.begin(), kVisOptions.end(), visSpec) == kVisOptions.end())
                {
                    std::stringstream errorMessage;
                    errorMessage << "\nError, did not recognize: \"" << visSpec << "\"\n"
                                 << "as an option for -v or --visualization as a specification\n"
                                 << "of a model to visualize, use -h or --help for help\n"
                                 << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                    throw std::invalid_argument(errorMessage.str());
                }
                visualizations.push_back(visSpec);
            }
            if (visualizations.empty())
            {
                std::stringstream errorMessage;
                errorMessage << "\nError, expected at least one string after\n"
                             << "-v or --visualize, received nothing. Use -h or\n"
                             << "--help for help\n"
                             << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                throw std::invalid_argument(errorMessage.str());
            }
        }
        else if (*arg == "-is" || *arg == "--iterationStrategy")
        {
            if (++arg == args.end() || arg->operator[](0) == '-')
            {
                std::stringstream errorMessage;
                errorMessage << "\nError, expected an iteration strategy after -is or --iterationStrategy.\n"
                             << "Expected one of: split_delete, modification, alternating\n"
                             << "Use -h or --help for help\n"
                             << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                throw std::invalid_argument(errorMessage.str());
            }
            iterationStrategy = *arg;
            boost::trim(iterationStrategy);
            if (std::find(kIterationStrategyOptions.begin(), kIterationStrategyOptions.end(), iterationStrategy) == kIterationStrategyOptions.end())
            {
                std::stringstream errorMessage;
                errorMessage << "\nError, did not recognize: \"" << iterationStrategy << "\"\n"
                             << "as an option for -is or --iterationStrategy.\n"
                             << "Expected one of: split_delete, modification, alternating\n"
                             << "Use -h or --help for help\n"
                             << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                throw std::invalid_argument(errorMessage.str());
            }
            ++arg;
        }
        else if (*arg == "-sim" || *arg == "--simulations")
        {
            // Reset to false when user explicitly specifies simulations
            runSDSimulation = false;
            runBPSimulation = false;
            runDLSimulation = false;
            
            bool foundSim = false;
            while (++arg != args.end() && arg->operator[](0) != '-')
            {
                std::string simSpec = *arg;
                boost::trim(simSpec);
                if (std::find(kSimOptions.begin(), kSimOptions.end(), simSpec) == kSimOptions.end())
                {
                    std::stringstream errorMessage;
                    errorMessage << "\nError, did not recognize: \"" << simSpec << "\"\n"
                                 << "as an option for -sim or --simulations as a specification\n"
                                 << "of a simulation to run. Valid options are: sd, bp, dl\n"
                                 << "Use -h or --help for help\n"
                                 << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                    throw std::invalid_argument(errorMessage.str());
                }
                
                // Set corresponding boolean flag
                if (simSpec == "sd") runSDSimulation = true;
                if (simSpec == "bp") runBPSimulation = true;
                if (simSpec == "dl") runDLSimulation = true;
                foundSim = true;
            }
            
            if (!foundSim)
            {
                std::stringstream errorMessage;
                errorMessage << "\nError, expected at least one simulation type after\n"
                             << "-sim or --simulations. Valid options are: sd, bp, dl\n"
                             << "Use -h or --help for help\n"
                             << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                throw std::invalid_argument(errorMessage.str());
            }
        }
        else if (*arg == "-sds" || *arg == "--sdSettings")
        {
            if (++arg == args.end() || arg->operator[](0) == '-')
            {
                std::stringstream errorMessage;
                errorMessage << "\nError, expected a path after -sds or --sdSettings.\n"
                             << "Use -h or --help for help\n"
                             << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                throw std::invalid_argument(errorMessage.str());
            }
            sd_settings_file = *arg;
            boost::trim(sd_settings_file);
            ++arg;
        }
        else if (*arg == "-bps" || *arg == "--bpSettings")
        {
            if (++arg == args.end() || arg->operator[](0) == '-')
            {
                std::stringstream errorMessage;
                errorMessage << "\nError, expected a path after -bps or --bpSettings.\n"
                             << "Use -h or --help for help\n"
                             << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                throw std::invalid_argument(errorMessage.str());
            }
            bp_settings_file = *arg;
            boost::trim(bp_settings_file);
            ++arg;
        }
        else if (*arg == "-dls" || *arg == "--dlSettings")
        {
            if (++arg == args.end() || arg->operator[](0) == '-')
            {
                std::stringstream errorMessage;
                errorMessage << "\nError, expected a path after -dls or --dlSettings.\n"
                             << "Use -h or --help for help\n"
                             << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
                throw std::invalid_argument(errorMessage.str());
            }
            dl_settings_file = *arg;
            boost::trim(dl_settings_file);
            ++arg;
        }
        else
        {
            std::stringstream errorMessage;
            errorMessage << "\nError, did not recognize argument: " << *arg << std::endl
                            << "Use -h or --help for help\n"
                            << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
            throw std::invalid_argument(errorMessage.str());
        }
    }

    // If weighted simulation and no weight define, set the default weights based on the modification sequence.
    if(mWeightedModification && mObjectiveWeights.empty())
    {
        // Set default based on included types sd,bp,sd in -m:
        if(modificationSequence.size() == 3)
        {
            double oneThird = 1.0 / 3.0;
            mObjectiveWeights = {oneThird,oneThird,oneThird};
        }
        else if(modificationSequence.size() == 2)
        {
            mObjectiveWeights = {0.0,0.0,0.0};
            for(const auto &mod : modificationSequence)
            {
                if(mod == "sd"){mObjectiveWeights[0] = 0.5;}
                else if(mod == "bp"){mObjectiveWeights[1] = 0.5;}
                else if(mod == "dl"){mObjectiveWeights[2] = 0.5;}
            }
        }
        else
        {
            std::stringstream errorMessage;
            errorMessage << "\nError, the -w weighted option can only be used with more then one objective in the -m options.\n"
                            << "Meaning that more then one objective is taken into acount. "<< std::endl
                            << "Use -h or --help for help\n"
                            << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
            throw std::invalid_argument(errorMessage.str());
        }
    }

    validate();
}

void scdp_config::validate() const
{
    if (inputFile.empty())
    {
        std::stringstream errorMessage;
        errorMessage << "\nError, expected an input file or line to be specified.\n"
                     << "use -h or --help for help\n"
                     << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
        throw std::invalid_argument(errorMessage.str());
    }
    if (modification.empty())
    {
        std::stringstream errorMessage;
        errorMessage << "\nError, expected an modification to be specified.\n"
                     << "use -h or --help for help\n"
                     << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
        throw std::invalid_argument(errorMessage.str());
    }
    if ((msFiles || resultLine) && (outputAll && outputBest))
    {
        std::stringstream errorMessage;
        errorMessage << "Cannot specify both 'all'  and 'best' for output.\n"
                     << "use -h or --help for help\n"
                     << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
        throw std::invalid_argument(errorMessage.str());
    }
    if ((msFiles || resultLine) && (!outputAll && !outputBest))
    {
        std::stringstream errorMessage;
        errorMessage << "Need to specify either 'all' or 'best' for output when using msFiles or resultLine.\n"
                     << "use -h or --help for help\n"
                     << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
        throw std::invalid_argument(errorMessage.str());
    }
    if(amountOfSpaceRemovalSet && ratioSet)
    {
        std::stringstream errorMessage;
        errorMessage << "Cannot specify both a ratio and an amount for space removal.\n"
                     << "use -h or --help for help\n"
                     << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
        throw std::invalid_argument(errorMessage.str());
    }
    
    // Validate that simulation flags match the modification sequence
    // (any combination of sd, bp, dl). If the modificationSequence has
    // already been filled during argument parsing, reuse it; otherwise
    // parse it here (e.g. for default configurations).
    std::vector<std::string> modSequence = modificationSequence.empty()
        ? parseModificationString(modification, kModBaseOptions)
        : modificationSequence;

    bool needsSD = std::find(modSequence.begin(), modSequence.end(), "sd") != modSequence.end();
    bool needsBP = std::find(modSequence.begin(), modSequence.end(), "bp") != modSequence.end();
    bool needsDL = std::find(modSequence.begin(), modSequence.end(), "dl") != modSequence.end();

    if (needsSD && !runSDSimulation)
    {
        std::stringstream errorMessage;
        errorMessage << "\nError, modification requires SD simulation but SD is disabled.\n"
                     << "Either include 'sd' in --sim or change modification type.\n"
                     << "use -h or --help for help\n"
                     << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
        throw std::invalid_argument(errorMessage.str());
    }
    if (needsBP && !runBPSimulation)
    {
        std::stringstream errorMessage;
        errorMessage << "\nError, modification requires BP simulation but BP is disabled.\n"
                     << "Either include 'bp' in --sim or change modification type.\n"
                     << "use -h or --help for help\n"
                     << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
        throw std::invalid_argument(errorMessage.str());
    }
    if (needsDL && !runDLSimulation)
    {
        std::stringstream errorMessage;
        errorMessage << "\nError, modification requires DL (daylighting) simulation but DL is disabled.\n"
                     << "Either include 'dl' in --sim or change modification type.\n"
                     << "use -h or --help for help\n"
                     << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
        throw std::invalid_argument(errorMessage.str());
    }
    if (dlVisualWriteFile && !runDLSimulation)
    {
        std::stringstream errorMessage;
        errorMessage << "\nError, cannot write DL visualization file when DL simulation is disabled.\n"
                     << "Either include 'dl' in --sim or remove the 'dlVisualWriteFile' option from -o/--output.\n"
                     << "use -h or --help for help\n"
                     << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
        throw std::invalid_argument(errorMessage.str());
    }
    if (sdVisualWriteFile && !runSDSimulation)
    {
        std::stringstream errorMessage;
        errorMessage << "\nError, cannot write SD visualization file when SD simulation is disabled.\n"
                     << "Either include 'sd' in --sim or remove the 'sdVisualWriteFile' option from -o/--output.\n"
                     << "use -h or --help for help\n"
                     << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
        throw std::invalid_argument(errorMessage.str());    
    }
    if (bpVisualWriteFile && !runBPSimulation)
    {
        std::stringstream errorMessage;
        errorMessage << "\nError, cannot write BP visualization file when BP simulation is disabled.\n"
                     << "Either include 'bp' in --sim or remove the 'bpVisualWriteFile' option from -o/--output.\n"
                     << "use -h or --help for help\n"
                     << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
        throw std::invalid_argument(errorMessage.str());    
    }
    if (mswVisualWriteFile && !runDLSimulation && !runSDSimulation && !runBPSimulation)
    {
        std::stringstream errorMessage;
        errorMessage << "\nError, cannot write msw visualization file when all simulations are disabled.\n"
                     << "Either include 'sd', 'bp', or 'dl' in --sim or remove the 'mswVisualWriteFile' option from -o/--output.\n"
                     << "use -h or --help for help\n"
                     << "(bso/optimizers/scdp/scdp_config.cpp)." << std::endl;
        throw std::invalid_argument(errorMessage.str());    
    }
} // 

std::string scdp_config::objectiveWeightsToString(const std::vector<double>& weights) const
{
	std::ostringstream stream;
	for (std::size_t i = 0; i < weights.size(); ++i)
	{
		if (i > 0)
		{
			stream << "-";
		}
		stream << weights[i];
	}
	return stream.str();
}

}}} // namespace bso::optimizers::scdp
