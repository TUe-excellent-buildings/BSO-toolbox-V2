#ifndef DELAUNAY_CPP
#define DELAUNAY_CPP

#include <bitset>
#include <Fade_2D.h>
#include <FadeExport.h>
#include <iomanip> // Allowes to use std::setprecision() to be able to enlarge the decimals printed to the console of a double					 

namespace bso { namespace utilities {

	std::vector<utilities::geometry::triangle> delaunay(const std::vector <utilities::geometry::vertex> delaunayPoints, const std::vector <utilities::geometry::line_segment> delaunayLines, const double mTol, bool outputPs)
	{
		// Input check
		if(delaunayPoints.size() <= 2)
		{
			std::stringstream errorMessage;
				errorMessage << "\nError, the inserted points in the delaunay triangulation are less then three. \n"
					 << "Meaning that a delaunay triangulation can not be generated. \n"
					 << "(bso/utilities/delaunay)." << std::endl;
				throw std::runtime_error(errorMessage.str());
		}		

		//Check the constrain lines
		std::vector<utilities::geometry::vertex> noDelauneyPoint;
		std::vector<utilities::geometry::vertex> pointOnConstraintLine;
		std::vector<utilities::geometry::line_segment> contrainLines;
		for(auto t: delaunayLines)
		{
			for(const auto k: t)
			{				
				bool found = false;
				for(const auto u: delaunayPoints) 
				{
					if(u.isSameAs(k)) found = true;
				}
				if(!found)
				{
					noDelauneyPoint.push_back(k);
				}
				if(t.isOnLine(k))
				{
					pointOnConstraintLine.push_back(k);
				}
			}
			for(auto k: delaunayLines)
			{
				if(k.isSameAs(t, mTol)){continue;}
				for(const auto s: k)
				{
					if(t.isOnLine(s, mTol))
					{
						pointOnConstraintLine.push_back(s);
						contrainLines.push_back(t);
					}
				}
			}
		}
		if(noDelauneyPoint.size() > 0)
		{
			std::stringstream errorMessage;
			errorMessage << "The following vertices are end vertices of the constraint lines, but not a delaunay points (Checked with tolerance): " << std::endl;
			for(const auto t: noDelauneyPoint){errorMessage << t;}
			errorMessage << std::endl	
						 << "The following are the delauney points: " << std::endl;
			for(const auto t: delaunayPoints){errorMessage << t << " ; ";}	
			errorMessage << std::endl;
			errorMessage << "(bso/utilities/delaunay)." << std::endl;
			throw std::runtime_error(errorMessage.str());
		}
		if(pointOnConstraintLine.size() > 0)
		{
			std::stringstream errorMessage;
			int counter=0;
			errorMessage << "The following vertices are vertices on the constrain line, this is not allowed and should have been solved in the splitLines function. See the following vertices: " << std::endl;
			for(const auto t: pointOnConstraintLine)
			{
				errorMessage << "vertex: " << t << " has been found on: " << contrainLines[counter] << std::endl;
				counter++;
			}
			errorMessage << "(bso/utilities/delaunay)." << std::endl;
			throw std::runtime_error(errorMessage.str());
		}
		

		// Start Delaunay triangulation process
		// start adding the points/constrainLines to delaunay triangulation algorithm Fade_2D
		GEOM_FADE2D::Fade_2D dt;
		
		for(const auto i: delaunayPoints)
		{
			dt.insert(GEOM_FADE2D::Point2(i.x(),i.y()));
		}
		std::vector<GEOM_FADE2D::Segment2> vSegments;
		for(const auto i: delaunayLines)
		{
			vSegments.push_back(GEOM_FADE2D::Segment2(GEOM_FADE2D::Point2(i[0].x(),i[0].y()),GEOM_FADE2D::Point2(i[1].x(),i[1].y())));
		}
		GEOM_FADE2D::ConstraintGraph2* pCG=dt.createConstraint(vSegments,GEOM_FADE2D::CIS_CONSTRAINED_DELAUNAY);
		
		
		// Makes the constrained delaunay a delaunay respecting the boundaries by adding the intersection point found on the boundary after delaunay
		bool applyConstrainedDelaunay = true;
		if(!applyConstrainedDelaunay)
		{
			double minLen(0.1);
			pCG->makeDelaunay(minLen);
		}

		// Draw; generate a Design.ps file for visualization in for example illustrator 
		if(outputPs) dt.show("Design.ps",true);		

		// Export fade2D data to Toolbox information format
		GEOM_FADE2D::FadeExport fadeExport;
		bool bCustomIndices(false); // To retrieve custom indices also
		bool bClear(false); // Clear memory in $dt
		dt.exportTriangulation(fadeExport,bCustomIndices,bClear);
		
		std::vector<utilities::geometry::triangle> delaunayTri; // fade_2D resulting delaunay triangles, but defined in BSO toolbox geometry classes
		for(int triIdx=0;triIdx<fadeExport.numTriangles;++triIdx)
		{ // Iterate through each triangle in Fade_2D and store triangle in Toolbox entities
			int vtxIdx0,vtxIdx1,vtxIdx2;
			fadeExport.getCornerIndices(triIdx,vtxIdx0,vtxIdx1,vtxIdx2);
			
			double x0,y0;
			double x1,y1;
			double x2,y2;
			fadeExport.getCoordinates(vtxIdx0,x0,y0);
			fadeExport.getCoordinates(vtxIdx1,x1,y1);
			fadeExport.getCoordinates(vtxIdx2,x2,y2);
			
			// Define triangle in Toolbox entities
			utilities::geometry::vertex C1 = {x0,y0,0};
			utilities::geometry::vertex C2 = {x1,y1,0};
			utilities::geometry::vertex C3 = {x2,y2,0};
			
			try
			{
				utilities::geometry::triangle tri = {{C1},{C2},{C3}};
				delaunayTri.push_back(tri);
			}
			catch(std::exception& e)
			{
				// Skip unviable triangles for the BSO toolbox
					// This is needed since FADE_2D calculates with a finer tolerance then the BSO toolbox allows.
					// Therefore, FADE_2D create unlogical triangles from the BSO Toolbox perspective regarding the BSO toolbox tolerances
					// the main reasons to skip a triangle are as follows:
					// - two cornerpoints of a single triangle are the same according to the BSO toolbox tolerances.
					// - the three points of the triangle are on the same line in accordance to the BSO toolbox tolerances.
					// See paper automatic partitioning methods for more information.
				continue;
			}
		}
		
		if(delaunayTri.size() == 0)
		{
			std::stringstream errorMessage;
				errorMessage << "\nError, there are no triangle generated during the delaunay triangulation. \n"
					 << "(bso/utilities/delaunay)." << std::endl;
				throw std::runtime_error(errorMessage.str());
		}
		
		return delaunayTri;
	} //delaunay()
	
