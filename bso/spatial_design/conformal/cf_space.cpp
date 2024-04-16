#ifndef CF_SPACE_CPP
#define CF_SPACE_CPP

namespace bso { namespace spatial_design { namespace conformal {
	
	cf_space::cf_space(const utilities::geometry::quad_hexahedron& rhs, cf_building_model* buildingModel)
	:  utilities::geometry::quad_hexahedron(rhs,buildingModel->tolerance())
	{ // 
		mBuildingModel = buildingModel;
		mCFCuboids.push_back(mBuildingModel->addCuboid(*this));
		mCFCuboids.back()->addSpace(this);
	} // 

	cf_space::cf_space(const utilities::geometry::quad_hexahedron& rhs, cf_building_model* buildingModel, std::vector<utilities::geometry::triangular_prism> triPrism)
	:  utilities::geometry::quad_hexahedron(rhs,buildingModel->tolerance())
	{ // 
		mBuildingModel = buildingModel;
		for(const auto i : triPrism)
		{
			mCFTriPrisms.push_back(mBuildingModel->addTriPrism(i));
			mCFTriPrisms.back()->addSpace(this);
		}
	} // 

	cf_space::cf_space(const utilities::geometry::quad_hexahedron& rhs, cf_building_model* buildingModel, std::vector<utilities::geometry::quad_hexahedron> quadHexa)
	:  utilities::geometry::quad_hexahedron(rhs,buildingModel->tolerance())
	{ // 
		mBuildingModel = buildingModel;
		for(const auto i : quadHexa)
		{
			mCFCuboids.push_back(mBuildingModel->addCuboid(i));
			mCFCuboids.back()->addSpace(this);
		}
	} // 
	
	void cf_space::checkVertex(cf_vertex* pPtr)
	{ // 
		// check if the vertex is in or on the space's geometry
		bool onGeometry = false;

		if ((this->isInside(*pPtr, mBuildingModel->tolerance()))) onGeometry = true;

		for (const auto& i : mPolygons)
		{
			if (onGeometry) break;
			if (i->isInside(*pPtr, mBuildingModel->tolerance()))
			{
				onGeometry = true;
				break;
			}
		}
		for (const auto& i : mLineSegments)
		{
			if (onGeometry) break;
			if (i.isOnLine(*pPtr, mBuildingModel->tolerance()))
			{
				onGeometry = true;
				break;
			}
		}
		if (!onGeometry) return;  // if it isn't return the function immediately

		// check each cuboid if it can be split by the vertex
		for (unsigned int i = 0; i < mCFCuboids.size(); ++i)
		{
			mCFCuboids[i]->split(pPtr);
		}
	} // 
	
} // conformal
} // spatial_design
} // bso

#endif // CF_SPACE_CPP