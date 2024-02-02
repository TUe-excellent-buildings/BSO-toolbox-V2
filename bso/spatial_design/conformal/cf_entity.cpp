#ifndef CF_GEOMETRY_ENTITY_CPP
#define CF_GEOMETRY_ENTITY_CPP

namespace bso { namespace spatial_design { namespace conformal {

	void cf_entity::addLine(cf_line* lPtr)
	{ // 
		if (std::find(mCFLines.begin(),mCFLines.end(),lPtr) == mCFLines.end())
		{
			mCFLines.push_back(lPtr);
		}
	} // 

	void cf_entity::addRectangle(cf_rectangle* recPtr)
	{ // 
		if (std::find(mCFRectangles.begin(),mCFRectangles.end(),recPtr) == mCFRectangles.end())
		{
			mCFRectangles.push_back(recPtr);
		}
	} // 

	void cf_entity::addTriangle(cf_triangle* triPtr)
	{ // 
		if (std::find(mCFTriangles.begin(),mCFTriangles.end(),triPtr) == mCFTriangles.end())
		{
			mCFTriangles.push_back(triPtr);
		}
	} // 
	
	void cf_entity::addCuboid(cf_cuboid* cubPtr)
	{ // 
		if (std::find(mCFCuboids.begin(),mCFCuboids.end(),cubPtr) == mCFCuboids.end())
		{
			mCFCuboids.push_back(cubPtr);
		}
	} // 
	
	void cf_entity::addTriPrism(cf_triPrism*		priPtr)
	{ // 
		if (std::find(mCFTriPrisms.begin(),mCFTriPrisms.end(),priPtr) == mCFTriPrisms.end())
		{
			mCFTriPrisms.push_back(priPtr);
		}
	} //

	void cf_entity::removeLine(cf_line* lPtr)
	{ // 
		mCFLines.erase(std::remove(mCFLines.begin(),mCFLines.end(),lPtr), mCFLines.end());
	} // 

	void cf_entity::removeRectangle(cf_rectangle* recPtr)
	{ // 
		mCFRectangles.erase(std::remove(mCFRectangles.begin(),mCFRectangles.end(),recPtr), mCFRectangles.end());
	} // 

	void cf_entity::removeTriangle(cf_triangle* triPtr)
	{ // 
		mCFTriangles.erase(std::remove(mCFTriangles.begin(),mCFTriangles.end(),triPtr), mCFTriangles.end());
	} // 
	
	void cf_entity::removeCuboid(cf_cuboid* cubPtr)
	{ // 
		mCFCuboids.erase(std::remove(mCFCuboids.begin(),mCFCuboids.end(),cubPtr), mCFCuboids.end());
	} // 
	
	void cf_entity::removeTriPrism(cf_triPrism*	triPtr)
	{ // 
		mCFTriPrisms.erase(std::remove(mCFTriPrisms.begin(),mCFTriPrisms.end(),triPtr), mCFTriPrisms.end());
	} // 

	void cf_entity::addPoint(cf_point* pPtr)
	{ // 
		if (std::find(mCFPoints.begin(),mCFPoints.end(),pPtr) == mCFPoints.end())
		{
			mCFPoints.push_back(pPtr);
		}
	} // 

	void cf_entity::addEdge(cf_edge* ePtr)
	{ // 
		if (std::find(mCFEdges.begin(),mCFEdges.end(),ePtr) == mCFEdges.end())
		{
			mCFEdges.push_back(ePtr);
		}
	} // 

	void cf_entity::addSurface(cf_surface* srfPtr)
	{ // 
		if (std::find(mCFSurfaces.begin(),mCFSurfaces.end(),srfPtr) == mCFSurfaces.end())
		{
			mCFSurfaces.push_back(srfPtr);
		}
	} // 

	void cf_entity::addSpace(cf_space* spPtr)
	{ // 
		if (std::find(mCFSpaces.begin(),mCFSpaces.end(),spPtr) == mCFSpaces.end())
		{
			mCFSpaces.push_back(spPtr);
		}
	} // 


} // conformal
} // spatial_design
} // bso

#endif // CF_GEOMETRY_ENTITY_CPP