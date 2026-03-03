#ifndef DL_MODEL_HPP
#define DL_MODEL_HPP

#include <ostream>
#include <sstream>
#include <tuple>
#include <map>
#include <vector>
#include <string>
#include <chrono> //measure time to run
// #include <bso/daylight/properties/surface_properties.hpp>
// #include <bso/daylight/geometry/floor.hpp>
// #include <bso/daylight/geometry/ceiling.hpp>
// #include <bso/daylight/geometry/wall.hpp>
// #include <bso/daylight/geometry/window.hpp>
// #include <bso/daylight/geometry/space.hpp>
// #include <string>
// #include <iomanip>

namespace bso { namespace daylight {

/*
This class allows for generation of radiance model: a 3D geometry based on the BSD and a grid onto which
the daylight calculation will perform. 

- note that the radiance files are automatically located from the BSO library installation:
	* radiance binaries in bso/daylight/radiance/bin/
	* Calibration files (.cal) in bso/daylight/radiance/cal/
	However, the libraries are specific to a linux system with:
	✓ 64-bit Linux systems (x86-64 architecture)
	✓ Modern Linux distributions (Ubuntu, Debian, Fedora, etc.)
	✓ Kernel version 3.2.0 or newer
	✓ Standard C library (glibc) - no special dependencies
	If another system is used, the radiance binaries and .cal files should be manually added
	to the system and the paths in dl_model.cpp should be adjusted accordingly.
	The current version of radiance used in the BSO toolbox is: 
	Radiance version 6.0a
	Build date: 2024-03-03
	Build ID: cc0d3bfe86
*/
	struct dl_results;

	class dl_model
	{
		private: 
		// Bellow, all variables are stored in case the copy constructor is called and the dl_model needs to be copied	
		std::vector<std::tuple<unsigned int, int>> SpaceID_AmountGridPoints;	
		std::vector<double> DF_Settings;	
		std::vector<double> DF_Evaluation;
		std::string mFolderName;	// Contains both input and output files for the radiance run.
		std::vector<std::vector<double>> DFresults; //[0]=SpaceID, [1]=ADF [2]=Dt% [3]=Dtm% [4]=Score 
		
		std::vector<std::vector<double>> roomInfo; /* SpaceID_MinZ_EI where E=external==0 or I=internal==1*/
		//for space separating wall rectangle for histogram 

		// For visualization
		std::vector<std::map<bso::spatial_design::conformal::cf_rectangle* /*rectangle*/, bool /*window Y/N*/>> mRectangleWindowYN; 
		std::vector<bso::spatial_design::conformal::cf_triangle*> mTriangles; // floor/ceiling triangles for visualization
		std::vector<bso::spatial_design::conformal::cf_space*> mcf_space;  
		std::vector<const bso::spatial_design::conformal::cf_rectangle*> mCeilings;
		std::vector<const bso::spatial_design::conformal::cf_triangle*> mCeilingsTri;
		std::vector<bso::utilities::geometry::vertex> mGridPoints; // grid points for visualization

		// // Specialized radiance geometry entities to support alternative grammar definitions.
		// std::vector<floor> mFloorEntities;
		// std::vector<ceiling> mCeilingEntities;
		// std::vector<wall> mWallEntities;
		// std::vector<window> mWindowEntities;
		// std::vector<space> mSpaceEntities;

		// // Model settings which are set in the dl_setting file
		// double horizontal_outdoor_illuminance; // [lx] described horizontal outdoor illuminance, used for daylight factor calculation, can be set in the dl_setting file. Typically 14400lx according to NEN.
		// double reference_height; // [mm] described height from the floor at which the grid points for illuminance evaluation is placed, typically 850mm for work plane according to NEN.
		// double space_margin; // [mm] described margin of space around the room, used for daylight factor calculation, can be set in the dl_setting file. Typically 500mm according to NEN.
		// double minGridSize; // [mm] minimum grid size for grid generation, used for daylight factor calculation, can be set in the dl_setting file. Typically 10.0m according to NEN.
		// double mDaylightfactor_target; // [%] described daylight factor target for the building, used for score calculation, can be set in the dl_setting file. Typically x% according to NEN.
		// double Daylightfactor_minimum_target; // [%] described minimum daylight factor target for the building, used for score calculation, can be set in the dl_setting file. Typically y% according to NEN.

