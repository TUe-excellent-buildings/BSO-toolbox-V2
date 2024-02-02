#ifndef TRIANGULAR_PRISM_HPP
#define TRIANGULAR_PRISM_HPP

#include <bso/utilities/geometry/polyhedron.hpp>
#include <bso/utilities/geometry/tetrahedron.hpp>

namespace bso { namespace utilities { namespace geometry {
	
	class triangular_prism : public polyhedron
	{
	protected:
		std::vector<tetrahedron> mTetrahedrons; // decomposition into tetrahedrons
		std::vector<triangle> mTriangles;
		std::vector<quadrilateral> mQuad;
		void sortPoints(const double& tol = 1e-3);
	public:
		triangular_prism();
		template <typename CONTAINER>
		triangular_prism(const CONTAINER& l, const double& tol = 1e-3);
		triangular_prism(const std::initializer_list<vertex>&& l, const double& tol = 1e-3);
		triangular_prism(const triangular_prism& rhs, const double& tol = 1e-3); // copy ctor, need this for mPolygons, to be copied properly
		polyhedron* clone();
		
		double getVolume() const;
		bool isInside(const vertex& p1, const double& tol = 1e-3) const;
		bool isInsideOrOn(const vertex& p1, const double& tol = 1e-3) const;
		
		const std::vector<tetrahedron>& 		getTetrahedrons() 		const { return mTetrahedrons;}
		const std::vector<triangle>&			getTriangles()			const { return mTriangles;}
		const std::vector<quadrilateral>&		getQuadrilateral()		const { return mQuad;}
		
		// void isOposide();
	};
	
} // namespace geometry
} // namespace utilities
} // namespace bso

#include <bso/utilities/geometry/triangular_prism.cpp>

#endif // TRIANGULAR_PRISM_HPP