#ifndef GEOMETRY_FUNCTIONS_CPP
#define GEOMETRY_FUNCTIONS_CPP

#include <bitset>
#include <Fade_2D.h>
#include <FadeExport.h>
#include <iomanip> // Allowes to use std::setprecision() to be able to enlarge the decimals printed to the console of a double		
#include <vector>
#include <map>
#include <bso/utilities/geometry.hpp>
#include <bso/utilities/geometry/polygon_non_convex.hpp>


namespace bso { namespace utilities {
	
	// void splitLines(const std::vector<utilities::geometry::vertex>& intsecV, std::vector<utilities::geometry::line_segment>& intsecL, bool removeDuplicates /*= true*/, const double mTol)
	// {					
	// 	// split lines in accordance to intersection points
	// 	for(int k=0; k<intsecL.size(); k++)
	// 	{
	// 		for(const auto l :intsecV)
	// 		{
	// 			if(intsecL[k].isOnLine(l, mTol))
	// 			{
	// 				utilities::geometry::line_segment one = {{(intsecL[k])[0]},{l}};
	// 				utilities::geometry::line_segment two = {{(intsecL[k])[1]},{l}};
					
	// 				intsecL.push_back(one);
	// 				intsecL.push_back(two);

	// 				intsecL.erase(intsecL.begin() + k);

	// 				k--; // Makes sure the for loop isn't snipping a constrain lines when one is erased.
	// 			}
	// 		}
	// 	}
		
	// 	if(removeDuplicates)
	// 	{
	// 		// remove duplicates from intsecL
	// 		auto intsecL_end = intsecL.end();
	// 		for(auto it = intsecL.begin(); it != intsecL_end; ++it)
	// 		{
	// 			intsecL_end = std::remove(it + 1, intsecL_end, *it);
	// 		}
	// 		intsecL.erase(intsecL_end, intsecL.end());
			
			
	// 		// remove duplicates which has switched vertexes in intsecL		
	// 		for(int k; k<intsecL.size(); k++)
	// 		{
	// 			for(auto m : intsecL)
	// 			{
	// 				if (m == intsecL[k]){continue;} // No tolerance needed. Just operator to make sure the same entities are not checked with each other.
	// 				if (m.isSameAs(intsecL[k], mTol)){intsecL.erase(intsecL.begin() + k);}
	// 			}
	// 		}
	// 	}	
	// }//splitLines

	// void convertToQuads(const std::vector <utilities::geometry::vertex>& delaunayPoints, const std::vector <utilities::geometry::line_segment>& orderedLinesOfZone, std::vector <utilities::geometry::quadrilateral>& quads, const double mTol /*= 1e-3*/, bool outputPs /*= false*/)
	// {
	// 	// Input check
	// 	if(delaunayPoints.size() <= 2)
	// 	{
	// 		std::stringstream errorMessage;
	// 			errorMessage << "\nError, the inserted points in the delaunay triangulation are less then three. \n"
	// 				 << "Meaning that a delaunay triangulation can not be generated. \n"
	// 				 << "(bso/utilities/delaunay)." << std::endl;
	// 			throw std::runtime_error(errorMessage.str());
	// 	}		

	// 	// Start Delaunay triangulation process
	// 	// start adding the points/zoneLines to delaunay triangulation algorithm Fade_2D
	// 	GEOM_FADE2D::Fade_2D dt;
	// 	for(const auto i: delaunayPoints){dt.insert(GEOM_FADE2D::Point2(i.x(),i.y()));}

	// 	// Start adding the zoneLines to delaunay triangulation algorithm Fade_2D
	// 	std::vector<GEOM_FADE2D::Segment2> vSegments;
	// 	for(const auto i: orderedLinesOfZone){vSegments.push_back(GEOM_FADE2D::Segment2(GEOM_FADE2D::Point2(i[0].x(),i[0].y()),GEOM_FADE2D::Point2(i[1].x(),i[1].y())));}
	// 	GEOM_FADE2D::ConstraintGraph2* pCG=dt.createConstraint(vSegments,GEOM_FADE2D::CIS_CONSTRAINED_GEOMETRY_FUNCTIONS);
		
	// 	// create zone
	// 	GEOM_FADE2D::Zone2* pZone0(dt.createZone(pCG,GEOM_FADE2D::ZL_INSIDE));

	// 	// Makes the constrained delaunay a delaunay respecting the boundaries by adding the intersection point found on the boundary after delaunay
	// 	bool applyConstrainedDelaunay = true;
	// 	if(!applyConstrainedDelaunay)
	// 	{
	// 		double minLen(0.1);
	// 		pCG->makeDelaunay(minLen);
	// 	}

