#ifndef BSO_GRAMMAR_DEFAULT_DL_GRAMMAR_CPP
#define BSO_GRAMMAR_DEFAULT_DL_GRAMMAR_CPP

#include <stdexcept>
#include <sstream>

namespace bso { namespace grammar {

/*
Deze text omschrijf het setup en doel van de functie hieronder.
Deze functie hieronder is gedeclareerd in het public sectie van de grammar class, 
maar is hier gedefineerd.
De reden hiervoor is zodat er in de toekomst meerdere soorten grammar eventueel 
geschreven kunnen worden om een dl_model object te defineren op verschillende manieren.

const std::string& fileName is de naam van de daylight setting file.
The function below will generate a DL model. This contents preparing the radiance scene, based on the ms representation and settingfile. 
Several txt files are created onto which radiance operaties. 
*/
	
template <>
bso::daylight::dl_model grammar::dl_grammar<DEFAULT_DL_GRAMMAR>(const std::string& fileName,
	const bool& useSurfaceTypeInsteadOfSpaceType)
{	
	// initialize a new DL model
	bso::daylight::dl_model mDLModel;
	// read the daylight settings and store in the grammar object
	this->mReadDLSettings(fileName);

    //retrieve needed objects
	const std::vector<std::string> ReflectanceValues =  this->getReflectanceValuesQH();
	const std::vector<std::string> WindowProperties =  this->getWindowPropertiesQH();
	const std::vector<double> DF_Settings =  this->getDF_SettingsQH();
	const std::vector<double> DF_Evaluation =  this->getDF_EvaluationQH();
    std::vector<std::tuple<unsigned int, int>> SpaceID_AmountGridPoints;
    const std::vector<std::tuple<unsigned int, std::vector<bso::utilities::geometry::vertex>>> ConstrainsPerSpace = mCFBuilding.getConstrainsPerSpaceQH();


//generate daylight model 

	//make new folder into which you save all files constructed
    std::string folderName = bso::grammar::rule_set::dl_rule_set::createFolder(fileName);
    
    //Open scenefile
    std::ofstream sceneFile(folderName+"/"+"scene.rad", std::ios::trunc);
    if (!sceneFile.is_open()) {std::cerr << "Failed to open scene.rad for writing\n";}

    bso::grammar::rule_set::dl_rule_set::writeMaterials(sceneFile, ReflectanceValues, WindowProperties);
	
	std::vector<bso::spatial_design::conformal::cf_surface> cfSurface;
    std::vector<unsigned int> SpaceID;

    std::vector<std::map<bso::spatial_design::conformal::cf_rectangle* /*rectangle*/, bool /*window Y/N*/>> RectangleWindowYN;
    std::map<bso::spatial_design::conformal::cf_rectangle* /*rectangle*/, bool /*window Y/N*/> map;
    std::vector<const bso::spatial_design::conformal::cf_rectangle* /*rectangle*/> Rectangles = {};
    std::vector<const bso::spatial_design::conformal::cf_rectangle* /*rectangle*/> Ceilings = {};
    std::vector<std::vector<double>> roomInfo /*external==0 or internal==1*/ = {};
    std::vector<double> SpaceID_MinZ_EI(3);


    //loop trough rectangles
	for (const auto i: this->getRectanglePropertiesQH())
	{

		bso::utilities::geometry::quadrilateral* quad = i.first; //rectangles
        std::vector<bso::utilities::geometry::vertex> quadVertex; //points
        double min_z = std::numeric_limits<double>::max();
        for(const auto j: (*quad))
        { //save all vertexes of a rectangle
            quadVertex.push_back(j);
            double z_value = j[2];  // Extract the z-coordinate
            if (z_value < min_z) {min_z = z_value;}
        }        

        bso::spatial_design::conformal::cf_rectangle* cfRec = i.first;
        int SpaceIDforRoominfo;
        for (const auto j: (cfRec->cfSurfaces()))
            {
                SpaceIDforRoominfo=((j->cfSpaces())[0])->getSpaceID(); //retrieve corresponding spaceID
            }

		if ((i.second->isSpaceSeparating()==true)) //each rectangle forming a space
		{
			if (i.second->isExternal()==true)
            {
                if (i.second->isFloor())
                {   
                    bso::grammar::rule_set::dl_rule_set::writeFloorRadiance(sceneFile, quadVertex);
                    bso::grammar::rule_set::dl_rule_set::writeCeilingRadiance(sceneFile, quadVertex);
                    map[cfRec] = false;
                    Ceilings.push_back(cfRec);
 
                    //later for grid surfaces need to be created
                    for (const auto j: (cfRec->cfSurfaces()))
                    {
                        cfSurface.push_back(*j); //retrieve floor surfaces
                        SpaceID.push_back(((j->cfSpaces())[0])->getSpaceID()); //retrieve corresponding spaceID
                    }

                RectangleWindowYN.push_back(map);
                } //isfloor

                else if (i.second->isWall())
                {
                    SpaceID_MinZ_EI[0] = SpaceIDforRoominfo;
                    SpaceID_MinZ_EI[1] = min_z;
                    SpaceID_MinZ_EI[2] = 0;

                    //== external 
                    //&& == space seperating 
                    //&& == wall
                    //save rectangles for cardinal direction later on
                    {
                    const bso::spatial_design::conformal::cf_rectangle* rec = i.first;
                    Rectangles.push_back(rec);
                    }

                    bool isWindow = bso::grammar::rule_set::dl_rule_set::isQuadInWindowArea(quadVertex, ConstrainsPerSpace); //check window properties

                    if (isWindow == true) 
                    {
                            if (bso::grammar::rule_set::dl_rule_set::isOpenExternal(cfRec)==true)
                            {
                                bso::grammar::rule_set::dl_rule_set::writeWindowRadiance(sceneFile, quadVertex);
                                map[cfRec] = true;
                            }
                            else if (bso::grammar::rule_set::dl_rule_set::isOpenExternal(cfRec)==false)
                            {
                                bso::grammar::rule_set::dl_rule_set::writeWallRadiance(sceneFile, quadVertex);
                                map[cfRec] = false;
                            }
                            
                    } //has windowproperties

                    else if (isWindow == false) //check window properties
                    {
                        bso::grammar::rule_set::dl_rule_set::writeWallRadiance(sceneFile, quadVertex);
                        map[cfRec] = false;
                    }
                RectangleWindowYN.push_back(map);
                } //isWall
            } //is external

			else if (i.second->isExternal()==false) // is internal
            {
                if (i.second->isFloor())
                {
                    bso::grammar::rule_set::dl_rule_set::writeFloorRadiance(sceneFile, quadVertex);
                    bso::grammar::rule_set::dl_rule_set::writeCeilingRadiance(sceneFile, quadVertex);
                    map[cfRec] = false;

                    //later for grid surfaces already need to be created
                    for (const auto j: (cfRec->cfSurfaces()))
                    {
                        cfSurface.push_back(*j);
                        SpaceID.push_back(((j->cfSpaces())[0])->getSpaceID());
                    } 
                RectangleWindowYN.push_back(map);    
                } //isfloor

                else if (i.second->isWall())
                {
                    SpaceID_MinZ_EI[0] = SpaceIDforRoominfo;
                    SpaceID_MinZ_EI[1] = min_z;
                    SpaceID_MinZ_EI[2] = 1;

                    bool isWindow = bso::grammar::rule_set::dl_rule_set::isQuadInWindowArea(quadVertex, ConstrainsPerSpace); //check window properties

                    if (isWindow == true) 
                    {
                       if (bso::grammar::rule_set::dl_rule_set::isOpenInternal(cfRec)==true)
                       {
                            bso::grammar::rule_set::dl_rule_set::writeWindowRadiance(sceneFile, quadVertex);
                            map[cfRec] = true;
                       }
                       else if (bso::grammar::rule_set::dl_rule_set::isOpenInternal(cfRec)==false)
                       {
                            bso::grammar::rule_set::dl_rule_set::writeWallRadiance(sceneFile, quadVertex);
                            map[cfRec] = false;
                       }
                    } //has windowproperties

                    else if (isWindow == false) //check window properties
                    {
                        bso::grammar::rule_set::dl_rule_set::writeWallRadiance(sceneFile, quadVertex);
                        map[cfRec] = false;
                    }

                RectangleWindowYN.push_back(map);
                } //isWall.
            } //is internal

		} //space seperating
        roomInfo.push_back(SpaceID_MinZ_EI);
	} //rectangles loop

    sceneFile.close();
 
    // Open gridfile
    std::ofstream gridFile(folderName+"/"+"grid.txt", std::ios::trunc);
    if (!gridFile.is_open()) {std::cerr << "Failed to open grid.rad for writing\n";}

    bso::grammar::rule_set::dl_rule_set::filterSurfacesForGrid(cfSurface, SpaceID);
    SpaceID_AmountGridPoints = bso::grammar::rule_set::dl_rule_set::generateGridNEN(cfSurface, gridFile, SpaceID, DF_Settings);
    gridFile.close(); 

    std::vector<bso::spatial_design::conformal::cf_space*> VectorCf_space;
    for (const auto i: this->getSpacePropertiesQH()) 
    {
        bso::spatial_design::conformal::cf_space* cf_space = i.first;
        VectorCf_space.push_back(cf_space);
    }
    mDLModel.setSpacePropertiesQH(VectorCf_space);

    //retrieve cardinal direction per outer Rectangles
    std::vector<std::vector<const bso::spatial_design::conformal::cf_rectangle*>> CardinalDirections = bso::grammar::rule_set::dl_rule_set::setCardinalDirection(Rectangles);

    this->setrectWindowProperties(RectangleWindowYN); //safe to grammar for sd
    mDLModel.setSpaceID_AmountGridPoints(SpaceID_AmountGridPoints);
    mDLModel.setfolderName(folderName);
    mDLModel.setDF_Settings(DF_Settings);
    mDLModel.setDF_Evaluation(DF_Evaluation);
    mDLModel.setWindowProperties(RectangleWindowYN); //safe to dl model for visualization
    mDLModel.setCardinalDirections(CardinalDirections);
    mDLModel.setCeilings(Ceilings);
    mDLModel.setRoomInfo(roomInfo);

	return mDLModel;
} // default_dl_grammar

/*
This function bellow allowes to call oppon the above function without defining the second variable namely the boolian.
*/
template <>
daylight::dl_model grammar::dl_grammar<DEFAULT_DL_GRAMMAR>(const std::string& fileName)
{
	return this->dl_grammar<DEFAULT_DL_GRAMMAR>(fileName,false);
}

} // namespace grammar
} // namespace bso

#endif // BSO_GRAMMAR_DEFAULT_DL_GRAMMAR_CPP