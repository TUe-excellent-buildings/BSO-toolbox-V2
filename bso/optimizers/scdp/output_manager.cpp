#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include <bso/utilities/data_export.hpp>
#include <bso/visualization/visualization.hpp>

namespace bso { namespace optimizers { namespace scdp {

output_manager::output_manager(const scdp_config& config) : mConfig(config)
{
	// Initialize the result directory if the option is set in the configuration
	if(mConfig.getResultDir()){setResultDir();}
}

// This logError function is used to log error messages to a file named "error_log" in the result directory if it exists,
// otherwise in the current working directory. The log includes the input file, modification type, number of iterations,
// iteration number where the error occurred and the error message itself.
void output_manager::logError(const std::string& errorMessage,
                              unsigned int errorIteration, bool verbose /*= false*/)
{	
	if(verbose){std::cout << "Start logging error to error_log file." << std::endl;}
	
	std::string errorLogPath = mResultDir.empty() ? "error_log" : mResultDir + "error_log";
	std::ofstream errorLog(errorLogPath, std::ios_base::app);
	if (!errorLog.is_open())
	{
		std::cerr << "Unable to open error_log file" 
		          << (mResultDir.empty() ? "." : " at path: " + errorLogPath) << std::endl;
		return;
	}

	errorLog << "inputFile:" << mConfig.getInputFile() 
	         << "\nThe following error message was obtained using the iterationStrategy: " << mConfig.getIterationStrategy() << ",\n"
			 << "and modification: " << mConfig.getModification() << ", with max iterations: " << mConfig.getMaxIterationsInSequence() << ",\n"
	         << "error occurred in iteration: " << errorIteration << "\n"
	         << errorMessage << "\n\n";

	if(verbose){std::cout << "Finished logging error to error_log file." << std::endl;}
}

// Function to create a result directory for the output files.
// The directory is created in the current working directory and named "results".
// The sub-directory is created based on the input file name, modification type, iteration number and current date and time.
bool output_manager::createResultDir(std::string& dir) const
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
			out("Directory created successfully.",true,true,mConfig.getVerbose());
		} 
		else 
		{
			out("Failed to create directory.",true,true,mConfig.getVerbose());
		}
	} 
	else 
	{
		resultDirGen = true;
		dir = path + "/";
			out("Directory already exists.",true,true,mConfig.getVerbose());
	}

	return resultDirGen;
} // createResultDir()

// Function to create a sub directory in the result directory for the output files.
// The sub-directory is created based on the input file name, modification type, iteration number and current date and time.
bool output_manager::createStandardSubResultDir(std::string& dir, bool shortName /*= false*/) const
{
	// create resultDir if not existing yet
	std::string path;
	bool resultDirGen = createResultDir(path);

	bool subResultDirGen = false;
	// create sub-directory of results if not existing yet and result dir was created or already exists
	if(resultDirGen)
	{
		std::string path2;
		if(shortName){path2 = path + mConfig.getInputFile() + "_" + mConfig.getModification() + "_-n=" + std::to_string(mConfig.getMaxIterationsInSequence());}
		else{path2 = path + mConfig.getInputFile() + "_" + mConfig.getModification() + "_-n=" + std::to_string(mConfig.getMaxIterationsInSequence()) + "_" + getCurrentDateTime();}
		if (!std::filesystem::exists(path2)) 
		{
			if (std::filesystem::create_directory(path2)) 
			{
				dir = path2 + "/";
				subResultDirGen = true;
				out("Sub-directory of results created successfully.",true,true,mConfig.getVerbose());
			} 
			else 
			{
				out("Sub-directory of results failed to create directory.",true,true,mConfig.getVerbose());
			}
		} 
		else 
		{
			dir = path2 + "/";
			subResultDirGen = true;
			out("Sub-directory of results directory already exists.",true,true,mConfig.getVerbose());
		}
	}
	return subResultDirGen;
} // createStandardSubResultDir()

