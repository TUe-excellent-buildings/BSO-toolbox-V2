#ifndef BSO_GRAMMAR_HPP
#define BSO_GRAMMAR_HPP

#include <bso/spatial_design/cf_building.hpp>
#include <bso/grammar/rule_set/rule_sets.hpp>
#include <bso/spatial_design/conformal/cf_building_model.hpp>

namespace bso { namespace grammar {


#ifdef SD_MODEL_HPP
class DEFAULT_SD_GRAMMAR;
#endif

#ifdef BP_MODEL_HPP
class DEFAULT_BP_GRAMMAR;
#endif

//#ifdef DL_MODEL_HPP //Hier word de class benoemt zodat de linker weet dat deze class bestaat
class DEFAULT_DL_GRAMMAR;
//#endif

class grammar
{
private:
	spatial_design::cf_building mCFBuilding;

	// rule sets
	std::map<spatial_design::conformal::cf_vertex*, rule_set::vertex_property*> mVertexProperties;
	std::map<spatial_design::conformal::cf_line*, rule_set::line_property*> mLineProperties;
	std::map<spatial_design::conformal::cf_rectangle*, rule_set::rectangle_property*> mRectangleProperties;
	std::map<spatial_design::conformal::cf_triangle*, rule_set::triangle_property*> mTriangleProperties;
	std::map<spatial_design::conformal::cf_cuboid*, rule_set::cuboid_property*> mCuboidProperties;
	std::map<spatial_design::conformal::cf_triPrism*, rule_set::triprism_property*> mTriprismProperties;

	std::map<spatial_design::conformal::cf_point*, rule_set::point_property*> mPointProperties;
	std::map<spatial_design::conformal::cf_edge*, rule_set::edge_property*> mEdgeProperties;
	std::map<spatial_design::conformal::cf_surface*, rule_set::surface_property*> mSurfaceProperties;
	std::map<spatial_design::conformal::cf_space*, rule_set::space_property*> mSpaceProperties;

	// Holds the sets of window corner-vertices either four for rectangles or three for triangles for each space, 
	// inserted in the conformal model so the rectangles/triangles in the conformal model reflects these boundaries.
	// It is stored here so windows vs. wall can be identified based on the presence of these corner-vertices in the rectangle/triangle.
	std::map<unsigned int, std::vector<bso::utilities::geometry::vertex>> mWindowsPerSpace;

	#ifdef SD_MODEL_HPP // variables and functions related to the structural design grammar
	std::map<spatial_design::conformal::cf_vertex*, rule_set::sd_vertex_rule*> mSDVertexRules;
	std::map<spatial_design::conformal::cf_line*, rule_set::sd_line_rule*> mSDLineRules;
	std::map<spatial_design::conformal::cf_rectangle*, rule_set::sd_rectangle_rule*> mSDRectangleRules;
	std::map<spatial_design::conformal::cf_triangle*, rule_set::sd_triangle_rule*> mSDTriangleRules;
	std::map<spatial_design::conformal::cf_cuboid*, rule_set::sd_cuboid_rule*> mSDCuboidRules;
	//std::map<spatial_design::conformal::cf_triPrism*, rule_set::sd_triprism_rule*> mSDTriprismRules; // for future if solid element representation NOBSD is required

	structural_design::sd_model mSDModel;
	std::vector<structural_design::sd_model> mIntermediateSDModels;
	std::map<std::pair<std::string,std::string>,
		structural_design::component::structure> mSDWallProperties;
	std::map<std::pair<std::string,std::string>,
		structural_design::component::structure> mSDFloorProperties;
    std::map<std::string, structural_design::component::structure> mSDSpaceProperties;
	structural_design::component::structure mLoadPanel;
	std::multimap<std::string, structural_design::component::load> mLoads;
	unsigned int mMeshSize = 1;

	void mReadSDSettings(const std::string& fileName); // see file grammar_sd_specifics.cpp for content of this function
	#endif

	#ifdef BP_MODEL_HPP // variables and functions related to the building physics grammar
	std::map<spatial_design::conformal::cf_space*, rule_set::bp_space_rule*> mBPSpaceRules;
	std::map<spatial_design::conformal::cf_rectangle*, rule_set::bp_rectangle_rule*> mBPRectangleRules;
	std::map<spatial_design::conformal::cf_triangle*, rule_set::bp_triangle_rule*> mBPTriangleRules;

