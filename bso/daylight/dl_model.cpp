#ifndef BSO_DL_MODEL_CPP
#define BSO_DL_MODEL_CPP

#include <filesystem>
#include <cstdlib>

namespace bso { namespace daylight {
	
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
	{
		
	} // ctor()
	
	dl_model::dl_model(const dl_model& rhs) 
	: SpaceID_AmountGridPoints(rhs.SpaceID_AmountGridPoints), DF_Settings(rhs.DF_Settings), DF_Evaluation(rhs.DF_Evaluation), folderName(rhs.folderName), 
		mRectangleWindowYN(rhs.mRectangleWindowYN), mcf_space(rhs.mcf_space), mCardinalDirections(rhs.mCardinalDirections), mCeilings(rhs.mCeilings),
		DFresults(rhs.DFresults), roomInfo(rhs.roomInfo)
	{

	} //copy constructor

	dl_model::~dl_model()
	{

	} // //dtor()	

	double roundToTwoDecimals(double num) 
	{
    	return std::round(num * 100) / 100;
	}

	double ScoreDFRoomNENminimize(double DtPercentage, double DtmPercentage)
	{
		/*
		if DtmPercentage >= 50 && DtPercentage >= 50 : Both targets met so 0
		if only DtmPercentage >= 50, score is 5 or lower, linearly interpolate score 0-5 depending on Dt (0-50%)
		if both are < 50%, linearly interpolate score 5-10 depending on Dtm (0-50%)
		*/

		double finalScore = 10;
		if (DtmPercentage >= 50 && DtPercentage >= 50) {finalScore = 0; }
		else if (DtmPercentage >= 50 && DtPercentage < 50) {finalScore = ( 5.0 - ((5.0/50.0)*DtPercentage)); }
		else if (DtmPercentage < 50 && DtPercentage < 50) {finalScore = 10.0 - ((5.0/50.0)*DtmPercentage); }
		return finalScore;
	}

	double ScoreDFRoomNENmaximize(double DtPercentage, double DtmPercentage)
	{
		/*
		if DtmPercentage >= 50 && DtPercentage >= 50 : Both targets met so 10/10
		if only DtmPercentage >= 50, score is atleast a 5, linearly interpolate score 5-10 depending on Dt (0-50%)
		if both are < 50%, linearly interpolate score 0-5 depending on Dtm (0-50%)
		*/

		double finalScore = 0;
		if (DtmPercentage >= 50 && DtPercentage >= 50) {finalScore = 10; }
		else if (DtmPercentage >= 50 && DtPercentage < 50) {finalScore = ( 5.0 + ((5.0/50.0)*DtPercentage)); }
		else if (DtmPercentage < 50 && DtPercentage < 50) {finalScore = ((5.0/50.0)*DtmPercentage); }
		return finalScore;
	}

