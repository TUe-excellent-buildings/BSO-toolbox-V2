#ifndef POLYGON_non_convex_HPP
#define POLYGON_non_convex_HPP

#include <bso/utilities/geometry/polygon.hpp>

namespace bso { namespace utilities { namespace geometry {
	
	// This class defines non_convex polygons without holes but is also able to handle convex polygons
	// The points inserted into this class should already be inserted in an ordered manner 
	// to allow for the correct definition of the non_convex polygon
	// See explanation at function SortPoint for more detail.
	class polygon_non_convex : public polygon
	{
	protected:
		void sortPoints(const double& tol = 1e-3);
		void triangulate(const double& tol = 1e-3);
		std::vector<triangle> mTriangles;
	public:
		polygon_non_convex();
		template <class CONTAINER>
		polygon_non_convex(const CONTAINER& l, const double& tol = 1e-3);
		polygon_non_convex(const std::initializer_list<vertex>&& l, const double& tol = 1e-3);
		//polygon_non_convex(const std::initializer_list<line_segment>&& l, const double& tol = 1e-3);
		polygon* clone();
		
		double getArea() const;
		bool isInside(const vertex& p1, const double& tol = 1e-3) const;
		bool isInsideOrOn(const vertex& p1, const double& tol = 1e-3) const;

		void convertToQuads(std::vector<quadrilateral>& quads);

		const auto& getTriangles() const {return mTriangles;}

		// default created center in the inherited polygon class is only valid for convex polygons, so function is deleted.
		vertex getCenter() const = delete;
		const vertex& center() const =delete;
	};

} // namespace geometry
} // namespace utilities
} // namespace bso

#include <bso/utilities/geometry/polygon_non_convex.cpp>

#endif // POLYGON_non_convex_HPP