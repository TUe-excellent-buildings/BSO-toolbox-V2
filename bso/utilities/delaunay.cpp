#ifndef DELAUNAY_CPP
#define DELAUNAY_CPP

#include <bitset>
//#include <Fade_2D.h>
//#include <FadeExport.h>
#include <iomanip> // Allowes to use std::setprecision() to be able to enlarge the decimals printed to the console of a double		
#include <vector>
#include <map>
#include <bso/utilities/geometry.hpp>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Triangulation_face_base_with_info_2.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>

// CGAL type definitions
typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Triangulation_vertex_base_with_info_2<unsigned, K> Vb;
typedef CGAL::Constrained_triangulation_face_base_2<K> Fb;
typedef CGAL::Triangulation_data_structure_2<Vb, Fb> TDS;
typedef CGAL::Constrained_Delaunay_triangulation_2<K, TDS> CDT;
typedef CDT::Point Point;
typedef CDT::Vertex_handle Vertex_handle;

namespace bso { namespace utilities {

	std::vector<utilities::geometry::triangle> extractTriangles(const CDT& cdt, const double mTol)
	{
		std::vector<utilities::geometry::triangle> delaunayTri;

		for (auto fit = cdt.finite_faces_begin(); fit != cdt.finite_faces_end(); ++fit)
		{
			Point p0 = fit -> vertex(0) -> point();
			Point p1 = fit -> vertex(1) -> point();
			Point p2 = fit -> vertex(2) -> point();

			utilities::geometry::vertex C1 = {p0.x(), p0.y(), 0};
			utilities::geometry::vertex C2 = {p1.x(), p1.y(), 0};
			utilities::geometry::vertex C3 = {p2.x(), p2.y(), 0};

			try
			{
				utilities::geometry::triangle tri = {{C1}, {C2}, {C3}};
				delaunayTri.push_back(tri);
			}
			catch(std::exception& e)
			{
				// skip unviable triangles
				continue;
			}
		}
		return delaunayTri;
	}


