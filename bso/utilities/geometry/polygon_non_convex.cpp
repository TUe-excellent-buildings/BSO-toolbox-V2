#ifndef POLYGON_NON_CONVEX_CPP
#define POLYGON_NON_CONVEX_CPP

#include <sstream>
#include <stdexcept>
#include <bso/utilities/delaunay.hpp>

namespace bso { namespace utilities {  namespace geometry {
	
	void polygon_non_convex::sortPoints(const double& tol /*= 1e-3*/)
	{ // 
		// This sortPoint functions passes the already good inserted order of points
		// to the polygon abstract class where the line_segments are generated following this order.
		// the good order of points is as follows: 
		// take for example a non-convex quadrilateral with vertices v1,v2,v3, and v4.
		// the sortPoints algorithm is going to 

		polygon::sortPoints(tol);

		// check if the line_segments intersect each other, since this would definitely be an invalid polygon
		for(const auto i: this->getLines())
		{
			for(const auto j: this->getLines())
			{
				if(i.intersectsWith(j,tol))
				{
					std::stringstream errorMessage;
					errorMessage << "The inserted polygon (which may be convex) has two lines which intersect, are the inserted points already sorted? points in the class are invalid for initializing polygon_non_convex from container.\n"
											<< "Received the following vertices: \n";
											for(const auto i: mVertices) errorMessage << i << "; ";
											errorMessage << "\n(bso/utilities/geometry/polygon_non_convex.cpp)" << std::endl;
					throw std::invalid_argument(errorMessage.str());
				}
			}
		}
	} //  sortPoints()

	void polygon_non_convex::triangulate(const double& tol /*= 1e-3*/)
	{
		std::vector<utilities::geometry::triangle> dTriangles = bso::utilities::delaunay(mVertices, mLineSegments);

		// remove triangle not inside non-convex triangle

	}

	polygon_non_convex::polygon_non_convex()
	{ // 
		
	} //  empty ctor

	template <class CONTAINER>
	polygon_non_convex::polygon_non_convex(const CONTAINER& l, const double& tol /*= 1e-3*/) : polygon(l,tol)
	{ // 
		try 
		{
			this->sortPoints(tol);
		}
		catch (std::exception& e)
		{
			std::stringstream errorMessage;
			errorMessage << "The inserted points in the class are invalid for initializing polygon_non_convex from container.\n"
									 << "Received the following error: \n"
									 << e.what()
									 << "\n(bso/utilities/geometry/polygon_non_convex.cpp)" << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}

		this->triangulate(tol); // used for a lot of geometry analysis functions
	} //  ctor with a container
	
	polygon_non_convex::polygon_non_convex(const std::initializer_list<vertex>&& l, const double& tol /*= 1e-3*/) : polygon(std::move(l),tol)
	{ // 
		try 
		{
			this->sortPoints(tol);
		}
		catch (std::exception& e)
		{
			std::stringstream errorMessage;
			errorMessage << "The inserted points in the class are invalid for initializing polygon_non_convex from container.\n"
									 << "Received the following error: \n"
									 << e.what()
									 << "\n(bso/utilities/geometry/polygon_non_convex.cpp)" << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}

		this->triangulate(tol); // used for a lot of geometry analysis functions
	} // ctor with initializer list
	
	polygon* polygon_non_convex::clone()
	{
		return new polygon_non_convex(*this);
	} // virtual copy constructor

	double polygon_non_convex::getArea() const
	{ 
		double area = 0;
		for(const auto i: mTriangles)
		{
			area = area + i.getArea();
		}
		return area;
	} //  getArea()

	bool polygon_non_convex::isInside(const vertex& p1, const double& tol /*= 1e-3*/) const
	{ 
		if (this->isInsideOrOn(p1,tol))
		{
			for (const auto& i : mLineSegments)
			{
				if (i.isOnLine(p1,tol)) return false;
			}
			for (const auto& i : mVertices)
			{
				if (i.isSameAs(p1,tol)) return false;
			}
			return true;
		}
		else return false;
	} //  isInside()

	bool polygon_non_convex::isInsideOrOn(const vertex& p1, const double& tol /*= 1e-3*/) const
	{ 
		for(const auto i: mTriangles)
		{
			if(i.isInsideOrOn(p1,tol))
			{
				return true;
			}
		}
		return false;
	} //  isInside()

	void polygon_non_convex::convertToQuads()
	{

	}

} // namespace geometry
} // namespace utilities
} // namespace bso

#endif // POLYGON_NON_CONVEX_CPP