	// 	// Draw; generate a Design.ps file for visualization in for example illustrator 
	// 	if(outputPs) 
	// 	{
	// 		pZone0->show("Design_Zone0.ps",true,true);
	// 		dt.show("Design.ps",true);		
	// 	}

	// 	std::vector<GEOM_FADE2D::Triangle2*> zTriangles;
	// 	pZone0->getTriangles(zTriangles);
	// 	std::vector<GEOM_FADE2D::Triangle2*> allTriangles;
	// 	dt.getTrianglePointers(allTriangles);

	// 	pZone0->show("Design_Zone0.ps",true,true);

	// 	// Try to find quad options
	// 	// Find a starting triangle with one oppositeTriangle or max two
	// 	std::vector<int> startTri1;
	// 	std::vector<int> startTri2;
	// 	std::vector<bool> zTrianglesChecked; // initialize to keep track of triangles already added.
	// 	std::vector<std::vector<int>> opposingTri; // make a vector for each triangle with their opposite triangles
	// 	std::vector<GEOM_FADE2D::Triangle2*>::iterator it;
	// 	std::vector<int> indexOppTri;
	// 	indexOppTri.resize(3);
	// 	for(int i=0; i < zTriangles.size(); i++)
	// 	{
	// 		GEOM_FADE2D::Triangle2* opposing_0 = zTriangles[i]->getOppositeTriangle(0);
	// 		GEOM_FADE2D::Triangle2* opposing_1 = zTriangles[i]->getOppositeTriangle(1);
	// 		GEOM_FADE2D::Triangle2* opposing_2 = zTriangles[i]->getOppositeTriangle(2);

	// 		// find start tri
	// 		if(opposing_0 == NULL && opposing_1 == NULL || opposing_1 == NULL && opposing_2 == NULL || opposing_0 == NULL && opposing_2 == NULL)
	// 		{
	// 			startTri1.push_back(i);
	// 		}
	// 		if (opposing_0 == NULL || opposing_1 == NULL || opposing_2 == NULL){startTri2.push_back(i);}
			
	// 		// Initiate track vector
	// 		zTrianglesChecked.push_back(false);

	// 		// store data in opposingTri
	// 		if(opposing_0 == NULL) {indexOppTri[0] = -1;}
	// 		else
	// 		{
	// 			it = std::find(zTriangles.begin(), zTriangles.end(), opposing_0);
	// 			if(it != zTriangles.end()) {indexOppTri[0] = it - zTriangles.begin();}
	// 			else {indexOppTri[0] = -1;} // opposing Triangle is not inside considered shape
	// 		}
	// 		if(opposing_1 == NULL) {indexOppTri[1] = -1;}
	// 		else
	// 		{
	// 			it = std::find(zTriangles.begin(), zTriangles.end(), opposing_1);
	// 			if(it != zTriangles.end()) {indexOppTri[1] = it - zTriangles.begin();}
	// 			else {indexOppTri[1] = -1;} // opposing Triangle is not inside considered shape
	// 		}
	// 		if(opposing_2 == NULL) {indexOppTri[2] = -1;}
	// 		else
	// 		{
	// 			it = std::find(zTriangles.begin(), zTriangles.end(), opposing_2);
	// 			if(it != zTriangles.end()) {indexOppTri[2] = it - zTriangles.begin();}
	// 			else {indexOppTri[2] = -1;} // opposing Triangle is not inside considered shape
	// 		}

	// 		opposingTri.push_back(indexOppTri);
	// 	}


	// 	// Start combining triangles
	// 	int count =0;
	// 	int indexTri; //index from zTriangles

	// 	// Define starting triangle
	// 	if(startTri1.size() > 0)
	// 	{
	// 		indexTri = startTri1[0];
	// 	}
	// 	else if(startTri2.size() > 0)
	// 	{
	// 		indexTri = startTri2[0];
	// 	}
	// 	else
	// 	{
	// 		/*error*/
	// 	}

	// 	std::vector <utilities::geometry::quadrilateral> quadsChoosen;
	// 	std::vector <GEOM_FADE2D::Triangle2*> triComboNotFound;
	// 	bool allChecked = false;
	// 	while(!allChecked)
	// 	{
	// 		std::vector<bool> options = {{false},{false},false};
	// 		std::vector<utilities::geometry::quadrilateral> quadOptions;
	// 		quadOptions.resize(3);
			
	// 		GEOM_FADE2D::Point2* corner0;
	// 		GEOM_FADE2D::Point2* corner1;
	// 		GEOM_FADE2D::Point2* corner2;
	// 		GEOM_FADE2D::Point2* corner3;