	std::vector<utilities::geometry::triangle> delaunay(
		const std::vector<utilities::geometry::vertex> delaunayPoints,
		const std::vector<utilities::geometry::line_segment> delaunayLines,
		const double mTol
		)
	{
		// CHECK INPUT
		if (delaunayPoints.size() <= 2)
		{
			std::stringstream errorMessage;
			errorMessage << "\nError, the inserted points in the delaunay triangulation are less than three. \n"
						 << "Meaning that a delaunay triangulation can not be generated. \n"
						 << "(bso/utilities/delaunay)." << std::endl;
			throw std::runtime_error(errorMessage.str());
		}

		// CHECK THE CONSTRAINT LINES
		std::vector<utilities::geometry::vertex> noDelaunayPoint;
		std::vector<utilities::geometry::vertex> pointOnConstraintLine;
		std::vector<utilities::geometry::line_segment> constraintLines;

		for(auto& t : delaunayLines)
		{
			for(auto& k : t)
			{
				bool found = false;
				for(auto& u : delaunayPoints)
				{
					if(u.isSameAs(k, mTol)) found = true;
				}
				if(!found) noDelaunayPoint.push_back(k);
			}

			for(auto& k : delaunayLines)
			{
				if(k.isSameAs(t, mTol)) continue;
				for(auto& s : k)
				{
					if(t.isOnLine(s, mTol))
					{
						pointOnConstraintLine.push_back(s);
						constraintLines.push_back(t);
					}
				}
			}
		}

		if(!pointOnConstraintLine.empty())
		{
			std::stringstream errorMessage;
			int counter = 0;
			errorMessage << "The following vertices are vertices on the constrain line, this is not allowed and should have been solved in the splitLines function. See the following vertices: " << std::endl;
			for(const auto t: pointOnConstraintLine)
			{
				errorMessage << "vertex: " << t << " has been found on: " << constraintLines[counter] << std::endl;
				counter++;
			}
			errorMessage << "(bso/utilities/delaunay)." << std::endl;
			throw std::runtime_error(errorMessage.str());
		}

		if(!noDelaunayPoint.empty())
		{
			std::stringstream errorMessage;
			errorMessage << "The following vertices are end vertices of the constraint lines, but not delaunay points (Checked with tolerance): " << std::endl;
			for(const auto t: noDelaunayPoint) {errorMessage << t;}
			errorMessage << std::endl
						 << "The following are the delaunay points: " << std::endl;
			for(const auto t: delaunayPoints) {errorMessage << t << " ; ";}
			errorMessage << std::endl;
			errorMessage << "(bso/utilities/delaunay)." << std::endl;
			throw std::runtime_error(errorMessage.str());
		}

		// CREATE CGAL Constrained Delaunay Triangulation
		CDT cdt;
		using Vertex_handle = CDT::Vertex_handle;
		std::map<std::pair<double,double>, Vertex_handle> coord_to_vh;

		// Insert all unique points (avoid duplicates)
		for (const auto& v : delaunayPoints) {
			std::pair<double,double> key{v.x(), v.y()};
			if (coord_to_vh.find(key) == coord_to_vh.end()) {
				Point p(v.x(), v.y());
				Vertex_handle vh = cdt.insert(p); // returns existing vh if identical point exists
				coord_to_vh[key] = vh;
			}
		}

		// Insert constraints, detect degenerate endpoints, and validate
		size_t idx = 0;
		for (const auto& line : delaunayLines) {
			const auto &va = line[0];
			const auto &vb = line[1];

			// Skip degenerate / nearly identical endpoints
			if (va.isSameAs(vb, mTol)) {
				std::cerr << "Skipped degenerate (zero-length) constraint at index " << idx
						  << " endpoint: " << va << std::endl;
				++idx;
				continue;
			}

			Point pA(va.x(), va.y());
			Point pB(vb.x(), vb.y());

			try {
				cdt.insert_constraint(pA, pB); // use Point overload
			} catch (const std::exception& e) {
				std::cerr << "Exception inserting constraint idx=" << idx
						  << " endpoints (" << va << ") - (" << vb << "): " << e.what() << std::endl;
				throw; // rethrow or continue depending on desired behavior
			}

			if (!cdt.is_valid()) {
				std::stringstream ss;
				ss << "CDT invalid after inserting constraint index " << idx
				   << " endpoints (" << va << ") - (" << vb << ")";
				throw std::runtime_error(ss.str());
			}
			++idx;
		}

		// Extract triangles
		std::vector<utilities::geometry::triangle> delaunayTri = extractTriangles(cdt, mTol);

		if(delaunayTri.empty())
		{
			std::stringstream errorMessage;
			errorMessage << "\nError, there are no triangles generated during the delaunay triangulation. \n"
						 << "(bso/utilities/delaunay)." << std::endl;
			throw std::runtime_error(errorMessage.str());
		}
		return delaunayTri;
	}
	
	
	// std::vector<utilities::geometry::triangle> delaunay(const std::vector <utilities::geometry::vertex> delaunayPoints, const std::vector <utilities::geometry::line_segment> delaunayLines, const double mTol, bool outputPs)
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

