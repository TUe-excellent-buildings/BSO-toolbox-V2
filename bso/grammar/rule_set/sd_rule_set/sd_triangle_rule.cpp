#ifndef BSO_GRAMMAR_SD_TRIANGLE_RULE_CPP
#define BSO_GRAMMAR_SD_TRIANGLE_RULE_CPP

namespace bso { namespace grammar { namespace rule_set { namespace sd_rule_set {

sd_triangle_rule::sd_triangle_rule(triangle_property* triangleProperty)
:	mTriangleProperty(triangleProperty), mStructure(structural_design::component::structure()),
	mLoadPanel(structural_design::component::structure())
{

} // ctor()

sd_triangle_rule::~sd_triangle_rule()
{

} // dtor()

void sd_triangle_rule::apply(bso::structural_design::sd_model& sd) const
{//NOTE; this void is a direct copy of the rectangular version
	
	// create structure if applicable
	structural_design::component::geometry* triGeometry = nullptr;

	if (mStructure.type() == "flat_shell")
	{ // create a flat shell at the location of the rectangle
		triGeometry = sd.addGeometry(*(mTriangleProperty->getTrianglePtr()));
		triGeometry->addStructure(mStructure);
	}
	else if (mStructure.type() == "quad_hexahedron")
	{
		triGeometry = sd.addGeometry(*(mTriangleProperty->getTrianglePtr()));
	}
	
	// assign possible loads
	// NOTE: all load cases are stored in their derived components x,y, or z. One, two or three mLoads may therefore described one load of a load case defined in the input file.

	if (mTriangleProperty->isFloor() && !mTriangleProperty->isRoof() && mLoads != nullptr)
	{ // apply a floor load
		for (const auto& i : *mLoads)
		{
			if (i.first == "live_load")
			{
				if (triGeometry == nullptr)
				{
					triGeometry = sd.addGeometry(*(mTriangleProperty->getTrianglePtr()));
				}
				triGeometry->addLoad(i.second);
			}
		}
	}
	// assign possible loads
	if (mTriangleProperty->isFloor() && mTriangleProperty->isRoof() && mLoads != nullptr)
	{ // apply a floor load
		for (const auto& i : *mLoads)
		{
			if (i.first == "roof_load")
			{
				if (triGeometry == nullptr)
				{
					triGeometry = sd.addGeometry(*(mTriangleProperty->getTrianglePtr()));
				}
				triGeometry->addLoad(i.second);
			}
		}
	}
	if (mTriangleProperty->isExternal() && mTriangleProperty->isAboveGroundSurface() && mLoads != nullptr)
	{ // apply wind loading
		for (const auto& i : *mLoads)
		{
			if (!(i.first == "wind_pressure" || i.first == "wind_suction" ||
						i.first == "wind_shear")) continue;
			auto load = i.second;
			utilities::geometry::vector triangleOutwardOrientation = -1*mTriangleProperty->getOrientation();
			triangleOutwardOrientation.normalize();

			utilities::geometry::vector loadOrientation;
			// load.DOF() is: Direction Of Force in either x/y/z defined as integer 0/1/2 corosponding to the assumed positive load direction (1,0,0)/(0,1,0)/(0,0,1).
			if (load.DOF() == 0) loadOrientation = {1,0,0};
			else if (load.DOF() == 1) loadOrientation = {0,1,0};
			else if (load.DOF() == 2) loadOrientation = {0,0,1};
			else
			{
				std::stringstream errorMessage;
				errorMessage << "\nError, while applying a sd triangle rule, encountered a\n"
										 << "wind load define in DOF: " << load.DOF() << ". Which is not valid.\n"
										 << "(bso/grammar/rule_set/sd_rule_set/sd_triangle_rule.cpp)" << std::endl;
				throw std::runtime_error(errorMessage.str());
			}
			// In case the magnitue < 0, then the direction of force is against the positive asumed direction.
			if (load.magnitude() < 0) loadOrientation *= -1;
			// beta = the angle between the load direction and the rectangleoutwardOrientation
			double beta = abs(triangleOutwardOrientation.getHeading().first - 
												loadOrientation.getHeading().first);
			if (beta > 180) beta = 360 - beta; // Take the shortest beta angle posible

			if (i.first == "wind_pressure" && (90 < beta && beta <= 180) && 
				!triangleOutwardOrientation.isVertical())
			{
				load *= loadOrientation.dot(-1*triangleOutwardOrientation);
				// no need for division by norm product since they are both normalized
			}
			else if (i.first == "wind_suction" && (0 <= beta && beta < 90) && 
				!triangleOutwardOrientation.isVertical())
			{
				load *= loadOrientation.dot(1*triangleOutwardOrientation);
				// no need for division by norm product since they are both normalized
			}
			else if (i.first == "wind_shear" && ((90 <= beta && beta < 180) || 
				triangleOutwardOrientation.isVertical()))
			{
				load *= (1 - loadOrientation.dot(1*triangleOutwardOrientation));
				// no need for division by norm product since they are both normalized
			}
			else continue;
			if (triGeometry == nullptr)
			{
				triGeometry = sd.addGeometry(*(mTriangleProperty->getTrianglePtr()));
			}
			triGeometry->addLoad(load);
		}
	}
	if ((triGeometry != nullptr) && (triGeometry->getStructures().size() == 0) &&
			(triGeometry->getLoads().size() != 0))
	{ // a low density load panel is required
		if (mLoadPanel.type() == "flat_shell")
		{ // if such a load panel has been defined, add the structure to the geometry
			triGeometry->addStructure(mLoadPanel);
		}
		else
		{
			std::stringstream errorMessage;
			errorMessage << "\nError, while applying an sd triangle rule, applied a load\n"
									 << "but there is no structure present to carry this load.\n"
									 << "And no suitable low stiffness load panel has been defined\n"
									 << "(bso/grammar/rule_set/sd_rule_set/sd_rectangle_rule.cpp)" << std::endl;
			throw std::runtime_error(errorMessage.str());
		}
	}
} // apply()

void sd_triangle_rule::assignStructure(structural_design::component::structure structure)
{
	mStructure = structure;
}

void sd_triangle_rule::assignStructure(const std::vector<structural_design::component::structure>&
	potentialStructure)
{// NOTE; this void is a direct copy of the rectangular version and is not up to date yet
	mStructure = structural_design::component::structure();
	mStructure.isGhostComponent() = true;
	for (const auto& i : potentialStructure)
	{
		if (!(i.isGhostComponent()) && mStructure.isGhostComponent())
		{
			mStructure = i;
			continue;
		}
		else if (!(mStructure.isGhostComponent()) && i.isGhostComponent())
		{
			continue;
		} // otherwise either both are ghost component or both are not
		
		/* This type of ellement is not defined for triangular prisms.
		if (i.type() == "quad_hexahedron")
		{ // quad_hexahedron
			if (mStructure.type() != "quad_hexahedron")
			{
				mStructure = i;
			}
			else if (mStructure.type() == "quad_hexahedron")
			{
				if (mStructure.E() < i.E())
				{
					mStructure = i;
				}
			}
		}
		else */if (i.type() == "flat_shell")
		{ // flat shell
			if (mStructure.type() != "flat_shell" && mStructure.type() != "quad_hexahedron")
			{
				mStructure = i;
			}
			else if (mStructure.type() == "flat_shell")
			{
				if (mStructure.thickness() < i.thickness())
				{
					mStructure = i;
				}
			}
		}
		else if (i.type() == "beam")
		{ // beam
			if (mStructure.type() != "beam" && mStructure.type() != "flat_shell" &&
					mStructure.type() != "quad_hexahedron")
			{
				mStructure = i;
			}
			else if (mStructure.type() == "beam")
			{
				if (mStructure.width()*mStructure.height() < i.width()*i.height())
				{
					mStructure = i;
				}
			}
		}
		else if (i.type() == "truss")
		{ // truss
			if (mStructure.type() != "beam" && mStructure.type() != "flat_shell" &&
					mStructure.type() != "truss" && mStructure.type() != "quad_hexahedron")
			{
				mStructure = i;
			}
			else if (mStructure.type() == "truss")
			{
				if (mStructure.A() < i.A())
				{
					mStructure = i;
				}
			}
		}
	}
} // assignStructure()

void sd_triangle_rule::assignLoadPanel(structural_design::component::structure loadPanel)
{
	mLoadPanel = loadPanel;
	mLoadPanel.isGhostComponent() = true;
	mLoadPanel.isVisible() = false;
}

void sd_triangle_rule::addLoads(std::multimap<std::string, structural_design::component::load>* loads)
{
	mLoads = loads;
} // addLoads()

} // namespace sd_rule_set
} // namespace rule_set
} // namespace grammar
} // namespace bso

#endif // BSO_GRAMMAR_SD_TRIANGLE_RULE_CPP