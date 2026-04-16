#ifndef DL_MODEL_CPP
#define DL_MODEL_CPP

#include <filesystem>
#include <cstdlib>
#include <unistd.h> // for getpid() on POSIX systems
#include <fstream>
#include <cmath>
#include <limits>
#include <iostream>

namespace bso { namespace daylight {
	
	// Static variable to keep track of the number of dl_model instances created, used for default folder naming convention.
	static int dl_model_count = 0; 

	// Helper function to get the radiance bin path relative to this library file
	static std::string GetRadianceBinPath() {
		std::filesystem::path filePath(__FILE__);
		std::filesystem::path daylightDir = filePath.parent_path();
		std::filesystem::path radianceBinPath = daylightDir / "radiance" / "bin";
		return radianceBinPath.string();
	}
	
	// Helper function to set RAYPATH for .cal files
	static void SetRadianceCalPath() {
		std::filesystem::path filePath(__FILE__);
		std::filesystem::path daylightDir = filePath.parent_path();
		std::filesystem::path radianceCalPath = daylightDir / "radiance" / "cal";
		setenv("RAYPATH", radianceCalPath.string().c_str(), 1);
	}
	
	dl_model::dl_model() 
	: mResultsGenerated(false)
	{
	
	} // ctor()
	
	dl_model::dl_model(const dl_model& rhs) 
	: SpaceID_AmountGridPoints(rhs.SpaceID_AmountGridPoints),
	  DF_Settings(rhs.DF_Settings),
	  DF_Evaluation(rhs.DF_Evaluation),
	  mFolderName(rhs.mFolderName),
		  mSpaceResults(rhs.mSpaceResults),
		  mBuildingResults(rhs.mBuildingResults),
	  mResultsGenerated(rhs.mResultsGenerated),
	  roomInfo(rhs.roomInfo),
	  mRectangleWindowYN(rhs.mRectangleWindowYN),
	  mTriangles(rhs.mTriangles),
	  mcf_space(rhs.mcf_space),
	  mCeilings(rhs.mCeilings),
	  mCeilingsTri(rhs.mCeilingsTri),
	  mGridPoints(rhs.mGridPoints)
	{

	} //copy constructor

	dl_model::~dl_model()
	{

	} // dtor()	

	std::string dl_model::createAndSetFolder(const std::string& settingFileName, const std::string& folderName /*= ""*/)
	{		
		// If no folder name is provided, create a per-process base directory
		// and store each daylight iteration in a subdirectory of that base.
		// This avoids collisions when multiple scdp_class/main processes run
		// in parallel, because each process gets its own "Run_<pid>" folder
		// and within that, "Iteration_<count>" subfolders.
		//
		// Pattern when folderName is empty:
		// iteration:  "Run_<pid>/Iteration_<count>"
		// If a folder name is provided explicitly, use that one instead.
		if (folderName == "")
		{
			pid_t pid = getpid();
			std::string base;
			base = "Run_" + std::to_string(static_cast<long long>(pid));
			mFolderName = base + "/Iteration_" + std::to_string(dl_model_count);
		}
		else
		{
			// Always force relative path
			if (!folderName.empty() && folderName[0] == '/')
				mFolderName = "./" + folderName.substr(1);
			else
				mFolderName = folderName;
		}

		std::string command = "mkdir -p \"" + mFolderName + "\""; // Use mkdir command to create directory

		int result = std::system(command.c_str());

		if (result != 0) 
		{   
			std::stringstream errorMessage;
					errorMessage << "Error, could not create directory: " << mFolderName << " to store the radiance files, \n"
								 << "Command executed: " << command << "\n" 
								 << "(bso/daylight/dl_model.cpp). " << std::endl;
					throw std::invalid_argument(errorMessage.str());
		}
		else    //if folder created, add dl_setting file also to it, since you can change it per run to change window properties. 
		{
			std::string copyCommand = "cp \"" + settingFileName + "\" \"" + mFolderName + "/\"";
			int copyResult = std::system(copyCommand.c_str());

			if (copyResult != 0) 
			{
				std::stringstream errorMessage;
					errorMessage << "Error, could not copy dl_setting file, \n"
								 << "Command executed: " << copyCommand << "\n" 
								 << "(bso/daylight/dl_model.cpp). " << std::endl;
					throw std::invalid_argument(errorMessage.str());
			}
		}

		dl_model_count++; // Increment the count of dl_model instances so the next folder created will have a different name if the default naming convention is used.
		return mFolderName;    
	} // createAndSetFolder()