	void dl_model::analyse()
	{
		const double HorizontalOutdoorIlluminance = DF_Settings[0];
		const std::string radianceBinPath = GetRadianceBinPath();
		
		// Set RAYPATH so Radiance can find .cal files
		SetRadianceCalPath();

		std::string command;
		
		// Generate sky
		const std::string irrad = std::to_string(HorizontalOutdoorIlluminance/179.0); //generate diffuse irradiance in watss/meter2 to derive the zenith brightness 
		const std::string skysettings = "-ang 45 0 -c -B "+irrad+" -g 0.2"; //-c=standard CIE overcast sky ,-B is zenetith brithness (irrad), -g ground reflectance
		command = radianceBinPath + "/gensky "+skysettings+" > ./"+folderName+"/sky.rad"; 
		if (std::system(command.c_str()) != 0) {
			std::cerr << "Failed to execute command: " << command << std::endl;
			return;
		}

		// Create outside
		std::ofstream outsideFile(folderName+"/"+"outside.rad", std::ios::trunc);
		if (!outsideFile.is_open()) {std::cerr << "Failed to open outside.rad for writing\n";}
		outsideFile << "skyfunc glow sky_glow\n0\n0\n4 1 1 1 0\n";
		outsideFile << "sky_glow source sky\n0\n0\n4 0 0 1 180\n";
		outsideFile << "skyfunc glow ground_glow\n0\n0\n4 1 1 1 0\n";
		outsideFile << "ground_glow source ground\n0\n0\n4 0 0 -1 180\n\n";
		outsideFile.close();

		// Compile scene
		command = radianceBinPath + "/oconv ./"+folderName+"/sky.rad ./"+folderName+"/outside.rad ./"+folderName+"/scene.rad > ./"+folderName+"/room.oct";
		if (std::system(command.c_str()) != 0) {
			std::cerr << "Failed to execute command: " << command << std::endl;
			return;
		}

		// Calculate illuminance
		std::string rtraceSettings = "-I -h -aa 0 -ab 8 -ad 4096 -as 1024 -lw 1e-6 -lr 16"; 
		//-I -h -aa 0 -ab 6 -ad 10240 -as 0 -lw 1e-6 -n 4       //Ward, 2024
		//-I -h -aa 0 -ab 8 -ad 4096 -as 1024 -lw 1e-6 -lr 16   //Kharvari, 2020
		command = radianceBinPath + "/rtrace "+ rtraceSettings + " -oov ./"+folderName+"/room.oct < ./"+folderName+"/grid.txt > ./"+folderName+"/results.txt";
		if (std::system(command.c_str()) != 0) {
			std::cerr << "Failed to execute command: " << command << std::endl;
			return;
		}

		// Make lux.
		command = radianceBinPath + "/rcalc -e '$1=179*(.265*$4+.670*$5+.065*$6)' ./"+folderName+"/results.txt > ./"+folderName+"/lux.txt";
		if (std::system(command.c_str()) != 0) {
			std::cerr << "Failed to execute command: " << command << std::endl;
			return;
		}

		// // Make DF
		// command = radianceBinPath + "/rcalc -e '$1=(100*179*(.265*$4+.670*$5+.065*$6))/"+std::to_string(HorizontalOutdoorIlluminance)+"' ./"+folderName+"/results.txt > ./"+folderName+"/DF.txt";
		// if (std::system(command.c_str()) != 0) {
		// 	std::cerr << "Failed to execute command: " << command << std::endl;
		// 	return;
		// }

	}; //analyse()

