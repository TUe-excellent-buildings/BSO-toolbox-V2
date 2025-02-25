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

#include <stdio.h>
#include <iostream>
#include "../../include_fade25d/Fade_2D.h"
#include "../../include_fade25d/Visualizer3.h"
using namespace std;
using namespace GEOM_FADE25D;

void createCircle(const Point2& center,double radius,vector<Segment2>& vSegmentsOut)
{
	int numPoints(8);
	vector<Point2> vCirclePoints;
	generateCircle(numPoints,center.x(),center.y(),center.z(),radius,radius,vCirclePoints);
	vector<Segment2> vPolygon;
	pointsToPolyline(vCirclePoints,true,vSegmentsOut);
}

void createRandomTriangulation(Fade_2D& dt,vector<Zone2*>& vZones)
{
	// Triangulate random surface points
	vector<Point2> vInputPoints;
	generateRandomSurfacePoints(
			50, // numPointsX
			50, // numPointsY
			1, // numCenters (determines the waviness)
			0,0,-200,1000,1000,200, // Bounds xmin,ymin,zmin,xmax,ymax,zmax
			vInputPoints, // Output vector
			1,  // some positive seed value for repeatable randomness
			false // points on regular grid = false
	);
	dt.insert(vInputPoints);

	// Create two zones within circles
	vector<Segment2> vCircle1,vCircle2;
	createCircle(Point2(300,500,0),200,vCircle1);
	createCircle(Point2(700,500,0),100,vCircle2);
    Zone2* pZone1(dt.createZone_cookieCutter(vCircle1,true));
    Zone2* pZone2(dt.createZone_cookieCutter(vCircle2,true));
	vZones.emplace_back(pZone1);
	vZones.emplace_back(pZone2);

	// Print
	cout<<"* createRandomTriangulation()"<<endl;
	cout<<"  Triangulation: "<<dt.numberOfTriangles()<<" triangles"<<endl;
	cout<<"  Zone1: "<<pZone1->getNumberOfTriangles()<<" triangles"<<endl;
	cout<<"  Zone2: "<<pZone2->getNumberOfTriangles()<<" triangles"<<endl;
}

void importFromPointTriples(std::vector<Zone2*>& vZones)
{
	cout<<"* importFromPointTriples()"<<endl;
	// Let's assume that you have triangles in your own datastructures. You can
	// import them to Fade2D by simply providing a vector of 3 corners per triangle.
	// We setup such a vector 'vYourCorners' now:

	// * 1 *   Setup
	vector<Triangle2*> vYourTriangles;
	for(Zone2* pZone:vZones) pZone->getTriangles(vYourTriangles);
	vector<Point2> vYourCorners;
	for(Triangle2* pT:vYourTriangles)
	{
		for(int i=0;i<3;++i)
		{
			vYourCorners.emplace_back(*pT->getCorner(i));
		}
	}
	cout<<"  Setup, vYourCorners: "<<vYourCorners.size()<<" points (= "<<vYourCorners.size()/3<<" triangles)"<<endl;

	// * 2 *   Import from vYourCorners to Fade_2D
	Fade_2D dt2;
	Zone2* pZone=dt2.importTriangles_robust(vYourCorners);
	cout<<"  imported pZone:\t"<<pZone->getNumberOfTriangles()<<endl;
	cout<<"  imported dt2:\t"<<dt2.numberOfTriangles()<<" triangles (with convex fill triangles)"<<endl;

	// Note: The importTriangles_robust() method corrects wrong orientations
	// and overlapping triangles automatically. This may result in a few
	// extra triangles.
	// In contrast, there is also an importTriangles() method, which does
	// fewer checks and is faster. But due to fewer checks, it can definitely
	// crash if you feed it with corrupt data. Thus, using the newer
	// importTriangles_robust() method is strongly recommended, except if
	// you have huge datasets and you are certain that they are 100% correct.
}


