#ifndef CF_BUILDING_MODEL_CPP
#define CF_BUILDING_MODEL_CPP

#include <bitset>
#include <Fade_2D.h>
#include <FadeExport.h>
#include <iomanip> // Allowes to use std::setprecision() to be able to enlarge the decimals printed to the console of a double					 

namespace bso { namespace spatial_design { namespace conformal {
	
	void cf_building_model::addSpace(const ms_space& msSpace)
	{ // 
		mCFSpaces.push_back(new cf_space(msSpace.getGeometry(), this));
		mCFSpaces.back()->setSpaceID(msSpace.getID());
		std::string possibleSpaceType;
		if (msSpace.getSpaceType(possibleSpaceType))
		{
			mCFSpaces.back()->setSpaceType(possibleSpaceType);
		}

		auto spPtr = mCFSpaces.back();
		for (const auto& i : *spPtr)
		{
			mCFPoints.push_back(new cf_point(i, this));
			mCFPoints.back()->addSpace(spPtr);
			spPtr->addPoint(mCFPoints.back());
		}
		
		for (const auto& i : spPtr->getLines())
		{
			mCFEdges.push_back(new cf_edge(i, this));
			mCFEdges.back()->addSpace(spPtr);
			spPtr->addEdge(mCFEdges.back());
		}
		
		std::vector<std::string> possibleSurfaceTypes;
		bool surfaceTypesAvailable = msSpace.getSurfaceTypes(possibleSurfaceTypes);
		if (surfaceTypesAvailable)
		{
			std::swap(possibleSurfaceTypes[0],possibleSurfaceTypes[2]);
			std::swap(possibleSurfaceTypes[4],possibleSurfaceTypes[5]);
		}
		auto typeIte = possibleSurfaceTypes.begin();
		for (const auto& i : spPtr->getPolygons())
		{
			mCFSurfaces.push_back(new cf_surface(*i, this));
			mCFSurfaces.back()->addSpace(spPtr);
			spPtr->addSurface(mCFSurfaces.back());
			if (surfaceTypesAvailable)
			{
				mCFSurfaces.back()->setSurfaceType(*typeIte);
				++typeIte;
			}
		}
	} // addSpace

