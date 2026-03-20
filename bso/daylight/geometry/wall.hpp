#ifndef BSO_DAYLIGHT_WALL_HPP
#define BSO_DAYLIGHT_WALL_HPP

#include <vector>
#include <string>
#include <bso/utilities/geometry/vertex.hpp>
#include <bso/daylight/properties/surface_properties.hpp>

namespace bso { namespace daylight {

class wall : public bso::daylight::properties::polygon_properties
{
private:
	std::vector<bso::utilities::geometry::vertex> mVertices;
	unsigned int mSpaceID = 0;
	bool mIsExternal = false;
	bool mIsOpen = false;
	std::string mMaterialName = "Wall";

public:
	wall() = default;
	wall(const std::vector<bso::utilities::geometry::vertex>& vertices, const unsigned int spaceID, const bool isExternal, const bool isOpen);

	const std::vector<bso::utilities::geometry::vertex>& getVertices() const {return mVertices;}
	unsigned int getSpaceID() const {return mSpaceID;}
	bool isExternal() const {return mIsExternal;}
	bool isOpen() const {return mIsOpen;}
	const std::string& getMaterialName() const {return mMaterialName;}
	void setMaterialName(const std::string& materialName) {mMaterialName = materialName;}
	bso::daylight::properties::polygon_properties& properties() {return *this;}
	const bso::daylight::properties::polygon_properties& properties() const {return *this;}
};

} // namespace daylight
} // namespace bso

#include <bso/daylight/geometry/wall.cpp>

#endif // BSO_DAYLIGHT_WALL_HPP
