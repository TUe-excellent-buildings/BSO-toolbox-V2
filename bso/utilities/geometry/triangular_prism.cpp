#ifndef TRIANGULAR_PRISM_CPP
#define TRIANGULAR_PRISM_CPP

namespace bso { namespace utilities { namespace geometry {

	void triangular_prism::sortPoints(const double& tol /*= 1e-3*/)
	{ // 
		if (mSize != 6)
		{	
			std::stringstream errorMessage;
			errorMessage << "Cannot sort triangular prism:\n"
										<< "received " << mSize << " vertices, expected 6: " << std::endl;
										for(const auto i: mVertices){errorMessage << i;}
										errorMessage << "(bso/utilities/geoemtry/triangular_prism.cpp)" << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}	
		
		triangle current, opposing;
		bool opposingFound = false;
		bool currentFound = false;
		
		for (unsigned int i = 1; i < 5; ++i) {
			for (unsigned int j = i+1; j < 6; ++j) {
				try
				{
					current = triangle({mVertices[0],mVertices[i],mVertices[j]},tol);
					currentFound = true;
				}
				catch (std::exception& e) { /*do nothing*/ } 
				
				std::vector<vertex> tempOpposing;
				tempOpposing.reserve(3);
				for (unsigned int l = 1; l < 6; ++l)
				{
					if (l != 0 && l != i && l != j)
					{
						tempOpposing.push_back(mVertices[l]);
					}
				}
				try
				{
					opposing = triangle(tempOpposing,tol);
					opposingFound = true;
				}
				catch(std::exception& e) { /*do nothing*/ } 
				
				bool intersection = false;
				if (opposingFound)
				{ 					
					// Check if anny vertex of current is in the plane of opposite
					for (const auto& l : current)
					{
						if(opposing.isCoplanar(l, tol))
						{
							intersection = true;
							break;
						}
					}
				}
				
				if (intersection) opposingFound = false;
				if (!opposingFound) currentFound = false;
				if (currentFound && opposingFound) break;
			}
			if (currentFound && opposingFound) break;
		}
		if (!currentFound && ! opposingFound)
		{
			std::stringstream errorMessage;
			errorMessage << "When initializing a triangular prism.\n"
									 << "Could not find two opposing triangles.\n"
									 << "The following vertices were argument:\n";
			for (const auto& i : mVertices) errorMessage << i.transpose() << std::endl;
			errorMessage << "Do the vertices define five planes?\n"
									 << "Do the vertices define three convex quadrilaterals?\n"
									 << "(bso/utilities/geoemtry/triangular_prism.cpp)" << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}

		
		unsigned int indexFive;
		bool indexFound = false;
		bool orderedCounterDirection = false;
		// find the indices of all the quadrilateral with which we can make a quadrilateral together with indices One and Two of the current triangle
		for (unsigned int i = 0; i < 3; ++i)
		{
			try
			{
				quadrilateral quad1({current[0],current[1],opposing[i],opposing[(i+1)%3]},tol);
				quadrilateral quad2({current[1],current[2],opposing[(i+1)%3],opposing[(i+2)%3]},tol);
				quadrilateral quad3({current[2],current[0],opposing[(i+2)%3],opposing[(i)]},tol);

				bool intersection = false;
				for (const auto& j : quad1.getLines())
				{
					for (const auto& k : quad2.getLines())
					{
						if (j.intersectsWith(k,tol))
						{
							intersection = true;
							break;
						}
					}
					if (intersection) break;
				}
				if (intersection) continue;

				line_segment l1 = {current[0], opposing[i]};
				line_segment l2 = {current[1], opposing[(i+1)%3]};
				if (!l1.intersectsWith(l2,tol)) 
				{
					indexFive = i;
				}
				else
				{
					indexFive = (i+1)%3;
					orderedCounterDirection = true;
				}
				indexFound = true;
				break;
			}
			catch(std::exception& e) { /*do nothing*/ }
		}
		if (!indexFound)
		{
			std::stringstream errorMessage;
			errorMessage << "When initializing a triangular prism.\n"
									 << "Could not correspond the vertices of two triangles faces in the triangular prism. \n"
									 << "The two faces at hand are \n" << current << "\nand\n" << opposing << std::endl
									 << "Do the vertices define five planes, from which the two triangles and three quad_hexahedrons?\n"
									 << "Do the vertices define three convex quadrilaterals?\n"
									 << "indexFive: " << indexFive << " - " << orderedCounterDirection << std::endl
									 << "(bso/utilities/geoemtry/triangular_prism.cpp)" << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
		
		
		// now all points can be sorted into mVertices
		mVertices.clear();
		mVertices.reserve(6);
		mSize = 6;
		for (const auto& i : current)
		{
			mVertices.push_back(i);
		}
		for (unsigned int i = 0; i < 3; i++)
		{
			if (!orderedCounterDirection) 
			{
				mVertices.push_back(opposing[(i+indexFive)%3]);
			}
			else 
			{
				mVertices.push_back(opposing[(indexFive-i)%3]);
			}
		}
		
		
		// now initialize all the edges of the quadrilateral faced hexahedron
		try
		{
			mLineSegments.clear();
			mLineSegments.reserve(9);
			mSizeLines = 9;
			for (unsigned int i = 0; i < 3; ++i)
				mLineSegments.push_back(line_segment({mVertices[i], mVertices[(i+1)%3]}));
			for (unsigned int i = 0; i < 3; ++i)
				mLineSegments.push_back(line_segment({mVertices[i], mVertices[i+3]}));
			for (unsigned int i = 0; i < 3; ++i)
				mLineSegments.push_back(line_segment({mVertices[i+3], mVertices[((i+1)%3)+3]}));
		}
		catch (std::exception& e)
		{
			std::stringstream errorMessage;
			errorMessage << "When initializing a triangular prism.\n"
									 << "Could not initialize the line segments that define the edges\n"
									 << "(bso/utilities/geoemtry/triangular_prism.cpp)" << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
		
		// now initialize all the quadrilaterals and triangles of the triangular prism
		try 
		{
			mPolygons.clear();
			mPolygons.reserve(5);
			mSizePolygons = 5;
			triangle* temp1 = new triangle({mVertices[0],mVertices[1],mVertices[2]},tol);
			mPolygons.push_back(temp1);
			mTriangles.push_back(*temp1);
			if((mPolygons.back())->getMSize() != 3)
			{
				std::stringstream errorMessage;
				errorMessage << "When initializing a triangular prism.\n"
										 << "Initialized the current triangular polygon with to mutch vertices, namely: " << (mPolygons.back())->getMSize() << ".\n"
										 << "(bso/utilities/geoemtry/triangular_prism.cpp)" << std::endl;
				throw std::invalid_argument(errorMessage.str());
			}
			for (unsigned int i = 0; i < 3; ++i)
			{
				quadrilateral* temp2 = new quadrilateral({mVertices[i],mVertices[(i+1)%3],mVertices[i+3],mVertices[((i+1)%3)+3]},tol);
				mPolygons.push_back(temp2);
				mQuad.push_back(*temp2);
				if((mPolygons.back())->getMSize() != 4)
				{
					std::stringstream errorMessage;
					errorMessage << "When initializing a triangular prism.\n"
											 << "Initialized the quad_hexahedron polygon with to mutch vertices, namely: " << (mPolygons.back())->getMSize() << "\n"
											 << "(bso/utilities/geoemtry/triangular_prism.cpp)" << std::endl;
					throw std::invalid_argument(errorMessage.str());
				}
			}
			triangle* temp3 = new triangle({mVertices[3],mVertices[4],mVertices[5]},tol);
			mPolygons.push_back(temp3);
			mTriangles.push_back(*temp3);
			if((mPolygons.back())->getMSize() != 3)
			{
				std::stringstream errorMessage;
				errorMessage << "When initializing a triangular prism.\n"
										 << "Initialized the opposite triangular polygon with to mutch vertices, namely: " << (mPolygons.back())->getMSize() << "\n"
										 << "(bso/utilities/geoemtry/triangular_prism.cpp)" << std::endl;
				throw std::invalid_argument(errorMessage.str());
			}
		}
		catch (std::exception& e)
		{
			std::stringstream errorMessage;
			errorMessage << "When initializing a triangular prism.\n"
									 << "Could not initialize the quadrilaterals and triangles that define the faces\n"
									 << "Received the following error:\n" << e.what() << "\n"
									 << "(bso/utilities/geoemtry/triangular_prism.cpp)" << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
		
		// finally decompose the triangular prism in tetrahedrons, for convenience
		mTetrahedrons.clear();
		mTetrahedrons.reserve(3);
		try 
		{
			mTetrahedrons.push_back(tetrahedron({mVertices[0],mVertices[1],mVertices[2],mVertices[3]},tol));
			mTetrahedrons.push_back(tetrahedron({mVertices[3],mVertices[4],mVertices[5],mVertices[1]},tol));
			mTetrahedrons.push_back(tetrahedron({mVertices[1],mVertices[2],mVertices[3],mVertices[5]},tol));
		}
		catch (std::exception& e)
		{
			std::stringstream errorMessage;
			errorMessage << "When initializing a triangular prism.\n"
									 << "Could not decompose it into tetrahedrons\n"
									 << "Received the following error:\n" << e.what() << "\n"
									 << "(bso/utilities/geoemtry/triangular_prism.cpp)" << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
		// done!
	} // sortPoints()
	
	
	triangular_prism::triangular_prism() : polyhedron()
	{ // 
	
	} // empty ctor

	template <typename CONTAINER>
	triangular_prism::triangular_prism(const CONTAINER& l, const double& tol /*= 1e-3*/) : polyhedron(l, tol)
	{ // 
		try 
		{
			this->sortPoints(tol);
		}
		catch (std::exception& e)
		{
			std::stringstream errorMessage;
			errorMessage << "Could not sort points when initializing triangular prism from container.\n"
									 << "Received the following error: \n"
									 << e.what()
									 << "\n(bso/utilities/geometry/triangular_prism.cpp)" << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
	} //

	triangular_prism::triangular_prism(const std::initializer_list<vertex>&& l, const double& tol /*= 1e-3*/) : polyhedron(std::move(l),tol)
	{ // 
		try 
		{
			this->sortPoints(tol);
		}
		catch (std::exception& e)
		{
			std::stringstream errorMessage;
			errorMessage << "Could not sort points when initializing triangular prism from initializer list.\n"
									 << "Received the following error: \n"
									 << e.what()
									 << "\n(bso/utilities/geometry/triangular_prism.cpp)" << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
	} //

	triangular_prism::triangular_prism(const triangular_prism& rhs, const double& tol /*= 1e-3*/) : polyhedron(rhs,tol)
	{ // copy ctor, need this for mPolygons, to be copied properly
		this->sortPoints(); // without try-catch construction, since it is initailized from a valid triangular prism
	} //
	
	polyhedron* triangular_prism::clone()
	{
		return new triangular_prism(*this);
	} // virtual copy constructor
	
	double triangular_prism::getVolume() const
	{ // 
		double volume = 0;	
		for (const auto& i : mTetrahedrons) volume += i.getVolume();
		return volume;
	} //

	bool triangular_prism::isInside(const vertex& p1, const double& tol /*= 1e-3*/) const
	{ // 
		// check if it is inside a tetrahedron
		for (const auto& i : mTetrahedrons) if (i.isInside(p1, tol)) return true;
		
		// check if it is on the surface on a tetrahedron
		bool onTetSurface = false;
		for (const auto& i : mTetrahedrons) if (i.isInsideOrOn(p1, tol)) 
		{
			onTetSurface = true;
			break;
		}
		
		if (onTetSurface)
		{
			// check if it is on the surface of one of the quadrilaterals
			bool onSurface = false;
			for (const auto& i : mPolygons)
			{
				if (i->isInsideOrOn(p1,tol))
				{
					onSurface = true;
					break;
				}
			}
			
			if (onSurface) return false;
			else return true;
		}
		else return false;
	} //

	bool triangular_prism::isInsideOrOn(const vertex& p1, const double& tol /*= 1e-3*/) const
	{ // 
		// check if it is inside a tetrahedron
		for (const auto& i : mTetrahedrons) if (i.isInsideOrOn(p1, tol)) return true;
		return false;
	} //

} // namespace geometry
} // namespace utilities
} // namespace bso

#endif // triangular_prism_CPP