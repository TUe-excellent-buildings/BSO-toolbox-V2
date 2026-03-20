#ifndef BSO_DAYLIGHT_WINDOW_HPP
#define BSO_DAYLIGHT_WINDOW_HPP

#include <vector>
#include <string>
#include <bso/utilities/geometry/vertex.hpp>
#include <bso/daylight/properties/window_properties.hpp>

namespace bso { namespace daylight {

class window : public bso::daylight::properties::window_properties
{
private:
	std::vector<bso::utilities::geometry::vertex> mVertices;
	unsigned int mSpaceID = 0;
	bool mIsExternal = false;
	bool mIsOpen = true;
	std::string mMaterialName = "window_glass";

public:
	window() = default;
	window(const std::vector<bso::utilities::geometry::vertex>& vertices, const unsigned int spaceID, const bool isExternal,
		const bool isOpen, const std::string& transmissivity, const std::string& transmittance);

	const std::vector<bso::utilities::geometry::vertex>& getVertices() const {return mVertices;}
	unsigned int getSpaceID() const {return mSpaceID;}
	bool isExternal() const {return mIsExternal;}
	bool isOpen() const {return mIsOpen;}
	const std::string& getMaterialName() const {return mMaterialName;}
	const std::string& getTransmissivity() const {return bso::daylight::properties::window_properties::getTransmissivity();}
	const std::string& getTransmittance() const {return bso::daylight::properties::window_properties::getTransmittance();}
	void setMaterialName(const std::string& materialName) {mMaterialName = materialName;}
	void setTransmissivity(const std::string& transmissivity) {bso::daylight::properties::window_properties::setTransmissivity(transmissivity);}
	void setTransmittance(const std::string& transmittance) {bso::daylight::properties::window_properties::setTransmittance(transmittance);}
	bso::daylight::properties::polygon_properties& properties() {return *this;}
	const bso::daylight::properties::polygon_properties& properties() const {return *this;}
};

} // namespace daylight
} // namespace bso

#include <bso/daylight/geometry/window.cpp>

#endif // BSO_DAYLIGHT_WINDOW_HPP
