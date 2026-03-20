#ifndef BSO_DAYLIGHT_FLOOR_CPP
#define BSO_DAYLIGHT_FLOOR_CPP

#include <bso/daylight/geometry/floor.hpp>

namespace bso { namespace daylight {

floor::floor(const std::vector<bso::utilities::geometry::vertex>& vertices, const unsigned int spaceID, const bool isExternal)
:	mVertices(vertices), mSpaceID(spaceID), mIsExternal(isExternal)
{

}

} // namespace daylight
} // namespace bso

#endif // BSO_DAYLIGHT_FLOOR_CPP
