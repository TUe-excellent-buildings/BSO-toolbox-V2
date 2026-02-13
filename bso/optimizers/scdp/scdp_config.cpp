#include <bso/optimizers/scdp/scdp_config.hpp>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <sstream>
#include <stdexcept>

#include <boost/algorithm/string.hpp>

#include <bso/utilities/trim_and_cast.hpp>

namespace bso { namespace optimizers { namespace scdp {

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
								<<   "\t" << "will be applied: \'sd\' for the sd-modifications;\n"
								<<   "\t" << "\'bp\' for the bp_modifications; \'weighted\' for sd and bp\n" 
								<< 	 "\t" << " making half the performance value per space; \'sdbp\' to alternate\n"
								<<   "\t" << "between sd and bp, starting with sd; \'bpsd\', dito\n"
								<<   "\t" << "but now starting with bp. Only one option is possible.\n"
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
			             "-p\t" << "or --performanceDivision, to specify by which the performances \n"
								<<	 "\t" << "of the spaces should be divided. Option are \'floorArea\', \'surfaceArea\',\n"
								<<   "\t" << "\'volumeArea\' or \'nothing\'. The default is \'floorArea\'.\n"
			             "-av\t"<< "or --averageDivision, to split all the spaces towards the smallest rib length in the building, \n"
								<< 	 "\t" << "without making any rib smaller then the smallest one. \n"
                                <<   "\t" << "Either enter \'true\' or \'false\'. The default is \'false\'.\n"
			            "-ad\t" << "or --addMethod, to specify an adding spaces method, the choices are: \n"
								<<   "\t" << "\'elevation\' for adding an elevation to the building\n"
								<<   "\t" << "\'shiftfill\' for shifting a section of the \n"
								<<   "\t" << "building and filling it with spaces\n"
								<<   "\t" << "\'worstAdapt\' for using either add elevation or shiftfillplane method \n"
								<<   "\t" << "depending on where the worst performing spaces are located. \n"
								<<   "\t" << "this is determined by finding the centre of to worst spaces\n"
								<<   "\t" << "and the centre of the building. The vector centre worst spaces -> centre building\n"
								<<   "\t" << "has an inclination. If inclination is bellow x then addElevation is applied otherwise shiftFillPlane\n"
								<<   "\t" << "The value x needs to be set behind the statement \'worstAdapt\'. \n"
			             "-o\t" << "or --output, to specify the different outputs:\n"
								<<   "\t" << "\'result_line\' prints a line with the input\n"
								<<   "\t" << "parameters and the results; \'verbose\' prints\n"
								<<   "\t" << "a descriptive outline of the process;\n"
								<<   "\t" << "\'plot\' will make a data file (needed for plotting), \n"
								<< 	 "\t" << "visualize the plot to screen and the plot will be written to a png file\n"
								<<	 "\t" << "\'combine_data\' will make a data file where separate runs of this, \n"
								<< 	 "\t" << "executable can be copy pasted behind each other. \n"
								<< 	 "\t" << "For this, the maximum amount of iterations for all runs in the sequence should be set. \n"
								<< 	 "\t" << "For the formatting of the excel file. As such, \'combine_data\' key should be followed by \n"
								<< 	 "\t" << "by an unsigned integer value of the maximum iterations. \n"
								<< 	 "\t" << "This file can be loaded into excel to compare individual runs of this executable.\n"
								<<   "\t" << "\'ms_files\' will write the ms designs to files\n"
								<<   "\t" << "\'all\' will write all the designs to the specified format\n"
								<<   "\t" << "\'best\' (default) will only write the best all the designs to\n"
								<<   "\t" << "the specified format. More than one option can be given,\n"
								<<   "\t" << "but \'all\' and \'best\' cannot be given simultaneously.\n"
                        << "-rd\t" << "or --resultDir, can be used to specify a string for a sub-directory name in directory results, \n "
                                <<   "\t" << "i.e. where the results will be stored.\n"
								<<   "\t" << "if name is \'standard\' then the sub-directory will consist of: \n"
								<< 	 "\t" << "inputFileName_modification_NrOfIterations_currentDateAndTime.\n"
								<<   "\t" << "The default for all outputs is in the current working directory.\n"
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
    							<<   "\t" << "More than one can be specified.\n"
			             << "-is\t" << "or --iterationStrategy, to specify which iteration strategy to use: \n"
								<<   "\t" << "\'split_delete\' runs only split-delete iterations (default),\n"
								<<   "\t" << "\'modification\' runs only modification iterations,\n"
								<<   "\t" << "\'alternating\' alternates between split-delete and modification iterations,\n"
								<<   "\t" << "starting with split-delete. Only one strategy is possible.\n"
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