void writePly(Fade_2D& dt,vector<Zone2*>& vZones)
{
	// Write the whole triangulation to a *.ply file
	const char* all_fn("triangulation.ply");
	dt.writePly(all_fn,false); // ...use bASCII=false for binary format

	// Write only one zone to a *.ply file
	const char* zone_fn("zone.ply");
	vZones[0]->writePly(zone_fn,false);

	cout<<"* writePly()"<<endl;
	cout<<"  triangulation: "<<dt.numberOfTriangles()<<" triangles"<<endl;
	cout<<"  zone: "<<vZones[0]->getNumberOfTriangles()<<" triangles"<<endl;
	cout<<"  Have created "<<all_fn<<", "<<zone_fn<<endl<<endl;
}

void readPly()
{
	// Note: importTrianglesFromPly() is robust in the sense that it will correct
	// not only wrong orientations but also selfintersections present in the *.ply
	// file. As a consequence you may get more triangles than contained in the file.
	Fade_2D dt1;
	const char *all_fn("triangulation.ply");
	dt1.importTrianglesFromPly(all_fn);
	cout << "* readPly()" << endl;
	cout << "  Triangulation: " << dt1.numberOfTriangles() << " triangles, read from "<<all_fn<<endl;

	Fade_2D dt2;
	const char *zone_fn("zone.ply");
	Zone2 *pZone2 = dt2.importTrianglesFromPly(zone_fn);
	cout << "  pZone2: " << pZone2->getNumberOfTriangles() << " triangles, read from "<<zone_fn<<endl;
}

void saveNative(Fade_2D& dt,vector<Zone2*>& vSaveZones)
{
	// Save the triangulation along with the zones
	const char* all_fn("triangulation_native.fade");
	cout<<"* saveNative()"<<endl;
	cout<<"  Triangulation: "<<dt.numberOfTriangles()<<endl;
	cout<<"  Zone1: "<<vSaveZones[0]->getNumberOfTriangles()<<endl;
	cout<<"  Zone2: "<<vSaveZones[1]->getNumberOfTriangles()<<endl;
	dt.saveTriangulation(all_fn,vSaveZones);

	// Alternatively we can save only the two zones
	const char* zones_fn("zones_native.fade");
	dt.saveZones(zones_fn,vSaveZones);

	// Or save an individual zone
	const char* zone_fn("one_zone_native.fade");
	vSaveZones[0]->save(zone_fn);

	cout<<"  Have created "<<all_fn<<", "<<zones_fn<<", "<<zone_fn<<endl<<endl;
}

void loadNative()
{
	Fade_2D dt1;
	vector<Zone2*> vZones1;
	// Load the triangulation with the zones
	const char* all_fn("triangulation_native.fade");
	dt1.load(all_fn,vZones1);
	cout<<"* loadNative()"<<endl;
	cout<<"  ...loading from "<<all_fn<<endl;
	cout<<"  Triangulation: "<<dt1.numberOfTriangles()<<endl;
	cout<<"  Zone1: "<<vZones1[0]->getNumberOfTriangles()<<endl;
	cout<<"  Zone2: "<<vZones1[1]->getNumberOfTriangles()<<endl;

	// Alternatively, we could only be interested in loading zones. Thereby,
	// the triangulation may contain additional triangles if required to
	// complete the convex hull of the zones.
	const char* zones_fn("zones_native.fade");
	cout<<"  ...loading from "<<zones_fn<<endl;
	Fade_2D dt2;
	vector<Zone2*> vZones2;
	dt2.load(zones_fn,vZones2);
	cout<<"  Triangulation: "<<dt2.numberOfTriangles()<<" triangles (including fill-triangles to complete the convex hull)"<<endl;
	for(Zone2* pZone:vZones2)
	{
		cout<<"  Loaded zone: "<<pZone->getNumberOfTriangles()<<" triangles"<<endl;
	}
}

void writeObj(Fade_2D& dt,vector<Zone2*>& vZones)
{
	const char* all_fn("triangulation.obj");
	dt.writeObj(all_fn);

	const char* zone_fn("zone.obj");
	vZones[0]->writeObj(zone_fn);

	cout<<"* writeObj()"<<endl;
	cout<<"  Have created "<<all_fn<<", "<<zone_fn<<endl;
}

