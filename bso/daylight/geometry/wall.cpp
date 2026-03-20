#ifndef BSO_DAYLIGHT_WALL_CPP
#define BSO_DAYLIGHT_WALL_CPP

#include <bso/daylight/geometry/wall.hpp>

namespace bso { namespace daylight {

wall::wall(const std::vector<bso::utilities::geometry::vertex>& vertices, const unsigned int spaceID, const bool isExternal, const bool isOpen)
:	mVertices(vertices), mSpaceID(spaceID), mIsExternal(isExternal), mIsOpen(isOpen)
{

}

} // namespace daylight
} // namespace bso

#endif // BSO_DAYLIGHT_WALL_CPP
