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
		std::vector<std::tuple<unsigned int, int>> SpaceID_AmountGridPoints;	// safe it, in case copy consttructor is called
		std::vector<double> DF_Settings;	// safe it, in case copy consttructor is called
		std::vector<double> DF_Evaluation;	// safe it, in case copy consttructor is called
		std::string folderName;	// safe it, in case copy consttructor is called
		std::vector<std::vector<double>> DFresults; //[0]=SpaceID, [1]=ADF [2]=Dt% [3]=Dtm% [4]=Score // safe it, in case copy consttructor is called
		
		std::vector<std::vector<double>> roomInfo; /* SpaceID_MinZ_EI where E=external==0 or I=internal==1*/
		//for space seperating wall rectangle for histogram // safe it, in case copy consttructor is called

		//for visualization
		std::vector<std::map<bso::spatial_design::conformal::cf_rectangle* /*rectangle*/, bool /*window Y/N*/>> mRectangleWindowYN; // safe it, in case copy consttructor is called
		std::vector<bso::spatial_design::conformal::cf_space*> mcf_space; // safe it, in case copy consttructor is called
		std::vector<std::vector<const bso::spatial_design::conformal::cf_rectangle*>> mCardinalDirections; // safe it, in case copy consttructor is called
		std::vector<const bso::spatial_design::conformal::cf_rectangle*> mCeilings;// safe it, in case copy consttructor is called

		public: 
		dl_model(); // emtry constructor --> initiate an object for naming but does not contain anything
		dl_model(const dl_model& rhs); // copy constructor --> describes how to copy an object
		~dl_model(); // destructor --> describes how to destroy an object; in this case destroy the directory and associated files for a Radiance run
		

		//set and get of DL object
		void setfolderName(const std::string& directoryname) {folderName = directoryname;}
		std::string getFolderName() const {return folderName;}
    	void setSpaceID_AmountGridPoints(const std::vector<std::tuple<unsigned int, int>>& points) {SpaceID_AmountGridPoints = points;}
		const std::vector<std::tuple<unsigned int, int>>& getSpaceID_AmountGridPoints() const {return SpaceID_AmountGridPoints;}
		void setDF_Settings(const std::vector<double>& settings) {DF_Settings = settings;}
		//const std::vector<double>& getDF_Settings() const {return DF_Settings;}
		void setDF_Evaluation(const std::vector<double>& evaluation) {DF_Evaluation = evaluation;}
		//const std::vector<double>& getDF_Evaluation() const {return DF_Evaluation;}
		void setDFresults(std::vector<std::vector<double>>& results) {DFresults = results;}

		void setRoomInfo(std::vector<std::vector<double>>& a) {roomInfo = a;}
		const std::vector<std::vector<double>>& getRoomInfo() const {return roomInfo;}
		
		//properties for viusalization
		void setWindowProperties(std::vector<std::map<bso::spatial_design::conformal::cf_rectangle* /*rectangle*/, bool /*window Y/N*/>>& rwYN) {mRectangleWindowYN =rwYN;}
		const std::vector<std::map<bso::spatial_design::conformal::cf_rectangle* /*rectangle*/, bool /*window Y/N*/>>& getWindowProperties() const {return mRectangleWindowYN;}
		void setSpacePropertiesQH(std::vector<bso::spatial_design::conformal::cf_space*>& temp_cf_space) {mcf_space = temp_cf_space;}
		const std::vector<bso::spatial_design::conformal::cf_space*> getSpacePropertiesQH() const { return mcf_space; }
		void setCardinalDirections(std::vector<std::vector<const bso::spatial_design::conformal::cf_rectangle*>>& directions) {mCardinalDirections = directions;}
		const std::vector<std::vector<const bso::spatial_design::conformal::cf_rectangle*>> getCardinalDirections() const {return mCardinalDirections;}
		void setCeilings(std::vector<const bso::spatial_design::conformal::cf_rectangle*>& ceilings) {mCeilings = ceilings;}
		const std::vector<const bso::spatial_design::conformal::cf_rectangle*> getCeilings() const {return mCeilings;}


		
		void analyse();
		std::string BuildingOrientationAdvice();
		
		/*
		This functions returns the results of the Radiance run, see BP and SD classes for examples on how this could be done.
		*/
		dl_results getTotalResults();
		dl_results getPartialResults(unsigned int& mID);

	}; //daylight class
	
	/*
	This struct below described the result output you obtain from your Radiance run and can be asked uppon by the function getTotalResults() or partialrsults;
	*/
	struct dl_results
	{
	double mDFpassbuilding = 0.0; //% rooms pass DF targets NEN
	double mDtbuilding = 0.0; //% rooms pass Dt 
	double mDtmbuilding = 0.0; //% rooms pass Dtm 
	double mTotalScore = 0.0; //cummulative sum of score per room. 
	double mADFbuilding = 0.0; //ADF score total
	double mADFroom = 0.0; //ADF per room for histogram
	};
	
	
} // namespace daylight
} // namespace bso


#include <bso/daylight/dl_model.cpp>

#endif //DL_MODEL_HPP