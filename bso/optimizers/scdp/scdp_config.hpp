#ifndef SCDP_CONFIG_HPP
#define SCDP_CONFIG_HPP

#include <string>
#include <vector>
#include <chrono>

namespace bso { namespace optimizers { namespace scdp {

class scdp_config
{
private:
    // Store argc, argv to know which arguments were used to generate the below variables,
    // and to initialize the visualization if needed.
    int argc;
    char** argv;

    // Base options
    std::string inputFile;
    std::string designName;
    unsigned int iterations = 0;
    
    // Modification according to which performance, see kModOptions for options
    std::string modification;
    std::vector<double> objectiveWeights = {0.5, 0.5};

    // Adaptation options
    std::string performanceDivision = "floorArea";
    std::string deleteMethod = "worst";
    std::string splitMethod = "longest";
    std::string addMethod = "worstAdapt";
    std::string iterationStrategy = "split_delete";

    // Simulation options - which simulations to run (boolean flags for performance)
    bool runSDSimulation = true;  // default: run structural design
    bool runBPSimulation = true;  // default: run building physics

    // extra building modification options
    bool removeSmallerFragments = false;
    bool averageSpace = false;

    // Parameters for delAdapt option of kAddOptions
    double inclinationSwitch = 20.0;

    // Parameters for definition of amount of space removal for any kDelOptions
    double spaceRemoveRatio = 0.1;
    bool ratioSet = false;
    int amountOfSpaceRemoval = 0;
    bool amountOfSpaceRemovalSet = false;
    double spaceModifyWorstRatio = 0.1;

    // Parameters for clustering when using the kDelOptions 'cluster'
    unsigned int kMin = 2;
    unsigned int kMax = 10;
    unsigned int rngSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count();

    // Output options
    bool resultLine = false;
    bool msFiles = false;
    bool plot = false;
    bool visualizePlot = false;
    bool verbose = false;
    bool outputAll = false;
    bool outputBest = false;
    bool resultDir = false;
    bool combineData = false;
    unsigned int maxIterationsInSequence = 0; // in case a combined data file is made, this should be set the maximum nr. of iterations.
    std::vector<std::string> visualizations; // see kVisOptions for options
    std::string setSubDirName = "";

    // Define the options for the different parameters, these are used for validation and parsing of program arguments.
    const std::vector<std::string> kVisOptions = {"ms", "msc", "msw", "tp", "tr", "de", "bo", "sd", "fe", "bp"};
    const std::vector<std::string> kModOptions = {"bp", "sd", "sdbp", "bpsd", "weighted"};
    const std::vector<std::string> kDelOptions = {"worst", "cluster", "off"};
    const std::vector<std::string> kSplitOptions = {"longest", "performance", "off"};
    const std::vector<std::string> kAddOptions = {"elevation", "shiftfill", "worstAdapt"};
    const std::vector<std::string> kPerOptions = {"floorArea", "surfaceArea", "volumeArea", "nothing"};
    const std::vector<std::string> kIterationStrategyOptions = {"split_delete", "modification", "alternating"};
    const std::vector<std::string> kSimOptions = {"sd", "bp"};

public:
    scdp_config(); // empty_constructor
    scdp_config(int argc, char* argv[]);

    void parseArguments(int argc, char* argv[]);
    void validate() const;

    // Getters for all parameters, these are used to access the parameters 
    // in the scdp_optimizer class which are only allowed to be set in the 
    // constructor of the scdp_config class. This is done to ensure that 
    // the parameters are not accidentally changed during the optimization process.
    int getArgc() const { return argc; }
    char** getArgv() const { return argv; }
    const std::string& getInputFile() const { return inputFile; }
    const std::string& getDesignName() const { return designName; }
    unsigned int getIterations() const { return iterations; }
    const std::string& getDeleteMethod() const { return deleteMethod; }
    const std::string& getSplitMethod() const { return splitMethod; }
    const std::string& getModification() const { return modification; }
    const std::vector<double>& getObjectiveWeights() const { return objectiveWeights; }
    const std::string& getAddMethod() const { return addMethod; }
    const std::string& getIterationStrategy() const { return iterationStrategy; }
    const std::string& getPerformanceDivision() const { return performanceDivision; }
    bool getRemoveSmallerFragments() const { return removeSmallerFragments; }
    bool getAverageSpace() const { return averageSpace; }
    double getInclinationSwitch() const { return inclinationSwitch; }
    double getSpaceRemoveRatio() const { return spaceRemoveRatio; }
    double getSpaceModifyWorstRatio() const { return spaceModifyWorstRatio; }
    int getAmountOfSpaceRemoval() const { return amountOfSpaceRemoval; }
    unsigned int getKMin() const { return kMin; }
    unsigned int getKMax() const { return kMax; }
    unsigned int getRngSeed() const { return rngSeed; }
    bool getResultLine() const { return resultLine; }
    bool getMsFiles() const { return msFiles; }
    bool getPlot() const { return plot; }
    bool getVisualizePlot() const { return visualizePlot; }
    bool getVerbose() const { return verbose; }
    bool getOutputAll() const { return outputAll; }
    bool getOutputBest() const { return outputBest; }
    bool getResultDir() const { return resultDir; }
    const std::string& getSubDirName() const { return setSubDirName; }
    bool getCombineData() const { return combineData; }
    unsigned int getMaxIterationsInSequence() const { return maxIterationsInSequence; }
    const std::vector<std::string>& getVisualizations() const { return visualizations; }
    bool getRunSDSimulation() const { return runSDSimulation; }
    bool getRunBPSimulation() const { return runBPSimulation; }
};

}}} // namespace bso::optimizers::scdp

#include <bso/optimizers/scdp/scdp_config.cpp>

#endif // SCDP_CONFIG_HPP