	// 	//Check the constrain lines
	// 	std::vector<utilities::geometry::vertex> noDelauneyPoint;
	// 	std::vector<utilities::geometry::vertex> pointOnConstraintLine;
	// 	std::vector<utilities::geometry::line_segment> contrainLines;
	// 	for(auto t: delaunayLines)
	// 	{
	// 		for(const auto k: t)
	// 		{				
	// 			bool found = false;
	// 			for(const auto u: delaunayPoints) 
	// 			{
	// 				if(u.isSameAs(k)) found = true;
	// 			}
	// 			if(!found)
	// 			{
	// 				noDelauneyPoint.push_back(k);
	// 			}
	// 			if(t.isOnLine(k))
	// 			{
	// 				pointOnConstraintLine.push_back(k);
	// 			}
	// 		}
	// 		for(auto k: delaunayLines)
	// 		{
	// 			if(k.isSameAs(t, mTol)){continue;}
	// 			for(const auto s: k)
	// 			{
	// 				if(t.isOnLine(s, mTol))
	// 				{
	// 					pointOnConstraintLine.push_back(s);
	// 					contrainLines.push_back(t);
	// 				}
	// 			}
	// 		}
	// 	}
	// 	if(noDelauneyPoint.size() > 0)
	// 	{
	// 		std::stringstream errorMessage;
	// 		errorMessage << "The following vertices are end vertices of the constraint lines, but not a delaunay points (Checked with tolerance): " << std::endl;
	// 		for(const auto t: noDelauneyPoint){errorMessage << t;}
	// 		errorMessage << std::endl	
	// 					 << "The following are the delauney points: " << std::endl;
	// 		for(const auto t: delaunayPoints){errorMessage << t << " ; ";}	
	// 		errorMessage << std::endl;
	// 		errorMessage << "(bso/utilities/delaunay)." << std::endl;
	// 		throw std::runtime_error(errorMessage.str());
	// 	}
	// 	if(pointOnConstraintLine.size() > 0)
	// 	{
	// 		std::stringstream errorMessage;
	// 		int counter=0;
	// 		errorMessage << "The following vertices are vertices on the constrain line, this is not allowed and should have been solved in the splitLines function. See the following vertices: " << std::endl;
	// 		for(const auto t: pointOnConstraintLine)
	// 		{
	// 			errorMessage << "vertex: " << t << " has been found on: " << contrainLines[counter] << std::endl;
	// 			counter++;
	// 		}
	// 		errorMessage << "(bso/utilities/delaunay)." << std::endl;
	// 		throw std::runtime_error(errorMessage.str());
	// 	}
		

	// 	// Start Delaunay triangulation process
	// 	// start adding the points/constrainLines to delaunay triangulation algorithm Fade_2D
	// 	GEOM_FADE2D::Fade_2D dt;
		
	// 	for(const auto i: delaunayPoints)
	// 	{
	// 		dt.insert(GEOM_FADE2D::Point2(i.x(),i.y()));
	// 	}
	// 	std::vector<GEOM_FADE2D::Segment2> vSegments;
	// 	for(const auto i: delaunayLines)
	// 	{
	// 		vSegments.push_back(GEOM_FADE2D::Segment2(GEOM_FADE2D::Point2(i[0].x(),i[0].y()),GEOM_FADE2D::Point2(i[1].x(),i[1].y())));
	// 	}
	// 	GEOM_FADE2D::ConstraintGraph2* pCG=dt.createConstraint(vSegments,GEOM_FADE2D::CIS_CONSTRAINED_DELAUNAY);
		
		
	// 	// Makes the constrained delaunay a delaunay respecting the boundaries by adding the intersection point found on the boundary after delaunay
	// 	bool applyConstrainedDelaunay = true;
	// 	if(!applyConstrainedDelaunay)
	// 	{
	// 		double minLen(0.1);
	// 		pCG->makeDelaunay(minLen);
	// 	}

	// 	// Draw; generate a Design.ps file for visualization in for example illustrator 
	// 	if(outputPs) dt.show("Design.ps",true);		

	// 	// Export fade2D data to Toolbox information format
	// 	GEOM_FADE2D::FadeExport fadeExport;
	// 	bool bCustomIndices(false); // To retrieve custom indices also
	// 	bool bClear(false); // Clear memory in $dt
	// 	dt.exportTriangulation(fadeExport,bCustomIndices,bClear);
		
