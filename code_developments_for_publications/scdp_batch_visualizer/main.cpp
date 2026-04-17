#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include <bso/spatial_design/ms_N_building.hpp>
#include <bso/visualization/models/dl_model.hpp>
#include <bso/visualization/visualization.hpp>

namespace fs = std::filesystem;

struct VisualizationJob
{
	std::string msFile;
	std::string colorFile;
	bool useColor;
	bool isDl = false;
	std::string dlFile;
};

enum class VisualizationMode
{
	Both,
	Colored,
	Plain,
	Dl,
	All
};

enum class SpaceIdMode
{
	On,
	Off
};

static void printUsage(const char* executableName)
{
	std::cerr << "Usage: " << executableName
			  << " [--mode=<both|colored|plain|dl|all>] <results_directory_1> [results_directory_2 ...]"
			  << std::endl;
	std::cerr << "Options:" << std::endl;
	std::cerr << "  -h, --help            Show this help and exit" << std::endl;
	std::cerr << "  --mode=<value>        Choose visualization mode: both, colored, plain, dl, all" << std::endl;
	std::cerr << "  --mode <value>        Same as above" << std::endl;
	std::cerr << "    both (default)        ms_N designs: both colored and plain" << std::endl;
	std::cerr << "    colored               ms_N designs: colored only" << std::endl;
	std::cerr << "    plain                 ms_N designs: plain only" << std::endl;
	std::cerr << "    dl                    DL model iterations only" << std::endl;
	std::cerr << "    all                   All visualizations (ms_N + DL)" << std::endl;
	std::cerr << "  --space-id=<on|off>   Show or hide space IDs for plain ms_N views (default: on)" << std::endl;
	std::cerr << "  --space-id <on|off>   Same as above" << std::endl;
	std::cerr << "  --show-space-id=<on|off>  Alias for --space-id" << std::endl;
	std::cerr << "  --show-space-id <on|off>  Alias for --space-id" << std::endl;
	std::cerr << "Examples:" << std::endl;
	std::cerr << "  " << executableName << " /path/to/results" << std::endl;
	std::cerr << "  " << executableName << " --mode=colored /path/to/results" << std::endl;
	std::cerr << "  " << executableName << " --mode=plain /path/to/results_a /path/to/results_b" << std::endl;
}

static bool isHelpArgument(const std::string& argument)
{
	return argument == "-h" || argument == "--help";
}

static bool parseModeValue(const std::string& modeValue, VisualizationMode& mode)
{
	if (modeValue == "both")
	{
		mode = VisualizationMode::Both;
		return true;
	}
	if (modeValue == "colored")
	{
		mode = VisualizationMode::Colored;
		return true;
	}
	if (modeValue == "plain")
	{
		mode = VisualizationMode::Plain;
		return true;
	}
	if (modeValue == "dl")
	{
		mode = VisualizationMode::Dl;
		return true;
	}
	if (modeValue == "all")
	{
		mode = VisualizationMode::All;
		return true;
	}
	return false;
}

static bool parseSpaceIdValue(const std::string& value, SpaceIdMode& mode)
{
	if (value == "on")
	{
		mode = SpaceIdMode::On;
		return true;
	}
	if (value == "off")
	{
		mode = SpaceIdMode::Off;
		return true;
	}
	return false;
}

static bool modeAllowsJob(const VisualizationMode mode, const VisualizationJob& job)
{
	if (mode == VisualizationMode::All) return true;
	if (job.isDl) return mode == VisualizationMode::Dl;
	if (mode == VisualizationMode::Dl) return false;
	if (mode == VisualizationMode::Both) return true;
	if (mode == VisualizationMode::Colored) return job.useColor;
	return !job.useColor; // Plain
}

static bool startsWith(const std::string& value, const std::string& prefix)
{
	return value.size() >= prefix.size() && value.compare(0, prefix.size(), prefix) == 0;
}

static bool endsWith(const std::string& value, const std::string& suffix)
{
	return value.size() >= suffix.size() &&
		value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
}

static bool parseUnsignedIntToken(const std::string& token, int& parsedValue)
{
	if (token.empty()) return false;

	for (char c : token)
	{
		if (!std::isdigit(static_cast<unsigned char>(c))) return false;
	}

	try
	{
		parsedValue = std::stoi(token);
		return true;
	}
	catch (...)
	{
		return false;
	}
}

