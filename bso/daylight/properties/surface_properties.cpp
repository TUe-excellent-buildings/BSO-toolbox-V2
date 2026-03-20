#ifndef BSO_DL_SURFACE_PROPERTIES_CPP
#define BSO_DL_SURFACE_PROPERTIES_CPP

#include <bso/daylight/properties/surface_properties.hpp>

namespace bso { namespace daylight { namespace properties {

polygon_properties::polygon_properties(const double reflectance, const double specularity, const double roughness)
	: mReflectance(reflectance), mSpecularity(specularity), mRoughness(roughness)
{

}

polygon_properties::~polygon_properties()
{

}

} // namespace properties
} // namespace daylight
} // namespace bso

#endif // BSO_DL_SURFACE_PROPERTIES_CPP