	// 	std::vector<utilities::geometry::triangle> delaunayTri; // fade_2D resulting delaunay triangles, but defined in BSO toolbox geometry classes
	// 	for(int triIdx=0;triIdx<fadeExport.numTriangles;++triIdx)
	// 	{ // Iterate through each triangle in Fade_2D and store triangle in Toolbox entities
	// 		int vtxIdx0,vtxIdx1,vtxIdx2;
	// 		fadeExport.getCornerIndices(triIdx,vtxIdx0,vtxIdx1,vtxIdx2);
			
	// 		double x0,y0;
	// 		double x1,y1;
	// 		double x2,y2;
	// 		fadeExport.getCoordinates(vtxIdx0,x0,y0);
	// 		fadeExport.getCoordinates(vtxIdx1,x1,y1);
	// 		fadeExport.getCoordinates(vtxIdx2,x2,y2);
			
	// 		// Define triangle in Toolbox entities
	// 		utilities::geometry::vertex C1 = {x0,y0,0};
	// 		utilities::geometry::vertex C2 = {x1,y1,0};
	// 		utilities::geometry::vertex C3 = {x2,y2,0};
			
	// 		try
	// 		{
	// 			utilities::geometry::triangle tri = {{C1},{C2},{C3}};
	// 			delaunayTri.push_back(tri);
	// 		}
	// 		catch(std::exception& e)
	// 		{
	// 			// Skip unviable triangles for the BSO toolbox
	// 				// This is needed since FADE_2D calculates with a finer tolerance then the BSO toolbox allows.
	// 				// Therefore, FADE_2D create unlogical triangles from the BSO Toolbox perspective regarding the BSO toolbox tolerances
	// 				// the main reasons to skip a triangle are as follows:
	// 				// - two cornerpoints of a single triangle are the same according to the BSO toolbox tolerances.
	// 				// - the three points of the triangle are on the same line in accordance to the BSO toolbox tolerances.
	// 				// See paper automatic partitioning methods for more information.
	// 			continue;
	// 		}
	// 	}
		
	// 	if(delaunayTri.size() == 0)
	// 	{
	// 		std::stringstream errorMessage;
	// 			errorMessage << "\nError, there are no triangle generated during the delaunay triangulation. \n"
	// 				 << "(bso/utilities/delaunay)." << std::endl;
	// 			throw std::runtime_error(errorMessage.str());
	// 	}
		
	// 	return delaunayTri;
	// } //delaunay()

	
	// std::vector<utilities::geometry::triangle> delaunayZoned(const std::vector <utilities::geometry::vertex> delaunayPoints, const std::vector <utilities::geometry::line_segment> orderedLinesOfZone, const double mTol, bool outputPs)
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
		
	// 	for(const auto i: delaunayPoints)
	// 	{
	// 		dt.insert(GEOM_FADE2D::Point2(i.x(),i.y()));
	// 	}

	// 	// Start adding the zoneLines to delaunay triangulation algorithm Fade_2D
	// 	std::vector<GEOM_FADE2D::Segment2> vSegments;
	// 	for(const auto i: orderedLinesOfZone)
	// 	{
	// 		vSegments.push_back(GEOM_FADE2D::Segment2(GEOM_FADE2D::Point2(i[0].x(),i[0].y()),GEOM_FADE2D::Point2(i[1].x(),i[1].y())));
	// 	}
	// 	GEOM_FADE2D::ConstraintGraph2* pCG=dt.createConstraint(vSegments,GEOM_FADE2D::CIS_CONSTRAINED_DELAUNAY);
		
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

	// 	std::vector<utilities::geometry::triangle> delaunayTri;
	// 	for(const auto i: zTriangles)
	// 	{
	// 		// Get FADE_2D variables:
	// 		GEOM_FADE2D::Point2* corner1 = i->getCorner(0);
	// 		GEOM_FADE2D::Point2* corner2 = i->getCorner(1);
	// 		GEOM_FADE2D::Point2* corner3 = i->getCorner(2);
	// 		double x0,y0,x1,y1,x2,y2;
	// 		corner1->xy(x0,y0);
	// 		corner2->xy(x1,y1);
	// 		corner3->xy(x2,y2);