static bool extractIterationFromDesignFileName(const std::string& fileName, int& iteration)
{
	// Expected pattern example: design_325_sdbpdl_4_...
	if (!startsWith(fileName, "design_")) return false;

	const std::size_t firstUnderscoreAfterPrefix = fileName.find('_', 7);
	if (firstUnderscoreAfterPrefix == std::string::npos) return false;

	const std::size_t secondUnderscoreAfterPrefix = fileName.find('_', firstUnderscoreAfterPrefix + 1);
	if (secondUnderscoreAfterPrefix == std::string::npos) return false;

	const std::size_t thirdUnderscoreAfterPrefix = fileName.find('_', secondUnderscoreAfterPrefix + 1);
	if (thirdUnderscoreAfterPrefix == std::string::npos) return false;

	const std::string iterationToken =
		fileName.substr(secondUnderscoreAfterPrefix + 1,
			thirdUnderscoreAfterPrefix - secondUnderscoreAfterPrefix - 1);

	return parseUnsignedIntToken(iterationToken, iteration);
}

static bool extractIterationFromMsOrColorFileName(const std::string& fileName, int& iteration)
{
	// Expected patterns example:
	// - ms_N_worst_bp_10_ms.txt
	// - ms_N_worst_bp_10_colors.txt
	std::string baseName = fileName;
	if (endsWith(baseName, "_ms.txt"))
	{
		baseName = baseName.substr(0, baseName.size() - std::string("_ms.txt").size());
	}
	else if (endsWith(baseName, "_colors.txt"))
	{
		baseName = baseName.substr(0, baseName.size() - std::string("_colors.txt").size());
	}
	else
	{
		return false;
	}

	const std::size_t lastUnderscore = baseName.rfind('_');
	if (lastUnderscore == std::string::npos) return false;

	const std::string iterationToken = baseName.substr(lastUnderscore + 1);
	return parseUnsignedIntToken(iterationToken, iteration);
}

static bool extractIterationFromDlFileName(const std::string& fileName, int& iteration)
{
	// Expected pattern example: dl_iteration_4.txt
	if (!startsWith(fileName, "dl_iteration_") || !endsWith(fileName, ".txt")) return false;

	const std::string withoutExt = fileName.substr(0, fileName.size() - 4);
	const std::size_t lastUnderscore = withoutExt.rfind('_');
	if (lastUnderscore == std::string::npos) return false;

	const std::string token = withoutExt.substr(lastUnderscore + 1);
	return parseUnsignedIntToken(token, iteration);
}

static int extractIterationOrFallback(const VisualizationJob& job)
{
	int iteration = 0;

	if (job.isDl)
	{
		const std::string dlFileName = fs::path(job.dlFile).filename().string();
		if (extractIterationFromDlFileName(dlFileName, iteration)) return iteration;
		return std::numeric_limits<int>::max();
	}

	const std::string msFileName = fs::path(job.msFile).filename().string();

	if (extractIterationFromDesignFileName(msFileName, iteration)) return iteration;
	if (extractIterationFromMsOrColorFileName(msFileName, iteration)) return iteration;

	if (job.useColor)
	{
		const std::string colorFileName = fs::path(job.colorFile).filename().string();
		if (extractIterationFromMsOrColorFileName(colorFileName, iteration)) return iteration;
	}

	return std::numeric_limits<int>::max();
}

static std::vector<fs::path> expandSearchDirectories(const std::vector<std::string>& inputDirectories)
{
	std::vector<fs::path> roots;
	std::set<std::string> seen;

	auto addRoot = [&roots, &seen](const fs::path& p)
	{
		if (!fs::exists(p) || !fs::is_directory(p)) return;
		const std::string key = p.lexically_normal().string();
		if (seen.insert(key).second)
		{
			roots.push_back(p);
		}
	};

	for (const std::string& directory : inputDirectories)
	{
		const fs::path providedPath(directory);
		if (!fs::exists(providedPath) || !fs::is_directory(providedPath))
		{
			std::cerr << "Skipping invalid directory: " << directory << std::endl;
			continue;
		}

		addRoot(providedPath);

		const fs::path visualizationSubdir = providedPath / "Visualization";
		addRoot(visualizationSubdir);

		if (providedPath.filename() == "Visualization")
		{
			addRoot(providedPath.parent_path());
		}
	}

	return roots;
}