	dl_results dl_model::getTotalResults()
	{
		dl_results resultsDL;
		const double HorizontalOutdoorIlluminance = DF_Settings[0];
		const double Dt = DF_Evaluation[0];
		const double Dtm = DF_Evaluation[1];

		// Read output Radiance
		std::ifstream inputFile("./"+folderName+"/lux.txt");
		if (!inputFile.is_open()) {std::cerr << "Error opening luxfile. (./"+folderName+"/lux.txt)" << std::endl; return {};}

		double sum = 0.0;
		int count = 0;
		std::vector<std::vector<double>> DFresults; //[0]=SpaceID, [1]=ADF [2]=Dt% [3]=Dtm% [4]=Score
		std::vector<double> DFpass;
		std::vector<double> Dtpass;
		std::vector<double> Dtmpass;
	
		// iterate per surface / space
		//one surface = one set of points (amount of gridpoints varies per surface)
		for (int i = 0; i < SpaceID_AmountGridPoints.size(); ++i) 
		{
				auto& tup = SpaceID_AmountGridPoints[i];
				unsigned int SpaceID = std::get<0>(tup);
				int gridPoints = std::get<1>(tup);

				int requiredExceedingCount = static_cast<int>(std::ceil(gridPoints/2.0)); //round up
				int pointsExceedingDt = 0;
				int pointsExceedingDtm = 0;

				//iterate over each gridpoint per surface (space)
				for (int j = 0; j < gridPoints; ++j) 
				{
					double Espace; // horizontal illumance at points at target level
					if (!(inputFile >> Espace)) {std::cerr << "Error reading Espace(lx) number from file." << std::endl; return {};}
					
					double DF = (Espace/HorizontalOutdoorIlluminance)*100; //daylight factor per point
					if (DF > Dt) {pointsExceedingDt++;}
					if (DF > Dtm) {pointsExceedingDtm++;}
					sum += DF;
				} //gridpoint per surface

			double ADF = roundToTwoDecimals((sum/gridPoints)); 
			int DFspace;
			double DtPercentage = static_cast<double>(std::ceil((100/gridPoints)*pointsExceedingDt)); //per room 
			double DtmPercentage = static_cast<double>(std::ceil((100/gridPoints)*pointsExceedingDtm));
			int resultDt = (pointsExceedingDt >= requiredExceedingCount) ? 1 : 0;
			int resultDtm = (pointsExceedingDtm >= requiredExceedingCount) ? 1 : 0;

			if (resultDt==1 && resultDtm==1) {DFspace = 1; //fullfills
			} else {DFspace = 0;} //does not fullfill

			DFpass.push_back(DFspace); //stores wether or not space fullfills requirements
			Dtpass.push_back(resultDt);
			Dtmpass.push_back(resultDtm);

			//[0]=SpaceID, [1]=ADF [2]=Dt% [3]=Dtm% [4]=Score(x/10)
			DFresults.push_back(std::vector<double>(5));
			DFresults[i][0] = SpaceID;
			DFresults[i][1] = ADF;
			DFresults[i][2] = DtPercentage;
			DFresults[i][3] = DtmPercentage;
			DFresults[i][4] = ScoreDFRoomNENminimize(DtPercentage, DtmPercentage);

			pointsExceedingDt = 0;
			pointsExceedingDtm = 0;
			sum = 0;
			ADF = 0.0;

		} //close loop that iterates per space
		setDFresults(DFresults);

		// Close the file
		inputFile.close();

		int scoreDF = 0; 
		int scoreDt = 0; 
		int scoreDtm = 0;
		double TotalScore = 0.0;
		double ADFbuilding = 0.0;


		for (int i = 0; i < DFpass.size(); ++i) { scoreDF += DFpass[i];}
		for (int i = 0; i < Dtpass.size(); ++i) { scoreDt += Dtpass[i];}
		for (int i = 0; i < Dtmpass.size(); ++i) { scoreDtm += Dtmpass[i];}
		for (int i = 0; i < DFresults.size(); ++i) {TotalScore += DFresults[i][4];}
		for (int i = 0; i < DFresults.size(); ++i) 
			{ if (DFresults[i][1] == 0) 
				{
					ADFbuilding += 0;
				} 
				else 
				{	
					ADFbuilding += (1 / DFresults[i][1]);
				}
			} 
		
		double DFPassBuilding = ((100/DFpass.size())*scoreDF);
		double DFPassDt = ((100/Dtpass.size())*scoreDt);
		double DFPassDtm = ((100/Dtmpass.size())*scoreDtm);

		if (ADFbuilding==0){ADFbuilding = std::numeric_limits<double>::infinity();} //infity bc devision by zero

		resultsDL.mDFpassbuilding = DFPassBuilding;
		resultsDL.mDtbuilding = DFPassDt;
		resultsDL.mDtmbuilding = DFPassDtm;
		resultsDL.mTotalScore = TotalScore;
		resultsDL.mADFbuilding = ADFbuilding;
		
		
		return resultsDL;
	}; //getTotalResults