	void cf_building_model::addSpace(const ms_space& msSpace, std::vector<utilities::geometry::quad_hexahedron> quadHexa)
	{
		// Check prerequisite for this function
		if(quadHexa.size() < 1)
		{
			std::stringstream errorMessage;
			errorMessage << "The amount of quadHexa inserted in the addSpaceD() function is to little to fill a space with quadHexa cells.\n"
									 << "The minumum required amount of quadHexa is 2 and the inserted amount is: " << quadHexa.size() << "\n"
									 << "Meaning that there are quadHexa(s) missing. \n"
									 << "The inserted quadHexa consist of the following: " << std::endl;
										for(const auto i: quadHexa)
										{
											errorMessage << i << std::endl;
										}
						errorMessage << "the considered space is: " << *(mCFSpaces.back()) << "\n"
									 << "(bso/utilities/geometry/cf_building_model.cpp)" << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
		if(mCFTriPrisms.size() > 0)
		{
			std::stringstream errorMessage;
			errorMessage << "The addSpace function is not allowed to be used when mCFTriPrisms contains content in the geometry conformal model. \n"
									 << "The mCFTriPrisms consist of: " << mCFTriPrisms.size() << " amount of triangular prisms \n"
									 << "the considered space is: " << *(mCFSpaces.back()) << "\n"
									 << "(bso/utilities/geometry/cf_building_model.cpp)" << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
				
		// Add and store the space with its according properties
		mCFSpaces.push_back(new cf_space(msSpace.getGeometry(), this, quadHexa));
		mCFSpaces.back()->setSpaceID(msSpace.getID());
		std::string possibleSpaceType;
		if (msSpace.getSpaceType(possibleSpaceType))
		{
			mCFSpaces.back()->setSpaceType(possibleSpaceType);
		}
		
		// Add the geometry entities to cf_geometry_model and the corrsponding cf_building_model geometry, i.e. a line from a space may exist for multiple lines in the cf_geometry_model
		auto spPtr = mCFSpaces.back();
		for (const auto& i : *spPtr)
		{
			mCFPoints.push_back(new cf_point(i, this)); // add points of space spPtr to mCFSpaces and initiate constructor, which adds the points to cf_geometry_model mCFVertices
			mCFPoints.back()->addSpace(spPtr); // store the space to which the points belongs in the mCFSpaces of cf_point
			spPtr->addPoint(mCFPoints.back()); // add the cf_point to the mCFSpaces of the cf_space to which it belongs point belongs
		}
		
		// Store all unique line entities from the to insert quadHexa for consideration of insertion into the geometry conformal model	
		std::vector<utilities::geometry::line_segment> quadHexaLine;
		for(const auto i: quadHexa)
		{
			for(auto j: i.getLines())
			{								
				bool isSameAs = false;
				for(const auto k: quadHexaLine)
				{
					if(k.isSameAs(j, mTol))
					{
						isSameAs = true;
					}
				}
				if(isSameAs == false)
				{
					quadHexaLine.push_back(j);
				}
			}
		}
		
		for (const auto& i : spPtr->getLines())
		{ 
			//Determine which lines are corresponding with which edges in the building conformal model
			std::vector<utilities::geometry::line_segment*> linesOnEdge;
			
			for(const auto j: quadHexaLine)
			{
				
				if(i.isOnLine(j[0], mTol) && i.isOnLine(j[1], mTol))
				{
					linesOnEdge.push_back(new utilities::geometry::line_segment(j));
				}
				else if(i.isOnLine(j[0], mTol))
				{
					if((i[0]).isSameAs(j[1], mTol) || (i[1]).isSameAs(j[1], mTol))
					{
						linesOnEdge.push_back(new utilities::geometry::line_segment(j));
					}
				}
				else if(i.isOnLine(j[1], mTol))
				{
					if((i[0]).isSameAs(j[0], mTol) || (i[1]).isSameAs(j[0], mTol))
					{
						linesOnEdge.push_back(new utilities::geometry::line_segment(j));
					}
				}
				else if((i[1]).isSameAs(j[0], mTol) && (i[0]).isSameAs(j[1], mTol))
				{
					linesOnEdge.push_back(new utilities::geometry::line_segment(j));
				}
				else if((i[0]).isSameAs(j[0], mTol) && (i[1]).isSameAs(j[1], mTol))
				{
					linesOnEdge.push_back(new utilities::geometry::line_segment(j));
				}				
			}

			// Add edges accordingly
			mCFEdges.push_back(new cf_edge(i, this, linesOnEdge));
			mCFEdges.back()->addSpace(spPtr);
			spPtr->addEdge(mCFEdges.back());
		}
		
		// Add surface types to the spaces
		std::vector<std::string> possibleSurfaceTypes;
		bool surfaceTypesAvailable = msSpace.getSurfaceTypes(possibleSurfaceTypes);
		if (surfaceTypesAvailable)
		{
			std::swap(possibleSurfaceTypes[0],possibleSurfaceTypes[2]);
			std::swap(possibleSurfaceTypes[4],possibleSurfaceTypes[5]);
		}
		auto typeIte = possibleSurfaceTypes.begin();
		
		
		for (const auto& i : spPtr->getPolygons())
		{
			//Determine which quads are corrsponding with with surfaces
			std::vector<utilities::geometry::triangle*> triOnSurface; // Empty variable for function call, none will be added since it is empty
			std::vector<utilities::geometry::quadrilateral*> quadOnSurface;
			
			for(const auto j: quadHexa)
			{
				for(const auto& k: j.getQuadrilateral())
				{
					bool allVOnSurface = true;
				
					for(const auto l: k)
					{
						if(i->isInsideOrOn(l, mTol) == false){allVOnSurface = false;}
					}
					
					if(allVOnSurface == true)
					{
						quadOnSurface.push_back(new utilities::geometry::quadrilateral(k)); // should the new key word be used here, do need a new memory adress since a const storing isn't posible
					}
				}
			}

			// Add surface and quads
			mCFSurfaces.push_back(new cf_surface(*i, this, triOnSurface, quadOnSurface));
			mCFSurfaces.back()->addSpace(spPtr);
			spPtr->addSurface(mCFSurfaces.back());
			if (surfaceTypesAvailable)
			{
				mCFSurfaces.back()->setSurfaceType(*typeIte);
				++typeIte;
			}
		}


		//Check if the space volume is considered fully filled with the inserted quad-hexahedrons cells
		double spaceVolume = (mCFSpaces.back())->getVolume();
		double cellVolume = 0;
		for(const auto i: quadHexa)
		{
			cellVolume = cellVolume + i.getVolume();
		}
		
		if((spaceVolume-cellVolume) < (0-10) || (spaceVolume-cellVolume) > (0+10)) // The standard tolerance is too small and trickers the error unrightfully fast
		{
			std::stringstream errorMessage;
			errorMessage << "When initializing multiple quad-hexahedron cells in a space in the addSpaceD() function.\n"
									 << "The space was not fully filled with the inserted cells. \n"
									 << "Meaning that there are cells missing. \n"
									 << "The difference between the space volume and the sum of all the cells is: " << (spaceVolume-cellVolume) << "\n"
									 << "the amount of triPrism in the space is: " << quadHexa.size() << " and consist of the following: " << std::endl;
										for(const auto i: quadHexa)
										{
											errorMessage << i << std::endl;
										}
						errorMessage << "the considered space is: " << *(mCFSpaces.back()) << "\n"
									 << "(bso/utilities/geometry/cf_building_model.cpp)" << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
	} // addSpace
	
	void cf_building_model::addSpaceD(const ms_space& msSpace, std::vector<utilities::geometry::triangular_prism> triPrismPtr)
	{
		// Check if the minimum amount of triPrisms are inserted
		if(triPrismPtr.size() < 2)
		{
			std::stringstream errorMessage;
			errorMessage << "The amount of triPrisms inserted in the addSpaceD() function is to little to fill a space with triPrism cells.\n"
									 << "The minumum required amount of triPrisms is 2 and the inserted amount is: " << triPrismPtr.size() << "\n"
									 << "Meaning that there are triPrism(s) missing. \n"
									 << "The inserted triPrism consist of the following: " << std::endl;
										for(const auto i: triPrismPtr)
										{
											errorMessage << i << std::endl;
										}
						errorMessage << "the considered space is: " << *(mCFSpaces.back()) << "\n"
									 << "(bso/utilities/geometry/cf_building_model.cpp)" << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
		if(mCFCuboids.size() > 0)
		{
			std::stringstream errorMessage;
			errorMessage << "The addSpace function is not allowed to be used when mCFCuboids contains content in the geometry conformal model. \n"
									 << "The mCFCuboids consist of: " << mCFCuboids.size() << " amount of triangular prisms \n"
									 << "the considered space is: " << *(mCFSpaces.back()) << "\n"
									 << "(bso/utilities/geometry/cf_building_model.cpp)" << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
		
		mCFSpaces.push_back(new cf_space(msSpace.getGeometry(), this, triPrismPtr));
		
		mCFSpaces.back()->setSpaceID(msSpace.getID());
		std::string possibleSpaceType;
		if (msSpace.getSpaceType(possibleSpaceType))
		{
			mCFSpaces.back()->setSpaceType(possibleSpaceType);
		}
		
		// Store all unique line entities from the to insert triPrism for consideration of insertion into the geometry conformal model	
		std::vector<utilities::geometry::line_segment> prismLine;
		for(const auto i: triPrismPtr)
		{
			for(auto j: i.getLines())
			{								
				bool isSameAs = false;
				for(const auto k: prismLine)
				{
					if(k.isSameAs(j, mTol))
					{
						isSameAs = true;
					}
				}
				if(isSameAs == false)
				{
					prismLine.push_back(j);
				}
			}
		}
			
		// Add the geometry entities to cf_geometry_model and the corrsponding cf_building_model geometry, i.e. a line from a space may exist for multiple lines in the cf_geometry_model
		auto spPtr = mCFSpaces.back();
		for (const auto& i : *spPtr)
		{
			mCFPoints.push_back(new cf_point(i, this)); // add points of space spPtr to mCFSpaces and initiate constructor, which adds the points to cf_geometry_model mCFVertices
			mCFPoints.back()->addSpace(spPtr); // store the space to which the points belongs in the mCFSpaces of cf_point
			spPtr->addPoint(mCFPoints.back()); // add the cf_point to the mCFSpaces of the cf_space to which it belongs point belongs
		}
		
		for (const auto& i : spPtr->getLines())
		{ 
			//Determine which lines are corresponding with which edges in the building conformal model
			std::vector<utilities::geometry::line_segment*> linesOnEdge;
			
			for(const auto j: prismLine)
			{
				
				if(i.isOnLine(j[0], mTol) && i.isOnLine(j[1], mTol))
				{
					linesOnEdge.push_back(new utilities::geometry::line_segment(j));
				}
				else if(i.isOnLine(j[0], mTol))
				{
					if((i[0]).isSameAs(j[1], mTol) || (i[1]).isSameAs(j[1], mTol))
					{
						linesOnEdge.push_back(new utilities::geometry::line_segment(j));
					}
				}
				else if(i.isOnLine(j[1], mTol))
				{
					if((i[0]).isSameAs(j[0], mTol) || (i[1]).isSameAs(j[0], mTol))
					{
						linesOnEdge.push_back(new utilities::geometry::line_segment(j));
					}
				}
				else if((i[1]).isSameAs(j[0], mTol) && (i[0]).isSameAs(j[1], mTol))
				{
					linesOnEdge.push_back(new utilities::geometry::line_segment(j));
				}
				else if((i[0]).isSameAs(j[0], mTol) && (i[1]).isSameAs(j[1], mTol))
				{
					linesOnEdge.push_back(new utilities::geometry::line_segment(j));
				}				
			}

			// Add edges acordingly
			mCFEdges.push_back(new cf_edge(i, this, linesOnEdge));
			mCFEdges.back()->addSpace(spPtr);
			spPtr->addEdge(mCFEdges.back());
		}
		
		std::vector<std::string> possibleSurfaceTypes;
		bool surfaceTypesAvailable = msSpace.getSurfaceTypes(possibleSurfaceTypes);
		if (surfaceTypesAvailable)
		{
			std::swap(possibleSurfaceTypes[0],possibleSurfaceTypes[2]);
			std::swap(possibleSurfaceTypes[4],possibleSurfaceTypes[5]);
		}
		auto typeIte = possibleSurfaceTypes.begin();
		
		
		for (const auto& i : spPtr->getPolygons())
		{
			//Determine which triangles/quads are corrsponding with which surfaces
			std::vector<utilities::geometry::triangle*> triOnSurface;
			std::vector<utilities::geometry::quadrilateral*> quadOnSurface;
			
			for(const auto j: triPrismPtr)
			{
				for(const auto& k: j.getTriangles())
				{
					bool allVOnSurface = true;
				
					for(const auto l: k)
					{
						if(i->isInsideOrOn(l, mTol) == false){allVOnSurface = false;}
					}
					
					if(allVOnSurface == true)
					{
						triOnSurface.push_back(new utilities::geometry::triangle(k)); // should the new key word be used here, do need a new memory adress since a const storing isn't posible
					}
				}
				for(const auto& k: j.getQuadrilateral())
				{
					bool allVOnSurface = true;
				
					for(const auto l: k)
					{
						if(i->isInsideOrOn(l, mTol) == false){allVOnSurface = false;}
					}
					
					if(allVOnSurface == true)
					{
						quadOnSurface.push_back(new utilities::geometry::quadrilateral(k)); // should the new key word be used here, do need a new memory adress since a const storing isn't posible
					}
				}
			}

			// Add surface and triangles
			mCFSurfaces.push_back(new cf_surface(*i, this, triOnSurface, quadOnSurface));
			mCFSurfaces.back()->addSpace(spPtr);
			spPtr->addSurface(mCFSurfaces.back());
			if (surfaceTypesAvailable)
			{
				mCFSurfaces.back()->setSurfaceType(*typeIte);
				++typeIte;
			}
		}

		//Check if the space volume is considered fully filled with the inserted triangular prism cells
		double spaceVolume = (mCFSpaces.back())->getVolume();
		double cellVolume = 0;
		for(const auto i: triPrismPtr)
		{
			cellVolume = cellVolume + i.getVolume();
		}
		
		
		if((spaceVolume-cellVolume) < (0-10) || (spaceVolume-cellVolume) > (0+10)) // The standard tolerance is too small and trickers the error unrightfully fast
		{
			std::stringstream errorMessage;
			errorMessage << "When initializing multiple triangular prism cells in a space in the addSpaceD() function.\n"
									 << "The space was not fully filled with the inserted cells. \n"
									 << "Meaning that there are cells missing. \n"
									 << "The difference between the space volume and the sum of all the cells is: " << (spaceVolume-cellVolume) << "\n"
									 << "the amount of triPrism in the space is: " << triPrismPtr.size() << " and consist of the following: " << std::endl;
										for(const auto i: triPrismPtr)
										{
											errorMessage << i << std::endl;
										}
						errorMessage << "the considered space is: " << *(mCFSpaces.back()) << "\n"
									 << "(bso/utilities/geometry/cf_building_model.cpp)" << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
	}// addSpaceD
	
	void cf_building_model::makeConformal()
	{ // 
		// check for line line intersections, and add the found vertex to the geometry model
		utilities::geometry::vertex pIntersection;
		for (unsigned int i = 0; i < mCFLines.size(); ++i)
		{
			for (unsigned int j = i+1; j < mCFLines.size(); ++j)
			{
				if (mCFLines[i]->intersectsWith(*(mCFLines[j]), pIntersection, mTol))
				{
					this->addVertex(pIntersection);
				}
			}
		}

		// check for line - rectangle intersections, and add the found vertex to the geometry model
		for (unsigned int i = 0; i < mCFRectangles.size(); ++i)
		{
			for (unsigned int j = 0; j < mCFLines.size(); ++j)
			{
				if (mCFRectangles[i]->intersectsWith(*(mCFLines[j]), pIntersection, mTol))
				{
					this->addVertex(pIntersection);
				}
			}
		}

		// check for each space if a partitiong at the z=0 plane is needed
		for (auto& j : mCFSpaces)
		{
			pIntersection = {j->getLines()[4][0].x(), j->getLines()[4][0].y(), 0}; //index 4 is always a vertical line of the space due to the ordering of the geometry
			if(j->getLines()[4].isOnLine(pIntersection, mTol))
			{
				this->addVertex(pIntersection);
			}
		}
		
		// check each vertex if it intersects with any space
		unsigned int i = 0;
		while (i < mCFVertices.size())
		{
			for (auto& j : mCFSpaces)
			{
				j->checkVertex(mCFVertices[i]);
			}
			++i;
		}

		// delete the lines, rectangles, and cuboids that were tagged for deletion
		mCFLines.erase(std::remove_if(mCFLines.begin(), mCFLines.end(), [](const auto& i){ return i->deletion(); }), mCFLines.end());
		mCFRectangles.erase(std::remove_if(mCFRectangles.begin(), mCFRectangles.end(), [](const auto& i){ return i->deletion(); }), mCFRectangles.end());
		mCFCuboids.erase(std::remove_if(mCFCuboids.begin(), mCFCuboids.end(), [](const auto& i){ return i->deletion(); }), mCFCuboids.end());
			
	} //  makeConformal()
	
	void cf_building_model::makeConformalND(const ms_N_building& msNModel) // Delaunay method
	{ //
		// Declarations for needed information per space
		std::vector<utilities::geometry::quad_hexahedron> spacesAtLevel;
		//std::vector<utilities::geometry::quad_hexahedron> spacesAtGround; // Declared in private section class to allow for visualization
		std::vector<utilities::geometry::vertex> highV;
		std::vector<utilities::geometry::vertex> LowV;
		
		// Declarations for needed information per building spatial design
		std::vector<double> allLevels; // stores all the floor levels
		//allLevels.push_back(0); // Always add the z=0 plane to ensure correct load application in the later stages
		std::vector<std::vector<double>> levelPerSpace;
		int count =0; //counts the amount of spaces
		
		//Get the above information
		for (const auto& i : msNModel)
		{
			//Add original space geometry
			spacesAtLevel.push_back(i->getGeometry());
			
			//Find the highest and lowest vertex
			utilities::geometry::vertex lowVertex = (spacesAtLevel.back())[0];
			utilities::geometry::vertex highVertex = (spacesAtLevel.back())[0];
			for (const auto j: spacesAtLevel.back())
			{
				if(lowVertex.z() > j.z())
				{//find the lowest vertex of SpaceGeomTemp_1 for the shifting of the space geometry
					lowVertex = j;
				}
				else if(highVertex.z() < j.z())
				{
					highVertex = j;
				}
			}
			highV.push_back(highVertex);
			LowV.push_back(lowVertex);
			
			//Generate the at ground level space geometry
			std::vector<utilities::geometry::vertex> vertices_1;
			utilities::geometry::vertex shift = {0,0,(LowV.back()).z()};
			for(auto k: (spacesAtLevel.back())){vertices_1.push_back(k-shift);}
			spacesAtGround.push_back(utilities::geometry::quad_hexahedron(vertices_1, mTol));
			
			// Determine all the floor levels in the building spatial designs
			bool highVUnique = true;
			bool lowVUnique = true;
			for(double j: allLevels)
			{
				if(highVertex.z()<=(j+mTol) && highVertex.z()>=(j-mTol))
				{
					highVUnique = false;
				}
				if(lowVertex.z()<=(j+mTol) && lowVertex.z()>=(j-mTol))
				{
					lowVUnique = false;
				}
			}
			if(highVUnique){allLevels.push_back(highVertex.z());}
			if(lowVUnique){allLevels.push_back(lowVertex.z());}
			
			count++;
		}
		
		// Sort allLevels for later use in triangular prism cell definition
		sort(allLevels.begin(), allLevels.end());
		
		
		// Declaration delauney variables
		std::vector <utilities::geometry::vertex> delauneyPoints; // Points to be inserted into the delauney triangulation
		// std::vector <utilities::geometry::line_segment> delauneyLines; //Lines where the delauney triangulation should constrain to; Declared in class to support posible visualization by long time storage of variable
		
		//Find the apropriate information for the delauney triangulation
		for(int i=0; i<count; i++)
		{
			//Add all unique cornerpoints at z=0 of spaceAtGround to the delauneyPoints
			for(const auto j: spacesAtGround[i])
			{
				utilities::geometry::vertex temp = {j.x(),j.y(),0};

				bool doubleInsert = false;
				for (const auto& m : delauneyPoints)
				{// check if temp already exist in delauneyPoints
					if (m == temp)
					{
						doubleInsert = true;
					}
				}
				if(doubleInsert == false)
				{//if not, then add to delaunayPoints
					delauneyPoints.push_back(temp);
				}
			}
		}
			
		for(int i=0; i<count; i++)
		{ 
			for (const auto& k : spacesAtGround[i].getLines())
			{
				// Add the unique lines of each space to delauneyLines
				utilities::geometry::vertex Ltemp1 = {k[0].x(),k[0].y(),0};
				utilities::geometry::vertex Ltemp2 = {k[1].x(),k[1].y(),0};
				utilities::geometry::line_segment Ltemp {{Ltemp1},{Ltemp2}}; //Note the line is newly defined for the case that some round of error occur during the shifting of the lines of the spacesAtGround[i]
				
				bool doubleInsert = false;
				for (const auto& m : delauneyLines)
				{// check if line k already exist in delauneyPoints
					if (m.isSameAs(Ltemp, mTol))
					{
						doubleInsert = true;
					}
				}
				if(Ltemp1.isSameAs(Ltemp2, mTol))
				{//Check if line has diferent end points
					doubleInsert = true;
				}
				if(doubleInsert == false)
				{//if both agree, then add to delaunayLines
					delauneyLines.push_back(Ltemp);
				}
				
				//Find line-line intersections, note: no line-rectangle intersection check needed since these are acounted for by the addition of the cornerpoints
				utilities::geometry::vertex pIntersection;
				for(int j=i+1; j<count; j++)
				{
					if(i == j){continue;}
					for (const auto& l : spacesAtGround[j].getLines())
					{
						if (k.intersectsWith(l, pIntersection, mTol))
						{
							//consider the translated to ground floor vertex of the space
							utilities::geometry::vertex temp = {pIntersection.x(),pIntersection.y(),0};
							
							bool doubleInsert = false;
							for (const auto& m : delauneyPoints)
							{// check if temp already exist in delauneyPoints
								if (m.isSameAs(temp, mTol))
								{
									doubleInsert = true;
								}
							}
							
							if(doubleInsert == false)
							{//if not, then add to delaunayPoints
								delauneyPoints.push_back(temp);
							}
						}
					}
				}
			}
		}
		
		// Split lines at intersection vertices and remove duplicated lines
		splitLines(delauneyPoints, delauneyLines);// this prevent the generation of triangles on one line or triangles containing duplicate vertices

		// Send all vertices and constrain lines to the delauney triangulation and receive all triangles
		std::vector<utilities::geometry::triangle> delaunayTri = delaunay(delauneyPoints, delauneyLines);
		
		// identify which triangles should be used to generate a triangular prism cell in the space and generate that space and accompanying geometric entities
		int i = 0;
		for(const auto& t : msNModel)
		{
			// Find the triangles in the space
			std::vector<utilities::geometry::triangle> triCell;
			for(const auto j : delaunayTri)
			{
				bool allNodesInside = true;
				for(const auto k: j)
				{
					if(!spacesAtGround[i].isInsideOrOn(k, mTol))
					{
						//Additional checks for prevention malfunctioning, which has been previously observed
						//Note; the tolerance of 0.001 is applied differently in each geometry check. 
						//Meaning that in the isOnLine check, the considered vertices onLine are not by definition within +0.001 and -0.001 from the line in distance.
						//However, within the isSameAs check are the vertices within 0.001 from each other
						//For this reason are all possible geometry checks applied.
						
						bool anyTrue = false;
						for(const auto l: spacesAtGround[i].getLines())
						{
							if(l.isOnLine(k, mTol))
							{
								anyTrue = true;
							}
						}

						for(const auto l : spacesAtGround[i])
						{
							if(l.isSameAs(k, mTol))
							{
								anyTrue = true;
							}
						}

						if(!anyTrue)
						{
							allNodesInside = false;
						}
					}
				}
				if(allNodesInside)
				{
					triCell.push_back(j);
				}
			}
			
			if(triCell.size() < 2)
			{
				std::stringstream errorMessage;
				errorMessage << "The amount of triCell found are to little to form a space fully filled with triPrism cells.\n"
										<< "The minumum required amount of triCells are 2 and the inserted amount is: " << triCell.size() << "\n"
										<< "Meaning that there are tricell(s) missing. \n"
										<< "The possibly found triCell consist of the following: " << std::endl;
											for(const auto i: triCell)
											{
												errorMessage << i << std::endl;
											}
						   errorMessage << "the considered space at ground is: " << spacesAtGround[i] << "\n"
										<< "(bso/utilities/geomtry/cf_building_model.cpp)" << std::endl;
							throw std::invalid_argument(errorMessage.str());
			}
			
			// Are there anny midLevels and if yes, which are they?
			std::vector<double> levelToConsider;			
			
			for(const double j: allLevels)
			{
				if(j>LowV[i].z() && j< highV[i].z())
				{
					levelToConsider.push_back(j);
				}
			}

			// Define all six vertices of the triangular prism and store in triPrism
			std::vector<utilities::geometry::triangular_prism> triPrism;
			for(const auto j: triCell)
			{
				std::vector<utilities::geometry::vertex> triPrismV;
				triPrismV.reserve(6);
				if(levelToConsider.size()>0)
				{
					//Generate triangular prism
					triPrismV.clear();
					for(const auto k: j)
					{// botum row prisms
						utilities::geometry::vertex low = {0,0, LowV[i].z()};
						utilities::geometry::vertex high = {0,0, levelToConsider[0]};
						triPrismV.push_back(k+low);
						triPrismV.push_back(k+high);
					}
					triPrism.push_back(utilities::geometry::triangular_prism(triPrismV, mTol));
					triPrismV.clear();
					for(const auto k: j)
					{// top row prisms
						utilities::geometry::vertex low = {0,0, levelToConsider[levelToConsider.size()-1]};
						utilities::geometry::vertex high = {0,0, highV[i].z()};
						triPrismV.push_back(k+low);
						triPrismV.push_back(k+high);
					}
					triPrism.push_back(utilities::geometry::triangular_prism(triPrismV, mTol));
					if(levelToConsider.size()>1)
					{
						for(int l=0; l<(levelToConsider.size()-1); l++)
						{// inbetween row prisms
							triPrismV.clear();
							for(const auto k: j)
							{
								utilities::geometry::vertex low = {0,0, levelToConsider[l]};
								utilities::geometry::vertex high = {0,0, levelToConsider[l+1]};
								triPrismV.push_back(k+low);
								triPrismV.push_back(k+high);
							}
							triPrism.push_back(utilities::geometry::triangular_prism(triPrismV, mTol));
						}
					}
				}
				else
				{
					for(const auto k: j)
					{
						utilities::geometry::vertex low = {0,0, LowV[i].z()};
						utilities::geometry::vertex high = {0,0, highV[i].z()};
						triPrismV.push_back(k+low);
						triPrismV.push_back(k+high);
						
					}
					triPrism.push_back(utilities::geometry::triangular_prism(triPrismV, mTol));
				}
			}
			
			// Send triPrismPtr to the AddSpace function to add the cells and spaces to the conformal model
			this->addSpaceD(*t, triPrism);
			i++;
		}		
	} //makeConformalND()
		
	std::vector<utilities::geometry::triangle> cf_building_model::delaunay(const std::vector <utilities::geometry::vertex> delauneyPoints, const std::vector <utilities::geometry::line_segment> delauneyLines)
	{
		// Input checks
		if(delauneyPoints.size() <= 2)
		{
			std::stringstream errorMessage;
				errorMessage << "\nError, the inserted points in the delaunay traingulation are less then three. \n"
					 << "Meaning that a delaunay triangulation can not be generated. \n"
					 << "(bso/spatial_design/conformal/cf_building_model)." << std::endl;
				throw std::runtime_error(errorMessage.str());
		}
		else if(delauneyLines.size() == 0)
		{
			std::stringstream errorMessage;
				errorMessage << "\nError, there are no constrain lines added to the delauney triangulation. \n"
					 << "Meaning that a delaunay triangulation can not be generated. \n"
					 << "(bso/spatial_design/conformal/cf_building_model)." << std::endl;
				throw std::runtime_error(errorMessage.str());
		}
		
		
		//Check for duplicate vertices in the delaunay point variable
		bool duplicates = false; 
		std::vector<utilities::geometry::vertex> duplicatePoints;
		for(int t =0; t< delauneyPoints.size(); t++)
		{ 
			for(int s=0; s<delauneyPoints.size(); s++)
			{
				if(s==t){continue;}
				else if(delauneyPoints[t].isSameAs(delauneyPoints[s], mTol))
				{
					duplicates = true;
					duplicatePoints.push_back(delauneyPoints[s]);
				}
			}
		}
		
		//Check the constrain lines
		std::vector<utilities::geometry::vertex> noDelauneyPoint;
		std::vector<utilities::geometry::vertex> pointOnConstraintLine;
		std::vector<utilities::geometry::line_segment> contrainLines;
		for(auto t: delauneyLines)
		{
			for(const auto k: t)
			{				
				bool found = false;
				for(const auto u: delauneyPoints) 
				{
					if(u.isSameAs(k)) found = true;
				}
				if(!found)
				{
					noDelauneyPoint.push_back(k);
				}
				if(t.isOnLine(k))
				{
					pointOnConstraintLine.push_back(k);
				}
			}
			for(auto k: delauneyLines)
			{
				if(k.isSameAs(t, mTol)){continue;}
				for(const auto s: k)
				{
					if(t.isOnLine(s, mTol))
					{
						pointOnConstraintLine.push_back(s);
						contrainLines.push_back(t);
					}
				}
			}
		}
		if(noDelauneyPoint.size() > 0)
		{
			std::stringstream errorMessage;
			errorMessage << "The following vertices are end vertices of the constraint lines, but not a delauney points (Checked with tolerance): " << std::endl;
			for(const auto t: noDelauneyPoint){errorMessage << t;}
			errorMessage << std::endl	
						 << "The following are the delauney points: " << std::endl;
			for(const auto t: delauneyPoints){errorMessage << t << " ; ";}	
			errorMessage << std::endl
						 << "the following ms_N_building Model is inserted: " << mMSNModel << std::endl;		 
			errorMessage << "(bso/spatial_design/conformal/cf_building_model)." << std::endl;
			throw std::runtime_error(errorMessage.str());
		}
		if(pointOnConstraintLine.size() > 0)
		{
			std::stringstream errorMessage;
			int counter=0;
			errorMessage << "The following vertices are vertices on the constrain line, this is not allowed and should have been solved in the splitLines function. See the following vertices: " << std::endl;
			for(const auto t: pointOnConstraintLine)
			{
				errorMessage << "vertex: " << t << " has been found on: " << contrainLines[counter] << std::endl;
				counter++;
			}
			errorMessage << "(bso/spatial_design/conformal/cf_building_model)." << std::endl;
			throw std::runtime_error(errorMessage.str());
		}
		

		// Start Delaunay triangulation process
		
		// start adding the points/constrainLines to delaunay triangulation algorithm Fade_2D
		GEOM_FADE2D::Fade_2D dt;
		
		for(const auto i: delauneyPoints)
		{
			dt.insert(GEOM_FADE2D::Point2(i.x(),i.y()));
		}
		std::vector<GEOM_FADE2D::Segment2> vSegments;
		for(const auto i: delauneyLines)
		{
			vSegments.push_back(GEOM_FADE2D::Segment2(GEOM_FADE2D::Point2(i[0].x(),i[0].y()),GEOM_FADE2D::Point2(i[1].x(),i[1].y())));
		}
		GEOM_FADE2D::ConstraintGraph2* pCG=dt.createConstraint(vSegments,GEOM_FADE2D::CIS_CONSTRAINED_DELAUNAY);
		
		
		// Makes the constrained delauney a delauney respecting the boundaries by adding the intersection point found on the boundary after delaunay
		bool aplyConstrainedDelaunay = true;
		if(!aplyConstrainedDelaunay)
		{
			double minLen(0.1);
			pCG->makeDelaunay(minLen);
		}

		// Draw; generate a Design.ps file for visualization in for example illustrator 
		// dt.show("Design.ps",true);		

		// Export fade2D data to Toolbox information format
		GEOM_FADE2D::FadeExport fadeExport;
		bool bCustomIndices(false); // To retrieve custom indices also
		bool bClear(false); // Clear memory in $dt
		dt.exportTriangulation(fadeExport,bCustomIndices,bClear);
		
		for(int triIdx=0;triIdx<fadeExport.numTriangles;++triIdx)
		{ // Iterate through each triangle in Fade_2D and store triangle in Toolbox entities
			int vtxIdx0,vtxIdx1,vtxIdx2;
			fadeExport.getCornerIndices(triIdx,vtxIdx0,vtxIdx1,vtxIdx2);
			
			double x0,y0;
			double x1,y1;
			double x2,y2;
			fadeExport.getCoordinates(vtxIdx0,x0,y0);
			fadeExport.getCoordinates(vtxIdx1,x1,y1);
			fadeExport.getCoordinates(vtxIdx2,x2,y2);
			
			// Define triangle in Toolbox entities
			utilities::geometry::vertex C1 = {x0,y0,0};
			utilities::geometry::vertex C2 = {x1,y1,0};
			utilities::geometry::vertex C3 = {x2,y2,0};
			
			// Skip unviable trianlges for the BSO toolbox
				// This is needed since FADE_2D calculates with a finer tolerance then the BSO toolbox allowes.
				// Therefore, FADE_2D create unlogical triangles from the BSO Toolbox perspective regarding the BSO toolbox tolerances
				// the main reasons to skip a triangle are as followes:
				// - two cornerpoints of a single triangle are the same acording to the BSO toolbox tolerances.
				// - the three points of the triangle are on the same line in acordance to the BSO toolbox tolerances.
				// See paper automatic partitioning methods for more information.
			try
			{
				utilities::geometry::triangle tri = {{C1},{C2},{C3}};
				delaunayTri.push_back(tri);
			}
			catch(std::exception& e)
			{
				//std::cout << "Triangle was unviable for BSO Toolbox" << std::endl;
				continue;
			}
		}
		
		if(delaunayTri.size() == 0)
		{
			std::stringstream errorMessage;
				errorMessage << "\nError, there are no triangle generated during the delaunay triangulation. \n"
					 << "(bso/spatial_design/conformal/cf_building_model)." << std::endl;
				throw std::runtime_error(errorMessage.str());
		}
		
		return delaunayTri;
	} //delaunay()
	
	void cf_building_model::splitLines(const std::vector<utilities::geometry::vertex>& intsecV, std::vector<utilities::geometry::line_segment>& intsecL)
	{					
		// split lines in acordance to intersection points
		for(int k=0; k<intsecL.size(); k++)
		{
			for(const auto l :intsecV)
			{
				if(intsecL[k].isOnLine(l, mTol))
				{
					utilities::geometry::line_segment one = {{(intsecL[k])[0]},{l}};
					utilities::geometry::line_segment two = {{(intsecL[k])[1]},{l}};
					
					intsecL.push_back(one);
					intsecL.push_back(two);

					intsecL.erase(intsecL.begin() + k);

					k--; // Makes sure the for loop isn't snipping a constrain lines when one is ereased.
				}
			}
		}
		
		// remove ducplicates from intsecL
		auto intsecL_end = intsecL.end();
		for(auto it = intsecL.begin(); it != intsecL_end; ++it)
		{
			intsecL_end = std::remove(it + 1, intsecL_end, *it);
		}
		intsecL.erase(intsecL_end, intsecL.end());
		
		
		// remove ducplicates which has switched vertexes in intsecL		
		for(int k; k<intsecL.size(); k++)
		{
			for(auto m : intsecL)
			{
				if (m == intsecL[k]){continue;} //No tolerance needed. Just opperator to make sure the same entities are not checked with each other.
				if (m.isSameAs(intsecL[k], mTol)){intsecL.erase(intsecL.begin() + k);}
			}
		}		
	}//cf_space::splitLines
	
	cf_building_model::cf_building_model(const cf_building_model& rhs)
	{
		cf_building_model* newPtr;
		int spaceCount1 = 0;
		int spaceCount2 = 0;
		for (const auto& i : rhs.mMSModel) spaceCount1++;
		for (const auto& i : rhs.mMSNModel) spaceCount2++;
		if(spaceCount1 > 0) 
		{
			if(rhs.quadHexaInput.size() == 0) newPtr = new cf_building_model(rhs.mMSModel, rhs.mTol);
			else newPtr =new cf_building_model(rhs.mMSModel, rhs.quadHexaInput, rhs.mTol);
		}
		if(spaceCount2 > 0) 
		{
			if(rhs.triPrismInput.size() == 0) newPtr = new cf_building_model(rhs.mMSNModel, rhs.mTol);
			else newPtr = new cf_building_model(rhs.mMSNModel, rhs.triPrismInput, rhs.mTol);
		}
		*this = *newPtr;
	} // copy ctor()

	cf_building_model::cf_building_model(const ms_building& msModel, const double& tol /*= 1e-3*/)
	:	cf_geometry_model(tol), mMSModel(msModel), mTol(tol)
	{ // 
		for (const auto& i : mMSModel)
		{
			addSpace(*i);
		}
		this->makeConformal();
	} // 

	cf_building_model::cf_building_model(const ms_building& msModel, const std::vector<utilities::geometry::quad_hexahedron>& quadHexa, const double& tol /*= 1e-3*/)
	:	cf_geometry_model(tol), mMSModel(msModel), mTol(tol), quadHexaInput(quadHexa)
	{ // 
		int count = 0;
		for (const auto& i : mMSModel)
		{
			std::vector<utilities::geometry::quad_hexahedron> quadHexaInside;
			bso::utilities::geometry::quad_hexahedron spaceGeom = i->getGeometry();
			for(const auto j: quadHexa)
			{
				if(spaceGeom.isInside((j.getCenter()),mTol))
				{
					quadHexaInside.push_back(j);
					count++;
				}
			}
			addSpace(*i, quadHexaInside);
		}
		if(count != quadHexa.size())
		{
			std::stringstream errorMessage;
			errorMessage << "The inserted quad Hexahedrons are not all considered inside a space or are too often added to a space." << std::endl
									<< "(bso/spatial_design/conformal/cf_building_model.cpp). " << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
	} // 

	cf_building_model::cf_building_model(const ms_N_building& msNModel, const double& tol /*= 1e-3*/)
	:	cf_geometry_model(tol), mMSNModel(msNModel), mTol(tol)
	{ // 
		makeConformalND(msNModel);
	} // 

	cf_building_model::cf_building_model(const ms_N_building& msNModel, const std::vector<utilities::geometry::triangular_prism>& triPrism, const double& tol /*= 1e-3*/)
	:	cf_geometry_model(tol), mMSNModel(msNModel), mTol(tol), triPrismInput(triPrism)
	{ // 
		int count = 0;
		for (const auto& i : msNModel)
		{
			std::vector<utilities::geometry::triangular_prism> triPrismInside;
			bso::utilities::geometry::quad_hexahedron spaceGeom = i->getGeometry();
			for(const auto j: triPrism)
			{
				if(spaceGeom.isInside((j.getCenter()),mTol))
				{
					triPrismInside.push_back(j);
					count++;
				}
			}
			addSpaceD(*i, triPrismInside);
		}
		if(count != triPrism.size())
		{
			std::stringstream errorMessage;
			errorMessage << "The inserted triPrism are not all considered inside a space or are too often added to a space." << std::endl
									<< "(bso/spatial_design/conformal/cf_building_model.cpp). " << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
	} // 
	
	cf_building_model::~cf_building_model()
	{ // 
		for (auto& i : mCFSpaces) delete i;
		for (auto& i : mCFSurfaces) delete i;
		for (auto& i : mCFEdges) delete i;
		for (auto& i : mCFPoints) delete i;
	} // 

	void cf_building_model::writeToFile(std::string fileName, int writeOption) const
	{
		std::ofstream output;
		if (!fileName.empty()) output.open(fileName.c_str());
		
		if (fileName.empty() || !output.is_open())
		{
			std::stringstream errorMessage;
			errorMessage << "Could not open the following file to write a conformal model design to:" << std::endl
									<< ((fileName.empty())? "no input file given" : fileName) << std::endl
									<< "(bso/spatial_design/conformal/cf_building_model.cpp). " << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}

		if(writeOption == 1)
		{
			output << *this;
		}
		else if(writeOption == 2)
		{
			output << "\n\n The conformal model contains the following geometry: \n"
				   << "\n The building conformal model contains the following geometry: \n"
				<< "	The cf_space of size()= "<< mCFSpaces.size() << " are: \n";
				for(const auto i: mCFSpaces) 
				{
					output << i->getSpaceID() << " : " << *i << "\n";
					i->writeGeometryToFile(output);
				}
				output << "		The cf_surfaces of size()= "<< mCFSurfaces.size() << " are: \n";
				for(const auto i: mCFSurfaces) 
				{
					output << "		Surface = "<< *i << " ";
					i->writeGeometryToFile(output);
				}
				output << "		The cf_edges of size()= "<< mCFEdges.size() << " are: \n";
				for(const auto i: mCFEdges) 
				{
					output << "		Edges = "<< *i << " ";
					i->writeGeometryToFile(output);
				}
				output << "		The cf_points of size()= "<< mCFPoints.size() << " are: \n";
				for(const auto i: mCFPoints)
				{
					output << "		Points = "<< *i << " ";
					i->writeGeometryToFile(output);
				} 

				output << "\n The geometry conformal model contains the following geometry: \n";
				output << "		The cf_triPrisms of size()= "<< mCFTriPrisms.size() << " are: \n";
				for(const auto i: mCFTriPrisms) 
				{
					output << "		Triangular prisms = "<< *i << " ";
					i->writeGeometryToFile(output);
				}
				output << "		The cf_triangles of size()= "<< mCFTriangles.size() << " are: \n";
				for(const auto i: mCFTriangles) 
				{
					output << "		Triangles = "<< *i << " ";
					i->writeGeometryToFile(output);
				}
				output << "		The cf_rectangles of size()= "<< mCFRectangles.size() << " are: \n";
				for(const auto i: mCFRectangles) 
				{
					output << "		Rectangles = "<< *i << " ";
					i->writeGeometryToFile(output);
				}
				output << "		The cf_lines of size()= "<< mCFLines.size() << " are: \n";
				for(const auto i: mCFLines) 
				{
					output << "		Lines = "<< *i << " ";
					i->writeGeometryToFile(output);
				}
				output << "		The cf_vertices of size()= "<< mCFVertices.size() << " are: \n";
				for(const auto i: mCFVertices) 
				{
					output << "		Vertices = "<< *i << " ";
					i->writeGeometryToFile(output);
				}
		}
		else if (writeOption == 3)
		{
			output << "\n\n The conformal model contains the following geometry: \n"
				   << "\n The building conformal model contains the following geometry: \n"
				<< "	The cf_space of size()= "<< mCFSpaces.size() << "  \n"
				<< "	The cf_surfaces of size()= "<< mCFSurfaces.size() << "  \n"
				<< "	The cf_edges of size()= "<< mCFEdges.size() << "  \n"
				<< "	The cf_points of size()= "<< mCFPoints.size() << "  \n"
				<< "\n The geometry conformal model contains the following geometry: \n"
				<< "	The cf_triPrisms of size()= "<< mCFTriPrisms.size() << "  \n"
				<< "	The cf_triangles of size()= "<< mCFTriangles.size() << "  \n"
				<< "	The cf_rectangles of size()= "<< mCFRectangles.size() << "  \n"
				<< "	The cf_lines of size()= "<< mCFLines.size() << "  \n"
				<< "	The cf_vertices of size()= "<< mCFVertices.size() << "  \n";
		}
		else /*do nothing*/;
	} // writeToFile()

	std::ostream& operator <<(std::ostream& stream, const cf_building_model& building)
	{
		bool first = true;
		std::string temp; 
		for (auto i : building.cfSpaces())
		{
			if (!first) stream << std::endl;
			else first = false;
			stream << i->getSpaceID()  << ", ";
			stream << *i;
			stream << std::endl;

			bool first2 = true;
			for(auto j: i->cfTriPrism())
			{
				if (!first2) stream << std::endl;
				else first2 = false;
				
				stream << *j;
			}

			bool first3 = true;
			for(auto j: i->cfCuboids())
			{
				if (!first3) stream << std::endl;
				else first3 = false;
				stream << *j;
			}
		}

		return stream;
	} // << operator
	
} // conformal
} // spatial_design
} // bso

#endif // CF_BUILDING_MODEL_CPP