		public: 
		dl_model(); // emtry constructor
		dl_model(const dl_model& rhs); // copy constructor
		~dl_model(); // destructor --> destroy the directory and associated files for a Radiance run
		
		// Set and get of DL object
		std::string createAndSetFolder(const std::string& settingFileName, const std::string& folderName = ""); // creates a folder for the radiance run and saves the setting file into it, if foldername is not provided, creates one based on the count of runs. Returns the name of the created folder.
		std::string getFolderName() const {return mFolderName;}
    	void setSpaceID_AmountGridPoints(const std::vector<std::tuple<unsigned int, int>>& points) {SpaceID_AmountGridPoints = points;}
		const std::vector<std::tuple<unsigned int, int>>& getSpaceID_AmountGridPoints() const {return SpaceID_AmountGridPoints;}
		void setDF_Settings(const std::vector<double>& settings) {DF_Settings = settings;}
		void setDF_Evaluation(const std::vector<double>& evaluation) {DF_Evaluation = evaluation;}
		void setDFresults(std::vector<std::vector<double>>& results) {DFresults = results;}

		void setRoomInfo(std::vector<std::vector<double>>& a) {roomInfo = a;}
		const std::vector<std::vector<double>>& getRoomInfo() const {return roomInfo;}
		void setGridPoints(std::vector<bso::utilities::geometry::vertex>& gridPoints) {mGridPoints = gridPoints;}
		const std::vector<bso::utilities::geometry::vertex>& getGridPoints() const {return mGridPoints;}
		
		// Set properties for visualization
		void setWindowProperties(std::vector<std::map<bso::spatial_design::conformal::cf_rectangle* /*rectangle*/, bool /*window Y/N*/>>& rwYN) {mRectangleWindowYN =rwYN;}
		const std::vector<std::map<bso::spatial_design::conformal::cf_rectangle* /*rectangle*/, bool /*window Y/N*/>>& getWindowProperties() const {return mRectangleWindowYN;}
		void setSpacePropertiesQH(std::vector<bso::spatial_design::conformal::cf_space*>& temp_cf_space) {mcf_space = temp_cf_space;}
		const std::vector<bso::spatial_design::conformal::cf_space*> getSpacePropertiesQH() const { return mcf_space; }
		void setCeilings(std::vector<const bso::spatial_design::conformal::cf_rectangle*>& ceilings) {mCeilings = ceilings;}
		const std::vector<const bso::spatial_design::conformal::cf_rectangle*> getCeilings() const {return mCeilings;}
		void setCeilingsTri(std::vector<const bso::spatial_design::conformal::cf_triangle*>& ceilingsTri) {mCeilingsTri = ceilingsTri;}
		const std::vector<const bso::spatial_design::conformal::cf_triangle*> getCeilingsTri() const {return mCeilingsTri;}
		void setTriangles(std::vector<bso::spatial_design::conformal::cf_triangle*>& triangles) {mTriangles = triangles;}
		const std::vector<bso::spatial_design::conformal::cf_triangle*> getTriangles() const {return mTriangles;}

		// void addFloorEntity(const floor& floorEntity) {mFloorEntities.push_back(floorEntity);}
		// void addCeilingEntity(const ceiling& ceilingEntity) {mCeilingEntities.push_back(ceilingEntity);}
		// void addWallEntity(const wall& wallEntity) {mWallEntities.push_back(wallEntity);}
		// void addWindowEntity(const window& windowEntity) {mWindowEntities.push_back(windowEntity);}
		// void addSpaceEntity(const space& spaceEntity) {mSpaceEntities.push_back(spaceEntity);}
		// void clearSpecializedGeometry()
		// {
		// 	mFloorEntities.clear();
		// 	mCeilingEntities.clear();
		// 	mWallEntities.clear();
		// 	mWindowEntities.clear();
		// 	mSpaceEntities.clear();
		// }
		// const std::vector<floor>& getFloorEntities() const {return mFloorEntities;}
		// const std::vector<ceiling>& getCeilingEntities() const {return mCeilingEntities;}
		// const std::vector<wall>& getWallEntities() const {return mWallEntities;}
		// const std::vector<window>& getWindowEntities() const {return mWindowEntities;}
		// const std::vector<space>& getSpaceEntities() const {return mSpaceEntities;}

		// Functions to request analysis
		void analyse();
		
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