	void splitLines(const std::vector<utilities::geometry::vertex>& intsecV, std::vector<utilities::geometry::line_segment>& intsecL, const double mTol)
	{					
		// split lines in accordance to intersection points
		for(int k=0; k<intsecL.size(); k++)
		{
			for(const auto l :intsecV)
			{
				if(intsecL[k].isOnLine(l, mTol))
				{
					utilities::geometry::line_segment one = {{(intsecL[k])[0]},{l}};
					utilities::geometry::line_segment two = {{(intsecL[k])[1]},{l}};
					
					intsecL.push_back(one);
					intsecL.push_back(two);

					intsecL.erase(intsecL.begin() + k);

					k--; // Makes sure the for loop isn't snipping a constrain lines when one is erased.
				}
			}
		}
		
		// remove duplicates from intsecL
		auto intsecL_end = intsecL.end();
		for(auto it = intsecL.begin(); it != intsecL_end; ++it)
		{
			intsecL_end = std::remove(it + 1, intsecL_end, *it);
		}
		intsecL.erase(intsecL_end, intsecL.end());
		
		
		// remove duplicates which has switched vertexes in intsecL		
		for(int k; k<intsecL.size(); k++)
		{
			for(auto m : intsecL)
			{
				if (m == intsecL[k]){continue;} //No tolerance needed. Just operator to make sure the same entities are not checked with each other.
				if (m.isSameAs(intsecL[k], mTol)){intsecL.erase(intsecL.begin() + k);}
			}
		}		
	}//splitLines

