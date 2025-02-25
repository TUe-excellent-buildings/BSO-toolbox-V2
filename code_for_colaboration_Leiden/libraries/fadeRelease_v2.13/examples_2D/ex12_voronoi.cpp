// (c) 2010 Geom e.U. Bernhard Kornberger, Graz/Austria.
//
// This file is part of the Fade2D library. You can use it for your
// personal non-commercial research. Licensees holding a commercial
// license may use or adapt this example in accordance with the
// Commercial License Agreement provided with the Software.
//
// This software is provided AS IS with NO WARRANTY OF ANY KIND,
// INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE.
//
// Contact: https://www.geom.at/contact/
//
// Fade2D-Examples       https://www.geom.at/category/fade2d-examples/
// Fade2D-Documentation: https://www.geom.at/fade2d/html/
//
// Fade2.5D-Examples:      https://www.geom.at/category/fade25d-examples/
// Fade2.5D-Documentation: https://www.geom.at/fade25d/html/
//
// Examples from practice: https://www.geom.at/category/practical-delaunay-meshing-cpp/
//
//
// If you are using Fade free of charge, we kindly request you to include a link
// to "https://www.geom.at/fade25d/html/" on your website.

#include <Fade_2D.h>
#include <stdlib.h>
#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <map>
#include "someTools.h"

using namespace GEOM_FADE2D;


template <typename T> inline void unusedValue(const T&){}

int ex12_voronoi_main()
{
	std::cout<<"* Example12: Voronoi - Point location in a Voronoi diagram"<<std::endl;

	// * Step 1 *   Create input points (on circles and on a grid). Each
	//              of these plays two roles: It is a Delaunay-Vertex
	//              AND a site of one Voronoi cell.
	std::vector<Point2> vPoints;
	generateCircle( 20,25,75,20,20,vPoints);
	generateCircle( 10,25,75,10,10,vPoints);
	generateCircle( 20,75,35,10,30,vPoints);
	for(double x=0;x<41.0;x+=10)
	{
		for(double y=0.0;y<41;y+=10)
		{
			vPoints.push_back(Point2(x+.5,y+.5));
		}
	}


	// * Step 2 *   Draw the Voronoi diagram and Delaunay triangulation
	//              Infinite Voronoi cells are automatically clipped.
	Fade_2D dt;
	dt.insert(vPoints);
	Voronoi2* pVoro(dt.getVoronoiDiagram());
	pVoro->show("voronoi_and_delaunay.pdf",true,false,true,true,false); // Options: Voro=true,Fill-Colors=false,Sites=true,Delaunay=true,Labels=false
	pVoro->show("voronoi_and_sites.pdf",true,true,true,false,false);  // Options: Voro=true,Fill-Colors=true,Sites=true,Delaunay=false,Labels=false


	// * Step 3 *   Assign arbitrary indices to the Voronoi cells. This
	//              is completely optional. You can use this feature
	//              for visualization OR to associate Voronoi cells with
	//              your own data structures.
	std::vector<VoroCell2*> vVoronoiCells;
	pVoro->getVoronoiCells(vVoronoiCells);
	for(size_t i=0;i<vVoronoiCells.size();++i)
	{
		VoroCell2* pVoroCell(vVoronoiCells[i]);
		pVoroCell->setCustomCellIndex(int(i));
	}

	// * Step 4 *   Locate the cell of an arbitrary query point.
	Point2 queryPoint(67,95);
	VoroCell2* pVoroCell(pVoro->locateVoronoiCell(queryPoint));

	Visualizer2 v("located_cell.pdf");
	Bbox2 bbx(dt.computeBoundingBox());
	bbx.enlargeRanges(1.5,true,1.0);
	v.setLimit(bbx); // Manually set a limit to clip the viewport.
	pVoro->show(&v,true,false,true,false,false); // Options: Voro=true,Fill-Colors=false,Sites=true,Delaunay=false,Labels=false
	v.addObject(Label(queryPoint," queryPoint"),Color(CBLUE));
	if(pVoroCell==NULL)
	{
		// Error - Voronoi diagram has no points or ALL points are collinear
		std::cout<<"Voronoi diagram invalid"<<std::endl;
		v.writeFile();
		return 1;
	}
	else
	{
		int cellIdx(pVoroCell->getCustomCellIndex());
		std::cout<<"Found the voronoi cell no. "<<cellIdx<<std::endl;
		// Color the located cell red
		v.addObject(pVoroCell,Color(CPALEGREEN,1.0,true));
		// Write a label that shows its custom cell-index
		Label l(*pVoroCell->getSite(),
				(" Voronoi\n Cell "+toString(cellIdx)).c_str(),false);
		v.addObject(l,Color(CBLACK));
		Point2 centroid;
		double area=pVoroCell->getCentroid(centroid);
		if(area<0)
		{
			std::cout<<"Cell "<<cellIdx<<" is infinite: No area, no centroid"<<std::endl;
		}
		else
		{
			std::cout<<"Cell "<<cellIdx<<": area="<<area<<", centroid="<<centroid<<std::endl;
			v.addObject(Label(centroid,"Centroid"),Color(CPURPLE));
		}

		v.writeFile();
	}


	// * Step 5 *   Benchmark mass point location (typ. 0.2 s for 1 mio queries)
	std::vector<Point2> vRandomPoints;
	generateRandomPoints(1000,0,100,vRandomPoints,1);
	Fade_2D dt2;
	dt2.insert(vRandomPoints);
	Voronoi2* pVoro2(dt2.getVoronoiDiagram());
	pVoro2->show("mass_location.pdf",true,true,false,false,false);  // Options: Voro=true,Fill-Colors=true,Sites=false,Delaunay=false,Labels=false

	timer("massLocate");
	for(int i=0;i<1000000;++i)
	{
		double x(100.0 * double(rand())/RAND_MAX);
		double y(100.0 * double(rand())/RAND_MAX);
		Point2 q(x,y);
		VoroCell2* pVoronoiCell(pVoro2->locateVoronoiCell(q));
		// ...we do nothing with pVoronoiCell...
		unusedValue(pVoronoiCell); // Avoids an unused-warning
	}
	timer("massLocate");


	return 0;
}