bool output_manager::createSubResultDir(std::string& dir) const
{
	// create resultDir if not existing yet
	std::string path;
	bool resultDirGen = createResultDir(path);

	// create sub-directory of results if not existing yet and result dir was created or already exists
	if(resultDirGen)
	{
		std::string path2 = path + mConfig.getSubDirName();
		if (!std::filesystem::exists(path2)) 
		{
			if (std::filesystem::create_directory(path2)) 
			{
				dir = path2 + "/";
				out("Sub-directory of results created successfully.",true,true,mConfig.getVerbose());
			} 
			else 
			{
				out("Sub-directory of results failed to create directory.",true,true,mConfig.getVerbose());
			}
		} 
		else 
		{
			dir = path2 + "/";
			out("Sub-directory of results directory already exists.",true,true,mConfig.getVerbose());
		}
	}

	return resultDirGen;
} // createSubResultDir()

void output_manager::setResultDir()
{
	std::string dir;
	if (mConfig.getSubDirName() == "")
	{
		createStandardSubResultDir(dir);
	}
	else if (mConfig.getSubDirName() == "shortStandard")
	{
		createStandardSubResultDir(dir, true);
	}
	else
	{
		createSubResultDir(dir);
	}
	mResultDir = dir;
} 

void output_manager::outputData(const OptimizationOutput& output)
{
	out("Start outputting data",true,true,mConfig.getVerbose());

	// Output the requested outputs:
	std::vector<unsigned int> outputSpaces;
	// if best, find which designs are best;
	if (mConfig.getOutputAll())
	{
		for (unsigned int i = 0; i < output.msDesigns.size(); ++i)
		{
			outputSpaces.push_back(i);
		}
	}

	std::string dir = mResultDir;

	if(mConfig.getPlot())
	{
		utilities::data_export dp(output.designPerformances, output.axisDesignPerformances, "Design Iteration");
		std::string fileScatter = dir+"scatterData_" + mConfig.getInputFile() + "_" + output.modification;
		std::string fileParetoFront = dir+"paretoFrontData_" + mConfig.getInputFile() + "_" + output.modification;
		std::string fileNonParetoFront = dir+"paretoNonFrontData_" + mConfig.getInputFile() + "_" + output.modification;
		std::string fileVector = dir+"vectorData_" + mConfig.getInputFile() + "_" + output.modification;
		std::string plotScatter = dir+"scatterPlot_" + mConfig.getInputFile() + "_" + output.modification +".png";
		std::string plotParetoFront = dir+"paretoFrontPlot_" + mConfig.getInputFile() + "_" + output.modification +".png";
		std::string plotVector = dir+"vectorPlot_" + mConfig.getInputFile() + "_" + output.modification +".png";
		try{dp.plotScatter2D(fileScatter, plotScatter, false, true);}
		catch(const std::exception& e){std::cout << "During the generation of the scatter plots, the following error occurred:" << e.what() << '\n';}
		try{dp.plotParetoFront2D(fileParetoFront, fileNonParetoFront, plotParetoFront, false, true);}
		catch(const std::exception& e){std::cout << "During the generation of the paretoFront plots, the following error occurred:" << e.what() << '\n';}
		try{dp.plotVector2D(fileVector, plotVector, mConfig.getVisualizePlot(), true);}
		catch(const std::exception& e){std::cout << "During the generation of the Vector2D plots, the following error occurred:" << e.what() << '\n';}
		out("Made all plots",true,true,mConfig.getVerbose());
	}

	if(mConfig.getCombineData())
	{		
		utilities::data_export dp(output.designPerformances, output.axisDesignPerformances, "Design Iteration");
		dp.addToCombinedDataFile("combined_data.csv", output.modification, mConfig.getInputFile(), output.maxIterationsInSequence, output.msDesigns, output.sd_total_load, output.exteriorSurfaceArea);
		dp.addToOverviewCombinedDataFile("overView_data.csv", output.modification, mConfig.getInputFile(), output.finalIteration, output.msDesigns[0]);
		out("Combined the data in multiple run file",true,true,mConfig.getVerbose());
	}
	
	for (const auto& i : outputSpaces)
	{
		if (mConfig.getResultLine())
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
		if (mConfig.getMsFiles())
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
	
	if (mConfig.getResultLine()){out("Printed result lines to terminal",true,true,mConfig.getVerbose());}
	if (mConfig.getMsFiles()){out("Generated ms_files for output",true,true,mConfig.getVerbose());}

	if (!mConfig.getVisualizations().empty()) visualization::endVisualization();
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
