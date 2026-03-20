#ifndef BSO_DL_POLYGONSURFACE_PROPERTIES_HPP
#define BSO_DL_POLYGONSURFACE_PROPERTIES_HPP

#include <map>
#include <string>

namespace bso { namespace daylight { namespace properties {

class polygon_properties
{
private:
	double mReflectance;
	double mSpecularity;
	double mRoughness;

public:
	polygon_properties(const double reflectance, const double specularity, const double roughness);
	~polygon_properties();

	double getReflectance() const {return mReflectance;}
	double getSpecularity() const {return mSpecularity;}
	double getRoughness() const {return mRoughness;}
};

} // namespace properties
} // namespace daylight
} // namespace bso

#include <bso/daylight/properties/surface_properties.cpp>

#endif // BSO_DL_POLYGONSURFACE_PROPERTIES_HPP