	dl_results dl_model::getPartialResults(unsigned int& mID)
	{
	//-----------same as getTotalResults()----------------------------------------------------------//
		dl_results resultsDL;
		const double HorizontalOutdoorIlluminance = DF_Settings[0];
		const double Dt = DF_Evaluation[0];
		const double Dtm = DF_Evaluation[1];

		// Read output Radiance
		std::ifstream inputFile("./"+folderName+"/lux.txt");
		if (!inputFile.is_open()) {std::cerr << "Error opening luxfile. (./"+folderName+"/lux.txt)" << std::endl; return {};}

		double sum = 0.0;
		int count = 0;
		std::vector<std::vector<double>> DFresults; //[0]=SpaceID, [1]=ADF [2]=Dt% [3]=Dtm% [4]=Score
		std::vector<double> DFpass;
		std::vector<double> Dtpass;
		std::vector<double> Dtmpass;
	
		// iterate per surface / space
		//one surface = one set of points (amount of gridpoints varies per surface)
		for (int i = 0; i < SpaceID_AmountGridPoints.size(); ++i) 
		{
				auto& tup = SpaceID_AmountGridPoints[i];
				unsigned int SpaceID = std::get<0>(tup);
				int gridPoints = std::get<1>(tup);

				int requiredExceedingCount = static_cast<int>(std::ceil(gridPoints/2.0)); //round up
				int pointsExceedingDt = 0;
				int pointsExceedingDtm = 0;

				//iterate over each gridpoint per surface (space)
				for (int j = 0; j < gridPoints; ++j) 
				{
					double Espace; // horizontal illumance at points at target level
					if (!(inputFile >> Espace)) {std::cerr << "Error reading Espace(lx) number from file." << std::endl; return {};}
					
					double DF = (Espace/HorizontalOutdoorIlluminance)*100; //daylight factor per point
					if (DF > Dt) {pointsExceedingDt++;}
					if (DF > Dtm) {pointsExceedingDtm++;}
					sum += DF;
				} //gridpoint per surface

			double ADF = roundToTwoDecimals((sum/gridPoints)); 
			int DFspace;
			double DtPercentage = static_cast<int>(std::ceil((100/gridPoints)*pointsExceedingDt)); //per room 
			double DtmPercentage = static_cast<int>(std::ceil((100/gridPoints)*pointsExceedingDtm));
			int resultDt = (pointsExceedingDt >= requiredExceedingCount) ? 1 : 0;
			int resultDtm = (pointsExceedingDtm >= requiredExceedingCount) ? 1 : 0;

			if (resultDt==1 && resultDtm==1) {DFspace = 1; //fullfills
			} else {DFspace = 0;} //does not fullfill

			DFpass.push_back(DFspace); //stores wether or not space fullfills requirements
			Dtpass.push_back(resultDt);
			Dtmpass.push_back(resultDtm);

			//[0]=SpaceID, [1]=ADF [2]=Dt% [3]=Dtm% [4]=Score(x/10)
			DFresults.push_back(std::vector<double>(5));
			DFresults[i][0] = SpaceID;
			DFresults[i][1] = ADF;
			DFresults[i][2] = DtPercentage;
			DFresults[i][3] = DtmPercentage;
			DFresults[i][4] = ScoreDFRoomNENminimize(DtPercentage, DtmPercentage);

			pointsExceedingDt = 0;
			pointsExceedingDtm = 0;
			sum = 0;
			ADF = 0.0;

		} //close loop that iterates per space
		setDFresults(DFresults);
		// Close the file
		inputFile.close();
		//-----------end same as getTotalResults()----------------------------------------------------------//


		//ask for result per space
			for (int q=0; q < DFresults.size(); ++q)
			{
				if (DFresults[q][0] == mID)
					{
						//[0]=SpaceID, [1]=ADF [2]=Dt% [3]=Dtm% [4]=Score(x/10)
						resultsDL.mDtbuilding = DFresults[q][2];
						resultsDL.mDtmbuilding = DFresults[q][3];
						resultsDL.mTotalScore = DFresults[q][4];
						resultsDL.mADFbuilding = (DFresults[q][1] == 0) ? 1000000 : (1/(DFresults[q][1]));
						resultsDL.mADFroom = DFresults[q][1];
						break;
					}
			}
		
		return resultsDL;
	}; //getPartialResults