	// 		// Define BSO toolbox variables:
	// 		utilities::geometry::vertex C1 = {x0,y0,0};
	// 		utilities::geometry::vertex C2 = {x1,y1,0};
	// 		utilities::geometry::vertex C3 = {x2,y2,0};
			
	// 		try
	// 		{
	// 			utilities::geometry::triangle tri = {{C1},{C2},{C3}};
	// 			delaunayTri.push_back(tri);
	// 		}
	// 		catch(std::exception& e)
	// 		{
	// 			// Skip unviable triangles for the BSO toolbox
	// 				// This is needed since FADE_2D calculates with a finer tolerance then the BSO toolbox allows.
	// 				// Therefore, FADE_2D create unlogical triangles from the BSO Toolbox perspective regarding the BSO toolbox tolerances
	// 				// the main reasons to skip a triangle are as follows:
	// 				// - two cornerpoints of a single triangle are the same according to the BSO toolbox tolerances.
	// 				// - the three points of the triangle are on the same line in accordance to the BSO toolbox tolerances.
	// 				// See paper automatic partitioning methods for more information.
	// 			continue;
	// 		}
	// 	}

	// 	if(delaunayTri.size() == 0)
	// 	{
	// 		std::stringstream errorMessage;
	// 			errorMessage << "\nError, there are no triangle generated during the delaunayZoned triangulation. \n"
	// 				 << "(bso/utilities/delaunay)." << std::endl;
	// 			throw std::runtime_error(errorMessage.str());
	// 	}
		
	// 	return delaunayTri;
	// } //delaunayZoned()

	// std::vector<std::vector<utilities::geometry::triangle>> delaunayMultipleZoned(const std::vector <utilities::geometry::vertex> delaunayPoints, 
	// 	const std::vector<std::vector<utilities::geometry::line_segment>> orderedLinesOfZone, const std::vector <utilities::geometry::line_segment> delaunayLines, 
	// 	bool getTriOutsideZones, bool getBothInAndOutZones, const double mTol, bool outputPs)
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
	// 	// start adding the points/ConstrainLines to delaunay triangulation algorithm Fade_2D
	// 	GEOM_FADE2D::Fade_2D dt;
		
	// 	for(const auto i: delaunayPoints)
	// 	{
	// 		dt.insert(GEOM_FADE2D::Point2(i.x(),i.y()));
	// 	}		
	// 	std::vector<GEOM_FADE2D::Segment2> vSegmentsConstLines;
	// 	for(const auto i: delaunayLines)
	// 	{
	// 		vSegmentsConstLines.push_back(GEOM_FADE2D::Segment2(GEOM_FADE2D::Point2(i[0].x(),i[0].y()),GEOM_FADE2D::Point2(i[1].x(),i[1].y())));
	// 	}
	// 	GEOM_FADE2D::ConstraintGraph2* pCGConstLines=dt.createConstraint(vSegmentsConstLines,GEOM_FADE2D::CIS_CONSTRAINED_DELAUNAY);

	// 	// Start adding the zoneLines to delaunay triangulation algorithm Fade_2D
	// 	std::vector<GEOM_FADE2D::ConstraintGraph2*> pCG;
	// 	for(const auto j: orderedLinesOfZone)
	// 	{
	// 		std::vector<GEOM_FADE2D::Segment2> vSegments;
	// 		for(const auto i: j)
	// 		{
	// 			vSegments.push_back(GEOM_FADE2D::Segment2(GEOM_FADE2D::Point2(i[0].x(),i[0].y()),GEOM_FADE2D::Point2(i[1].x(),i[1].y())));
	// 		}
	// 		GEOM_FADE2D::ConstraintGraph2* temp=dt.createConstraint(vSegments,GEOM_FADE2D::CIS_CONSTRAINED_DELAUNAY);
	// 		pCG.push_back(temp);
	// 	}
		