	double roundToTwoDecimals(double num) 
	{
    	return std::round(num * 100.0) / 100.0;
	}

	double ScoreDFRoomNENminimize(double DtPercentage, double DtmPercentage)
	{
		/*
		if DtmPercentage >= 50 && DtPercentage >= 50 : Both targets met so 0
		if only DtmPercentage >= 50, score is 5 or lower, linearly interpolate score 0-5 depending on Dt (0-50%)
		if both are < 50%, linearly interpolate score 5-10 depending on Dtm (0-50%)
		*/

		double finalScore = 10.0;
		if (DtmPercentage >= 50.0 && DtPercentage >= 50.0) {finalScore = 0.0; }
		else if (DtmPercentage >= 50.0 && DtPercentage < 50.0) {finalScore = ( 5.0 - ((5.0/50.0)*DtPercentage)); }
		else if (DtmPercentage < 50.0 && DtPercentage < 50.0) {finalScore = 10.0 - ((5.0/50.0)*DtmPercentage); }
		return finalScore;
	}

	double ScoreDFRoomNENmaximize(double DtPercentage, double DtmPercentage)
	{
		/*
		if DtmPercentage >= 50 && DtPercentage >= 50 : Both targets met so 10/10
		if only DtmPercentage >= 50, score is atleast a 5, linearly interpolate score 5-10 depending on Dt (0-50%)
		if both are < 50%, linearly interpolate score 0-5 depending on Dtm (0-50%)
		*/

		double finalScore = 0.0;
		if (DtmPercentage >= 50.0 && DtPercentage >= 50.0) {finalScore = 10.0; }
		else if (DtmPercentage >= 50.0 && DtPercentage < 50.0) {finalScore = ( 5.0 + ((5.0/50.0)*DtPercentage)); }
		else if (DtmPercentage < 50.0 && DtPercentage < 50.0) {finalScore = ((5.0/50.0)*DtmPercentage); }
		return finalScore;
	}

	void dl_model::analyze()
	{
		// reset any previous results for a new analysis run
		mSpaceResults.clear();
		mResultsGenerated = false;

		const double HorizontalOutdoorIlluminance = DF_Settings[0];
		const std::string radianceBinPath = GetRadianceBinPath();
		
		// Set RAYPATH so Radiance can find .cal files
		SetRadianceCalPath();

		std::string command;
		
		// Generate sky
		const std::string irrad = std::to_string(HorizontalOutdoorIlluminance/179.0); //generate diffuse irradiance in watts/m2 to derive the zenith brightness 
		const std::string skysettings = "-ang 45 0 -c -B "+irrad+" -g 0.2"; //-c=standard CIE overcast sky ,-B is zenith brightness (irrad), -g ground reflectance
		command = "\"" + radianceBinPath + "/gensky\" " + skysettings + " > \"./" + mFolderName + "/sky.rad\"";
		if (std::system(command.c_str()) != 0) 
		{
			std::stringstream errorMessage;
				errorMessage << "Error, could not generate the sky for the Radiance run, \n"
								<< "Failed to execute command: " << command << "\n" 
								<< "(bso/daylight/dl_model.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
		}

		// Create outside
		std::ofstream outsideFile(mFolderName+"/"+"outside.rad", std::ios::trunc);
		if (!outsideFile.is_open()) 
		{
			std::stringstream errorMessage;
				errorMessage << "Error, could not open outside.rad for the Radiance run, \n"
								<< "Failed to execute command: " << command << "\n" 
								<< "(bso/daylight/dl_model.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
		}
		outsideFile << "skyfunc glow sky_glow\n0\n0\n4 1 1 1 0\n";
		outsideFile << "sky_glow source sky\n0\n0\n4 0 0 1 180\n";
		outsideFile << "skyfunc glow ground_glow\n0\n0\n4 1 1 1 0\n";
		outsideFile << "ground_glow source ground\n0\n0\n4 0 0 -1 180\n\n";
		outsideFile.close();

		// Compile scene
		command = "\"" + radianceBinPath + "/oconv\" \"./" + mFolderName + "/sky.rad\" \"./" + mFolderName + "/outside.rad\" \"./" + mFolderName + "/scene.rad\" > \"./" + mFolderName + "/room.oct\"";
		if (std::system(command.c_str()) != 0) 
		{
			std::stringstream errorMessage;
				errorMessage << "Error, could not compile the scene for the Radiance run, \n"
								<< "Failed to execute command: " << command << "\n" 
								<< "(bso/daylight/dl_model.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
		}

		// Calculate illuminance
		std::string rtraceSettings = "-I -h -aa 0 -ab 8 -ad 4096 -as 1024 -lw 1e-6 -lr 16"; 
		//-I -h -aa 0 -ab 6 -ad 10240 -as 0 -lw 1e-6 -n 4       //Ward, 2024
		//-I -h -aa 0 -ab 8 -ad 4096 -as 1024 -lw 1e-6 -lr 16   //Kharvari, 2020
		command = "\"" + radianceBinPath + "/rtrace\" " + rtraceSettings + " -oov \"./" + mFolderName + "/room.oct\" < \"./" + mFolderName + "/grid.txt\" > \"./" + mFolderName + "/results.txt\"";
		if (std::system(command.c_str()) != 0) 
		{
			std::stringstream errorMessage;
				errorMessage << "Error, could not calculate the illuminance for the Radiance run, \n"
								<< "Failed to execute command: " << command << "\n" 
								<< "(bso/daylight/dl_model.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
		}

		// Make lux.
		command = "\"" + radianceBinPath + "/rcalc\" -e '$1=179*(.265*$4+.670*$5+.065*$6)' \"./" + mFolderName + "/results.txt\" > \"./" + mFolderName + "/lux.txt\"";
		if (std::system(command.c_str()) != 0) 
		{			
			std::stringstream errorMessage;
				errorMessage << "Error, could not make the lux text file containing the illuminance values for the Radiance run, \n"
								<< "Failed to execute command: " << command << "\n" 
								<< "(bso/daylight/dl_model.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
		}

		// After Radiance has produced lux.txt, generate per-space and building results once.
		generateResults();

	}; //analyse()

