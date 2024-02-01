#ifndef CF_TRIANGLE_CPP
#define CF_TRIANGLE_CPP

namespace bso { namespace spatial_design { namespace conformal {
	
	cf_triangle::cf_triangle(const utilities::geometry::triangle& rhs, cf_geometry_model* geometryModel)
	: utilities::geometry::triangle(rhs, geometryModel->tolerance())
	{
		mGeometryModel = geometryModel;
		for (const auto& i : mVertices)
		{
			mCFVertices.push_back(mGeometryModel->addVertex(i));
			mCFVertices.back()->addTriangle(this);
		}
		for (const auto& i : mLineSegments)
		{
			mCFLines.push_back(mGeometryModel->addLine(i));
			mCFLines.back()->addTriangle(this);
		}
	} // ctor
	
	cf_triangle::~cf_triangle()
	{
		for (const auto& i : mCFVertices) i->removeTriangle(this);
		for (const auto& i : mCFLines) i->removeTriangle(this);
	} // dtor
	
	
} // conformal
} // spatial_design
} // bso

#endif // CF_TRIANGLE_CPP