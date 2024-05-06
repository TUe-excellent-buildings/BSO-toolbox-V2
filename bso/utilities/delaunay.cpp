#ifndef DELAUNAY_CPP
#define DELAUNAY_CPP

#include <bitset>
#include <Fade_2D.h>
#include <FadeExport.h>
#include <iomanip> // Allowes to use std::setprecision() to be able to enlarge the decimals printed to the console of a double		
#include <vector>
#include <bso/utilities/geometry.hpp>		 


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

	
	std::vector<utilities::geometry::triangle> delaunayZoned(const std::vector <utilities::geometry::vertex> delaunayPoints, const std::vector <utilities::geometry::line_segment> orderedLinesOfZone, const double mTol, bool outputPs)
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

		// Start Delaunay triangulation process
		// start adding the points/zoneLines to delaunay triangulation algorithm Fade_2D
		GEOM_FADE2D::Fade_2D dt;
		
		for(const auto i: delaunayPoints)
		{
			dt.insert(GEOM_FADE2D::Point2(i.x(),i.y()));
		}

		// Start adding the zoneLines to delaunay triangulation algorithm Fade_2D
		std::vector<GEOM_FADE2D::Segment2> vSegments;
		for(const auto i: orderedLinesOfZone)
		{
			vSegments.push_back(GEOM_FADE2D::Segment2(GEOM_FADE2D::Point2(i[0].x(),i[0].y()),GEOM_FADE2D::Point2(i[1].x(),i[1].y())));
		}
		GEOM_FADE2D::ConstraintGraph2* pCG=dt.createConstraint(vSegments,GEOM_FADE2D::CIS_CONSTRAINED_DELAUNAY);
		
		// create zone
		GEOM_FADE2D::Zone2* pZone0(dt.createZone(pCG,GEOM_FADE2D::ZL_INSIDE));

		// Makes the constrained delaunay a delaunay respecting the boundaries by adding the intersection point found on the boundary after delaunay
		bool applyConstrainedDelaunay = true;
		if(!applyConstrainedDelaunay)
		{
			double minLen(0.1);
			pCG->makeDelaunay(minLen);
		}

		// Draw; generate a Design.ps file for visualization in for example illustrator 
		if(outputPs) 
		{
			pZone0->show("Design_Zone0.ps",true,true);
			dt.show("Design.ps",true);		
		}

		std::vector<GEOM_FADE2D::Triangle2*> zTriangles;
		pZone0->getTriangles(zTriangles);

		std::vector<utilities::geometry::triangle> delaunayTri;
		for(const auto i: zTriangles)
		{
			// Get FADE_2D variables:
			GEOM_FADE2D::Point2* corner1 = i->getCorner(0);
			GEOM_FADE2D::Point2* corner2 = i->getCorner(1);
			GEOM_FADE2D::Point2* corner3 = i->getCorner(2);
			double x0,y0,x1,y1,x2,y2;
			corner1->xy(x0,y0);
			corner2->xy(x1,y1);
			corner3->xy(x2,y2);

			// Define BSO toolbox variables:
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
				errorMessage << "\nError, there are no triangle generated during the delaunayZoned triangulation. \n"
					 << "(bso/utilities/delaunay)." << std::endl;
				throw std::runtime_error(errorMessage.str());
		}
		
		return delaunayTri;
	} //delaunayZoned()
	
	
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

	void convertToQuads(const std::vector <utilities::geometry::vertex>& delaunayPoints, const std::vector <utilities::geometry::line_segment>& orderedLinesOfZone, std::vector <utilities::geometry::quadrilateral>& quads, const double mTol /*= 1e-3*/, bool outputPs /*= false*/)
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

		// Start Delaunay triangulation process
		// start adding the points/zoneLines to delaunay triangulation algorithm Fade_2D
		GEOM_FADE2D::Fade_2D dt;
		for(const auto i: delaunayPoints){dt.insert(GEOM_FADE2D::Point2(i.x(),i.y()));}

		// Start adding the zoneLines to delaunay triangulation algorithm Fade_2D
		std::vector<GEOM_FADE2D::Segment2> vSegments;
		for(const auto i: orderedLinesOfZone){vSegments.push_back(GEOM_FADE2D::Segment2(GEOM_FADE2D::Point2(i[0].x(),i[0].y()),GEOM_FADE2D::Point2(i[1].x(),i[1].y())));}
		GEOM_FADE2D::ConstraintGraph2* pCG=dt.createConstraint(vSegments,GEOM_FADE2D::CIS_CONSTRAINED_DELAUNAY);
		
		// create zone
		GEOM_FADE2D::Zone2* pZone0(dt.createZone(pCG,GEOM_FADE2D::ZL_INSIDE));

		// Makes the constrained delaunay a delaunay respecting the boundaries by adding the intersection point found on the boundary after delaunay
		bool applyConstrainedDelaunay = true;
		if(!applyConstrainedDelaunay)
		{
			double minLen(0.1);
			pCG->makeDelaunay(minLen);
		}

		// Draw; generate a Design.ps file for visualization in for example illustrator 
		if(outputPs) 
		{
			pZone0->show("Design_Zone0.ps",true,true);
			dt.show("Design.ps",true);		
		}

		std::vector<GEOM_FADE2D::Triangle2*> zTriangles;
		pZone0->getTriangles(zTriangles);
		std::vector<GEOM_FADE2D::Triangle2*> allTriangles;
		dt.getTrianglePointers(allTriangles);

		pZone0->show("Design_Zone0.ps",true,true);

		// Try to find quad options
		// Find a starting triangle with one oppositeTriangle or max two
		std::vector<int> startTri1;
		std::vector<int> startTri2;
		std::vector<bool> zTrianglesChecked; // initialize to keep track of triangles already added.
		std::vector<std::vector<int>> opposingTri; // make a vector for each triangle with their opposite triangles
		std::vector<GEOM_FADE2D::Triangle2*>::iterator it;
		std::vector<int> indexOppTri;
		indexOppTri.resize(3);
		for(int i=0; i < zTriangles.size(); i++)
		{
			GEOM_FADE2D::Triangle2* opposing_0 = zTriangles[i]->getOppositeTriangle(0);
			GEOM_FADE2D::Triangle2* opposing_1 = zTriangles[i]->getOppositeTriangle(1);
			GEOM_FADE2D::Triangle2* opposing_2 = zTriangles[i]->getOppositeTriangle(2);

			// find start tri
			if(opposing_0 == NULL && opposing_1 == NULL || opposing_1 == NULL && opposing_2 == NULL || opposing_0 == NULL && opposing_2 == NULL)
			{
				startTri1.push_back(i);
			}
			if (opposing_0 == NULL || opposing_1 == NULL || opposing_2 == NULL){startTri2.push_back(i);}
			
			// Initiate track vector
			zTrianglesChecked.push_back(false);

			// store data in opposingTri
			if(opposing_0 == NULL) {indexOppTri[0] = -1;}
			else
			{
				it = std::find(zTriangles.begin(), zTriangles.end(), opposing_0);
				if(it != zTriangles.end()) {indexOppTri[0] = it - zTriangles.begin();}
				else {indexOppTri[0] = -1;} // opposing Triangle is not inside considered shape
			}
			if(opposing_1 == NULL) {indexOppTri[1] = -1;}
			else
			{
				it = std::find(zTriangles.begin(), zTriangles.end(), opposing_1);
				if(it != zTriangles.end()) {indexOppTri[1] = it - zTriangles.begin();}
				else {indexOppTri[1] = -1;} // opposing Triangle is not inside considered shape
			}
			if(opposing_2 == NULL) {indexOppTri[2] = -1;}
			else
			{
				it = std::find(zTriangles.begin(), zTriangles.end(), opposing_2);
				if(it != zTriangles.end()) {indexOppTri[2] = it - zTriangles.begin();}
				else {indexOppTri[2] = -1;} // opposing Triangle is not inside considered shape
			}

			opposingTri.push_back(indexOppTri);
		}


		// Start combining triangles
		int count =0;
		int indexTri; //index from zTriangles

		// Define starting triangle
		if(startTri1.size() > 0)
		{
			indexTri = startTri1[0];
		}
		else if(startTri2.size() > 0)
		{
			indexTri = startTri2[0];
		}
		else
		{
			/*error*/
		}

		std::vector <utilities::geometry::quadrilateral> quadsChoosen;
		std::vector <GEOM_FADE2D::Triangle2*> triComboNotFound;
		bool allChecked = false;
		while(!allChecked)
		{
			std::vector<bool> options = {{false},{false},false};
			std::vector<utilities::geometry::quadrilateral> quadOptions;
			quadOptions.resize(3);
			
			GEOM_FADE2D::Point2* corner0;
			GEOM_FADE2D::Point2* corner1;
			GEOM_FADE2D::Point2* corner2;
			GEOM_FADE2D::Point2* corner3;

			// init considered triangle value's
			corner0 = zTriangles[indexTri]->getCorner(0);
			corner1 = zTriangles[indexTri]->getCorner(1);
			corner2 = zTriangles[indexTri]->getCorner(2);
			double x0,y0,x1,y1,x2,y2,x3,y3;
			corner0->xy(x0,y0);
			corner1->xy(x1,y1);
			corner2->xy(x2,y2);

			if((opposingTri[indexTri])[0] != -1)
			{
				for(int ith=0; ith<3; ith++)
				{
					corner3 = zTriangles[(opposingTri[indexTri])[0]]->getCorner(ith);
					if (corner3!=corner0 && corner3!=corner1 && corner3!=corner2) break;
				}
				corner3->xy(x3,y3);
				// Define BSO toolbox variables:
				std::vector<utilities::geometry::vertex> mVertices = {{x0,y0,0},{x1,y1,0},{x2,y2,0},{x3,y3,0}};
				// Check for valid quadrilateral
				utilities::geometry::line_segment l01 = {mVertices[1], mVertices[0]};
				utilities::geometry::line_segment l23 = {mVertices[3], mVertices[2]};
				utilities::geometry::line_segment l03 = {mVertices[3], mVertices[0]};
				utilities::geometry::line_segment l12 = {mVertices[2], mVertices[1]};
				if (l01.intersectsWith(l23,mTol)){std::iter_swap(mVertices.begin() + 1, mVertices.begin() + 2);}
				else if (l03.intersectsWith(l12, mTol)){std::iter_swap(mVertices.begin() + 2, mVertices.begin() + 3);}
				utilities::geometry::line_segment l02 = {mVertices[2], mVertices[0]};
				utilities::geometry::line_segment l13 = {mVertices[3], mVertices[1]};
				if (!l02.intersectsWith(l13, mTol)){/*non-convex quadrilateral*/;}
				else
				{
					quadOptions[0] = utilities::geometry::quadrilateral(mVertices);
					options[0] = true;
				}
			}
			if((opposingTri[indexTri])[1] != -1)
			{
				for(int ith=0; ith<3; ith++)
				{
					corner3 = zTriangles[(opposingTri[indexTri])[1]]->getCorner(ith);
					if (corner3!=corner0 && corner3!=corner1 && corner3!=corner2) break;
				}
				corner3->xy(x3,y3);
				// Define BSO toolbox variables:
				std::vector<utilities::geometry::vertex> mVertices = {{x0,y0,0},{x1,y1,0},{x2,y2,0},{x3,y3,0}};
				// Check for valid quadrilateral
				utilities::geometry::line_segment l01 = {mVertices[1], mVertices[0]};
				utilities::geometry::line_segment l23 = {mVertices[3], mVertices[2]};
				utilities::geometry::line_segment l03 = {mVertices[3], mVertices[0]};
				utilities::geometry::line_segment l12 = {mVertices[2], mVertices[1]};
				if (l01.intersectsWith(l23,mTol)){std::iter_swap(mVertices.begin() + 1, mVertices.begin() + 2);}
				else if (l03.intersectsWith(l12, mTol)){std::iter_swap(mVertices.begin() + 2, mVertices.begin() + 3);}
				utilities::geometry::line_segment l02 = {mVertices[2], mVertices[0]};
				utilities::geometry::line_segment l13 = {mVertices[3], mVertices[1]};
				if (!l02.intersectsWith(l13, mTol)){/*non-convex quadrilateral*/;}
				else
				{
					quadOptions[1] = utilities::geometry::quadrilateral(mVertices);
					options[1] = true;
				}
			}
			if((opposingTri[indexTri])[2] != -1)
			{
				for(int ith=0; ith<3; ith++)
				{
					corner3 = zTriangles[(opposingTri[indexTri])[2]]->getCorner(ith);
					if (corner3!=corner0 && corner3!=corner1 && corner3!=corner2) break;
				}
				corner3->xy(x3,y3);
				// Define BSO toolbox variables:
				std::vector<utilities::geometry::vertex> mVertices = {{x0,y0,0},{x1,y1,0},{x2,y2,0},{x3,y3,0}};
				// Check for valid quadrilateral
				utilities::geometry::line_segment l01 = {mVertices[1], mVertices[0]};
				utilities::geometry::line_segment l23 = {mVertices[3], mVertices[2]};
				utilities::geometry::line_segment l03 = {mVertices[3], mVertices[0]};
				utilities::geometry::line_segment l12 = {mVertices[2], mVertices[1]};
				if (l01.intersectsWith(l23,mTol)){std::iter_swap(mVertices.begin() + 1, mVertices.begin() + 2);}
				else if (l03.intersectsWith(l12, mTol)){std::iter_swap(mVertices.begin() + 2, mVertices.begin() + 3);}
				utilities::geometry::line_segment l02 = {mVertices[2], mVertices[0]};
				utilities::geometry::line_segment l13 = {mVertices[3], mVertices[1]};
				if (!l02.intersectsWith(l13, mTol)){/*non-convex quadrilateral*/;}
				else
				{
					quadOptions[2] = utilities::geometry::quadrilateral(mVertices);
					options[2] = true;
				}
			}

			// Choose best option, which is the options closest to a rectangle
			double difRectangle = 90+90+90+90; // too high value to receive
			int bestOption = 5; // set to default value to check later if bestOption is found
			for(int j=0; j<3; j++)
			{
				if(options[j])
				{
					if(quadOptions[j].getDifRectangle() < difRectangle){bestOption = j;}
				}
			}
			
			bool found = false;
			if(bestOption==5) // option is not found
			{
				zTrianglesChecked[indexTri] = true;
				triComboNotFound.push_back(zTriangles[indexTri]);
			}
			else
			{
				// Define quad and next tri connected to just found quad
				quadsChoosen.push_back(quadOptions[bestOption]);
				zTrianglesChecked[indexTri] = true;
				zTrianglesChecked[(opposingTri[indexTri])[bestOption]] = true;

				// find index opposing of the current opposing and check if it is already in a Quad
				bool found = false;
				for(int j=0; j<3;j++)
				{
					if(opposingTri[(opposingTri[indexTri])[bestOption]][j] != -1 && zTrianglesChecked[opposingTri[(opposingTri[indexTri])[bestOption]][j]] == false)
					{
						indexTri = opposingTri[(opposingTri[indexTri])[bestOption]][j];
						found = true;
						break;
					}
				}
			}

			if(!found)
			{				
				// Check for not checked Triangles
				for(int j=indexTri+1; j!=indexTri; j++)
				{
					if(j >= zTrianglesChecked.size()){j = 0;}
					else if(zTrianglesChecked[j] == false) // Triangle not yet in a quad
					{
						// define new indexTri
						indexTri = j;
						found = true;
						break;
					}
				}
				if(!found)
				{
					allChecked = true; //This wil terminate the while loop
				}
			}
		}

		

		// Make quads out of remaining triComboNotFound

		std::cout << "the choosen quads are: " << std::endl;
		for(const auto i: quadsChoosen)
		{
			std::cout << i << std::endl;
		}
		std::cout << "tri combo's not found are: " << std::endl;
		for(const auto i: triComboNotFound)
		{
			std::cout << *i << std::endl;
		}


		for(auto i: triComboNotFound)
		{			
			// Read fade_2D data
			GEOM_FADE2D::Point2* corner0 = i->getCorner(0);
			GEOM_FADE2D::Point2* corner1 = i->getCorner(1);
			GEOM_FADE2D::Point2* corner2 = i->getCorner(2);
			double x0,y0,x1,y1,x2,y2,x3,y3;
			corner0->xy(x0,y0);
			corner1->xy(x1,y1);
			corner2->xy(x2,y2);
			std::vector<utilities::geometry::vertex> mVerticesTri = {{x0,y0,0},{x1,y1,0},{x2,y2,0}};
			utilities::geometry::triangle tri(mVerticesTri);
			
			// Make BSO geometry data
			std::vector<utilities::geometry::vertex> mVertices;
			mVertices.resize(4);
			mVertices[0] = tri.getCenter();
			std::vector<utilities::geometry::line_segment> triLines = tri.getLines();
			utilities::geometry::vertex centreRib1 = triLines[0].getCenter();
			utilities::geometry::vertex centreRib2 = triLines[1].getCenter();
			utilities::geometry::vertex centreRib3 = triLines[2].getCenter();

			mVertices[1] = tri[0];
			mVertices[2] = centreRib1;
			mVertices[3] = centreRib3;
			quadsChoosen.push_back(utilities::geometry::quadrilateral(mVertices));

			mVertices[1] = tri[1];
			mVertices[2] = centreRib1;
			mVertices[3] = centreRib2;
			quadsChoosen.push_back(utilities::geometry::quadrilateral(mVertices));

			mVertices[1] = tri[2];
			mVertices[2] = centreRib2;
			mVertices[3] = centreRib3;
			quadsChoosen.push_back(utilities::geometry::quadrilateral(mVertices));
		}

		quads = quadsChoosen;
	}

} // namespace utilities
} // namespace bso


#endif // DELAUNAY_CPP