	void dl_model::generateResults()
	{
		if (mResultsGenerated) return;

		const double HorizontalOutdoorIlluminance = DF_Settings[0];
		const double Dt = DF_Evaluation[0];
		const double Dtm = DF_Evaluation[1];

		// Read output Radiance
		std::ifstream inputFile("./"+mFolderName+"/lux.txt");
		if (!inputFile.is_open()) {
			std::stringstream errorMessage;
			errorMessage << "Error, could not open the output lux.txt file from Radiance. " << std::endl
									<< "(bso/daylight/dl_model.cpp). " << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}

		mSpaceResults.clear();
		mSpaceResults.reserve(SpaceID_AmountGridPoints.size());

		// iterate per surface / space
		// one surface = one set of points (amount of gridpoints varies per surface)
		int valuesRead = 0;

		for (int i = 0; i < SpaceID_AmountGridPoints.size(); ++i) 
		{
			auto& tup = SpaceID_AmountGridPoints[i];
			unsigned int SpaceID = std::get<0>(tup);
			int gridPoints = std::get<1>(tup);

			int requiredExceedingCount = static_cast<int>(std::ceil(gridPoints/2.0)); //round up
			int pointsExceedingDt = 0;
			int pointsExceedingDtm = 0;
			double sumDF = 0.0;

			//iterate over each gridpoint per surface (space)
			for (int j = 0; j < gridPoints; ++j) 
			{
				double Espace; // horizontal illuminance at points at target level
				if (!(inputFile >> Espace)) 
				{
					inputFile.close();
					std::stringstream errorMessage;
					errorMessage << "Error, reading horizontal illuminance values from lux.txt.\n"
								 << "Radiance produced fewer readable values than expected for the generated grid.\n"
								 << "Read " << valuesRead << " values before the stream failed while processing SpaceID " << SpaceID
								 << " (grid point " << (j + 1) << " of " << gridPoints << ").\n"
								 << "Input file: ./" << mFolderName << "/lux.txt\n"
								 << "(bso/daylight/dl_model.cpp). " << std::endl;
					throw std::invalid_argument(errorMessage.str());
				}
				++valuesRead;
				
				double DF = (Espace/HorizontalOutdoorIlluminance)*100.0; //daylight factor per point
				if (DF > Dt) {pointsExceedingDt++;}
				if (DF > Dtm) {pointsExceedingDtm++;}
				sumDF += DF;
			} //gridpoint per surface

			double ADF = sumDF/static_cast<double>(gridPoints); //average daylight factor per space
			int resultDt = (pointsExceedingDt >= requiredExceedingCount) ? 1 : 0;
			int resultDtm = (pointsExceedingDtm >= requiredExceedingCount) ? 1 : 0;
			bool passDt = (resultDt == 1);
			bool passDtm = (resultDtm == 1);
			bool passDF = passDt && passDtm;
			double DtPercentage = std::ceil((100.0/static_cast<double>(gridPoints))*pointsExceedingDt); //per room 
			double DtmPercentage = std::ceil((100.0/static_cast<double>(gridPoints))*pointsExceedingDtm);
			double score = ScoreDFRoomNENminimize(DtPercentage, DtmPercentage);

			dl_space_result res;
			res.mSpaceID       = SpaceID;
			res.mADF           = ADF;
			res.mADFInverse   = (ADF == 0.0) ? 1000000.0 : (1.0 / ADF); // store inverse for optimization performance
			res.mDtPercentage  = DtPercentage;
			res.mDtmPercentage = DtmPercentage;
			res.mScore         = score;
			res.mPassDF        = passDF;
			res.mPassDt        = passDt;
			res.mPassDtm       = passDtm;
			mSpaceResults.push_back(res);
		}

		inputFile.close();

		// Compute building-level aggregate results based on mSpaceResults
		int scoreDF = 0; 
		int scoreDt = 0; 
		int scoreDtm = 0;
		double TotalScore = 0.0;
		double ADFbuilding = 0.0;
		double ADFbuilding_inverse = 0.0;

		for (const auto& s : mSpaceResults)
		{
			if (s.mPassDF)  { ++scoreDF; }
			if (s.mPassDt)  { ++scoreDt; }
			if (s.mPassDtm) { ++scoreDtm; }
			TotalScore += s.mScore;	
			ADFbuilding += s.mADF;
			ADFbuilding_inverse += s.mADFInverse;
		}
		
		const double n = static_cast<double>(mSpaceResults.size());
		if (n > 0.0)
		{
			mBuildingResults.mDFPassbuilding = (100.0/n) * static_cast<double>(scoreDF);
			mBuildingResults.mDtBuilding     = (100.0/n) * static_cast<double>(scoreDt);
			mBuildingResults.mDtmBuilding    = (100.0/n) * static_cast<double>(scoreDtm);
		}
		mBuildingResults.mTotalScore      = TotalScore;
		mBuildingResults.mADFBuilding     = ADFbuilding;
		mBuildingResults.mADFBuildingInverse = ADFbuilding_inverse;

		mResultsGenerated = true;
	} // generateResults()

