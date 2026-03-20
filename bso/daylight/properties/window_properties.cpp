#ifndef BSO_DL_WINDOW_PROPERTIES_CPP
#define BSO_DL_WINDOW_PROPERTIES_CPP

#include <bso/daylight/properties/window_properties.hpp>

namespace bso { namespace daylight { namespace properties {

window_properties::window_properties(const std::string& transmissivity, const std::string& transmittance)
:	mTransmissivity(transmissivity), mTransmittance(transmittance)
{

}

} // namespace properties
} // namespace daylight
} // namespace bso

#endif // BSO_DL_WINDOW_PROPERTIES_CPP
