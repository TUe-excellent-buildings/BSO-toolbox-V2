#ifndef BSO_DAYLIGHT_CEILING_HPP
#define BSO_DAYLIGHT_CEILING_HPP

#include <vector>
#include <string>
#include <bso/utilities/geometry/vertex.hpp>
#include <bso/daylight/properties/surface_properties.hpp>

namespace bso { namespace daylight {

class ceiling : public bso::daylight::properties::polygon_properties
{
private:
	std::vector<bso::utilities::geometry::vertex> mVertices;
	unsigned int mSpaceID = 0;
	bool mIsExternal = false;
	std::string mMaterialName = "Ceiling";

public:
	ceiling() = default;
	ceiling(const std::vector<bso::utilities::geometry::vertex>& vertices, const unsigned int spaceID, const bool isExternal);

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

#include <bso/daylight/geometry/ceiling.cpp>

#endif // BSO_DAYLIGHT_CEILING_HPP
