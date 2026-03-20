#ifndef BSO_DL_WINDOW_PROPERTIES_HPP
#define BSO_DL_WINDOW_PROPERTIES_HPP

#include <string>
#include <bso/daylight/properties/surface_properties.hpp>

namespace bso { namespace daylight { namespace properties {

class window_properties : public polygon_properties
{
private:
	std::string mTransmissivity = "";
	std::string mTransmittance = "";

public:
	window_properties() = default;
	window_properties(const std::string& transmissivity, const std::string& transmittance);
	~window_properties() = default;

	const std::string& getTransmissivity() const {return mTransmissivity;}
	const std::string& getTransmittance() const {return mTransmittance;}
	void setTransmissivity(const std::string& transmissivity) {mTransmissivity = transmissivity;}
	void setTransmittance(const std::string& transmittance) {mTransmittance = transmittance;}
};

} // namespace properties
} // namespace daylight
} // namespace bso

#include <bso/daylight/properties/window_properties.cpp>

#endif // BSO_DL_WINDOW_PROPERTIES_HPP