	// 		// init considered triangle value's
	// 		corner0 = zTriangles[indexTri]->getCorner(0);
	// 		corner1 = zTriangles[indexTri]->getCorner(1);
	// 		corner2 = zTriangles[indexTri]->getCorner(2);
	// 		double x0,y0,x1,y1,x2,y2,x3,y3;
	// 		corner0->xy(x0,y0);
	// 		corner1->xy(x1,y1);
	// 		corner2->xy(x2,y2);

	// 		if((opposingTri[indexTri])[0] != -1)
	// 		{
	// 			for(int ith=0; ith<3; ith++)
	// 			{
	// 				corner3 = zTriangles[(opposingTri[indexTri])[0]]->getCorner(ith);
	// 				if (corner3!=corner0 && corner3!=corner1 && corner3!=corner2) break;
	// 			}
	// 			corner3->xy(x3,y3);
	// 			// Define BSO toolbox variables:
	// 			std::vector<utilities::geometry::vertex> mVertices = {{x0,y0,0},{x1,y1,0},{x2,y2,0},{x3,y3,0}};
	// 			// Check for valid quadrilateral
	// 			utilities::geometry::line_segment l01 = {mVertices[1], mVertices[0]};
	// 			utilities::geometry::line_segment l23 = {mVertices[3], mVertices[2]};
	// 			utilities::geometry::line_segment l03 = {mVertices[3], mVertices[0]};
	// 			utilities::geometry::line_segment l12 = {mVertices[2], mVertices[1]};
	// 			if (l01.intersectsWith(l23,mTol)){std::iter_swap(mVertices.begin() + 1, mVertices.begin() + 2);}
	// 			else if (l03.intersectsWith(l12, mTol)){std::iter_swap(mVertices.begin() + 2, mVertices.begin() + 3);}
	// 			utilities::geometry::line_segment l02 = {mVertices[2], mVertices[0]};
	// 			utilities::geometry::line_segment l13 = {mVertices[3], mVertices[1]};
	// 			if (!l02.intersectsWith(l13, mTol)){/*non-convex quadrilateral*/;}
	// 			else
	// 			{
	// 				quadOptions[0] = utilities::geometry::quadrilateral(mVertices);
	// 				options[0] = true;
	// 			}
	// 		}
	// 		if((opposingTri[indexTri])[1] != -1)
	// 		{
	// 			for(int ith=0; ith<3; ith++)
	// 			{
	// 				corner3 = zTriangles[(opposingTri[indexTri])[1]]->getCorner(ith);
	// 				if (corner3!=corner0 && corner3!=corner1 && corner3!=corner2) break;
	// 			}
	// 			corner3->xy(x3,y3);
	// 			// Define BSO toolbox variables:
	// 			std::vector<utilities::geometry::vertex> mVertices = {{x0,y0,0},{x1,y1,0},{x2,y2,0},{x3,y3,0}};
	// 			// Check for valid quadrilateral
	// 			utilities::geometry::line_segment l01 = {mVertices[1], mVertices[0]};
	// 			utilities::geometry::line_segment l23 = {mVertices[3], mVertices[2]};
	// 			utilities::geometry::line_segment l03 = {mVertices[3], mVertices[0]};
	// 			utilities::geometry::line_segment l12 = {mVertices[2], mVertices[1]};
	// 			if (l01.intersectsWith(l23,mTol)){std::iter_swap(mVertices.begin() + 1, mVertices.begin() + 2);}
	// 			else if (l03.intersectsWith(l12, mTol)){std::iter_swap(mVertices.begin() + 2, mVertices.begin() + 3);}
	// 			utilities::geometry::line_segment l02 = {mVertices[2], mVertices[0]};
	// 			utilities::geometry::line_segment l13 = {mVertices[3], mVertices[1]};
	// 			if (!l02.intersectsWith(l13, mTol)){/*non-convex quadrilateral*/;}
	// 			else
	// 			{
	// 				quadOptions[1] = utilities::geometry::quadrilateral(mVertices);
	// 				options[1] = true;
	// 			}
	// 		}
	// 		if((opposingTri[indexTri])[2] != -1)
	// 		{
	// 			for(int ith=0; ith<3; ith++)
	// 			{
	// 				corner3 = zTriangles[(opposingTri[indexTri])[2]]->getCorner(ith);
	// 				if (corner3!=corner0 && corner3!=corner1 && corner3!=corner2) break;
	// 			}
	// 			corner3->xy(x3,y3);
	// 			// Define BSO toolbox variables:
	// 			std::vector<utilities::geometry::vertex> mVertices = {{x0,y0,0},{x1,y1,0},{x2,y2,0},{x3,y3,0}};
	// 			// Check for valid quadrilateral
	// 			utilities::geometry::line_segment l01 = {mVertices[1], mVertices[0]};
	// 			utilities::geometry::line_segment l23 = {mVertices[3], mVertices[2]};
	// 			utilities::geometry::line_segment l03 = {mVertices[3], mVertices[0]};
	// 			utilities::geometry::line_segment l12 = {mVertices[2], mVertices[1]};
	// 			if (l01.intersectsWith(l23,mTol)){std::iter_swap(mVertices.begin() + 1, mVertices.begin() + 2);}
	// 			else if (l03.intersectsWith(l12, mTol)){std::iter_swap(mVertices.begin() + 2, mVertices.begin() + 3);}
	// 			utilities::geometry::line_segment l02 = {mVertices[2], mVertices[0]};
	// 			utilities::geometry::line_segment l13 = {mVertices[3], mVertices[1]};
	// 			if (!l02.intersectsWith(l13, mTol)){/*non-convex quadrilateral*/;}
	// 			else
	// 			{
	// 				quadOptions[2] = utilities::geometry::quadrilateral(mVertices);
	// 				options[2] = true;
	// 			}
	// 		}