	boost::posix_time::time_duration mBPWarmUpDuration;
	boost::posix_time::time_duration mBPTimeStepSize;
	double mBPGroundTemperature = 0;
	std::map<std::string,boost::posix_time::time_period> mBPSimulationPeriods;
	std::map<std::string,bso::building_physics::properties::space_settings> mBPSpaceSettings;
	std::map<std::pair<std::string,std::string>,bso::building_physics::properties::construction> mBPWallConstructions;
	std::map<std::pair<std::string,std::string>,bso::building_physics::properties::construction> mBPFloorConstructions;
	std::map<std::pair<std::string,std::string>,bso::building_physics::properties::glazing> mBPWallGlazings;
	std::map<std::pair<std::string,std::string>,bso::building_physics::properties::glazing> mBPFloorGlazings;

	building_physics::bp_model mBPModel;
	std::vector<building_physics::bp_model> mIntermediateBPModels;
	void mReadBPSettings(const std::string& fileName); // see file grammar_bp_specific.cpp for content of this function
	#endif
		
	#ifdef DL_MODEL_HPP // variables and functions related to the daylight grammar
	/*
	Store here the variables defined in the 'setting file' for the daylight grammar variables.
	These variables are obtained with the help of the function mReadDLSettings()
	which is defined in file grammar_dl_specific.cpp in directory /bso/grammar 

	Furthermore, store the object of the new class mDLRectangleRules here,
	since it uses if-statements to determine the properties of the geometry, 
	therefore is it a grammar and should the rules be places at the grammar class.
	This might seam as a complex method to structure thing and ofcourse we can discus other option, however,
	since the toolbox is already structured this way, it is also benificial to keep the current structure 
	as a constent to fit the bso-toolbox current structuring
	*/
	
	// dl parameters from settings file
	std::vector<std::string> mReflectanceValues;
	std::vector<std::string> mWindowProperties;
	std::vector<double> mWindowDimensions;
	std::vector<double> mDF_Settings; // [0] = ?, [1] = reference height, [2] = space margin, [3] = max grid size
	std::vector<double> mDF_Evaluation;
	std::vector<std::map<bso::spatial_design::conformal::cf_rectangle* /*rectangle*/, bool /*window Y/N*/>> mRectangleWindowYNGram = {};

	// rule sets/grammars to define the dl_model according to your rectangles
	std::map<spatial_design::conformal::cf_rectangle*, rule_set::dl_rectangle_rule*> mDLRectangleRules;

	// see file grammar_dl_specific.cpp for implementation of these functions
	void mReadDLSettings(const std::string& fileName); 
	#endif

public:
	grammar(const spatial_design::cf_building& cf);
	~grammar();

	#ifdef SD_MODEL_HPP
	template <typename T = DEFAULT_SD_GRAMMAR, typename...ARGS>
	bso::structural_design::sd_model sd_grammar(const ARGS&...);
	const std::vector<structural_design::sd_model>& getIntermediateSDModels() const
		{return mIntermediateSDModels;}
	#endif

	#ifdef BP_MODEL_HPP
	template <typename T = DEFAULT_BP_GRAMMAR, typename...ARGS>
	bso::building_physics::bp_model bp_grammar(const ARGS&...);
	const std::vector<building_physics::bp_model> getIntermediateBPModels() const
		{return mIntermediateBPModels;}
	#endif

	#ifdef DL_MODEL_HPP 
	template <typename T = DEFAULT_DL_GRAMMAR, typename...ARGS>
	bso::daylight::dl_model dl_grammar(const ARGS&...);
	
	#endif

	std::vector<bso::utilities::geometry::quadrilateral> getExteriorWalls();
	std::vector<bso::utilities::geometry::quadrilateral> getExteriorQuadrilaterals();
	std::vector<bso::spatial_design::conformal::cf_space*> getSpacesOnBoundary(bool spacesOnBottomIsAlsoBoundary = false);
};

} // namespace grammar
} // namespace bso

#include <bso/grammar/grammar.cpp>

#ifdef SD_MODEL_HPP
#include <bso/grammar/grammar_sd_specifics.cpp>
#include <bso/grammar/sd_grammars/default_sd_grammar.cpp>
#include <bso/grammar/sd_grammars/volume_sd_grammar.cpp>
#endif

#ifdef BP_MODEL_HPP
#include <bso/grammar/grammar_bp_specifics.cpp>
#include <bso/grammar/bp_grammars/default_bp_grammar.cpp>
#endif

#ifdef DL_MODEL_HPP 
#include <bso/grammar/grammar_dl_specifics.cpp>
#include <bso/grammar/dl_grammars/default_dl_grammar.cpp>
#endif

#endif // BSO_GRAMMAR_HPP
