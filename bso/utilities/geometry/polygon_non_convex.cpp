#ifndef POLYGON_NON_CONVEX_CPP
#define POLYGON_NON_CONVEX_CPP

#include <sstream>
#include <stdexcept>
#include <bso/utilities/geometry/polygon.hpp>
#include <bso/utilities/delaunay.hpp>
#include <bso/utilities/geometry_functions.hpp>

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
		if(!(lineSegments[lineSegments.size()-1])[1].isSameAs((lineSegments[0])[0], tol)){correct =false;}
		else
		{
			for(long unsigned int i=0; i<lineSegments.size()-1; i++)	
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
											for(const auto& i: mVertices) errorMessage << i << "; ";
											errorMessage << "Resulting in the following lines: \n";
											for(const auto& i: lineSegments) errorMessage << i << "; ";
											errorMessage << "\n(bso/utilities/geometry/polygon_non_convex.cpp)" << std::endl;
					throw std::invalid_argument(errorMessage.str());
		}

		// check if the line_segments intersect each other, since this would definitely be an invalid polygon
		for(const auto& i: this->getLines())
		{
			for(const auto& j: this->getLines())
			{
				if(i.intersectsWith(j,tol))
				{
					std::stringstream errorMessage;
					errorMessage << "The inserted polygon (which may be convex) has two lines which intersect, are the inserted points already sorted? points in the class are invalid for initializing polygon_non_convex from container.\n"
											<< "Received the following vertices: \n";
											for(const auto& i: mVertices) errorMessage << i << "; ";
											errorMessage << "\n(bso/utilities/geometry/polygon_non_convex.cpp)" << std::endl;
					throw std::invalid_argument(errorMessage.str());
				}
			}
		}

		// redefine the centre of the polygon_non_convex
	} //  sortPoints()

	void polygon_non_convex::triangulate(const double& tol /*= 1e-3*/)
	{
		vector mNormal = this->normal();
		if(mNormal.isHorizontal())
		{
			std::stringstream errorMessage;
			errorMessage << "The polygon_non_convex is vertical, since the delaunay triangulation \n"
									 << "is performed in the z=0 plane, it would be unable to get any results.\n"
									 << "\n(bso/utilities/geometry/polygon_non_convex.cpp)" << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
		try 
		{
			std::vector<triangle> z0Triangles = bso::utilities::delaunayZoned(mVertices, mLineSegments);
			// Translate the triangles back to polygon_non_convex plane from z=0 plane
			vertex unitVertexZ = {0,0,1};
			for(auto& tri: z0Triangles)
			{
				std::vector<vertex> triVertexes;
				for(auto& triVer: tri)
				{
					// Calculate the line (cornerpoint Tri + {0,0,1}) intersects point with the plane
					vector v1 = triVer + unitVertexZ-triVer;
					vector v2 = mVertices[0]-triVer;
					double r = (mNormal.dot(v2))/(mNormal.dot(v1));
					triVertexes.push_back(triVer + r * v1);
				}
				tri = triangle(triVertexes); // replace z0Triangles with translated triangles
			}
			mTriangles = z0Triangles;
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

	// polygon_non_convex::polygon_non_convex(const std::initializer_list<line_segment>&& l, const double& tol /*= 1e-3*/) : polygon(std::move(l),tol)
	// { // 		
	// 	try 
	// 	{
	// 		this->sortPoints(tol);
	// 	}
	// 	catch (std::exception& e)
	// 	{
	// 		std::stringstream errorMessage;
	// 		errorMessage << "The inserted points in the class are invalid for initializing polygon_non_convex from container.\n"
	// 								 << "Received the following error: \n"
	// 								 << e.what()
	// 								 << "\n(bso/utilities/geometry/polygon_non_convex.cpp)" << std::endl;
	// 		throw std::invalid_argument(errorMessage.str());
	// 	}

	// 	this->triangulate(tol); // used for a lot of geometry analysis functions
	// } // ctor with initializer list
	
	polygon* polygon_non_convex::clone()
	{
		return new polygon_non_convex(*this);
	} // virtual copy constructor

	double polygon_non_convex::getArea() const
	{ 
		double area = 0;
		for(const auto& i: mTriangles)
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
		for(const auto& i: mTriangles)
		{
			if(i.isInsideOrOn(p1,tol))
			{
				return true;
			}
		}
		return false;
	} //  isInside()

	// This function checks if the polygon_non_convex is overlapping with another polygon_non_convex at any location, 
	// however when the polygons_non_convex are only touching each others line_segments or edges, then they are considered non-overlapping.
	// bool polygon_non_convex::isOverlapping(const polygon_non_convex& pol, const double& tol /*= 1e-3*/) const
	// {
	// 	// Note: this function is not working as it should yet!!

	// 	// To determine if two polygons intersect :

	// 	// If two polygons have a non-void intersection then one of the following will happen:

	// 	// A) One of them has a corner inside the interior of the other one.
	// 	// B) One of them has a whole edge inside the interior of the other one (the corners of that edge may not necessarily be in the interior). This means the middle of that edge will be inside the interior.
	// 	// C) The polygons are identical.

	// 	// First check if the polygon_non_convex are coplanar
	// 	for(const auto& i: mVertices)
	// 	{
	// 		if(!pol.isCoplanar(i,tol))
	// 		{
	// 			return false;
	// 		}
	// 	}

	// 	// A) Check the vertices of the polygons are inside at any location
	// 	for(const auto& i: pol.mVertices)
	// 	{
	// 		if(this->isInside(i,tol))
	// 		{
	// 			return true;
	// 		}
	// 	}
	// 	for(const auto& i: this->mVertices)
	// 	{
	// 		if(pol.isInside(i,tol))
	// 		{
	// 			return true;
	// 		}
	// 	}

	// 	// B) Check if any of the line_segments intersect, split lines at intersection and see if the middle of the line_segment is inside a polygon
	// 	std::vector<vertex> intsecV;
	// 	std::vector<line_segment> allLines;
	// 	vertex pInt;
	// 	for(const auto& i: mLineSegments)
	// 	{
	// 		allLines.push_back(i);
	// 		for(const auto& j: pol.mLineSegments)
	// 		{
	// 			if(i.intersectsWith(j,pInt,tol)){intsecV.push_back(pInt);}
	// 		}
	// 	}
	// 	for(const auto& j: pol.mLineSegments) {allLines.push_back(j);}
	// 	for(const auto& i: pol.mVertices){intsecV.push_back(i);}
	// 	for(const auto& i: mVertices){intsecV.push_back(i);}
	// 	bso::utilities::splitLines(intsecV,allLines,tol);
	// 	for(const auto& i: allLines)
	// 	{
	// 		if(pol.isInside(i.getCenter(),tol)){return true;}
	// 		if(this->isInside(i.getCenter(),tol)){return true;}
	// 	}

	// 	// C) Check if the polygons are identical
	// 	if(this->isSameAs(pol,tol))
	// 	{
	// 		return true;
	// 	}

	// 	return false;
	// } //isOverlapping()

	bool polygon_non_convex::isSameAs(const polygon_non_convex& pol, const double& tol /*= 1e-3*/) const
	{
		// Check if the polygons are the same
		if(mVertices.size() != pol.mVertices.size())
		{
			return false;
		}
		for(const auto& i: mVertices)
		{
			bool foundTheSame = false;
			for(const auto& j: pol.mVertices)
			{
				if(i.isSameAs(j,tol))
				{
					foundTheSame = true;
					
				}
			}
			if(!foundTheSame){return false;}
		}
		return true;
	}

	// void polygon_non_convex::convertToQuads(std::vector<quadrilateral>& quads)
	// {
	// 	bso::utilities::convertToQuads(mVertices, mLineSegments, quads, 0.001, true);
	// }

} // namespace geometry
} // namespace utilities
} // namespace bso

#endif // POLYGON_NON_CONVEX_CPP