	// 		// Choose best option, which is the options closest to a rectangle
	// 		double difRectangle = 90+90+90+90; // too high value to receive
	// 		int bestOption = 5; // set to default value to check later if bestOption is found
	// 		for(int j=0; j<3; j++)
	// 		{
	// 			if(options[j])
	// 			{
	// 				if(quadOptions[j].getDifRectangle() < difRectangle){bestOption = j;}
	// 			}
	// 		}
			
	// 		bool found = false;
	// 		if(bestOption==5) // option is not found
	// 		{
	// 			zTrianglesChecked[indexTri] = true;
	// 			triComboNotFound.push_back(zTriangles[indexTri]);
	// 		}
	// 		else
	// 		{
	// 			// Define quad and next tri connected to just found quad
	// 			quadsChoosen.push_back(quadOptions[bestOption]);
	// 			zTrianglesChecked[indexTri] = true;
	// 			zTrianglesChecked[(opposingTri[indexTri])[bestOption]] = true;

	// 			// find index opposing of the current opposing and check if it is already in a Quad
	// 			bool found = false;
	// 			for(int j=0; j<3;j++)
	// 			{
	// 				if(opposingTri[(opposingTri[indexTri])[bestOption]][j] != -1 && zTrianglesChecked[opposingTri[(opposingTri[indexTri])[bestOption]][j]] == false)
	// 				{
	// 					indexTri = opposingTri[(opposingTri[indexTri])[bestOption]][j];
	// 					found = true;
	// 					break;
	// 				}
	// 			}
	// 		}

	// 		if(!found)
	// 		{				
	// 			// Check for not checked Triangles
	// 			int j = indexTri;
	// 			do
	// 			{
	// 				j++;
	// 				if(j >= zTrianglesChecked.size()){j = 0;}
	// 				if(zTrianglesChecked[j] == false) // Triangle not yet in a quad
	// 				{
	// 					// define new indexTri
	// 					indexTri = j;
	// 					found = true;
	// 					break;
	// 				}
	// 			} while (j!=indexTri);
	// 			if(!found)
	// 			{
	// 				allChecked = true; //This wil terminate the while loop
	// 			}
	// 		}
	// 	}

		
	// 	// Make quads out of remaining triComboNotFound
	// 	std::cout << "the choosen quads are: " << std::endl;
	// 	for(const auto i: quadsChoosen)
	// 	{
	// 		std::cout << i << std::endl;
	// 	}
	// 	std::cout << "tri combo's not found are: " << std::endl;
	// 	for(const auto i: triComboNotFound)
	// 	{
	// 		std::cout << *i << std::endl;
	// 	}


	// 	for(auto i: triComboNotFound)
	// 	{			
	// 		// Read fade_2D data
	// 		GEOM_FADE2D::Point2* corner0 = i->getCorner(0);
	// 		GEOM_FADE2D::Point2* corner1 = i->getCorner(1);
	// 		GEOM_FADE2D::Point2* corner2 = i->getCorner(2);
	// 		double x0,y0,x1,y1,x2,y2,x3,y3;
	// 		corner0->xy(x0,y0);
	// 		corner1->xy(x1,y1);
	// 		corner2->xy(x2,y2);
	// 		std::vector<utilities::geometry::vertex> mVerticesTri = {{x0,y0,0},{x1,y1,0},{x2,y2,0}};
	// 		utilities::geometry::triangle tri(mVerticesTri);
			
	// 		// Make BSO geometry data
	// 		std::vector<utilities::geometry::vertex> mVertices;
	// 		mVertices.resize(4);
	// 		mVertices[0] = tri.getCenter();
	// 		std::vector<utilities::geometry::line_segment> triLines = tri.getLines();
	// 		utilities::geometry::vertex centreRib1 = triLines[0].getCenter();
	// 		utilities::geometry::vertex centreRib2 = triLines[1].getCenter();
	// 		utilities::geometry::vertex centreRib3 = triLines[2].getCenter();

