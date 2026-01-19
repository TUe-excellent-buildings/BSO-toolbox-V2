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
#include <string>
#include <iomanip>
#include <algorithm>
#include "someTools.h"

using namespace GEOM_FADE2D;
using namespace std;

// A ShapeStruct holds its name, boundary segments and
// optionally hole segments
struct ShapeStruct
{
	explicit ShapeStruct(const std::string& name_):name(name_)
	{}
	// Data
	std::string name; // Shape name
	vector<Segment2> vBoundarySegments; // Shape boundary
	vector<vector<Segment2> > vvHoles; // vectors of segments for n holes
};

// createShape() constructs random shapes. Replace that with your
// own shape data
void createShape(
						ShapeStruct& shape,
						int numHoles
						)
{
	// Create random numbers for the outer boundary
	static int seed(1); // Static seed for the random number generator
	vector<double> vRand;
	generateRandomNumbers(4,10,100,vRand,++seed);
	double xmin(vRand[0]),ymin(vRand[1]),xrange(vRand[2]),yrange(vRand[3]);

	// Construct the random bounding box
	Point2 p0(xmin,ymin);
	Point2 p1(xmin,ymin+yrange);
	Point2 p2(xmin+xrange,ymin);
	Point2 p3(xmin+xrange,ymin+yrange);
	shape.vBoundarySegments.push_back(Segment2(p0,p1));
	shape.vBoundarySegments.push_back(Segment2(p1,p3));
	shape.vBoundarySegments.push_back(Segment2(p3,p2));
	shape.vBoundarySegments.push_back(Segment2(p0,p2));

	// Create $numHoles holes
	for(int k=0;k<numHoles;++k)
	{
		vector<double> vCenterX,vCenterY;
		vector<Point2> vPoints;
		generateRandomNumbers(1,xmin+0.1*xrange,xmin+0.9*xrange,vCenterX,++seed);
		generateRandomNumbers(1,ymin+0.1*yrange,ymin+0.9*yrange,vCenterY,++seed);
		double radiusX((std::min)(vCenterX[0]-xmin,xmin+xrange-vCenterX[0]));
		double radiusY((std::min)(vCenterY[0]-ymin,ymin+yrange-vCenterY[0]));
		generateCircle(15,vCenterX[0],vCenterY[0],0.9*radiusX,0.9*radiusY,vPoints);
		shape.vvHoles.push_back(vector<Segment2>());
		for(size_t i=0;i<vPoints.size();++i)
		{
			shape.vvHoles.back().push_back(Segment2(vPoints[i],vPoints[(i+1)%vPoints.size()]));
		}
	}
}

// Inserts the segments of the shape and returns a zone
Zone2* insertZone(Fade_2D& dt,ShapeStruct& shape)
{
	// Create the boundary zone
	GASSEX(!shape.vBoundarySegments.empty());
	ConstraintGraph2* pBoundaryCG=dt.createConstraint(shape.vBoundarySegments,CIS_CONSTRAINED_DELAUNAY);
	GASSEX(pBoundaryCG!=NULL);
	GASSEX(pBoundaryCG->isPolygon());
	Zone2* pZone(dt.createZone(pBoundaryCG,ZL_INSIDE));
	GASSEX(pZone!=NULL);

	// Create and subtract the holes (if any)
	for(size_t i=0;i<shape.vvHoles.size();++i)
	{
		vector<Segment2>& vHole(shape.vvHoles[i]);
		if(vHole.empty()) continue; // Unusable
		ConstraintGraph2* pHoleCG=dt.createConstraint(vHole,CIS_CONSTRAINED_DELAUNAY);
		GASSEX(pHoleCG->isPolygon());
		Zone2* pHoleZone(dt.createZone(pHoleCG,ZL_INSIDE));
		GASSEX(pHoleZone!=NULL);
		pZone=zoneDifference(pZone,pHoleZone);
		GASSEX(pZone!=NULL);
	}
	return pZone;
}


int ex6_booleanOps2_main()
{
	std::cout<<"* Example6: Boolean operations on polygons with holes"<<std::endl;
	// * 1 *   Create 3 random ShapeStructs: Each has one vector with
	// boundary segments and $NUMHOLES vectors for the holes.
	const int NUMHOLES(2);
	vector<ShapeStruct> vShapes;
	vShapes.push_back(ShapeStruct("Alpha"));
	vShapes.push_back(ShapeStruct("Geta"));
	vShapes.push_back(ShapeStruct("Gamma"));
	for(size_t i=0;i<vShapes.size();++i)
	{
		createShape(vShapes[i],NUMHOLES);
	}

	// * 2 *   Convert the ShapeStructs to Zones and visualize them
	Fade_2D dt;
	vector<Zone2*> vZones;
	for(size_t i=0;i<vShapes.size();++i)
	{
		ShapeStruct& shape(vShapes[i]);
		Zone2* pZone=insertZone(dt,shape);
		GASSEX(pZone!=NULL);
		vZones.push_back(pZone);
		pZone->show(("example6_zone"+shape.name+".pdf").c_str(),false,false);
	}
	dt.show("example6_constrainedDelaunay.pdf");

	// * 3 *   Boolean union operation
	Zone2* pResultZone(vZones[0]);
	for(size_t i=1;i<vZones.size();++i) pResultZone=zoneUnion(pResultZone,vZones[i]);

	// * 4 *   Output
	pResultZone->show("example6_union.pdf",false,true);
	vector<Triangle2*> vTriangles;
	pResultZone->getTriangles(vTriangles);
	cout<<"pResultZone, Number of triangles: "<<vTriangles.size()<<endl;
	cout<<"pResultZone, 2D-area: "<<pResultZone->getArea2D()<<endl;

	return 0;
}
