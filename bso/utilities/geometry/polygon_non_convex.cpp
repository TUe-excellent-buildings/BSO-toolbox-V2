#ifndef POLYGON_NON_CONVEX_CPP
#define POLYGON_NON_CONVEX_CPP

#include <sstream>
#include <stdexcept>
#include <bso/utilities/geometry/polygon.hpp>
#include <bso/utilities/delaunay.hpp>

namespace bso { namespace utilities {  namespace geometry {
	
	void polygon_non_convex::sortPoints(const double& tol /*= 1e-3*/)
	{ // 
		// This sortPoint functions passes the already good inserted order of points
		// to the polygon abstract class where the line_segments are generated following this order.
		// the good order of points is as follows: 
		// take for example a non-convex quadrilateral with vertices v1,v2,v3, and v4.
		// the polygon::sortPoints algorithm is going to connect the points in the following lines: {v1,v2};{v2,v3};{v3,v4},{v4,v1}

		polygon::sortPoints(tol);

		std::vector<line_segment> lineSegments = this->getLines();
		
		// check if the line_segments start and end with the next vertex in the container, since this would definitely be an invalid polygon
		bool correct = true;
		if(!(lineSegments[lineSegments.size()])[1].isSameAs((lineSegments[0])[0], tol)){correct =false;}
		else
		{
			for(int i=0; i<lineSegments.size()-1; i++)	
			{
				if(!(lineSegments[i])[1].isSameAs((lineSegments[i+1])[0], tol)){correct = false;}
			}
		}
		if(!correct)
		{
			std::stringstream errorMessage;
					errorMessage << "The inserted polygon (which may be convex) has lines in its sequence\n"
											<< "which does not start and end with the next line as it should.\n"
											<< "Received the following vertices: \n";
											for(const auto i: mVertices) errorMessage << i << "; ";
											errorMessage << "Resulting in the following lines: \n";
											for(const auto i: lineSegments) errorMessage << i << "; ";
											errorMessage << "\n(bso/utilities/geometry/polygon_non_convex.cpp)" << std::endl;
					throw std::invalid_argument(errorMessage.str());
		}

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

		// redefine the centre of the polygon_non_convex
	} //  sortPoints()

	void polygon_non_convex::triangulate(const double& tol /*= 1e-3*/)
	{
		try 
		{
			mTriangles = bso::utilities::delaunayZoned(mVertices, mLineSegments);
		}
		catch (std::exception& e)
		{
			std::stringstream errorMessage;
			errorMessage << "The triangulation of the polygon_non_convex did not work.\n"
									 << "Received the following error: \n"
									 << e.what()
									 << "\n(bso/utilities/geometry/polygon_non_convex.cpp)" << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
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

	void polygon_non_convex::convertToQuads(std::vector<quadrilateral>& quads)
	{
		bso::utilities::convertToQuads(mVertices, mLineSegments, quads);
	}

} // namespace geometry
} // namespace utilities
} // namespace bso

#endif // POLYGON_NON_CONVEX_CPP