	// 		mVertices[1] = tri[0];
	// 		mVertices[2] = centreRib1;
	// 		mVertices[3] = centreRib3;
	// 		quadsChoosen.push_back(utilities::geometry::quadrilateral(mVertices));

	// 		mVertices[1] = tri[1];
	// 		mVertices[2] = centreRib1;
	// 		mVertices[3] = centreRib2;
	// 		quadsChoosen.push_back(utilities::geometry::quadrilateral(mVertices));

	// 		mVertices[1] = tri[2];
	// 		mVertices[2] = centreRib2;
	// 		mVertices[3] = centreRib3;
	// 		quadsChoosen.push_back(utilities::geometry::quadrilateral(mVertices));
	// 	}

	// 	quads = quadsChoosen;
	// }

	std::vector<utilities::geometry::line_segment> getOuterBounds2D(std::vector<utilities::geometry::line_segment> lines, double mTol)
	{ // reference: https://stackoverflow.com/questions/31169125/outermost-polygon-from-a-set-of-edges
		// Split inserted lines at all their vertices
		if(lines.size() == 0)
		{
			std::stringstream errorMessage;
				errorMessage << "No lines were inserted in the getOuterBounds2D function. " << std::endl
										<< "(bso/utilities/delaunay.cpp)." << std::endl;
				throw std::invalid_argument(errorMessage.str());
		}
		// split the lines at the end vertices of the lines and remove duplicates
		std::vector<utilities::geometry::vertex> lineV;
		for(const auto i: lines)
		{
			lineV.push_back(i[0]);
			lineV.push_back(i[1]);
		}
		splitLines(lineV, lines, mTol);		
		
		// Declarations
		std::vector<utilities::geometry::line_segment> outerBound;
		std::vector<utilities::geometry::vertex> connectivityVertex; //Note the id vertex of this variable form a pair with the same id from vector connectivityLines
		std::vector<std::vector<utilities::geometry::line_segment>> connectivityLines;
		utilities::geometry::vertex startV = (lines[0])[0];
		std::vector<utilities::geometry::line_segment> connectedLines;
		int c=0;

		for(const auto i: lines)
		{
			c=0;
			while(c < connectivityVertex.size())
			{
				if(connectivityVertex[c].isSameAs(i[0], mTol)) break;
				c++;
			}
			if(c != connectivityVertex.size())
			{
				// adjust vector of vertex
				connectivityLines[c].push_back(i);
			}
			else
			{
				// not found therefor add new vertex
				std::vector<utilities::geometry::line_segment> connectedLines;
				connectedLines.push_back(i);
				connectivityVertex.push_back(i[0]);
				connectivityLines.push_back(connectedLines);
			}

			c = 0;
			while(c < connectivityVertex.size())
			{
				if(connectivityVertex[c].isSameAs(i[1], mTol)) break;
				c++;
			}
			if(c != connectivityVertex.size())
			{
				// adjust vector of vertex
				connectivityLines[c].push_back({i[1],i[0]});
			}
			else
			{
				// not found therefor add new vertex
				std::vector<utilities::geometry::line_segment> connectedLines;
				connectedLines.push_back({i[1],i[0]}); //switch vertices so that the vector is pointing outwards from considered vertex
				connectivityVertex.push_back(i[1]);
				connectivityLines.push_back(connectedLines);
			}

			//find lowest x and y of vertexes
			if(i[0].x()<=startV.x())
			{
				if(i[0].y()<startV.y()) {startV = i[0];}
			}
			if(i[1].x()<=startV.x())
			{
				if(i[1].y()<startV.y()) {startV = i[1];}
			}
		}

		// find starting line segment
		utilities::geometry::line_segment consideredLine;
		bso::utilities::geometry::vertex consideredV;
		double azimuth;
		double smallestAzimuth;

		c = 0;
		while(c < connectivityVertex.size())
		{
			if(connectivityVertex[c].isSameAs(startV, mTol)) break;
			c++;
		}
		if(c != connectivityVertex.size())
		{
			smallestAzimuth = 360- ((connectivityLines[c])[0]).getVector().getHeading().first; // anti clockwise
			consideredLine = ((connectivityLines[c])[0]);
			for(int i=1; i<connectivityLines[c].size(); i++)
			{
				double azimuth =  360 - ((connectivityLines[c])[i]).getVector().getHeading().first; // anti clockwise
				if(azimuth < smallestAzimuth) consideredLine = (connectivityLines[c])[i];
			}
			// Found start outerBound line
			outerBound.push_back(consideredLine);
			// Define the other vertex then the start vertex for searching for next outerBound line:
			if((consideredLine)[0].isSameAs(startV, mTol)) {consideredV = (consideredLine)[1];}
			else {consideredV = (consideredLine)[0];}
		}
		else
		{
			std::stringstream errorMessage;
				errorMessage << "Start vertex for determining the bounds of a set of lines has not been found. " << std::endl
										<< "The considered start vertex is: " << startV << std::endl
										<< "(bso/utilities/delaunay.cpp)." << std::endl;
				throw std::invalid_argument(errorMessage.str());
		}
		bso::utilities::geometry::vertex lastConsideredV;
		while(!consideredV.isSameAs(startV, mTol))
		{
			lastConsideredV = consideredV;
			bso::utilities::geometry::vector flipV = (outerBound.back())[0] - (outerBound.back())[1];
			double currentAzimuth = flipV.getHeading().first;
			
			c = 0;
			while(c < connectivityVertex.size())
			{
				if(connectivityVertex[c].isSameAs(consideredV, mTol)) break;
				c++;
			}
			if(c != connectivityVertex.size())
			{
				std::vector<utilities::geometry::line_segment> linesAtV = connectivityLines[c];
				// if(linesAtV.size() <= 1)
				// {
				// 	std::stringstream errorMessage;
				// 	errorMessage << "LinesAtV only consist of zero or one line. By definition, " << std::endl
				// 							<< "no continuation of the outerBound is possible. " << std::endl
				// 							<< "(bso/utilities/delaunay.cpp)." << std::endl;
				// 	throw std::invalid_argument(errorMessage.str());
				// }
				int count = 0;
				for(const auto i: linesAtV)
				{
					if(outerBound.back().isSameAs(i, mTol)) continue; // find line other then already know outbound
					if(count == 0)
					{
						smallestAzimuth = currentAzimuth - i.getVector().getHeading().first; // anti clockwise
						if(smallestAzimuth < 0) smallestAzimuth = 360 + smallestAzimuth;
						consideredLine = i;
						count++;
						continue;
					}
					double azimuth =  currentAzimuth - i.getVector().getHeading().first; // anti clockwise
					if(azimuth < 0) azimuth = 360 + azimuth;
					if(azimuth < smallestAzimuth) consideredLine = i;
				}
				// Found start outerBound line
				outerBound.push_back(consideredLine);
				
				// Define the other vertex then the considered vertex for searching for next outerBound line:
				consideredV = (consideredLine)[1];
				if(lastConsideredV.isSameAs(consideredV, mTol))
				{
					std::stringstream errorMessage;
					errorMessage << "The considered vertex is not changed, as would be expected. " << std::endl
											<< "The considered start vertex is: " << consideredV << std::endl
											<< "The lastConsideredV is: " << lastConsideredV << std::endl
											<< "The just added consideredLine is: " << consideredLine << std::endl
											<< "The content of outerBound is: " << std::endl;
											for(auto& add: outerBound) errorMessage << add << std::endl;
											errorMessage << "The content of linesAtV is: " << std::endl;
											for(auto& add: linesAtV) errorMessage << add << std::endl;
											errorMessage << "(bso/utilities/delaunay.cpp)." << std::endl;
					throw std::invalid_argument(errorMessage.str());
				}
			}
			else
			{
				std::stringstream errorMessage;
				errorMessage << "The considered vertex has not been found while determining the bounds of a set of lines. " << std::endl
										<< "The considered start vertex is: " << consideredV << std::endl
										<< "(bso/utilities/delaunay.cpp)." << std::endl;
				throw std::invalid_argument(errorMessage.str());
			}
		}

		return outerBound;
	}

