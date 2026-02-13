#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include <bso/utilities/data_export.hpp>
#include <bso/visualization/visualization.hpp>

namespace bso { namespace optimizers { namespace scdp {

// Static member initialization
clock_t output_manager::sBegin = 0;
clock_t output_manager::sEnd = 0;

output_manager::output_manager(const scdp_config& config)
{

}

const std::string& output_manager::getResultDir() const
{
    return mResultDir;
}

void output_manager::logError(const std::string& errorMessage,
                              const std::string& inputFile,
                              const std::string& modification,
                              unsigned int iterations,
                              unsigned int errorIteration)
{
    std::ofstream errorLog("error_log", std::ios_base::app);
    if (!errorLog.is_open())
    {
        std::cerr << "Unable to open error_log file." << std::endl;
        return;
    }

    errorLog << "inputFile:" << inputFile << "\nThe following error message was obtained using modification: "
             << modification << ", \nwith max iterations: " << iterations
             << ", error occurred in iteration: " << errorIteration << "\n";
    errorLog << errorMessage << "\n\n";
}

// Function to create a result directory for the output files.
// The directory is created in the current working directory and named "results".
// The sub-directory is created based on the input file name, modification type, iteration number and current date and time.
bool output_manager::createResultDir(std::string& dir, bool verbose) const
{
	// create resultDir path if not existing yet
	bool resultDirGen = false;
	std::string path = "results";
	if (!std::filesystem::exists(path)) 
	{
		if (std::filesystem::create_directory(path)) 
		{
			resultDirGen = true;
			dir = path + "/";
			output_manager::out("Directory created successfully.",true,true,verbose);
		} 
		else 
		{
			output_manager::out("Failed to create directory.",true,true,verbose);
		}
	} 
	else 
	{
		resultDirGen = true;
		dir = path + "/";
		output_manager::out("Directory already exists.",true,true,verbose);
	}

	return resultDirGen;
} // createResultDir()

// Function to create a sub directory in the result directory for the output files.
// The sub-directory is created based on the input file name, modification type, iteration number and current date and time.
void output_manager::getSubResultDir(std::string& dir, std::string inputFile, std::string modification, unsigned int iterationNr, bool verbose) const
{
	// create resultDir if not existing yet
	std::string path;
	bool resultDirGen = createResultDir(path, verbose);

	// create sub-directory of results if not existing yet
	if(resultDirGen)
	{
		std::string path2 = path + inputFile + "_" + modification + "_-n=" + std::to_string(iterationNr) + "_" + getCurrentDateTime();
		if (!std::filesystem::exists(path2)) 
		{
			if (std::filesystem::create_directory(path2)) 
			{
				dir = path2 + "/";
				output_manager::out("Sub-directory of results created successfully.",true,true,verbose);
			} 
			else 
			{
				output_manager::out("Sub-directory of results failed to create directory.",true,true,verbose);
			}
		} 
		else 
		{
			dir = path2 + "/";
			output_manager::out("Sub-directory of results directory already exists.",true,true,verbose);
		}
	}
} // getSubResultDir()

void output_manager::outputData(const OptimizationOutput& output)
{
	output_manager::out("Start outputting data",true,true,output.config.getVerbose());
	// Output the requested outputs:
	std::vector<unsigned int> outputSpaces;
	// if best, find which designs are best;
	if (output.config.getOutputAll())
	{
		for (unsigned int i = 0; i < output.msDesigns.size(); ++i)
		{
			outputSpaces.push_back(i);
		}
	}

	std::string dir = "";
	if(output.config.getResultDir()) {getSubResultDir(dir, output.config.getInputFile(), output.modification, output.maxIterationsInSequence, output.config.getVerbose());}

	if(output.config.getPlot())
	{
		utilities::data_export dp(output.designPerformances, output.axisDesignPerformances, "Design Iteration");
		std::string fileScatter = dir+"scatterData_" + output.config.getInputFile() + "_" + output.modification;
		std::string fileParetoFront = dir+"paretoFrontData_" + output.config.getInputFile() + "_" + output.modification;
		std::string fileNonParetoFront = dir+"paretoNonFrontData_" + output.config.getInputFile() + "_" + output.modification;
		std::string fileVector = dir+"vectorData_" + output.config.getInputFile() + "_" + output.modification;
		std::string plotScatter = dir+"scatterPlot_" + output.config.getInputFile() + "_" + output.modification +".png";
		std::string plotParetoFront = dir+"paretoFrontPlot_" + output.config.getInputFile() + "_" + output.modification +".png";
		std::string plotVector = dir+"vectorPlot_" + output.config.getInputFile() + "_" + output.modification +".png";
		try{dp.plotScatter2D(fileScatter, plotScatter, false, true);}
		catch(const std::exception& e){std::cout << "During the generation of the scatter plots, the following error occurred:" << e.what() << '\n';}
		try{dp.plotParetoFront2D(fileParetoFront, fileNonParetoFront, plotParetoFront, false, true);}
		catch(const std::exception& e){std::cout << "During the generation of the paretoFront plots, the following error occurred:" << e.what() << '\n';}
		try{dp.plotVector2D(fileVector, plotVector, output.config.getVisualizePlot(), true);}
		catch(const std::exception& e){std::cout << "During the generation of the Vector2D plots, the following error occurred:" << e.what() << '\n';}
		output_manager::out("Made all plots",true,true,output.config.getVerbose());
	}

	if(output.config.getCombineData())
	{		
		utilities::data_export dp(output.designPerformances, output.axisDesignPerformances, "Design Iteration");
		dp.addToCombinedDataFile("combined_data.csv", output.modification, output.config.getInputFile(), output.maxIterationsInSequence, output.msDesigns, output.sd_total_load, output.exteriorSurfaceArea);
		dp.addToOverviewCombinedDataFile("overView_data.csv", output.modification, output.config.getInputFile(), output.finalIteration, output.msDesigns[0]);
		output_manager::out("Combined the data in multiple run file",true,true,output.config.getVerbose());
	}
	
	for (const auto& i : outputSpaces)
	{
		if (output.config.getResultLine())
		{
			std::cout << output.designName << "\t"
								<< output.modification << "\t"
								<< i << "\t"
								<< output.designPerformances[i](0) << "\t"
								<< output.designPerformances[i](1) << "\t" << std::endl;
								if(output.deleteMethod == "cluster")
								{
									std::cout << output.designName << output.RNGSeed << std::endl;
								}
			
		}
		if (output.config.getMsFiles())
		{
			std::stringstream fileNameStream;
			fileNameStream << dir << output.designName << "_"
										<< output.modification << "_"
										<< i << "_"
										<< output.designPerformances[i](0) << "_"
										<< output.designPerformances[i](1) << "_";
										if(output.deleteMethod == "cluster")
										{
											fileNameStream << output.RNGSeed;
										}
			std::string fileName = fileNameStream.str();
			output.msDesigns[i].writeToFile(fileName);
		}
	}
	
	if (output.config.getResultLine()){output_manager::out("Printed result lines to terminal",true,true,output.config.getVerbose());}
	if (output.config.getMsFiles()){output_manager::out("Generated ms_files for output",true,true,output.config.getVerbose());}

	if (!output.config.getVisualizations().empty()) visualization::endVisualization();
}

std::string output_manager::getCurrentDateTime()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d_%H-%M-%S");
    return ss.str();
}

}}} // namespace bso::optimizers::scdp
