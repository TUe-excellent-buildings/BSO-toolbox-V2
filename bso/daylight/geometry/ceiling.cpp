#ifndef BSO_DAYLIGHT_CEILING_CPP
#define BSO_DAYLIGHT_CEILING_CPP

#include <bso/daylight/geometry/ceiling.hpp>

namespace bso { namespace daylight {

ceiling::ceiling(const std::vector<bso::utilities::geometry::vertex>& vertices, const unsigned int spaceID, const bool isExternal)
:	mVertices(vertices), mSpaceID(spaceID), mIsExternal(isExternal)
{

}

} // namespace daylight
} // namespace bso

#endif // BSO_DAYLIGHT_CEILING_CPP
