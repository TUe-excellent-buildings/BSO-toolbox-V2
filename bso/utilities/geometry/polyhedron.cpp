#ifndef POLYHEDRON_CPP
#define POLYHEDRON_CPP

#include <iomanip> // allows for std::setprecision use

namespace bso { namespace utilities { namespace geometry {
	
	template <class CONTAINER>
	void polyhedron::initFromVertices(const CONTAINER& l, const double& tol /*= 1e-3*/)
	{ //
		mSize = std::distance(std::begin(l), std::end(l));
		mSizeLines = 0;
		mSizePolygons = 0;
		mVertices.clear();
		mVertices.reserve(mSize);
		mVertices.insert(mVertices.end(),std::begin(l),std::end(l));
		
		// remove non unique vertices
		for (unsigned int i = 0; i < mSize ; ++i)
		{
			for (unsigned int j = i+1; j < mSize; ++j)
			{
				if ((mVertices[i] - mVertices[j]).isZero(tol))
				{
					mSize--;
					mVertices.erase(mVertices.begin() + j);
				}
			}
		}
		
		if (mSize < 4)
		{
			std::stringstream errorMessage;
			errorMessage << "Cannot initialize polyhedron with less than four unique vertices.\n"
									 << mSize << " vertices were provided.\n"
									 << "The provided vertices after removing duplicates were: " << std::endl;
									 for (unsigned int i = 0; i < mSize ; ++i)
									 {
										errorMessage << mVertices[i];
									 }
									 errorMessage << "(bso/utilities/geometry/polyhedron.cpp)" << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
		
		bool allVerticesOnOneplane = true;
		for (unsigned int i = 1; i < mSize-2; i++)
		{
			vector v1 = {mVertices[i]-mVertices[0]};
			vector v2 = {mVertices[i+1]-mVertices[0]};
			vector v3 = {mVertices[i+2]-mVertices[0]};
			if (v3.isZero(tol) || v2.isZero(tol) || v1.isZero(tol)) continue;
			vector v1x2 = v1.cross(v2);
			vector v1x3 = v1.cross(v3);
			if (!v1x2.isParallel(v1x3,tol))
			{
				allVerticesOnOneplane = false;
				break;
			}
		}
		
		if (allVerticesOnOneplane)
		{
			std::stringstream errorMessage;
			errorMessage << "Cannot initialize polyhedron with vertices that are all on one plane.\n"
									 << "The following vertices were argument: \n";
			for (const auto& i : mVertices) errorMessage << i.transpose() << std::endl;
			errorMessage << "(bso/utilities/geometry/polyhedron.cpp)" << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
		
		mCenter = {0,0,0};
		for (const auto& i : mVertices) mCenter += i;
		mCenter /= mSize;
	} // initFromVertices()

	polyhedron::polyhedron()
	{ //
		mSize = 0;
		mSizeLines = 0;
		mSizePolygons = 0;
	} // empty ctor

	template <typename CONTAINER>
	polyhedron::polyhedron(const CONTAINER& l, const double& tol /*= 1e-3*/)
	{ // this also works as a copy constructor via the .begin() and .end() functions
		this->initFromVertices(l, tol);
	} // ctor with arbitrary container of vertices

	polyhedron::polyhedron(const std::initializer_list<vertex>&& l, const double& tol /*= 1e-3*/)
	{ //
		this->initFromVertices(l, tol);
	} // ctor with initailizer list of vertices

	polyhedron::~polyhedron()
	{ //
		for (auto i : mPolygons) delete i;
	} // dtor

	double polyhedron::getSurfaceArea() const
	{ //
		double surfaceArea = 0;
		for (const auto& i: mPolygons)
		{
			surfaceArea += i->getArea();
		}
		return surfaceArea;
	} // getSurg

	vertex polyhedron::getCenter() const
	{ //
		return mCenter;
	} // getCenter()

	const vertex& polyhedron::center() const
	{ //
		return mCenter;
	} // center()
	
	bool polyhedron::isSameAs(const polyhedron& pol, const double& tol /*= 1e-3*/) const
	{
		if (mSize != pol.mSize) return false;
		for (const auto& i : mVertices)
		{
			bool vertexFound = false;
			for (const auto& j : pol)
			{
				if (j.isSameAs(i,tol))
				{
					vertexFound = true;
					break;
				}
			}
			if (!vertexFound) return false;
		}
		return true;
	} // isSameAs

	// This function only hold for convex polyhedra, which is the only type of polyhedra we are working with in this toolbox, so it is not virtual and does not need to be implemented for each derived class.
	bool polyhedron::testIntersection(const polyhedron& other, const double& tol /*= 1e-3*/) const
	{ //  function based on: chrome-extension://efaidnbmnnnibpcajpcglclefindmkaj/https://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
		const polyhedron& C0 = *this;
		const polyhedron& C1 = other;

		auto outwardNormal = [](const polyhedron& poly, const polygon* face) -> vector
		{
			vector n = face->normal();
			if (n.dot(face->center() - poly.center()) < 0.0)
			{
				n = -n;
			}
			return n;
		};

		// The vertices are projected to the form P + t * D. The return value is +1 if all t > 0, 
		// = 1 if all t < 0, but 0 otherwise , in which case the line splits the polygon projection .
		auto whichSide = [tol](const polyhedron& poly, const vertex& P, const vector& D) -> int
		{
			int positive = 0;
			int negative = 0;
			for (const auto& v : poly.mVertices)
			{
				// project a vertex onto the line defined by P and D, 
				// and check on which side of P it is, with a tolerance for numerical errors.
				const double t = D.dot(v - P);
				if (t > tol)
				{
					++positive;
				}
				else if (t < -tol)
				{
					++negative;
				}
				if (positive && negative)
				{
					// Time saved by not projecting the rest of the vertices.
					return 0;
				}
			}
			return (positive > 0 ? +1 : -1);
		};

		// Test faces of C0 for separation.
		for (unsigned int i = 0; i < C0.mPolygons.size(); ++i)
		{
			const polygon* face = C0.mPolygons[i];
			const vertex& P = (*face)[0];
			const vector N = outwardNormal(C0, face);
			if (whichSide(C1, P, N) > 0)
			{
				return false;
			}
		}

		// Test faces of C1 for separation.
		for (unsigned int i = 0; i < C1.mPolygons.size(); ++i)
		{
			const polygon* face = C1.mPolygons[i];
			const vertex& P = (*face)[0];
			const vector N = outwardNormal(C1, face);
			if (whichSide(C0, P, N) > 0)
			{
				return false;
			}
		}

		// Test cross products of pairs of edge directions, one edge direction from each polyhedron.
		for (unsigned int i0 = 0; i0 < C0.mLineSegments.size(); ++i0)
		{
			const line_segment& edge0 = C0.mLineSegments[i0];
			const vertex& P = edge0[0];
			const vector D0 = edge0[1] - edge0[0];
			for (unsigned int i1 = 0; i1 < C1.mLineSegments.size(); ++i1)
			{
				const line_segment& edge1 = C1.mLineSegments[i1];
				const vector D1 = edge1[1] - edge1[0];
				const vector N = D0.cross(D1);
				if (!N.isZero(tol))
				{
					const int side0 = whichSide(C0, P, N);
					if (side0 == 0)
					{
						continue;
					}
					const int side1 = whichSide(C1, P, N);
					if (side1 == 0)
					{
						continue;
					}
					if (side0 * side1 < 0)
					{
						return false;
					}
				}
			}
		}
		return true;
	}
	
	const vertex& polyhedron::operator [] (const unsigned int& index) const
	{
		return mVertices[index];
	}

	std::ostream& operator << (std::ostream& stream, const polyhedron& pol)
	{ //
		Eigen::IOFormat layout(Eigen::StreamPrecision, Eigen::DontAlignCols, ",", ",", "{", "}", "", "");
		bool first = true;
		stream /*<< std::fixed << std::setprecision(16) */<< "{";
		for (const auto& i : pol.mVertices)
		{
			if (first)
			{
				first = false;
				stream << i.transpose().format(layout);
			}
			else stream << "," << i.transpose().format(layout);
		}
		stream << "}";
		return stream;
	} // ofstream operator

} // namespace geometry
} // namespace utilities
} // namespace bso

#endif // POLYHEDRON_CPP