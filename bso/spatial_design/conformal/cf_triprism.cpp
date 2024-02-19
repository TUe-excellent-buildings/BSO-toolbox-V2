#ifndef CF_TRIPRISM_CPP
#define CF_TRIPRISM_CPP

namespace bso { namespace spatial_design { namespace conformal {
	
	cf_triPrism::cf_triPrism(const utilities::geometry::triangular_prism& rhs, cf_geometry_model* geometryModel)
	: utilities::geometry::triangular_prism(rhs, geometryModel->tolerance())
	{
		mGeometryModel = geometryModel;
		for (const auto& i : mVertices)
		{
			mCFVertices.push_back(mGeometryModel->addVertex(i));
			mCFVertices.back()->addTriPrism(this);
		}
		for (const auto& i : mLineSegments)
		{
			mCFLines.push_back(mGeometryModel->addLine(i));
			mCFLines.back()->addTriPrism(this);
		}
		for (const auto& i : mPolygons)
		{
			if(i->getMSize() == 3)
			{
				mCFTriangles.push_back(mGeometryModel->addTriangle(*i));
				mCFTriangles.back()->addTriPrism(this);
			}
			else if(i->getMSize() == 4)
			{
				mCFRectangles.push_back(mGeometryModel->addRectangle(*i));
				mCFRectangles.back()->addTriPrism(this);
			}
			else
			{
				std::stringstream errorMessage;
					errorMessage << "\nError, found a polygon with more then the allowed amound of vertices\n"
											 << "The amound of vertices should be either 3 or 4 and is " << mVertices.size() << "\n"
											 << "(bso/spatial_design/conformal/cf_triprism)." << std::endl;
					throw std::runtime_error(errorMessage.str());
			}
		}
	} // ctor

	void cf_triPrism::setAreas(double side1, double side2, double side3){
		// Set the values of the private variables with the provided values
		areaSide1 = side1;
		areaSide2 = side2;
		areaSide3 = side3;
		// Calculate the total area of the triangular prism
		totalArea = areaSide1 + areaSide2 + areaSide3;
	}
	
	cf_triPrism::~cf_triPrism()
	{ //function still needs to be updated
		for (const auto& i : mCFVertices) i->removeTriPrism(this);
		for (const auto& i : mCFLines) i->removeTriPrism(this);
		for (const auto& i : mCFTriangles) i->removeTriPrism(this);
		for (const auto& i : mCFRectangles) i->removeTriPrism(this);
	} // dtor

	// bool cf_triPrism::operator == (const cf_triPrism& rhs) const
	// {
	// 	auto sizeLeft = mCFVertices.size();
	// 	auto sizeRight = rhs.mCFVertices.size();
	// 	if (sizeLeft != sizeRight) return false;
	// 	for (int z = 0; z < sizeLeft; z++)
	// 	{	
	// 		if ((*(mCFVertices[i])) != (*(rhs.mCFVertices[i]))) return false;
	// 	}

	// 	return true;
	// }

	// bool cf_triPrism::operator != (const cf_triPrism& rhs) const
	// {
	// 	return !(*this == rhs);
	// }

} // conformal
} // spatial_design
} // bso

#endif // CF_TRIPRISM_CPP