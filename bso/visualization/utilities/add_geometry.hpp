#ifndef VIS_ADD_GEOMETRY_HPP
#define VIS_ADD_GEOMETRY_HPP

#include <bso/utilities/geometry/quadrilateral.hpp>

#include <cmath>
#include <vector>

namespace bso { namespace visualization
{

inline bool& showModelLegend()
{
	static bool enabled = true;
	return enabled;
}

inline bool& showModelOverlayPlanes()
{
	static bool enabled = true;
	return enabled;
}

inline std::vector<bso::utilities::geometry::quadrilateral>& modelOverlayPlanes()
{
	static std::vector<bso::utilities::geometry::quadrilateral> planes;
	return planes;
}

inline void setModelOverlayPlanes(const std::vector<bso::utilities::geometry::quadrilateral>& planes)
{
	modelOverlayPlanes() = planes;
}

inline void clearModelOverlayPlanes()
{
	modelOverlayPlanes().clear();
}

inline bso::utilities::geometry::quadrilateral makeModelOverlayPlane(
	const bso::utilities::geometry::vertex& location,
	bso::utilities::geometry::vector normal,
	const double halfSize = 30000.0)
{
	if (normal.isZero(1e-9)) return {};
	normal.normalize();

	bso::utilities::geometry::vector reference = {0, 0, 1};
	if (std::abs(normal.dot(reference)) > 0.9) reference = {0, 1, 0};

	auto axis1 = normal.cross(reference);
	if (axis1.isZero(1e-9))
	{
		reference = {1, 0, 0};
		axis1 = normal.cross(reference);
	}
	axis1.normalize();
	auto axis2 = normal.cross(axis1);
	axis2.normalize();

	return bso::utilities::geometry::quadrilateral({
		location - axis1 * halfSize - axis2 * halfSize,
		location + axis1 * halfSize - axis2 * halfSize,
		location + axis1 * halfSize + axis2 * halfSize,
		location - axis1 * halfSize + axis2 * halfSize
	});
}

inline void setModelOverlayPlane(
	const bso::utilities::geometry::vertex& location,
	bso::utilities::geometry::vector normal,
	const double halfSize = 30000.0)
{
	setModelOverlayPlanes({makeModelOverlayPlane(location, normal, halfSize)});
}

} // namespace visualization
} // namespace bso

#endif // VIS_ADD_GEOMETRY_HPP
