#ifndef DL_MODEL_HPP
#define DL_MODEL_HPP

#include <ostream>
#include <sstream>
#include <tuple>
#include <map>
#include <chrono> //measure time to run
// #include <string>
// #include <iomanip>

namespace bso { namespace daylight {

/*
This class allows for generation of radiance model: a 3D geometry based on the BSD and a grid onto which
the daylight calculation will perform. 

- note that in order for this class to function, in your current directory you need to 
	have installed
		* radiance
		* radiance-6.0.cc0d3bfe86-Linux directory
		* rayinit.cal
		* skybright.cal
		* window.cal
		* winxmit.cal
*/
	struct dl_results;

	class dl_model
	{
		private: 
		// Bellow, all variables are stored in case the copy constructor is called and the dl_model needs to be copied	
		std::vector<std::tuple<unsigned int, int>> SpaceID_AmountGridPoints;	
		std::vector<double> DF_Settings;	
		std::vector<double> DF_Evaluation;	
		std::string folderName;	
		std::vector<std::vector<double>> DFresults; //[0]=SpaceID, [1]=ADF [2]=Dt% [3]=Dtm% [4]=Score 
		
		std::vector<std::vector<double>> roomInfo; /* SpaceID_MinZ_EI where E=external==0 or I=internal==1*/
		//for space separating wall rectangle for histogram 

		// For visualization
		std::vector<std::map<bso::spatial_design::conformal::cf_rectangle* /*rectangle*/, bool /*window Y/N*/>> mRectangleWindowYN; 
		std::vector<bso::spatial_design::conformal::cf_space*> mcf_space; 
		std::vector<std::vector<const bso::spatial_design::conformal::cf_rectangle*>> mCardinalDirections; 
		std::vector<const bso::spatial_design::conformal::cf_rectangle*> mCeilings;

		public: 
		dl_model(); // emtry constructor
		dl_model(const dl_model& rhs); // copy constructor
		~dl_model(); // destructor --> destroy the directory and associated files for a Radiance run
		
		// Set and get of DL object
		void setfolderName(const std::string& directoryname) {folderName = directoryname;}
		std::string getFolderName() const {return folderName;}
    	void setSpaceID_AmountGridPoints(const std::vector<std::tuple<unsigned int, int>>& points) {SpaceID_AmountGridPoints = points;}
		const std::vector<std::tuple<unsigned int, int>>& getSpaceID_AmountGridPoints() const {return SpaceID_AmountGridPoints;}
		void setDF_Settings(const std::vector<double>& settings) {DF_Settings = settings;}
		void setDF_Evaluation(const std::vector<double>& evaluation) {DF_Evaluation = evaluation;}
		void setDFresults(std::vector<std::vector<double>>& results) {DFresults = results;}

		void setRoomInfo(std::vector<std::vector<double>>& a) {roomInfo = a;}
		const std::vector<std::vector<double>>& getRoomInfo() const {return roomInfo;}
		
		// Set properties for visualization
		void setWindowProperties(std::vector<std::map<bso::spatial_design::conformal::cf_rectangle* /*rectangle*/, bool /*window Y/N*/>>& rwYN) {mRectangleWindowYN =rwYN;}
		const std::vector<std::map<bso::spatial_design::conformal::cf_rectangle* /*rectangle*/, bool /*window Y/N*/>>& getWindowProperties() const {return mRectangleWindowYN;}
		void setSpacePropertiesQH(std::vector<bso::spatial_design::conformal::cf_space*>& temp_cf_space) {mcf_space = temp_cf_space;}
		const std::vector<bso::spatial_design::conformal::cf_space*> getSpacePropertiesQH() const { return mcf_space; }
		void setCardinalDirections(std::vector<std::vector<const bso::spatial_design::conformal::cf_rectangle*>>& directions) {mCardinalDirections = directions;}
		const std::vector<std::vector<const bso::spatial_design::conformal::cf_rectangle*>> getCardinalDirections() const {return mCardinalDirections;}
		void setCeilings(std::vector<const bso::spatial_design::conformal::cf_rectangle*>& ceilings) {mCeilings = ceilings;}
		const std::vector<const bso::spatial_design::conformal::cf_rectangle*> getCeilings() const {return mCeilings;}

		// Functions to request analysis
		void analyse();
		std::string BuildingOrientationAdvice();
		
		// This functions returns the results of the Radiance run, see BP and SD classes for examples on how this could be done.
		dl_results getTotalResults();
		dl_results getPartialResults(unsigned int& mID);

	}; //daylight class
	
	/*
	//This struct below described the result output you obtain from your Radiance run
	//and can be asked upon by the function getTotalResults() or partialresults(unsigned int& mID)
	mDFpassbuilding =  the % of rooms which pass the DF targets from the NEN-norms
	mDtbuilding = the % of rooms which pass the Dt 
	mDtmbuilding = the % of rooms which pass Dtm 
	mTotalScore = the cumulative sum of score per room.
	mADFbuilding = the ADF score of total building
	mADFroom = the ADF per room for a histogram
	*/
	struct dl_results
	{
	double mDFpassbuilding = 0.0; 
	double mDtbuilding = 0.0;  
	double mDtmbuilding = 0.0;  
	double mTotalScore = 0.0; 
	double mADFbuilding = 0.0; 
	double mADFroom = 0.0; 
	};
	
	
} // namespace daylight
} // namespace bso


#include <bso/daylight/dl_model.cpp>

#endif //DL_MODEL_HPP