static std::vector<VisualizationJob> collectJobs(const std::vector<std::string>& inputDirectories)
{
	std::vector<fs::path> colorFiles;
	std::set<std::string> designFiles;
	std::set<std::string> msTxtFiles;
	std::vector<std::string> dlFiles;

	const std::vector<fs::path> searchDirectories = expandSearchDirectories(inputDirectories);
	for (const fs::path& root : searchDirectories)
	{
		for (const auto& entry : fs::directory_iterator(root))
		{
			if (!entry.is_regular_file()) continue;

			const fs::path p = entry.path();
			const std::string fileName = p.filename().string();
			const std::string fullPath = p.string();

			if (startsWith(fileName, "dl_iteration_") && endsWith(fileName, ".txt"))
			{
				dlFiles.push_back(fullPath);
				continue;
			}

			if (endsWith(fileName, "_colors.txt"))
			{
				colorFiles.push_back(p);
				continue;
			}

			if (startsWith(fileName, "design_"))
			{
				designFiles.insert(fullPath);
				continue;
			}

			if (endsWith(fileName, "_ms.txt"))
			{
				msTxtFiles.insert(fullPath);
			}
		}
	}

	std::vector<VisualizationJob> jobs;
	std::set<std::string> pairedMsFiles;

	for (const fs::path& colorPath : colorFiles)
	{
		const std::string colorName = colorPath.filename().string();
		const std::string msName =
			colorName.substr(0, colorName.size() - std::string("_colors.txt").size()) + "_ms.txt";
		const fs::path msPath = colorPath.parent_path() / msName;

		if (!fs::exists(msPath) || !fs::is_regular_file(msPath))
		{
			std::cerr << "Color file without matching ms file, skipping: "
					  << colorPath.string() << std::endl;
			continue;
		}

		VisualizationJob job;
		job.msFile = msPath.string();
		job.colorFile = colorPath.string();
		job.useColor = true;
		jobs.push_back(job);
		pairedMsFiles.insert(msPath.string());
	}

	for (const std::string& filePath : designFiles)
	{
		VisualizationJob job;
		job.msFile = filePath;
		job.colorFile = "";
		job.useColor = false;
		jobs.push_back(job);
	}

	for (const std::string& filePath : msTxtFiles)
	{
		if (pairedMsFiles.find(filePath) != pairedMsFiles.end()) continue;

		VisualizationJob job;
		job.msFile = filePath;
		job.colorFile = "";
		job.useColor = false;
		jobs.push_back(job);
	}

	for (const std::string& filePath : dlFiles)
	{
		VisualizationJob job;
		job.isDl = true;
		job.dlFile = filePath;
		jobs.push_back(job);
	}

	std::sort(jobs.begin(), jobs.end(),
		[](const VisualizationJob& lhs, const VisualizationJob& rhs)
		{
			const int lhsIteration = extractIterationOrFallback(lhs);
			const int rhsIteration = extractIterationOrFallback(rhs);

			if (lhsIteration != rhsIteration)
			{
				return lhsIteration < rhsIteration;
			}

			if (lhs.msFile == rhs.msFile)
			{
				return lhs.colorFile < rhs.colorFile;
			}
			return lhs.msFile < rhs.msFile;
		});

	return jobs;
}