	// 	// create zones from the zoneLines
	// 	std::vector<GEOM_FADE2D::Zone2*> zonePtr;
	// 	for(const auto i: pCG)
	// 	{
	// 		GEOM_FADE2D::Zone2* pZone0(dt.createZone(i,GEOM_FADE2D::ZL_INSIDE));
	// 		zonePtr.push_back(pZone0);
	// 	}

	// 	// Makes the constrained delaunay a delaunay respecting the boundaries by adding the intersection point found on the boundary after delaunay
	// 	bool applyConstrainedDelaunay = true;
	// 	if(!applyConstrainedDelaunay)
	// 	{
	// 		double minLen(0.1);
	// 		for(const auto i: pCG)
	// 		{
	// 			i->makeDelaunay(minLen);
	// 		}
	// 		pCGConstLines->makeDelaunay(minLen);
	// 	}

	// 	// Draw; generate a Design.ps file for visualization in for example illustrator 
	// 	if(outputPs) 
	// 	{
	// 		dt.show("Design.ps",true);		
	// 	}

		
	// 	std::vector<std::vector<utilities::geometry::triangle>> allDelaunayTri;
	// 	if(!getTriOutsideZones || getBothInAndOutZones)
	// 	{
	// 		std::vector<std::vector<GEOM_FADE2D::Triangle2*>> zTrianglesInZones;
	// 		for(int i=0; i < zonePtr.size(); i++)
	// 		{
	// 			zonePtr[i]->getTriangles(zTrianglesInZones[i]);
	// 		}
			
	// 		for(const auto j: zTrianglesInZones)
	// 		{
	// 			std::vector<utilities::geometry::triangle> delaunayTri;
	// 			for(const auto i: j)
	// 			{
	// 				// Get FADE_2D variables:
	// 				GEOM_FADE2D::Point2* corner1 = i->getCorner(0);
	// 				GEOM_FADE2D::Point2* corner2 = i->getCorner(1);
	// 				GEOM_FADE2D::Point2* corner3 = i->getCorner(2);
	// 				double x0,y0,x1,y1,x2,y2;
	// 				corner1->xy(x0,y0);
	// 				corner2->xy(x1,y1);
	// 				corner3->xy(x2,y2);

	// 				// Define BSO toolbox variables:
	// 				utilities::geometry::vertex C1 = {x0,y0,0};
	// 				utilities::geometry::vertex C2 = {x1,y1,0};
	// 				utilities::geometry::vertex C3 = {x2,y2,0};
					
	// 				try
	// 				{
	// 					utilities::geometry::triangle tri = {{C1},{C2},{C3}};
	// 					delaunayTri.push_back(tri);
	// 				}
	// 				catch(std::exception& e)
	// 				{
	// 					// Skip unviable triangles for the BSO toolbox
	// 						// This is needed since FADE_2D calculates with a finer tolerance then the BSO toolbox allows.
	// 						// Therefore, FADE_2D create unlogical triangles from the BSO Toolbox perspective regarding the BSO toolbox tolerances
	// 						// the main reasons to skip a triangle are as follows:
	// 						// - two cornerpoints of a single triangle are the same according to the BSO toolbox tolerances.
	// 						// - the three points of the triangle are on the same line in accordance to the BSO toolbox tolerances.
	// 						// See paper automatic partitioning methods for more information.
	// 					continue;
	// 				}
	// 			}
	// 			allDelaunayTri.push_back(delaunayTri);
	// 		}
	// 	}
	// 	else if(getBothInAndOutZones || getTriOutsideZones)
	// 	{
	// 		std::vector<GEOM_FADE2D::Triangle2*> zTrianglesOutZones;
	// 		GEOM_FADE2D::Zone2* pZoneGlobalDif(dt.createZone(NULL, GEOM_FADE2D::ZL_GLOBAL));
	// 		for(const auto i: zonePtr)
	// 		{ // Subtract the zonePtr zones from the global zones
	// 			GEOM_FADE2D::Zone2* pZoneDifference(zoneDifference(pZoneGlobalDif,i));
	// 			pZoneGlobalDif = pZoneDifference;
	// 		}
	// 		pZoneGlobalDif->getTriangles(zTrianglesOutZones); //get triangles
			
