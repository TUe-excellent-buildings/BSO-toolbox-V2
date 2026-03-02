#ifndef BSO_GRAMMAR_CPP
#define BSO_GRAMMAR_CPP

namespace bso { namespace grammar {

grammar::grammar(const spatial_design::cf_building& cf)
:	mCFBuilding(cf),
	mLoadPanel(structural_design::component::structure())
{
	for (const auto& i : mCFBuilding.cfVertices())
	{
		mVertexProperties[i] = new rule_set::vertex_property(i);
	}
	for (const auto& i : mCFBuilding.cfLines())
	{
		mLineProperties[i] = new rule_set::line_property(i);
	}
	for (const auto& i : mCFBuilding.cfRectangles())
	{
		mRectangleProperties[i] = new rule_set::rectangle_property(i);
	}
	for (const auto& i : mCFBuilding.cfTriangles())
	{
		mTriangleProperties[i] = new rule_set::triangle_property(i);
	}
	for (const auto& i : mCFBuilding.cfCuboids())
	{
		mCuboidProperties[i] = new rule_set::cuboid_property(i);
	}
	for (const auto& i : mCFBuilding.cfTriPrism())
	{
		mTriprismProperties[i] = new rule_set::triprism_property(i);
	}
	for (const auto& i : mCFBuilding.cfPoints())
	{
		mPointProperties[i] = new rule_set::point_property(i);
	}
	for (const auto& i : mCFBuilding.cfEdges())
	{
		mEdgeProperties[i] = new rule_set::edge_property(i);
	}
	for (const auto& i : mCFBuilding.cfSurfaces())
	{
		mSurfaceProperties[i] = new rule_set::surface_property(i);
	}
	for (const auto& i : mCFBuilding.cfSpaces())
	{
		mSpaceProperties[i] = new rule_set::space_property(i);
	}

	// Retrieve window constraints from the CF building model based on either QH or TP conformal model.
	if(mCFBuilding.getWindowConstrainsPerSpaceQH().size() > 0)
	{
		mWindowsPerSpace = mCFBuilding.getWindowConstrainsPerSpaceQH();	
	}
	else if(mCFBuilding.getWindowConstrainsPerSpaceTP().size() > 0)
	{
		mWindowsPerSpace = mCFBuilding.getWindowConstrainsPerSpaceTP();	
	}
} // ctor()

grammar::~grammar()
{
	for (auto& i : mVertexProperties) delete i.second;
	for (auto& i : mLineProperties) delete i.second;
	for (auto& i : mRectangleProperties) delete i.second;
	for (auto & i: mTriangleProperties) delete i.second;												 
	for (auto& i : mCuboidProperties) delete i.second;
	for (auto & i: mTriprismProperties) delete i.second;												 
	for (auto& i : mPointProperties) delete i.second;
	for (auto& i : mEdgeProperties) delete i.second;
	for (auto& i : mSurfaceProperties) delete i.second;
	for (auto& i : mSpaceProperties) delete i.second;
} // dtor()

std::vector<bso::utilities::geometry::quadrilateral> grammar::getExteriorWalls()
{
	std::vector<bso::utilities::geometry::quadrilateral> exteriorWalls;
	std::map<spatial_design::conformal::cf_rectangle*, rule_set::rectangle_property*>::iterator it;
	for (it = mRectangleProperties.begin(); it != mRectangleProperties.end(); it++)
	{
		if((it->second)->isWall() && (it->second)->isExternal())
		{
			exteriorWalls.push_back(*(it->first));
		}
	}
	return exteriorWalls;
}

std::vector<bso::utilities::geometry::quadrilateral> grammar::getExteriorQuadrilaterals()
{
	std::vector<bso::utilities::geometry::quadrilateral> exteriorSurfaces;
	std::map<spatial_design::conformal::cf_rectangle*, rule_set::rectangle_property*>::iterator it;
	for (it = mRectangleProperties.begin(); it != mRectangleProperties.end(); it++)
	{
		if((it->second)->isExternal())
		{
			exteriorSurfaces.push_back(*(it->first));
		}
	}
	return exteriorSurfaces;
}

std::vector<bso::spatial_design::conformal::cf_space*> grammar::getSpacesOnBoundary(bool spacesOnBottomIsAlsoBoundary)
{
	std::vector<bso::spatial_design::conformal::cf_space*> boundaryCFSpaces;
	std::map<spatial_design::conformal::cf_rectangle*, rule_set::rectangle_property*>::iterator it;
	for (it = mRectangleProperties.begin(); it != mRectangleProperties.end(); it++)
	{
		if((it->second)->isExternal())
		{
			// skip space where only the floor is an external surface if spacesOnBottomIsAlsoBoundary is false
			if(!spacesOnBottomIsAlsoBoundary && (it->second)->isFloor()){continue;} 
			for (auto& j : it->first->cfSurfaces())
			{
				for (auto& space : j->cfSpaces())
				{
					bool spaceAlreadyIncluded = false;
					for(auto& existingSpace : boundaryCFSpaces)
					{
						if(existingSpace == space)
						{
							spaceAlreadyIncluded = true;
							break;
						}
					}
					if(!spaceAlreadyIncluded)
					{
						boundaryCFSpaces.push_back(space);
					}				
				}
			}
		}
	}
	std::map<spatial_design::conformal::cf_triangle*, rule_set::triangle_property*>::iterator itTri;
	for (itTri = mTriangleProperties.begin(); itTri != mTriangleProperties.end(); itTri++)
	{
		if((itTri->second)->isExternal())
		{
			// Skip spaces where only the floor is an external surface if spacesOnBottomIsAlsoBoundary is false
			if(!spacesOnBottomIsAlsoBoundary && (itTri->second)->isFloor()){continue;} 
			for (auto& j : itTri->first->cfSurfaces())
			{
				for (auto& space : j->cfSpaces())
				{
					bool spaceAlreadyIncluded = false;
					for(auto& existingSpace : boundaryCFSpaces)
					{
						if(existingSpace == space)
						{
							spaceAlreadyIncluded = true;
							break;
						}
					}
					if(!spaceAlreadyIncluded)
					{
						boundaryCFSpaces.push_back(space);
					}				
				}
			}
		}
	}

	return boundaryCFSpaces;
}
	
} // namespace grammar
} // namespace bso

#endif // BSO_GRAMMAR_CPP