	void getNOBSDAtGround(const bso::spatial_design::ms_N_building& msNModel, std::vector<std::pair<bso::spatial_design::ms_space*, utilities::geometry::quad_hexahedron>>& spaceAtGroundV, std::vector<double>& allLevels, const double mTol)
	{
		for (const auto i : msNModel)
		{
			//Get original space geometry
			
			utilities::geometry::quad_hexahedron spaceGeom = i->getGeometry();
						
			//Find the highest and lowest vertex to add to allLevels and for the definition of the shifting vector
			utilities::geometry::vertex lowVertex = spaceGeom[0];
			utilities::geometry::vertex highVertex = spaceGeom[0];
			for (const auto j: spaceGeom)
			{
				if(lowVertex.z() > j.z()) lowVertex = j;
				else if(highVertex.z() < j.z()) highVertex = j;
			}			
			allLevels.push_back(highVertex.z());
			allLevels.push_back(lowVertex.z());

			//Generate the at ground level space geometry
			std::vector<utilities::geometry::vertex> vertices_1;
			utilities::geometry::vertex shift = {0,0,lowVertex.z()};
			if(lowVertex.z() < mTol && lowVertex.z() > -mTol) for(auto k: spaceGeom){vertices_1.push_back(k);}
			else for(auto k: spaceGeom){vertices_1.push_back(k-shift);}
			utilities::geometry::quad_hexahedron atGround = utilities::geometry::quad_hexahedron(vertices_1, mTol);

			std::pair<bso::spatial_design::ms_space*, utilities::geometry::quad_hexahedron> spaceGeomPair (i,atGround);
			spaceAtGroundV.push_back(spaceGeomPair);
		}
		
		// Sort vector, bring all duplicates to end of vector plus get cut off point, then erase duplicates from the end of vector allLevels:
		sort(allLevels.begin(), allLevels.end()); 
		auto it = unique(allLevels.begin(), allLevels.end()); 
		allLevels.erase(it, allLevels.end()); 
	} //getNOBSDAtGround

	void getNOBSDAtGround(const bso::spatial_design::ms_N_building& msNModel, std::vector<std::pair<bso::spatial_design::ms_space*, utilities::geometry::quad_hexahedron>>& spaceAtGroundV, const double mTol)
	{
		for (const auto i : msNModel)
		{
			//Get original space geometry
			utilities::geometry::quad_hexahedron spaceGeom = i->getGeometry();
						
			//Find the highest and lowest vertex to add to allLevels and for the definition of the shifting vector
			utilities::geometry::vertex lowVertex = spaceGeom[0];
			for (const auto j: spaceGeom)
			{
				if(lowVertex.z() > j.z()) lowVertex = j;
			}			

			//Generate the at ground level space geometry
			std::vector<utilities::geometry::vertex> vertices_1;
			utilities::geometry::vertex shift = {0,0,lowVertex.z()};
			if(lowVertex.z() < mTol && lowVertex.z() > -mTol) for(auto k: spaceGeom){vertices_1.push_back(k);}
			else for(auto k: spaceGeom){vertices_1.push_back(k-shift);}
			utilities::geometry::quad_hexahedron atGround = utilities::geometry::quad_hexahedron(vertices_1, mTol);

			std::pair<bso::spatial_design::ms_space*, utilities::geometry::quad_hexahedron> spaceGeomPair (i,atGround);
			spaceAtGroundV.push_back(spaceGeomPair);
		}
	} //getNOBSDAtGround

	void getQuadAtZ0(std::vector <utilities::geometry::quadrilateral>& quadAtZ0, std::vector<std::pair<bso::spatial_design::ms_space*, utilities::geometry::quad_hexahedron>>& spacePair, const double mTol)
	{
		for(const auto j: spacePair)
		{
			// Add the unique lines of each space at Z to quadAtZ0
			for (const auto& k : (j.second).getQuadrilateral())
			{
				if (k.getCenter().z() > mTol && k.getCenter().z() < -mTol && k.getNormal().isVertical()) continue; // only consider horizontal floors which are at z=0
				
				bool doubleInsert = false;
				for (const auto& m : quadAtZ0)
				{// check if quad k already exist in quadAtZ0
					if (k.isSameAs(m,mTol))
					{
						doubleInsert = true;
					}
				}
				if(doubleInsert == false)
				{//if agree, then add to quadAtZ0
					quadAtZ0.push_back(k);
				}
			}
		}
	}

