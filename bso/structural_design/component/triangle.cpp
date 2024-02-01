#ifndef SD_TRIANGLE_CPP
#define SD_TRIANGLE_CPP

#include <bso/structural_design/component/derived_ptr_to_vertex.hpp>

#include <sstream>
#include <stdexcept>

namespace bso { namespace structural_design { namespace component {

	template <typename TRIANGLE_INITIALIZER>
	triangle::triangle(const TRIANGLE_INITIALIZER& l)
	:	bso::utilities::geometry::triangle(l),
		geometry()
	{
		mIsTriangle = true;
	} // ctor 
	
	triangle::triangle(std::initializer_list<bso::utilities::geometry::vertex>&& l)
	:	bso::utilities::geometry::triangle(std::move(l)),
		geometry()
	{
		mIsTriangle = true;
	} // ctor from initializer list

	triangle::~triangle()
	{
		
	} // dtor

	void triangle::addStructure(const structure& s)
	{
		if (s.type() == "flat_shell")
		{
			geometry::addStructure(s);
		}
		else 
		{
			std::stringstream errorMessage;
			errorMessage << "\nError when assigning structure to triangle geometry.\n"
									 << "Cannot assign structure with type: \"" << s.type() << "\"" 
									 << "to a triangle geometry\n"
									 << "(bso/structural_design/component/triangle.cpp)" << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
	} // addStructure()
	
	void triangle::mesh(const unsigned int& n1, std::vector<point*>& pointStore)
	{
		if(n1 % 2) // Check if n1 is even, if not then no mesh for a triangle can be made.
		{
			std::stringstream errorMessage;
			errorMessage << "\nError when creating a mesh structure for a triangle geometry.\n"
									 << "Cannot create a mesh with an oneven mesh size of: " << n1 << "\n" 
									 << "(bso/structural_design/component/triangle.cpp)" << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
		
		// Resize mMeshedPoints to the future final amount of meshPoints
		mMeshedPoints.clear();
		int resizeCount = 0;
		for(int i=0; i < n1/2; i++)	resizeCount = resizeCount + (n1-2*i)*3;
		resizeCount++;
		mMeshedPoints.resize(resizeCount);

		namespace geom = bso::utilities::geometry;

		// Scale the triangle shape to define the triangular devisions of the mesh
		geom::vertex center = this->getCenter();
		std::vector<geom::triangle*> scaledTri;
		for(double i=0; i < 0.999; i=i+1/((double)n1/2))// smaller then 0.999 to take into acount a tolerance
		{// define triangles from the outside in starting with the initial triangular geometry
			std::vector<geom::vertex> triVertex;
			for(const auto j: mVertices)
			{
				geom::vertex vPtr = j+(center-j)*i;
				triVertex.push_back(vPtr);
			}
			geom::triangle* triPtr = new geom::triangle(triVertex);
			scaledTri.push_back(triPtr);
		}
		
		// Devide the lines of the triangles in the vertices to be used as mesh points and store in the geometrys own meshPoint and the global inserted PointStore.
		int rowCount = 0;
		unsigned long highestID = 0;
		int count = 0;
		geom::vertex meshPoint;
		bool pointFound;
		for(const auto i: scaledTri)
		{
			// make vectors of the lines of the triangles
			std::vector<geom::vector> lineTriVec;
			lineTriVec.push_back((i[0])[1]-(i[0])[0]);
			lineTriVec.push_back((i[0])[2]-(i[0])[1]);
			lineTriVec.push_back((i[0])[0]-(i[0])[2]);
			int subDevide = n1-2*rowCount;
			
			// define the vertices on each of the lines of the triangles with the privious defined vectors
			for (int k=0; k < 3; k++)
			{// iterate trough all lineTriVec
				for(int j=0; j<subDevide; j++) 
				{
					meshPoint = (i[0])[k]+(lineTriVec[k]/(double)subDevide)*(double)j;

					pointFound = false;
					for (const auto& k : pointStore)
					{
						if (k->isSameAs(meshPoint))
						{
							mMeshedPoints[count++] = k;
							pointFound = true;
							break;
						}
						if (highestID < k->getID()) highestID = k->getID();
					}
					if (!pointFound)
					{
						pointStore.push_back(new point(highestID++,meshPoint));
						mMeshedPoints[count++] = pointStore.back();
					}
				}
			}
			rowCount++;
		}

		// Add the last mesh point, namely the centre point of all triangles.
		pointFound = false;
		for (const auto& k : pointStore)
		{
			if (k->isSameAs(center))
			{
				mMeshedPoints[count++] = k;
				pointFound = true;
				break;
			}
			if (highestID < k->getID()) highestID = k->getID();
		}
		if (!pointFound)
		{
			pointStore.push_back(new point(highestID++,center));
			mMeshedPoints[count++] = pointStore.back();
		}

		// Resize mElementPoints to the future final amount of elements
		mElementPoints.clear();
		resizeCount = 0;
		for(int i=0; i < n1/2; i++)	resizeCount = resizeCount + (n1-1-2*i)*3;
		mElementPoints.resize(resizeCount);

		// Pair the points that define an element together
		int cornerCount = 1;
		int rowMeshSize = n1; // n1 MeshSize on the considered row in the following for loop
		int startTriRow = 0; // index of first mMeshPoint on the considered triangular row
		int i = 0; // index of first mMeshPoint to define an element
		for(int elementIndex=0; elementIndex < resizeCount; elementIndex++) // each element to define
		{
			if(i == startTriRow + rowMeshSize*3-2) // end of row and transition to next row 
			{
				mElementPoints[elementIndex] = {
					mMeshedPoints[i], 		mMeshedPoints[i+1],
					mMeshedPoints[rowMeshSize*3+i-(cornerCount-1)*2-1], mMeshedPoints[i+2]};
				
				startTriRow = i+2;
				rowMeshSize = rowMeshSize-2; // adjust rowMeshSize to describe the amount of divitions on the next outer triangle to define element on. 
				cornerCount = 1; //reset per row
				i++; // skip i+1 since point nr. i+1 is inside the just defined cornerellement
				i++;
			}
			else if(i == startTriRow + rowMeshSize*cornerCount-1)// define the second or third triangle corner element
			{
				mElementPoints[elementIndex] = {
					mMeshedPoints[i], 		mMeshedPoints[i+1],
					mMeshedPoints[rowMeshSize*3+i-(cornerCount-1)*2-1], mMeshedPoints[i+2]};
				cornerCount++;
				i++; // skip i+1 since point nr. i+1 is inside the just defined cornerellement
				i++;
			}
			else
			{
				// define standart element between triangular corner elements or the first triangular corner element
				mElementPoints[elementIndex] = {
					mMeshedPoints[i], 		mMeshedPoints[(i+1)],
					mMeshedPoints[rowMeshSize*3+i-(cornerCount-1)*2], mMeshedPoints[rowMeshSize*3+i-(cornerCount-1)*2-1]};
				i++;
			}
		}
		
		// assign the constraints to all the points
		for (auto& i : mConstraints)
		{
			for (auto& j : mMeshedPoints)
			{
				j->addConstraint(i);
			}
		}
		
		// assign the loads to all the points
		for (auto& i : mElementPoints)
		{ // for each element
			geom::quadrilateral elementGeometry = derived_ptr_to_vertex(i);
			geom::vertex elementCenter = elementGeometry.getCenter();

			for (unsigned int j = 0; j < 4; ++j)
			{
				geom::line_segment lTemp1 = {elementGeometry[j], elementGeometry[(j+1)%4]};
				geom::line_segment lTemp2 = {elementGeometry[j], elementGeometry[(j-1)%4]};
				geom::quadrilateral partitionGeometry = {
					elementGeometry[j], elementCenter, lTemp1.getCenter(), lTemp2.getCenter()};
				double partitionArea = partitionGeometry.getArea();
				
				// find the point that corresponds to vertex j
				for (auto& k : i)
				{ // for each point k of the element
					if (k->isSameAs(elementGeometry[j]))
					{ // if it is the same as vertex i of the element geometry
						for (auto & l : mLoads)
						{ // the for each load l, add the nodal element load to point k
							k->addLoad(l * partitionArea);
						}
					}
				}
			}
		}
	}

	
} // namespace component
} // namespace structural_design
} // namespace bso

#endif // SD_TRIANGLE_CPP