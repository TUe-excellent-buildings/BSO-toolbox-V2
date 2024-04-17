#ifndef DELAUNAY_CPP
#define DELAUNAY_CPP

#include <bitset>
#include <Fade_2D.h>
#include <FadeExport.h>
#include <iomanip> // Allowes to use std::setprecision() to be able to enlarge the decimals printed to the console of a double					 

namespace bso { namespace utilities {

	std::vector<utilities::geometry::triangle> delaunay(const std::vector <utilities::geometry::vertex> delaunayPoints, const std::vector <utilities::geometry::line_segment> delaunayLines)
	{
		// Input checks
		if(delaunayPoints.size() <= 2)
		{
			std::stringstream errorMessage;
				errorMessage << "\nError, the inserted points in the delaunay triangulation are less then three. \n"
					 << "Meaning that a delaunay triangulation can not be generated. \n"
					 << "(bso/spatial_design/conformal/cf_building_model)." << std::endl;
				throw std::runtime_error(errorMessage.str());
		}
		else if(delaunayLines.size() == 0)
		{
			std::stringstream errorMessage;
				errorMessage << "\nError, there are no constrain lines added to the delaunay triangulation. \n"
					 << "Meaning that a delaunay triangulation can not be generated. \n"
					 << "(bso/spatial_design/conformal/cf_building_model)." << std::endl;
				throw std::runtime_error(errorMessage.str());
		}
		
		
		//Check for duplicate vertices in the delaunay point variable
		bool duplicates = false; 
		std::vector<utilities::geometry::vertex> duplicatePoints;
		for(int t =0; t< delaunayPoints.size(); t++)
		{ 
			for(int s=0; s<delaunayPoints.size(); s++)
			{
				if(s==t){continue;}
				else if(delaunayPoints[t].isSameAs(delaunayPoints[s], mTol))
				{
					duplicates = true;
					duplicatePoints.push_back(delaunayPoints[s]);
				}
			}
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
			errorMessage << std::endl
						 << "the following ms_N_building Model is inserted: " << mMSNModel << std::endl;		 
			errorMessage << "(bso/spatial_design/conformal/cf_building_model)." << std::endl;
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
			errorMessage << "(bso/spatial_design/conformal/cf_building_model)." << std::endl;
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
		// dt.show("Design.ps",true);		

		// Export fade2D data to Toolbox information format
		GEOM_FADE2D::FadeExport fadeExport;
		bool bCustomIndices(false); // To retrieve custom indices also
		bool bClear(false); // Clear memory in $dt
		dt.exportTriangulation(fadeExport,bCustomIndices,bClear);
		
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
			
			// Skip unviable triangles for the BSO toolbox
				// This is needed since FADE_2D calculates with a finer tolerance then the BSO toolbox allows.
				// Therefore, FADE_2D create unlogical triangles from the BSO Toolbox perspective regarding the BSO toolbox tolerances
				// the main reasons to skip a triangle are as follows:
				// - two cornerpoints of a single triangle are the same according to the BSO toolbox tolerances.
				// - the three points of the triangle are on the same line in accordance to the BSO toolbox tolerances.
				// See paper automatic partitioning methods for more information.
			try
			{
				utilities::geometry::triangle tri = {{C1},{C2},{C3}};
				delaunayTri.push_back(tri);
			}
			catch(std::exception& e)
			{
				//std::cout << "Triangle was unviable for BSO Toolbox" << std::endl;
				continue;
			}
		}
		
		if(delaunayTri.size() == 0)
		{
			std::stringstream errorMessage;
				errorMessage << "\nError, there are no triangle generated during the delaunay triangulation. \n"
					 << "(bso/spatial_design/conformal/cf_building_model)." << std::endl;
				throw std::runtime_error(errorMessage.str());
		}
		
		return delaunayTri;
	} //delaunay()
	
	void splitLines(const std::vector<utilities::geometry::vertex>& intsecV, std::vector<utilities::geometry::line_segment>& intsecL)
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
	}//cf_space::splitLines
	

} // namespace utilities
} // namespace bso


#endif // DELAUNAY_CPP