	// 		std::vector<utilities::geometry::triangle> delaunayTri;
	// 		for(const auto i: zTrianglesOutZones)
	// 		{
	// 			// Get FADE_2D variables:
	// 			GEOM_FADE2D::Point2* corner1 = i->getCorner(0);
	// 			GEOM_FADE2D::Point2* corner2 = i->getCorner(1);
	// 			GEOM_FADE2D::Point2* corner3 = i->getCorner(2);
	// 			double x0,y0,x1,y1,x2,y2;
	// 			corner1->xy(x0,y0);
	// 			corner2->xy(x1,y1);
	// 			corner3->xy(x2,y2);

	// 			// Define BSO toolbox variables:
	// 			utilities::geometry::vertex C1 = {x0,y0,0};
	// 			utilities::geometry::vertex C2 = {x1,y1,0};
	// 			utilities::geometry::vertex C3 = {x2,y2,0};
				
	// 			try
	// 			{
	// 				utilities::geometry::triangle tri = {{C1},{C2},{C3}};
	// 				delaunayTri.push_back(tri);
	// 			}
	// 			catch(std::exception& e)
	// 			{
	// 				// Skip unviable triangles for the BSO toolbox
	// 					// This is needed since FADE_2D calculates with a finer tolerance then the BSO toolbox allows.
	// 					// Therefore, FADE_2D create unlogical triangles from the BSO Toolbox perspective regarding the BSO toolbox tolerances
	// 					// the main reasons to skip a triangle are as follows:
	// 					// - two cornerpoints of a single triangle are the same according to the BSO toolbox tolerances.
	// 					// - the three points of the triangle are on the same line in accordance to the BSO toolbox tolerances.
	// 					// See paper automatic partitioning methods for more information.
	// 				continue;
	// 			}
	// 		}
	// 		allDelaunayTri.push_back(delaunayTri);

	// 		// Draw; generate a Design.ps file for visualization in for example illustrator 
	// 		if(outputPs) 
	// 		{
	// 			pZoneGlobalDif->show("Design_zone_outside.ps", true, true);		
	// 		}
	// 	}

	// 	bool noTri = false;
	// 	for(const auto k: allDelaunayTri)
	// 	{
	// 		if(k.size() == 0 ){noTri = true;}
	// 	}

	// 	if(allDelaunayTri.size() == 0 || noTri)
	// 	{
	// 		std::stringstream errorMessage;
	// 			errorMessage << "\nError, there are no triangle generated during the delaunayZoned triangulation in either a zone or in general. \n"
	// 				 << "(bso/utilities/delaunay)." << std::endl;
	// 			throw std::runtime_error(errorMessage.str());
	// 	}

	// 	return allDelaunayTri;
	// } //delaunayMultipleZoned()
	
	
	void splitLines(const std::vector<utilities::geometry::vertex>& intsecV, std::vector<utilities::geometry::line_segment>& intsecL, bool removeDuplicates /*= true*/, const double mTol)
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
		
		if(removeDuplicates)
		{
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
					if (m == intsecL[k]){continue;} // No tolerance needed. Just operator to make sure the same entities are not checked with each other.
					if (m.isSameAs(intsecL[k], mTol)){intsecL.erase(intsecL.begin() + k);}
				}
			}
		}	
	}//splitLines


} // namespace utilities
} // namespace bso


#endif // DELAUNAY_CPP

