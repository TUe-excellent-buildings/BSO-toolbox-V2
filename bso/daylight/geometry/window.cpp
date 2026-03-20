#ifndef BSO_DAYLIGHT_WINDOW_CPP
#define BSO_DAYLIGHT_WINDOW_CPP

#include <bso/daylight/geometry/window.hpp>

namespace bso { namespace daylight {

window::window(const std::vector<bso::utilities::geometry::vertex>& vertices, const unsigned int spaceID, const bool isExternal,
	const bool isOpen, const std::string& transmissivity, const std::string& transmittance)
:	bso::daylight::properties::window_properties(transmissivity, transmittance),
	mVertices(vertices), mSpaceID(spaceID), mIsExternal(isExternal), mIsOpen(isOpen)
{

}

} // namespace daylight
} // namespace bso

#endif // BSO_DAYLIGHT_WINDOW_CPP
