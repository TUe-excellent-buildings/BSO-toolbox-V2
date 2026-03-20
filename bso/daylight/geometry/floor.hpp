#ifndef BSO_DAYLIGHT_FLOOR_HPP
#define BSO_DAYLIGHT_FLOOR_HPP

#include <vector>
#include <string>
#include <bso/utilities/geometry/vertex.hpp>
#include <bso/daylight/properties/surface_properties.hpp>

namespace bso { namespace daylight {

class floor : public bso::daylight::properties::polygon_properties
{
private:
	std::vector<bso::utilities::geometry::vertex> mVertices;
	unsigned int mSpaceID = 0;
	bool mIsExternal = false;
	std::string mMaterialName = "Floor";

public:
	floor() = default;
	floor(const std::vector<bso::utilities::geometry::vertex>& vertices, const unsigned int spaceID, const bool isExternal);

	const std::vector<bso::utilities::geometry::vertex>& getVertices() const {return mVertices;}
	unsigned int getSpaceID() const {return mSpaceID;}
	bool isExternal() const {return mIsExternal;}
	const std::string& getMaterialName() const {return mMaterialName;}
	void setMaterialName(const std::string& materialName) {mMaterialName = materialName;}
	bso::daylight::properties::polygon_properties& properties() {return *this;}
	const bso::daylight::properties::polygon_properties& properties() const {return *this;}
};

} // namespace daylight
} // namespace bso

#include <bso/daylight/geometry/floor.cpp>

#endif // BSO_DAYLIGHT_FLOOR_HPP