	void getLinesAtZ0(std::vector <utilities::geometry::line_segment>& linesAtZ, std::vector<std::pair<bso::spatial_design::ms_space*, utilities::geometry::quad_hexahedron>>& spacePair, const double mTol)
	{
		for(const auto j: spacePair)
		{
			// Add the unique lines of each space at Z to linesAtZ
			for (const auto& k : (j.second).getLines())
			{
				utilities::geometry::vertex Ltemp1 = {k[0].x(),k[0].y(),0};
				utilities::geometry::vertex Ltemp2 = {k[1].x(),k[1].y(),0};
				utilities::geometry::line_segment Ltemp {{Ltemp1},{Ltemp2}}; 
				
				bool doubleInsert = false;
				for (const auto& m : linesAtZ)
				{// check if line k already exist in linesAtZ
					if (m.isSameAs(Ltemp, mTol))
					{
						doubleInsert = true;
					}
				}
				if(Ltemp1.isSameAs(Ltemp2, mTol))
				{//Check if line has different end points
					doubleInsert = true;
				}
				if(doubleInsert == false)
				{//if both agree, then add to linesAtZ
					linesAtZ.push_back(Ltemp);
				}
			}
		}
	}

	void getDelaunayVariables(std::vector <utilities::geometry::vertex>& delaunayPoints, std::vector <utilities::geometry::line_segment>& delaunayLines, std::vector<std::pair<bso::spatial_design::ms_space*, utilities::geometry::quad_hexahedron>>& spacePair, const double mTol)
	{
		for(const auto j: spacePair)
		{
			// Add all unique cornerpoints at z=0 of spaceAtGround to the delaunayPoints
			for(auto v: j.second)
			{
				utilities::geometry::vertex temp = {v.x(),v.y(),0};
				bool doubleInsert = false;
				for (const auto& m : delaunayPoints)
				{// check if temp already exist in delaunayPoints
					if (m == temp)
					{
						doubleInsert = true;
					}
				}
				if(doubleInsert == false)
				{//if not, then add to delaunayPoints
					delaunayPoints.push_back(temp);
				}
			}

			// Add the unique lines of each space to delaunayLines
			for (const auto& k : (j.second).getLines())
			{
				utilities::geometry::vertex Ltemp1 = {k[0].x(),k[0].y(),0};
				utilities::geometry::vertex Ltemp2 = {k[1].x(),k[1].y(),0};
				utilities::geometry::line_segment Ltemp {{Ltemp1},{Ltemp2}}; //Note the line is newly defined for the case that some round of error occur during the shifting of the lines of the spacesAtGround[i]
				
				bool doubleInsert = false;
				for (const auto& m : delaunayLines)
				{// check if line k already exist in delaunayLines
					if (m.isSameAs(Ltemp, mTol))
					{
						doubleInsert = true;
					}
				}
				if(Ltemp1.isSameAs(Ltemp2, mTol))
				{//Check if line has different end points
					doubleInsert = true;
				}
				if(doubleInsert == false)
				{//if both agree, then add to delaunayLines
					delaunayLines.push_back(Ltemp);
				}
				
				//Find line-line intersections and add to delaunayPoints, then during splitLines() the lines will be split at this delaunayPoint
				//Note: no line-rectangle intersection check needed since these are accounted for by the addition of the cornerpoints
				utilities::geometry::vertex pIntersection;
				for(auto q: spacePair)
				{	
					if((q.second).isSameAs((j.second), mTol)){continue;}
					for (const auto& l : (q.second).getLines())
					{
						if (k.intersectsWith(l, pIntersection, mTol))
						{
							//consider the translated to ground floor vertex of the space
							utilities::geometry::vertex temp = {pIntersection.x(),pIntersection.y(),0};
							
							bool doubleInsert = false;
							for (const auto& m : delaunayPoints)
							{// check if temp already exist in delaunayPoints
								if (m.isSameAs(temp, mTol))
								{
									doubleInsert = true;
								}
							}
							
							if(doubleInsert == false)
							{//if not, then add to delaunayPoints
								delaunayPoints.push_back(temp);
							}
						}
					}
				}
			}
		}

		splitLines(delaunayPoints, delaunayLines);
	}

} // namespace utilities
} // namespace bso


#endif // DELAUNAY_CPP

