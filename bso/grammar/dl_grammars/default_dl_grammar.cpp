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
The function below will generate a DL model. This contents preparing the radiance scene, 
based on the ms representation and settingfile. 
Several txt files are created onto which radiance operaties. 
*/
	
template <>
bso::daylight::dl_model grammar::dl_grammar<DEFAULT_DL_GRAMMAR>(const std::string& fileName)
{	
	// initialize a new DL model
	bso::daylight::dl_model mDLModel;

    // make new folder into which you save all files constructed for this DL_model object.
    std::string folderName = mDLModel.createAndSetFolder(fileName);

	// read the daylight settings and store in the grammar object
	this->mReadDLSettings(fileName);

    // Open scenefile and write material functions to the scenefile based on the retrieved daylight settings from the file.
    std::ofstream sceneFile(folderName+"/"+"scene.rad", std::ios::trunc);
    if (!sceneFile.is_open()) {std::cerr << "Failed to open scene.rad for writing\n";}
    bso::grammar::rule_set::dl_rule_set::resetPolygonCounters(); // Reset global polygon counters for each new scene file, so the inserted polygons in the scene file have correct numbering.
	bso::grammar::rule_set::dl_rule_set::writeMaterials(sceneFile, mReflectanceValues, mWindowProperties);
	
    // Define helper varialbles
	std::vector<bso::spatial_design::conformal::cf_surface> cfFloorSurface; // Collect floor surfaces for grid generation later.
    std::vector<unsigned int> SpaceID;
    std::vector<std::map<bso::spatial_design::conformal::cf_rectangle* /*rectangle*/, bool /*window Y/N*/>> RectangleWindowYN;
	std::vector<bso::spatial_design::conformal::cf_triangle*> Triangles; // Collect floor/ceiling triangles for visualization later.
    std::vector<const bso::spatial_design::conformal::cf_rectangle* /*rectangle*/> Ceilings = {};
	std::vector<const bso::spatial_design::conformal::cf_triangle* /*triangle*/> CeilingsTri = {};
    std::vector<std::vector<double>> roomInfo /*external==0 or internal==1*/ = {};

    // Loop trough rectangles and write the needed rectangles with the correct properties to the scenefile to building the actual scene in radiance.
	for (const auto i: mRectangleProperties)
	{
		// Fresh map for each rectangle iteration
		std::map<bso::spatial_design::conformal::cf_rectangle* /*rectangle*/, bool /*window Y/N*/> map;
		std::vector<double> SpaceID_MinZ_EI(3);
	
		// Retrieve rectangle information in the maps of the grammar object for easy access later
        bso::spatial_design::conformal::cf_rectangle* cfRec = i.first; // cf_rectangle
        bso::utilities::geometry::quadrilateral* quad = i.first; //rectangles
        std::vector<bso::utilities::geometry::vertex> quadVertex; //points
        double min_z = std::numeric_limits<double>::max();
        for(const auto j: (*quad))
        { //save all vertexes of a rectangle
            quadVertex.push_back(j);
            double z_value = j[2];  // Extract the z-coordinate
            if (z_value < min_z) {min_z = z_value;}
        }    
        
        // Check all properties of the rectangles and write to scene file based on that.
		if (!i.second->isSpaceSeparating()){continue; /*Skip non-space-separating rectangles*/}
        int SpaceIDforRoominfo = ((cfRec->cfSurfaces()[0]->cfSpaces())[0])->getSpaceID(); // renders a segmentation fault without isSpaceSeparating() continue check, because some rectangles do not have surfaces and spaces associated to them.

		bool isExternal = i.second->isExternal();
		bool isFloor = i.second->isFloor();
		bool isWall = i.second->isWall();

		// Handle floors (both external and internal have same logic)
		if (isFloor)
		{
			bso::grammar::rule_set::dl_rule_set::writePolygonFloorRadiance(sceneFile, quadVertex);
			bso::grammar::rule_set::dl_rule_set::writePolygonCeilingRadiance(sceneFile, quadVertex);
			map[cfRec] = false;
			
			if (isExternal){Ceilings.push_back(cfRec);}

			// Collect surfaces for grid generation
			for (const auto j: (cfRec->cfSurfaces()))
			{
				cfFloorSurface.push_back(*j);
				SpaceID.push_back(((j->cfSpaces())[0])->getSpaceID());
			}
			
			RectangleWindowYN.push_back(map);
			continue;
		}

		// Handle walls
		if (isWall)
		{
			SpaceID_MinZ_EI[0] = SpaceIDforRoominfo;
			SpaceID_MinZ_EI[1] = min_z;
			SpaceID_MinZ_EI[2] = isExternal ? 0 : 1;

			bool isWindow;
			if(mTriangleProperties.size() > 0 /*i.e. ms_N_building with TP conformal model is provided*/) isWindow = bso::grammar::rule_set::dl_rule_set::isTPQuadInWindowArea(quadVertex, mWindowsPerSpace, 1e-3);
			else isWindow = bso::grammar::rule_set::dl_rule_set::isQuadInWindowArea(quadVertex, mWindowsPerSpace, 1e-3);
			bool isOpen = isExternal ? bso::grammar::rule_set::dl_rule_set::isOpenExternal(cfRec)
			                         : bso::grammar::rule_set::dl_rule_set::isOpenInternal(cfRec);

			// Write window or wall based on window area and open/closed status
			if (isWindow && isOpen)
			{
				bso::grammar::rule_set::dl_rule_set::writeWindowRadiance(sceneFile, quadVertex, i.second);
				map[cfRec] = true;
			}
			else
			{
				bso::grammar::rule_set::dl_rule_set::writePolygonWallRadiance(sceneFile, quadVertex);
				map[cfRec] = false;
			}

			RectangleWindowYN.push_back(map);
		}

		roomInfo.push_back(SpaceID_MinZ_EI);
	} // rectangles loop

    // Loop trough triangles and write the needed triangles with the correct properties to the scenefile to building the actual scene in radiance.
	for (const auto i: mTriangleProperties)
	{	
		// Retrieve the triangle in the map for easy access later
        bso::spatial_design::conformal::cf_triangle* cfTri = i.first; // cf_triangle
        bso::utilities::geometry::triangle* tri = i.first; //triangles
        std::vector<bso::utilities::geometry::vertex> triVertex; //points
        double min_z = std::numeric_limits<double>::max();
        for(const auto j: (*tri))
        { //save all vertexes of a triangle
            triVertex.push_back(j);
            double z_value = j[2];  // Extract the z-coordinate
            if (z_value < min_z) {min_z = z_value;}
        }        

        // Check all properties of the triangles and write to scene file based on that.
		if (!i.second->isSpaceSeparating()){continue; /*Skip non-space-separating triangles*/}
    
		bool isExternal = i.second->isExternal();
		bool isFloor = i.second->isFloor();
		bool isWall = i.second->isWall();

		// Handle floors (both external and internal have same logic) Triangles only occur for floors in the current implementation, so no need to check for walls here.
		if (isFloor) // should always be true for triangles in current implementation, but good to check just in case future implementations add triangles for walls or other elements.
		{
			bso::grammar::rule_set::dl_rule_set::writePolygonFloorRadiance(sceneFile, triVertex);
			bso::grammar::rule_set::dl_rule_set::writePolygonCeilingRadiance(sceneFile, triVertex);
			
			if (isExternal){CeilingsTri.push_back(cfTri);} // for visualization

			// Collect surfaces for grid generation
			for (const auto j: (cfTri->cfSurfaces()))
			{
				cfFloorSurface.push_back(*j);
				SpaceID.push_back(((j->cfSpaces())[0])->getSpaceID());
			}

			Triangles.push_back(cfTri); // for visualization
			continue;
		}
        else
        {
            std::stringstream errorMessage;
            errorMessage << "A different type of triangle is found then a floor in the default_dl_grammar." << std::endl
                                    << "This implementation only supports floor triangles." << std::endl
                                    << "(bso/grammar/dl_grammars/default_dl_grammar.cpp). " << std::endl;
            throw std::invalid_argument(errorMessage.str());
        }
	} // triangle loop

    sceneFile.close();
 
    // Open gridfile
    std::ofstream gridFile(folderName+"/"+"grid.txt", std::ios::trunc);
    if (!gridFile.is_open()) {std::cerr << "Failed to open grid.rad for writing\n";}

    // Generate the grid
    std::vector<std::tuple<unsigned int, int>> SpaceID_AmountGridPoints;
    bso::grammar::rule_set::dl_rule_set::filterSurfacesForGrid(cfFloorSurface, SpaceID);
	std::vector<bso::utilities::geometry::vertex> gridPoints;
	SpaceID_AmountGridPoints = bso::grammar::rule_set::dl_rule_set::generateGridNEN(cfFloorSurface, gridFile, SpaceID, mDF_Settings, gridPoints);
    gridFile.close(); 

    std::vector<bso::spatial_design::conformal::cf_space*> VectorCf_space;
    for (const auto i: mSpaceProperties) 
    {
        bso::spatial_design::conformal::cf_space* cf_space = i.first;
        VectorCf_space.push_back(cf_space);
    }
    mDLModel.setSpacePropertiesQH(VectorCf_space);

    // Send all relevant data to the dl model
    mRectangleWindowYNGram = RectangleWindowYN; //safe to grammar for sd
    mDLModel.setSpaceID_AmountGridPoints(SpaceID_AmountGridPoints);
	mDLModel.setDF_Settings(mDF_Settings);
	mDLModel.setDF_Evaluation(mDF_Evaluation);
    mDLModel.setWindowProperties(RectangleWindowYN); //safe to dl model for visualization
    mDLModel.setCeilings(Ceilings);
	mDLModel.setTriangles(Triangles);
    mDLModel.setRoomInfo(roomInfo);
	mDLModel.setGridPoints(gridPoints); // safe to dl model for visualization

	return mDLModel;
} // default_dl_grammar

} // namespace grammar
} // namespace bso

#endif // BSO_GRAMMAR_DEFAULT_DL_GRAMMAR_CPP