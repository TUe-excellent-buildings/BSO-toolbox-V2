#ifndef BSO_GRAMMAR_TRIANGLE_PROPERTY_CPP
#define BSO_GRAMMAR_TRIANGLE_PROPERTY_CPP

namespace bso { namespace grammar { namespace rule_set {

triangle_property::triangle_property(bso::spatial_design::conformal::cf_triangle* triangle)
:	mTriangle(triangle), mOrientation(triangle->getNormal().normalized())
{
	mIsTriangleProperty = true;
		
	if (mTriangle->cfSurfaces().size() == 1) mIsExternal = true;
	if (mTriangle->cfSurfaces().size() == 1 ||
			mTriangle->cfSurfaces().size() == 2) mIsSpaceSeparating = true;
	
	if (mIsExternal)
	{ // Check!; this may not be the same for triangles and for rectangles and it is currently exectly the same setup as the rectangle
		bso::utilities::geometry::vertex spaceCenter = 
			mTriangle->cfSurfaces().front()->cfSpaces().front()->getCenter();
		bso::utilities::geometry::vertex checkVertex = mTriangle->getCenter() +
			(sqrt(mTriangle->getArea()) * mOrientation);
		if (!(mTriangle->sameSide(spaceCenter,checkVertex)))
		{
			mOrientation *= -1;
		}
	}
	
	if (abs(mOrientation.getHeading().second) > 45) mIsFloor = true;
	else mIsWall = true;
	
	for (const auto& i : *mTriangle)
	{
		if (i(2) > 1e-9)
		{
			mIsAboveGroundSurface = true;
			break;
		}
	}
	
	if (mOrientation.isVertical())
	{
		if (mTriangle->getCenter()(2) > -1e-9 && //Why is there a value defined for minimum?
				mTriangle->getCenter()(2) <  1e-9) 
		{
			mIsBelowGroundSurface = false;
		}
		else mIsBelowGroundSurface = !mIsAboveGroundSurface;
	}
	else mIsBelowGroundSurface = !mIsAboveGroundSurface;
	
	if (mIsFloor && mIsExternal && !mIsBelowGroundSurface)
	{
		if(mOrientation(2) < -1e-9) mIsRoof = true;
		else mIsOverhang = true;
	}
	
	if (mIsSpaceSeparating)
	{
		std::string space_type_1 = mTriangle->cfSurfaces()[0]->cfSpaces().front()->getSpaceType();
		std::string surface_type_1 = mTriangle->cfSurfaces()[0]->getSurfaceType();
		std::string space_type_2, surface_type_2;
		std::string externalType = "E";
		std::string groundType = "G";

		if (mIsExternal)
		{
			if (mIsAboveGroundSurface)
			{
				space_type_2 = externalType;
				surface_type_2 = externalType;
			}
			else
			{
				space_type_2 = groundType;
				surface_type_2 = groundType;
			}
		}
		else
		{
			space_type_2 = mTriangle->cfSurfaces()[1]->cfSpaces().front()->getSpaceType();
			surface_type_2 = mTriangle->cfSurfaces()[1]->getSurfaceType();
			
			std::stringstream errorMessage;
			errorMessage << "\nError, while initializing a triangle property, encountered\n"
									 << "a type assignment to either a space or surface which is\n"
									 << "reserved for internal use in the toolbox. Type encountered: ";
			std::string fileLocation = "(bso/grammar/rule_set/triangle_property.cpp)";
			if (space_type_1 == externalType || space_type_2 == externalType ||
					surface_type_1 == externalType || surface_type_2 == externalType)
			{
				
				errorMessage << "\"" << externalType << "\"\n" << fileLocation << std::endl;
				throw std::runtime_error(errorMessage.str());
			}
			if (space_type_1 == groundType || space_type_2 == groundType ||
					surface_type_1 == groundType || surface_type_2 == groundType)
			{
				errorMessage << "\"" << groundType << "\"\n" << fileLocation << std::endl;
				throw std::runtime_error(errorMessage.str());
			}
		}
		mSpaceTypes = std::minmax(space_type_1, space_type_2);
		mSurfaceTypes = std::minmax(surface_type_1, surface_type_2);
	}
} // ctor()

triangle_property::~triangle_property()
{

} // dtor()

} // namespace rule_set
} // namespace grammar
} // namespace bso

#endif // BSO_GRAMMAR_TRIANGLE_PROPERTY_CPP