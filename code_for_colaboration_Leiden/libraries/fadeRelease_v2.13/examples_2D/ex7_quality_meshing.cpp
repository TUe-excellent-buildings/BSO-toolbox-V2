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

using namespace std;
using namespace GEOM_FADE2D;

void initialZone();
void strategy_default();
void strategy_growFactor();
void strategy_maxLength();
void strategy_customParameters();
void strategy_gridMeshing();
Zone2* createSimpleZone(Fade_2D& dt);



int ex7_qualityMeshing_main()
{
	std::cout<<"* Example7: Quality Meshing"<<std::endl;
	initialZone();
	strategy_default();
	strategy_growFactor();
	strategy_maxLength();
	strategy_gridMeshing();
	strategy_customParameters();
	return 0;
}

// Shows a simple zone
void initialZone()
{
	Fade_2D dt;
	Zone2* pZone=createSimpleZone(dt);
	pZone->show("example7_initialZone.pdf",true,true); // show all triangles=true, show constraints=true
}

// Uses MeshGenParams with all default parameters
void strategy_default()
{
	Fade_2D dt;
	Zone2* pZone=createSimpleZone(dt);
	MeshGenParams params(pZone);
	dt.refineAdvanced(&params);
	pZone->show("example7_default.pdf",true,true);
}

// Uses MeshGenParams and restricts the growFactor parameter
void strategy_growFactor()
{
	Fade_2D dt;
	Zone2* pZone=createSimpleZone(dt);

	MeshGenParams params(pZone);
	params.growFactor=5.0;
	dt.refineAdvanced(&params);
	pZone->show("example7_growFactor.pdf",true,true);
}

// Uses MeshGenParams and restricts the maximum edge length
void strategy_maxLength()
{
	Fade_2D dt;
	Zone2* pZone=createSimpleZone(dt);
	MeshGenParams params(pZone);
	params.maxEdgeLength=10.0;
	dt.refineAdvanced(&params);
	pZone->show("example7_maxLength.pdf",true,true);
}

void strategy_gridMeshing()
{
	Fade_2D dt;
	Zone2* pZone=createSimpleZone(dt);
	MeshGenParams params(pZone);
	params.gridLength=3.0;
	params.gridVector=Vector2(1.0,0.0);
//	params.gridVector=Vector2(1.0,0.3);

	dt.refineAdvanced(&params);
	pZone->unifyGrid(0.01); // Consistent diagonals (optional)
	pZone->show("example7_gridMeshing.pdf",true,true);
}


class CustomParameters:public MeshGenParams
{
public:
	explicit CustomParameters(Zone2* pZone):MeshGenParams(pZone)
	{
	}
	double getMaxTriangleArea(Triangle2* pT)
	{
		Point2 barycenter(pT->getBarycenter());
		if(barycenter.x()<20 && barycenter.y()<40)
		{
			// Dense meshing in the lower left corner
			return 1.0;
		}
		else
		{
			// No density restriction otherwise
			return 10.0;
		}
	}
};

// Uses CustomParameters with a custom getMaxTriangleArea method
void strategy_customParameters()
{
	Fade_2D dt;
	Zone2* pZone=createSimpleZone(dt);

	CustomParameters params(pZone);
	dt.refineAdvanced(&params);
	pZone->show("example7_customParameters.pdf",true,true);
}




Zone2* createSimpleZone(Fade_2D& dt)
{
	// Create a shape
	std::vector<Point2> vConstraintPoints;
	vConstraintPoints.push_back(Point2(30,0));
	vConstraintPoints.push_back(Point2(80,0));
	vConstraintPoints.push_back(Point2(55,20));
	vConstraintPoints.push_back(Point2(100,20));
	vConstraintPoints.push_back(Point2(100,100));
	vConstraintPoints.push_back(Point2(54.5,22));
	vConstraintPoints.push_back(Point2(0,100));
	vConstraintPoints.push_back(Point2(0,20));
	vConstraintPoints.push_back(Point2(54,20));

	std::vector<Segment2> vSegments;
	for(size_t i=0;i<vConstraintPoints.size();++i)
	{
		Point2& p0(vConstraintPoints[i]);
		Point2& p1(vConstraintPoints[(i+1)%vConstraintPoints.size()]);
		vSegments.push_back(Segment2(p0,p1));
	}

	ConstraintGraph2* pCG(NULL);
	pCG=dt.createConstraint(vSegments,CIS_CONSTRAINED_DELAUNAY);
	dt.applyConstraintsAndZones();
	Zone2* pZone(dt.createZone(pCG,ZL_INSIDE));
	return pZone;

}