	std::string dl_model::BuildingOrientationAdvice()
	{
			double ADFaverageNorth;
			double ADFaverageEast;
			double ADFaverageSouth;
			double ADFaverageWest;

			for (size_t i=0; i< mCardinalDirections.size(); ++i)
			{
				std::vector<double> ADFperSpace;
				for (const bso::spatial_design::conformal::cf_rectangle* rect : mCardinalDirections[i])
				{
					double SpaceID;
					for (const auto j: (rect->cfSurfaces()))
					{
						SpaceID = (((j->cfSpaces())[0])->getSpaceID());
					}

					for (int j=0; j<DFresults.size(); ++j) //std::vector<std::vector<double>> DFresults; //per [i] : [i][0]=SpaceID, [1]=ADF [2]=Dt% [3]=Dtm% [4]=Score
					{
						if (SpaceID == DFresults[j][0])
						{
							ADFperSpace.push_back(DFresults[j][1]);
							break;
						}
					}
				}

				double sum = std::accumulate(ADFperSpace.begin(), ADFperSpace.end(), 0.0);
				double average = (ADFperSpace.size() > 0) ? sum / ADFperSpace.size() : 0.0;
			
				if (i==0) {ADFaverageNorth = average;}
				if (i==1) {ADFaverageEast = average;}
				if (i==2) {ADFaverageSouth = average;}
				if (i==3) {ADFaverageWest = average;}
			}

			double max[4] = {ADFaverageNorth, ADFaverageEast, ADFaverageSouth, ADFaverageWest};
			std::sort(max, max+4);

			//max ADF should be north side
			//if there equal values the max, order of below is leading
			std::ostringstream oss;	
			oss << std::fixed << std::setprecision(2);  // Set the precision to 2 decimal places
			std::string ADFperSide;

			if (max[3] == ADFaverageSouth)
			{
				std::vector<std::vector<const bso::spatial_design::conformal::cf_rectangle*>> temp(4);
				temp[2] = mCardinalDirections[0]; 
				temp[3] = mCardinalDirections[1];
				temp[0] = mCardinalDirections[2];
				temp[1] = mCardinalDirections[3];
				mCardinalDirections = temp;

				oss << "ADF per Cardinal Direction: "
					<< "North: " << ADFaverageSouth
					<< ", East: " << ADFaverageWest
					<< ", South: " << ADFaverageNorth
					<< " and West: " << ADFaverageEast;
				ADFperSide = oss.str();
			}
			else if (max[3] == ADFaverageWest) 
			{
				std::vector<std::vector<const bso::spatial_design::conformal::cf_rectangle*>> temp(4);
				
				temp[2] = mCardinalDirections[1];
				temp[3] = mCardinalDirections[2];
				temp[0] = mCardinalDirections[3];
				temp[1] = mCardinalDirections[0];
				mCardinalDirections = temp;

				oss << "ADF per Cardinal Direction: "
					<< "North: " << ADFaverageEast
					<< ", East: " << ADFaverageSouth
					<< ", South: " << ADFaverageWest
					<< " and West: " << ADFaverageNorth;
				ADFperSide = oss.str();
			}
			else if (max[3] == ADFaverageEast) 
			{
				std::vector<std::vector<const bso::spatial_design::conformal::cf_rectangle*>> temp(4);			
				temp[2] = mCardinalDirections[3];
				temp[3] = mCardinalDirections[0];
				temp[0] = mCardinalDirections[1];
				temp[1] = mCardinalDirections[2];
				mCardinalDirections = temp;

				oss << "ADF per Cardinal Direction: "
					<< "North: " << ADFaverageWest
					<< ", East: " << ADFaverageNorth
					<< ", South: " << ADFaverageEast
					<< " and West: " << ADFaverageSouth;
				ADFperSide = oss.str();
			}
			else if (max[3] == ADFaverageNorth) 
			{
				//nothings happens, cardinal directions (advice) are already in correct order
				oss << "ADF per Cardinal Direction: "
					<< "North: " << ADFaverageNorth
					<< ", East: " << ADFaverageEast
					<< ", South: " << ADFaverageSouth
					<< " and West: " << ADFaverageWest;
				ADFperSide = oss.str();
			}
				

	return ADFperSide;

	};//cardinal directions

} // namespace daylight
} // namespace bso

#endif // BSO_DL_MODEL_CPP