	dl_building_results dl_model::getTotalResults()
	{
		// If results have not been generated or there are no spaces, return default-initialized result
		if (!mResultsGenerated || mSpaceResults.empty())
		{
			return dl_building_results{};
		}
		return mBuildingResults;
	}; //getTotalResults

	dl_building_results dl_model::getPartialResults(unsigned int& mID)
	{
		dl_building_results resultsDL;
		if (!mResultsGenerated || mSpaceResults.empty())
		{
			return resultsDL;
		}

		for (const auto& s : mSpaceResults)
		{
			if (s.mSpaceID == mID)
			{
				resultsDL.mDtBuilding         = s.mDtPercentage;
				resultsDL.mDtmBuilding        = s.mDtmPercentage;
				resultsDL.mTotalScore         = s.mScore;
				resultsDL.mADFBuilding        = s.mADF;
				resultsDL.mADFBuildingInverse = s.mADFInverse;
				break;
			}
		}
		return resultsDL;
	}; //getPartialResults

	const dl_space_result& dl_model::getSpaceResult(unsigned int& mID) const
	{
		// If results have not been generated (analyze not called), return empty default.
		static const dl_space_result emptyResult;
		if (!mResultsGenerated || mSpaceResults.empty())
		{
			return emptyResult;
		}
		// Return reference to the matching space result if found.
		for (const auto& s : mSpaceResults)
		{
			if (s.mSpaceID == mID)
			{
				return s;
			}
		}
		// If not found, return a static default-initialized result.
		return emptyResult;
	}

} // namespace daylight
} // namespace bso

#endif // DL_MODEL_CPP
