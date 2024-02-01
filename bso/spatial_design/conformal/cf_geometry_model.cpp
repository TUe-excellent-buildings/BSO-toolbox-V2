#ifndef CF_GEOMETRY_MODEL_CPP
#define CF_GEOMETRY_MODEL_CPP

namespace bso { namespace spatial_design { namespace conformal {
	
	cf_geometry_model::cf_geometry_model(const double& tol /*= 1e-3*/)
	{ // 
		mTol = tol;
		mDec = -log10(tol);
	} // empty ctor, nothing to initialize

	cf_geometry_model::~cf_geometry_model()
	{ // 
		// beware order is important here!
		for (auto& i : mCFCuboids) delete i;
		for (auto& i : mCFTriPrisms) delete i;
		for (auto& i : mCFRectangles) delete i;
		for (auto& i : mCFTriangles) delete i;
		for (auto& i : mCFLines) delete i;
		for (auto& i : mCFVertices) delete i;

		mCFVertices.clear();
		mCFLines.clear();
		mCFRectangles.clear();
		mCFTriangles.clear();
		mCFCuboids.clear();
		mCFTriPrisms.clear();
	} // 

	cf_vertex* cf_geometry_model::addVertex(const bso::utilities::geometry::vertex& p)
	{ // 
		for (const auto& i : mCFVertices)
		{
			if (i->isSameAs(p, mTol))
			{
				return i;
			}
		}
		mCFVertices.push_back(new cf_vertex(p));
		return mCFVertices.back();
	} // 

	cf_line* cf_geometry_model::addLine(const bso::utilities::geometry::line_segment& l)
	{ // 
		for (const auto& i : mCFLines)
		{
			if (i->isSameAs(l, mTol))
			{
				return i;
			}
		}
		mCFLines.push_back(new cf_line(l, this));
		return mCFLines.back();
	} // 

	cf_rectangle* cf_geometry_model::addRectangle(const bso::utilities::geometry::quadrilateral& quad)
	{ // 
		for (const auto& i : mCFRectangles)
		{
			if (i->isSameAs(quad, mTol))
			{
				return i;
			}
		}
		mCFRectangles.push_back(new cf_rectangle(quad, this));
		return mCFRectangles.back();
	} // 

	cf_triangle* cf_geometry_model::addTriangle(const bso::utilities::geometry::triangle& tri)
	{ // 
		for (const auto& i : mCFTriangles)
		{
			if (i->isSameAs(tri, mTol))
			{
				return i;
			}
		}
		mCFTriangles.push_back(new cf_triangle(tri, this));
		return mCFTriangles.back();
	} //   
	
	cf_cuboid* cf_geometry_model::addCuboid(const bso::utilities::geometry::quad_hexahedron& qhex)
	{ // 
		for (const auto& i : mCFCuboids)
		{
			if (i->isSameAs(qhex, mTol))
			{
				return i;
			}
		}
		mCFCuboids.push_back(new cf_cuboid(qhex, this));
		return mCFCuboids.back();
	} // 
	
	cf_triPrism* cf_geometry_model::addTriPrism(const bso::utilities::geometry::triangular_prism& triPtr)
	{
		for (const auto& i : mCFTriPrisms)
		{
			if (i->isSameAs(triPtr, mTol))
			{
				return i;
			}
		}
		mCFTriPrisms.push_back(new cf_triPrism(triPtr, this));
		return mCFTriPrisms.back();
	}																							  

	void cf_geometry_model::removeLine(cf_line* lPtr)
	{ // 
		mCFLines.erase(std::remove(mCFLines.begin(), mCFLines.end(), lPtr), mCFLines.end());
    delete lPtr;
	} // 

	void cf_geometry_model::removeRectangle(cf_rectangle* recPtr)
	{ // 
		mCFRectangles.erase(std::remove(mCFRectangles.begin(), mCFRectangles.end(), recPtr), mCFRectangles.end());
    delete recPtr;
	} // 
	
	void cf_geometry_model::removeTriangle(cf_triangle* triPtr)
	{ // 
		mCFTriangles.erase(std::remove(mCFTriangles.begin(), mCFTriangles.end(), triPtr), mCFTriangles.end());
    delete triPtr;
	} // 

	void cf_geometry_model::removeCuboid(cf_cuboid* cubPtr)
	{ // 
		mCFCuboids.erase(std::remove(mCFCuboids.begin(), mCFCuboids.end(), cubPtr), mCFCuboids.end());
    delete cubPtr;
	} // 
	
	void cf_geometry_model::removeTriPrism(cf_triPrism* triPtr)
	{ // 
		mCFTriPrisms.erase(std::remove(mCFTriPrisms.begin(), mCFTriPrisms.end(), triPtr), mCFTriPrisms.end());
    delete triPtr;
	} // 
	
} // conformal
} // spatial_design
} // bso

#endif // CF_GEOMETRY_MODEL_CPP