	void sortLinesToChain(std::vector<utilities::geometry::line_segment>& lines,  const double mTol /*= 1e-3*/)
	{
		// Sort lines to form a chain of lines (i.e. vertex 1 of line 0 connect to vertex 0 of line 1, etc.) instead of random stored lines.
		// Note; for now this function only works when the lines form a continues line.
		if(lines.size() <= 1)
		{
			return; // since line are already sorted
		}
		bool reverse = false;
		bool done = false;
		int i=0;
		int condition = lines.size();
		int vID = 1;
		while(!done)
		{	
			bool anyConnected = false;
			// find nex lineAtLevel which is connected
			if(!reverse)
			{
				if(i+1 == lines.size())
				{
					anyConnected = true;
					break;
				}
				for(int j=i+1; j < lines.size(); j++)
				{
					if(lines[i][vID].isSameAs(lines[j][1], mTol))
					{
						bso::utilities::geometry::line_segment temp;
						temp = lines[i+1];
						lines[i+1] = {lines[j][1],lines[j][0]};
						if(j != i+1)
						{
							lines[j] = temp;
						}
						anyConnected = true;
						break;
					} 
					else if (lines[i][vID].isSameAs(lines[j][0], mTol))
					{
						if(j != i+1)
						{
							bso::utilities::geometry::line_segment temp;
							temp = lines[i+1];
							lines[i+1] = lines[j];
							lines[j] = temp;
						}
						anyConnected = true;
						break;
					}
				}
				if(!anyConnected)
				{
					if(vID == 1)
					{
						// shift the content of the vector to the back of the vector and reverse iteration order
						int shift = condition-1 - i;
						std::vector<bso::utilities::geometry::line_segment> temp = lines;
						
						int s = shift;
						for(int count = 0; count < lines.size(); count++)
						{
							lines[s]= temp[count];
							s ++;
							if(s == lines.size()) {s = 0;}
						}
						
						vID = 0;
						i = shift+1; // +1 to compensate for the i-- at the end
						reverse = true;
					}
				}
			}
			else
			{
				bool anyConnected = false;
				// find next lineAtLevel which is connected
				if(i-1 < 0)
				{
					anyConnected = true;
					break;
				}
				for(int j=i-1; j >= 0 ; j--)
				{
					if(lines[i][vID].isSameAs(lines[j][1], mTol))
					{
						if(j != i-1)
						{
							bso::utilities::geometry::line_segment temp;
							temp = lines[i-1];
							lines[i-1] = lines[j];
							lines[j] = temp;
						}
						anyConnected = true;
						break;
					} 
					else if (lines[i][vID].isSameAs(lines[j][0], mTol))
					{
						bso::utilities::geometry::line_segment temp;
						temp = lines[i-1];
						lines[i-1] = {lines[j][1],lines[j][0]};
						if(j != i-1)
						{
							lines[j] = temp;
						}
						anyConnected = true;
						break;
					}
				}
				if(!anyConnected)
				{
					if(vID == 0)
					{
						// should start new line in future when surfaces are not continues
						done = true;
					}

				}
			}
			if(reverse){i--;}
			if(i >= lines.size()) {done = true;}
			else if(i < 0) {done = true;}
			if(!reverse){i++;}
		}
	}

