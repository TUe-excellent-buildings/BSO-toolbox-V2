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
// The directory is created in the current working directory by default and
// named according to mConfig.getResultBaseDir() (default: "results").
// The sub-directory is created based on the input file name, modification
// type, iteration number and current date and time.
bool output_manager::createResultDir(std::string& dir) const
{
	// create resultDir path if not existing yet
	bool resultDirGen = false;
	std::string path = mConfig.getResultBaseDir();
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
		if(shortName)
		{
			path2 = path + mConfig.getInputFile() + "_" + mConfig.getModification();
			if(mConfig.getWeightedModification())path2 += "_" + mConfig.getWeightedModification();
			path2 += "_-n=" + std::to_string(mConfig.getIterations());
		}
		else
		{
			path2 = path + mConfig.getInputFile() + "_" + mConfig.getModification();
			if(mConfig.getWeightedModification()) path2 += "_" + mConfig.getWeightedModification();
			path2 += "_-n=" + std::to_string(mConfig.getIterations()) + "_" + getCurrentDateTime();
		}
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
	std::vector<unsigned int> outputDesignsIndex;
	// if best, find which designs are best;
	if (mConfig.getOutputAll())
	{
		for (unsigned int i = 0; i < output.msDesigns.size(); ++i)
		{
			outputDesignsIndex.push_back(i);
		}
	}

	std::string dir = mResultDir;

	if(mConfig.getPlot())
	{
		// Number of stored iterations and objectives
		const std::size_t nrIterations  = output.designPerformances.size();
		const std::size_t nrObjectives  = output.axisDesignPerformances.size();

		// 2D scatter/pareto/vector plots are only meaningful when exactly two
		// objective dimensions are active.
		if(nrObjectives == 2 && nrIterations > 0)
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
		}
		// Multi-axis line plot is produced whenever there is at least one
		// objective and at least one iteration.
		if(nrIterations > 0 && nrObjectives > 0)
		{
			utilities::data_export dp(output.designPerformances, output.axisDesignPerformances, "Design Iteration");
			std::string fileMultiAxis = dir+"multiAxisData_" + mConfig.getInputFile() + "_" + output.modification;
			std::string plotMultiAxis = dir+"multiAxisPlot_" + mConfig.getInputFile() + "_" + output.modification +".png";
			try{dp.plotMultiAxis2D(fileMultiAxis, plotMultiAxis, mConfig.getVisualizePlot(), true);}
			catch(const std::exception& e){std::cout << "During the generation of the multi axis plots, the following error occurred:" << e.what() << '\n';}
		}
		// 3D objective-space and improvement-curve plots require at least
		// three objective dimensions (sd, bp, dl_ADFbuilding). The data_export
		// helpers will further validate that these specific names are present.
		if(nrIterations > 0 && nrObjectives >= 3)
		{ 
			// Please note, this assumes a sd, bp and dl output where only the
			// dl_ADFbuilding is taken into account for the improvement curves.
			// If the objectives are different, the improvement-curve and 3D plots
			// may not be generated as expected since the exporters look for
			// specific objective names in the axisDataTypes to know which one is which.
			utilities::data_export dp(output.designPerformances, output.axisDesignPerformances, "Design Iteration");
			std::string fileObjective3D = dir+"objective3DData_" + mConfig.getInputFile() + "_" + output.modification;
			std::string plotObjective3D = dir+"objective3DPlot_" + mConfig.getInputFile() + "_" + output.modification +".png";
			std::string fileImprovementCurves = dir+"improvementCurvesData_" + mConfig.getInputFile() + "_" + output.modification;
			std::string plotImprovementCurves = dir+"improvementCurvesPlot_" + mConfig.getInputFile() + "_" + output.modification +".png";
			try{dp.plotObjectiveSpace3D(fileObjective3D, plotObjective3D, mConfig.getVisualizePlot(), true);} 
			catch(const std::exception& e){std::cout << "During the generation of the objective space 3D plots, the following error occurred:" << e.what() << '\n';}
			// By default do not show the knee point in the improvement-curve plot.
			try{dp.plotImprovementCurves(fileImprovementCurves, plotImprovementCurves, mConfig.getVisualizePlot(), true, false);} 
			catch(const std::exception& e){std::cout << "During the generation of the improvement curves plots, the following error occurred:" << e.what() << '\n';}
		}
		out("Made all plots",true,true,mConfig.getVerbose());
	}

	if(mConfig.getCombineData())
	{		
		utilities::data_export dp(output.designPerformances, output.axisDesignPerformances, "Design Iteration");
		dp.addToCombinedDataFile("combined_data.csv", output.modification, mConfig.getInputFile(), output.maxIterationsInSequence, output.msDesigns, output.sd_total_load, output.exteriorSurfaceArea);
		//dp.addToOverviewCombinedDataFile("overView_data.csv", output.modification, mConfig.getInputFile(), output.finalIteration, output.msDesigns[0]);
		out("Combined the data in multiple run file",true,true,mConfig.getVerbose());
	}

	if (mConfig.getResultLine()) // setup terminal description before outputting the result lines
	{
		// run indicators and descriptions:
		std::cout << output.designName << "\t" << output.modification << std::endl;
			if(output.weightedModification) std::cout << "weighted with: "; for(const auto& weight: output.objectiveWeights) std::cout << weight << " "; std::cout << std::endl;
			if(output.deleteMethod == "cluster"){std::cout << "settings for clustering: "<< "\t" << output.designName << "\t" << output.RNGSeed << std::endl;}
			// Axis descibtion
			std::cout << "Iteration";
			for(const auto& axisName: output.axisDesignPerformances){std::cout << "\t" << axisName;}
			std::cout << std::endl;
	}
	
	for (const auto& i : outputDesignsIndex)
	{
		if (mConfig.getResultLine()) // outputting the result lines to terminal
		{
			std::cout << i;
			const auto& dp = output.designPerformances[i];
			for (unsigned int j = 0; j < dp.size(); ++j)
			{
				std::cout << "\t" << dp(j);
			}
			std::cout << std::endl;
		}
		if (mConfig.getMsFiles()) // outputting the ms files for the designs
		{
			std::stringstream fileNameStream;
			fileNameStream << dir << output.designName << "_"
										<< output.modification << "_"
										<< i << "_";
										const auto& dp = output.designPerformances[i];
										for (unsigned int j = 0; j < dp.size(); ++j)
										{
											fileNameStream << dp(j) << "_";
										}
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