void writeElements(Fade_2D& dt)
{
	// Fetch the triangles
	vector<Triangle2*> vAllT;
	dt.getTrianglePointers(vAllT);

	// Fetch the vertices
	vector<Point2*> vVertices;
	dt.getVertexPointers(vVertices);

	// Create directed edges
	vector<Edge2> vDirectedEdges;
	dt.getUndirectedEdges(vDirectedEdges);
	vector<Segment2> vSegments;
	for(Edge2& e:vDirectedEdges) vSegments.emplace_back(Segment2(*e.getSrc(),*e.getTrg()));

	cout<<"* writeElements()"<<endl;
	cout<<"  vAllT:\t"<<vAllT.size()<<endl;
	cout<<"  vVertices:\t"<<vVertices.size()<<endl;
	cout<<"  vSegments:\t"<<vSegments.size()<<endl;

	const char* points_fn("points.bin");
	const char* segments_fn("segments.bin");

	// Write the unique points
	writePointsBIN(points_fn,vVertices);

	// Write the segments
	writeSegmentsBIN(segments_fn,vSegments);

	cout<<"  Have created "<<points_fn<<" and "<<segments_fn<<endl;

}

void readElements()
{
	// Reads individual elements that have been written before using writePointsBIN() and writeSegmentsBIN()

	const char* points_fn("points.bin");
	std::vector<Point2> vPoints;
	readPointsBIN(points_fn,vPoints,true); // Use bWithHeader=true for files that have been written with writePointsBIN()
	cout << "* readElements()" << endl;
	cout << "  ...reading from " << points_fn << endl;
	cout << "  vPoints: " << vPoints.size()<<endl;

	const char* segments_fn("segments.bin");
	std::vector<Segment2> vSegments;
	readSegmentsBIN(segments_fn,vSegments);
	cout << "  ...reading from " << segments_fn << endl;
	cout << "  vSegments: " << vSegments.size()<<endl;
	cout<<endl;
}


void demoFadeExport(Fade_2D& dt)
{
	// FadeExport is a simple struct designed primarily for exporting data in memory,
	// facilitating easier integration of data from a Fade_2D object into your own
	// data structures. For details about the FadeExport struct, you may refer to
	// the article available at https://www.geom.at/triangulation-export/.
	// However, for the sake of completeness and self-containedness, FadeExport is
	// briefly utilized below:
    FadeExport fadeExport; // Simple struct, see "FadeExport.h"
    bool bCustomIndices(false); // To retrieve custom indices also
    bool bClear(false); // Clear memory in dt to save memory
    dt.exportTriangulation(fadeExport,bCustomIndices,bClear);

    cout<<"* demoFadeExport()"<<endl;
	cout<<"  numPoints: "<<fadeExport.numPoints<<endl;
	cout<<"  numTriangles: "<<fadeExport.numTriangles<<endl;
	cout<<"  ...see also https://www.geom.at/triangulation-export/"<<endl<<endl;
}


int main(int argc,char** argv)
{
	cout<<"\n-------------------------------------------"<<endl;
	// Create a triangulation and two zones
	Fade_2D dt;
	vector<Zone2*> vZones;
	createRandomTriangulation(dt,vZones);

	cout<<"\n-------------------------------------------"<<endl;
	// Import a triangulation from point triples
	importFromPointTriples(vZones);

	cout<<"\n-------------------------------------------"<<endl;
	// Write and read .PLY
	writePly(dt,vZones);
	readPly();

	// Save and load in native format (recommended, if possible)
	cout<<"\n-------------------------------------------"<<endl;
	saveNative(dt,vZones);
	loadNative();

	// Write a triangulation to .OBJ
	cout<<"\n-------------------------------------------"<<endl;
	writeObj(dt,vZones);

	// Write and read individual elements
	cout<<"\n-------------------------------------------"<<endl;
	writeElements(dt);
	readElements();

	// The FadeExport struct
	cout<<"\n-------------------------------------------"<<endl;
	demoFadeExport(dt);

	cout<<"\n-------------------------------------------"<<endl;
	cout<<"normal program end"<<endl;
	return 0;
}