	bool hasCourtyard(std::vector<bso::utilities::geometry::quadrilateral> exteriorWalls, std::vector<std::vector<bso::utilities::geometry::line_segment>>& outerboundsExport, std::vector<bso::utilities::geometry::triangle>& triangleExport, const double mTol /*= 1e-3*/)
	{
		bool hasCourtyard = false;
		
		// Find all elevations in the building
		std::vector<std::pair<double,std::vector<bso::utilities::geometry::line_segment>>> linesAtElevation;
		for(const auto& wall: exteriorWalls)
		{
			for(const auto& point: wall)
			{
				bool isAdded = false;
				for(auto k: linesAtElevation)
				{
					if(point.z() == k.first) 
					{
						isAdded = true;
						break;
					}
				}
				if(!isAdded){linesAtElevation.emplace_back(point.z(), std::vector<bso::utilities::geometry::line_segment>{});}
			}
		}

		// Sort linesAtElevation by the double value in the std::pair and remove the pair with the highest double value (i.e. roof of the building)
		std::sort(linesAtElevation.begin(), linesAtElevation.end(), comparisonFirstOfPair); //comparisonFirstOfPair function defined in delaunay.hpp
		if (!linesAtElevation.empty()) {linesAtElevation.pop_back();}
		
		// Get the horizontal lines at each elevation
		for(const auto& wall: exteriorWalls)
		{
			bool firstLine = false;
			bso::utilities::geometry::line_segment line1;
			for(const auto line: wall.getLines())
			{
				if(line.getVector().isHorizontal())
				{
					// Get both horizontal lines, by storing and continuing the for loop. 
					if(firstLine == false)
					{
						firstLine = true;
						line1 = line;
						continue;
					}
		
					// Add lines if at elevation and not already added
					bool line1IsBottomLine = line1[0].z() < line[0].z();
					for(auto& elevation: linesAtElevation)
					{
						bool add = false;
						if(line1IsBottomLine){add = (elevation.first >= line1[0].z()- mTol && elevation.first < line[0].z()- mTol);}
						else{add = (elevation.first >= line[0].z()- mTol && elevation.first < line1[0].z() - mTol);}
						if(add) 
						{ 
							bso::utilities::geometry::line_segment toAddLine = {{line[0].x(),line[0].y(),elevation.first},{line[1].x(),line[1].y(),elevation.first}};
							bool toAddLineAlreadyAdded = false;
							for(const auto& existingLine: elevation.second)
							{
								if(toAddLine.isSameAs(existingLine,mTol))
								{
									toAddLineAlreadyAdded = true;
									break;
								}
							}
							if(!toAddLineAlreadyAdded){elevation.second.push_back(toAddLine);}
						}
					}
				}
			}
		}

		// Identify each outbound in the exterior walls
		std::vector<std::pair<double,std::vector<std::vector<bso::utilities::geometry::line_segment>>>> outerBoundPerElevation;
		for(const auto& elevation: linesAtElevation)
		{
			std::vector<bso::utilities::geometry::line_segment> remainingLines = elevation.second;
			std::vector<std::vector<bso::utilities::geometry::line_segment>> outerBound;
			while(remainingLines.size() > 0)
			{
				if(remainingLines.size() <= 3)
				{
					std::stringstream errorMessage;
					errorMessage << "The are less then three lines in container to get the outer bounds of these lines might form. " << std::endl
											<< "As such the program is terminated, since otherwise a while loop will not end. "<< std::endl
											<< "(bso/utilities/delaunay.cpp)." << std::endl;
					throw std::invalid_argument(errorMessage.str());
				}
				std::vector<bso::utilities::geometry::line_segment> outerBoundLines = getOuterBounds2D(remainingLines, mTol);
				outerBound.push_back(outerBoundLines);
				remainingLines.clear();

				for(const auto& j: elevation.second)
				{
					bool isFoundInAnyOuterBound = false;
					for(const auto& k: outerBound)
					{
						for(const auto& l: k)
						{
							if(l.isSameAs(j,mTol))
							{
								isFoundInAnyOuterBound = true;
								break;
							}
						}
						if(isFoundInAnyOuterBound){break;}
					}
					if(!isFoundInAnyOuterBound){remainingLines.push_back(j);}
				}
			}

			auto pTemp = std::make_pair(elevation.first,outerBound);
			outerBoundPerElevation.push_back(pTemp);
		}

		for(auto& o: outerBoundPerElevation)
		{
			for(auto l: o.second){outerboundsExport.push_back(l);}
		}

		// Identify from the outerBoundPerElevation if any are courtyards
		std::vector<std::pair<double,std::vector<bso::utilities::geometry::polygon_non_convex>>> courtyardsPerElevation;
		for(const auto& elevation: outerBoundPerElevation)
		{
			courtyardsPerElevation.emplace_back(elevation.first, std::vector<bso::utilities::geometry::polygon_non_convex>{});
			if(elevation.second.size() > 1)
			{
				// Redefine the outerbound to a polygon_non_convex, so we can use its geometry checks.
				std::vector<bso::utilities::geometry::polygon_non_convex> polyOutbounds;
				for(const auto outboundLines: elevation.second)
				{
					std::vector<bso::utilities::geometry::vertex> outerboundVertexes;
					for(const auto& line: outboundLines) {outerboundVertexes.push_back(line[0]);}
					if(outerboundVertexes.size()<3)
					{
						std::stringstream errorMessage;
						errorMessage << "The are less then three vertexes in container, " << std::endl
												<< "therefor a polygon_non_convex would not be possible to be defined. " << std::endl
												<< "(bso/utilities/delaunay.cpp)." << std::endl;
						throw std::invalid_argument(errorMessage.str());
					}
					polyOutbounds.push_back(bso::utilities::geometry::polygon_non_convex(outerboundVertexes));
					for(auto& t: polyOutbounds.back().getTriangles())
					{
						triangleExport.push_back(t);
					}
				}
				for(int i=0; i<polyOutbounds.size(); i++) // iterate trough each outerBound at elevation
				{
					for(int j=i+1; j<polyOutbounds.size(); j++) // iterate trough each outerBound at elevation
					{
						// Check if any vertex of outbound j is inside outbound i, if so, then outbound j is a courtyard in outbound i.
						for(const auto vertexj: polyOutbounds[j])
						{
							if(polyOutbounds[i].isInside(vertexj, mTol))
							{
								courtyardsPerElevation.back().second.push_back(polyOutbounds[j]);
								break;
							}
						}
						// Alternatively, check if any vertex of outbound i is inside outbound j, if so, then outbound i is a courtyard in outbound j.
						for(const auto vertexi: polyOutbounds[i])
						{
							if(polyOutbounds[j].isInside(vertexi, mTol))
							{
								courtyardsPerElevation.back().second.push_back(polyOutbounds[i]);
								break;
							}
						}
					}
				}
			}
		}

		for(const auto& i: courtyardsPerElevation)
		{
			if(i.second.size() > 0)
			{
				hasCourtyard = true;
			}
		}

		return hasCourtyard;
	}


} // namespace utilities
} // namespace bso


#endif // GEOMETRY_FUNCTIONS_CPP