int main(int argc, char* argv[])
{
	for (int i = 1; i < argc; ++i)
	{
		if (isHelpArgument(argv[i]))
		{
			printUsage(argv[0]);
			return 0;
		}
	}

	if (argc < 2)
	{
		printUsage(argv[0]);
		return 1;
	}

	VisualizationMode mode = VisualizationMode::Both;
	SpaceIdMode spaceIdMode = SpaceIdMode::On;
	std::vector<std::string> inputDirectories;
	for (int i = 1; i < argc; ++i)
	{
		const std::string argument = argv[i];

		if (startsWith(argument, "--mode="))
		{
			VisualizationMode parsedMode;
			if (!parseModeValue(argument.substr(std::string("--mode=").size()), parsedMode))
			{
				std::cerr << "Invalid mode value: " << argument << std::endl;
				printUsage(argv[0]);
				return 1;
			}
			mode = parsedMode;
			continue;
		}

		if (argument == "--mode")
		{
			if (i + 1 >= argc)
			{
				std::cerr << "Missing value after --mode" << std::endl;
				printUsage(argv[0]);
				return 1;
			}
			VisualizationMode parsedMode;
			if (!parseModeValue(argv[++i], parsedMode))
			{
				std::cerr << "Invalid mode value: " << argv[i] << std::endl;
				printUsage(argv[0]);
				return 1;
			}
			mode = parsedMode;
			continue;
		}

		if (startsWith(argument, "--space-id=") || startsWith(argument, "--show-space-id="))
		{
			const std::size_t separatorPos = argument.find('=');
			SpaceIdMode parsedSpaceIdMode;
			if (!parseSpaceIdValue(argument.substr(separatorPos + 1), parsedSpaceIdMode))
			{
				std::cerr << "Invalid space-id value: " << argument << std::endl;
				printUsage(argv[0]);
				return 1;
			}
			spaceIdMode = parsedSpaceIdMode;
			continue;
		}

		if (argument == "--space-id" || argument == "--show-space-id")
		{
			if (i + 1 >= argc)
			{
				std::cerr << "Missing value after " << argument << std::endl;
				printUsage(argv[0]);
				return 1;
			}

			SpaceIdMode parsedSpaceIdMode;
			if (!parseSpaceIdValue(argv[++i], parsedSpaceIdMode))
			{
				std::cerr << "Invalid space-id value: " << argv[i] << std::endl;
				printUsage(argv[0]);
				return 1;
			}
			spaceIdMode = parsedSpaceIdMode;
			continue;
		}

		// Support bare mode values as positional arguments (e.g., ./main /path dl)
		{
			VisualizationMode parsedMode;
			if (parseModeValue(argument, parsedMode))
			{
				mode = parsedMode;
				continue;
			}
		}

		inputDirectories.push_back(argument);
	}

	if (inputDirectories.empty())
	{
		std::cerr << "No input directories provided." << std::endl;
		printUsage(argv[0]);
		return 1;
	}

	std::vector<VisualizationJob> jobs = collectJobs(inputDirectories);
	if (jobs.empty())
	{
		std::cerr << "No compatible visualization files found in the provided directories." << std::endl;
		return 1;
	}

	jobs.erase(
		std::remove_if(jobs.begin(), jobs.end(),
			[mode](const VisualizationJob& job)
			{
				return !modeAllowsJob(mode, job);
			}),
		jobs.end());

	if (jobs.empty())
	{
		std::cerr << "No visualization cases match the selected mode." << std::endl;
		return 1;
	}

	{
		int msNCount = 0;
		int dlCount = 0;
		for (const auto& job : jobs)
		{
			if (job.isDl) ++dlCount;
			else ++msNCount;
		}
		if (msNCount > 0)
			std::cout << "Found " << msNCount << " ms_N visualization case(s)." << std::endl;
		if (dlCount > 0)
			std::cout << "Found " << dlCount << " DL model visualization case(s)." << std::endl;
	}
	std::cout << "Space ID labels for plain ms_N views: "
			  << ((spaceIdMode == SpaceIdMode::On) ? "on" : "off") << std::endl;

	try
	{
		int visArgc = 1;
		char* visArgv[] = {argv[0], nullptr};
		bso::visualization::initVisualization(visArgc, visArgv);

		int addedViewports = 0;
		for (const VisualizationJob& job : jobs)
		{
			try
			{
				if (job.isDl)
				{
					const std::string title = fs::path(job.dlFile).filename().string();
					bso::visualization::vpmanager.addviewport(
						new bso::visualization::viewport(
							new bso::visualization::DL_Model(job.dlFile)));
					++addedViewports;
				}
				else
				{
					bso::spatial_design::ms_N_building msN(job.msFile);
					const std::string title = fs::path(job.msFile).filename().string();

					if (job.useColor)
					{
						bso::visualization::visualize(job.colorFile, msN, "spaces", title, 1.0);
					}
					else
					{
						const std::string plainType =
							(spaceIdMode == SpaceIdMode::On) ? "spaces" : "";
						bso::visualization::visualize(msN, plainType, title, 1.0);
					}
					++addedViewports;
				}
			}
			catch (const std::exception& e)
			{
				std::cerr << "Skipping unreadable/invalid file: "
						  << (job.isDl ? job.dlFile : job.msFile)
						  << "\nReason: " << e.what() << std::endl;
			}
		}

		if (addedViewports == 0)
		{
			std::cerr << "No valid viewports could be created." << std::endl;
			return 1;
		}

		std::cout << "Opening visualization window with " << addedViewports
				  << " viewport(s)." << std::endl;
		bso::visualization::endVisualization();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Visualization failed: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