            if (spaceRemoveRatio <= 0.0 - 0.00001 || spaceRemoveRatio >= 1.0 + 0.00001)
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

            ratioSet = true;
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

            if (amountOfSpaceRemoval <= 0)
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

            amountOfSpaceRemovalSet = true;
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
            if (std::find(kModOptions.begin(), kModOptions.end(), modification) == kModOptions.end())
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
                deleteMethod = *arg;
                boost::trim(deleteMethod);
                if (std::find(kDelOptions.begin(), kDelOptions.end(), deleteMethod) == kDelOptions.end())
                {
                    std::stringstream errorMessage;
                    errorMessage << "\nError, did not recognize: " << deleteMethod << " as an\n"
                                 << "argument for -d or --deleteMethod. Use -h or\n"
                                 << "--help for help\n";
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
                                 << "--help for help\n";
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
                                 << "--help for help\n";
                    throw std::invalid_argument(errorMessage.str());
                }
            }
        }
        else if (*arg == "-p" || *arg == "--performanceDivision")
        {
            if (++arg == args.end() || arg->operator[](0) == '-')
            {
                std::stringstream errorMessage;
                errorMessage << "\nError, expected a space performance division argument to be\n"
                             << "specified after -p or --performanceDivision. Use -h or\n"
                             << "--help for help\n";
                throw std::invalid_argument(errorMessage.str());
            }
            performanceDivision = *arg;
            boost::trim(performanceDivision);
            if (std::find(kPerOptions.begin(), kPerOptions.end(), performanceDivision) == kPerOptions.end())
            {
                std::stringstream errorMessage;
                errorMessage << "\nError, did not recognize: \"" << performanceDivision << "\"\n"
                             << "as an option for -p or --performanceDivision.\n"
                             << "use -h or --help for help\n";
                throw std::invalid_argument(errorMessage.str());
            }
            if (++arg != args.end() && arg->operator[](0) != '-')
            {
                std::stringstream errorMessage;
                errorMessage << "\nError, only one argument can be specified after\n"
                             << "-p or --performanceDivision. Use -h or -- help for help\n";
                throw std::invalid_argument(errorMessage.str());
            }
        }
        else if (*arg == "-av" || *arg == "--averageDivision")
        {
            while (++arg != args.end() && arg->operator[](0) != '-')
            {
                std::string outSpec = *arg;
                boost::trim(outSpec);
                if (outSpec == "true") averageSpace = true;
                else if (outSpec == "false") averageSpace = false;
                else
                {
                    std::stringstream errorMessage;
                    errorMessage << "\nError, did not recognize: " << outSpec << " as an\n"
                                 << "argument for -av or --averageDivision. Use -h or\n"
                                 << "--help for help\n";
                    throw std::invalid_argument(errorMessage.str());
                }
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
                                         << "--help for help\n";
                            throw std::invalid_argument(errorMessage.str());
                        }
                    }
                }
                else
                {
                    std::stringstream errorMessage;
                    errorMessage << "\nError, did not recognize: " << addMethod << " as an\n"
                                 << "argument for -ad or --addMethod. Use -h or\n"
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
                rngSeed = bso::utilities::trim_and_cast_uint(*arg);
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
                if (outSpec == "result_line") resultLine = true;
                else if (outSpec == "verbose") verbose = true;
                else if (outSpec == "plot")
                {
                    plot = true;
                    resultDir = true;
                    if (++arg != args.end())
                    {
                        outSpec = *arg;
                        if (outSpec == "true") {visualizePlot = true;}
                        else if (outSpec == "false") {visualizePlot = false;}
                        else {--arg;}
                    }
                    continue;
                }
                else if (outSpec == "ms_files")
                {
                    msFiles = true;
                    resultDir = true;
                }
                else if (outSpec == "all") outputAll = true;
                else if (outSpec == "best") outputBest = true;
                else if (outSpec == "combine_data")
                {
                    combineData = true;
                    if (++arg != args.end())
                    {
                        if (arg->operator[](0) == '-')
                        {
                            std::stringstream errorMessage;
                            errorMessage << "\nError, expected an unsigned integer value to be defined to \n"
                                         << "state the maximum nr. of iteration in the sequence. \n"
                                         << "argument for -o or --output. Use -h or\n"
                                         << "--help for help\n";
                            throw std::invalid_argument(errorMessage.str());
                        }
                        try
                        {
                            maxIterationsInSequence = bso::utilities::trim_and_cast_int(*arg);
                        }
                        catch (const std::exception& e)
                        {
                            std::stringstream errorMessage;
                            errorMessage << "\nError, expected an unsigned integer value to be defined to \n"
                                         << "state the maximum nr. of iteration in the sequence. \n"
                                         << "however, during trim anc cast integer got the following error: "
                                         << e.what()
                                         << "argument for -o or --output. Use -h or\n"
                                         << "--help for help\n";
                            throw std::invalid_argument(errorMessage.str());
                        }
                    }
                    continue;
                }
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
        else if (*arg == "-rd" || *arg == "--resultDir")
		{
			if (++arg == args.end() || arg->operator[](0) == '-')
			{
				std::stringstream errorMessage;
                errorMessage << "\nError, expected a string after -rd or --resultDir to specify\n"
                             << " a name for the sub-directory in directory results.\n"
                             << "Use -h or --help for help\n";
                throw std::invalid_argument(errorMessage.str()); 
			}
			else
			{
				resultDir = true;
				setSubDirName = *arg;
				boost::trim(setSubDirName);
				++arg;
			}
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
                                 << "of a model to visualize, use -h or --help for help\n";
                    throw std::invalid_argument(errorMessage.str());
                }
                visualizations.push_back(visSpec);
            }
            if (visualizations.empty())
            {
                std::stringstream errorMessage;
                errorMessage << "\nError, expected at least one string after\n"
                             << "-v or --visualize, received nothing. Use -h or\n"
                             << "--help for help\n";
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
                             << "Use -h or --help for help\n";
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
                             << "Use -h or --help for help\n";
                throw std::invalid_argument(errorMessage.str());
            }
            ++arg;
        }
        else
        {
            std::stringstream errorMessage;
            errorMessage << "\nError, did not recognize argument: " << *arg << std::endl;
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
                     << "use -h or --help for help\n";
        throw std::invalid_argument(errorMessage.str());
    }
    if (modification.empty())
    {
        std::stringstream errorMessage;
        errorMessage << "\nError, expected an modification to be specified.\n"
                     << "use -h or --help for help\n";
        throw std::invalid_argument(errorMessage.str());
    }
    if ((msFiles || resultLine) && (outputAll && outputBest))
    {
        std::stringstream errorMessage;
        errorMessage << "Cannot specify both 'all'  and 'best' for output.\n"
                     << "use -h or --help for help\n";
        throw std::invalid_argument(errorMessage.str());
    }
    if ((msFiles || resultLine) && (!outputAll && !outputBest))
    {
        std::stringstream errorMessage;
        errorMessage << "Need to specify either 'all' or 'best' for output when using msFiles or resultLine.\n"
                     << "use -h or --help for help\n";
        throw std::invalid_argument(errorMessage.str());
    }
    if(amountOfSpaceRemovalSet && ratioSet)
    {
        std::stringstream errorMessage;
        errorMessage << "Cannot specify both a ratio and an amount for space removal.\n"
                     << "use -h or --help for help\n";
        throw std::invalid_argument(errorMessage.str());
    }
}

}}} // namespace bso::optimizers::scdp
