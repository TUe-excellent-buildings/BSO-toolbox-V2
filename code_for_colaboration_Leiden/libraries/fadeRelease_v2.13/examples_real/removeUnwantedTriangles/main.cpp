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
#include <Fade_2D.h>
#include <Visualizer3.h>

using namespace std;
using namespace GEOM_FADE25D;

// This function returns the edge length that is greater than n percent
// of the edge lengths of dt.
double getLengthPercentile(const Fade_2D& dt,double n)
{
	// Fetch the edges from dt
	vector<Edge2> vEdges;
	dt.getUndirectedEdges(vEdges);
	if(vEdges.empty()) return 0;

	// Construct a sorted vector of edge lengths
	vector<double> vLengths;
	for(const Edge2& e:vEdges) vLengths.emplace_back(e.getLength25D());
	sort(vLengths.begin(),vLengths.end());

	// Return the length at position n*size
	return sqrt(vLengths[size_t(n*vLengths.size())]);
}

// A customized peel predicate based on PeelPredicateTS that allows
// users to define their own criteria for removing or keeping border
// triangles.
class PeelDeciderAggressive:public PeelPredicateTS
{
public:
	PeelDeciderAggressive(double maxEdgeLen_):
		sqMaxEdgeLen(maxEdgeLen_*maxEdgeLen_)
	{}

	// This function is called from peelOffIf() with a border triangle
	// pT. It will remove pT if this function returns true.
	bool operator()(const Triangle2* pT,std::set<Triangle2*>* pCurrentSet)
	{
		// * 1 *   If one edge of pT is larger than the maximum edge
		//         length then it must be removed - return true!
		for(int i=0;i<3;++i)
		{
			double sqLen(pT->getSquaredEdgeLength25D(i));
			if(sqLen>sqMaxEdgeLen) return true; // Too large!
		}

		// * 2 *   An almost vertical border triangle must be removed.
		Vector2 nv(pT->getNormalVector()); // Normal vector
		Vector2 up(0,0,1); // Vertical vector
		double cosPhi(nv*up); // cos(angle)
		double angle;
		if(cosPhi>=-1.0 && cosPhi<=1.0)
		{
			angle=acos(cosPhi)*57.2958; // Normal case, compute the angle
		}
		else
		{
			// Robustness in case of numeric inaccuracy
			if(cosPhi>1.0) angle=0;
				else angle=180.0; // cosPhi<-1.0
		}
		if(angle>85.0)
		{
			// Triangle almost vertical - return true to remove it
			return true;
		}

		// * 3 *   Remove pT if a vertex opposite to a border edge
		//         has a very large interior angle.
		for (int i = 0; i < 3; ++i)
        {
			// Is the opposite edge of i a border edge of the zone?
			Triangle2* pNeigT(pT->getOppositeTriangle(i));
			if(	pNeigT==NULL ||
				pCurrentSet->find(pNeigT)==pCurrentSet->end() )
			{
				// Yes, border. Check the interior angle:
				if (pT->getInteriorAngle25D(i) > 140)
				{
					return true;
				}
			}
        }
        return false;
	}
	double sqMaxEdgeLen;
};

// Custom Predicate derived from PeelPredicateTS
class PeelDeciderCareful:public PeelPredicateTS
{
public:
	PeelDeciderCareful(double maxEdgeLen_):sqMaxEdgeLen(maxEdgeLen_*maxEdgeLen_)
	{
	}
	bool operator()(const Triangle2* pT,std::set<Triangle2*>* )
	{
		// Edge length
		for(int i=0;i<3;++i)
		{
			double sqLen(pT->getSquaredEdgeLength25D(i));
			if(sqLen>sqMaxEdgeLen) return true; // Too large!
		}
		return false;
	}
	double sqMaxEdgeLen;
};



int main()
{
	// * Step 1 *   Load the point cloud ((((((ATTENTION, HEADER))))))
	vector<Point2> vOrgPoints;
	bool bOK=readPointsBIN("point_cloud.bin",vOrgPoints,true);
	if (!bOK) return 1;

	size_t orgSize(vOrgPoints.size());
	cout<<"Original number of points: "<<orgSize<<endl;

	{
		// Visualization
		Visualizer3 vis("points.list");
		vis.writePoints(vOrgPoints,2,Visualizer3::CBLACK);
	}

	// * Step 2 *   Reduce the cloud size
	CloudPrepare cloudPrep;
	cloudPrep.add(vOrgPoints);
	vOrgPoints.clear(); // In order to save memory!
	SumStrategy sumStrategy(SMS_MINIMUM); // SMS_MINIMUM serves ground filtering
	ConvexHullStrategy chStrategy(CHS_MINHULL); // CHS_NOHULL or CHS_MAXHULL or CHS_MINHULL
	int approxNumPoints(int(0.3*orgSize)); // The desired (approx.) number of output points
	cout << "approxNumPoints=" << approxNumPoints << endl;
	cloudPrep.uniformSimplifyNum(approxNumPoints,sumStrategy,chStrategy);
	cout << "reduced..." << endl;
	cout<<"Reduced number of points: "<<cloudPrep.getNumPoints()<<endl;

	{
		// Visualization
		Visualizer3 vis("points_reduced.list");
		vector<Point2> vReducedPoints;
		cloudPrep.getPoints(vReducedPoints);
		vis.writePoints(vReducedPoints,2,Visualizer3::CBLACK);
	}


	// * Step 3 *   Triangulate the reduced point cloud
	Fade_2D dt;
	dt.insert(&cloudPrep,true);

	{
		// Visualization
		dt.showGeomview("unfiltered_triangles.list",Visualizer3::CORANGE);
	}


	// * Step 4 *   Filtering Stage 1: An aggressive peel decider applied by
	//              peelOffIf() decides to remove triangles that satisfy one
	//              of the following conditions:
	//              1) Their edge length is greater than 3 times the median edge length.
	//              2) They have a large interior angle opposite a border edge.
	//              3) They are almost vertical.
	//              Connected components are not split during this stage.
	double medianLen(getLengthPercentile(dt,0.5));
	Zone2* pGlobalZone(dt.createZone(NULL,ZL_GLOBAL));
	PeelDeciderAggressive aggressiveDecider(3*medianLen);
	Zone2* pFiltered1=peelOffIf(pGlobalZone,true,&aggressiveDecider); // bAvoidSplit=true
	if(pFiltered1==NULL) // Unexpected: No triangles left - return 1
	{
		cout<<"pFiltered1=NULL, peelOffIf has removed everything)"<<endl;
		return 1;
	}

	{
		// Visualization
		pFiltered1->showGeomview("filtered1.list",Visualizer3::CORANGE);
	}

	// * 5 *   Filtering Stage 2: A careful peel decider applied by
	//         peelOffIf decides to remove only triangles with edges
	//         greater than 10 times the median edge length.
	//         This time peelOffIf() may split connected components
	PeelDeciderCareful carefulDecider(10*medianLen);
	Zone2* pFiltered2=peelOffIf(pFiltered1,false,&carefulDecider); // bAvoidSplit=false
	if(pFiltered2==NULL) // Unexpected: No triangles left - return 1
	{
		cout<<"pFiltered2=NULL, peelOffIf has removed everything)"<<endl;
		return 1;
	}

	{
		// Visualization
		pFiltered2->showGeomview("filtered2.list",Visualizer3::CORANGE);
	}

	// * 6 *   Form connected components from the remaining triangles
	vector<Triangle2*> vT2;
	pFiltered2->getTriangles(vT2);
	vector<vector<Triangle2*> > vvT;
	getConnectedComponents(vT2,vvT);
	cout<<"number of connected components="<<vvT.size()<<endl;

	{
		// Visualization
		Visualizer3 vcomp("components.list");
		for(vector<Triangle2*>& vT:vvT)
		{
			vcomp.writeTriangles(vT,Visualizer3::getNextColor(),false);
		}
	}

	// * 7 *   Keep only connected components with more than x triangles
	vector<Triangle2*> vFinalT;
	int numComponentsKept(0);
	const double MINIMUM_NUM_T(50);
	for(vector<Triangle2*>& vT:vvT)
	{
		if(vT.size()>MINIMUM_NUM_T)
		{
			++numComponentsKept;
			copy(vT.begin(),vT.end(),back_inserter(vFinalT));
		}
	}
	cout<<"number of kept components="<<numComponentsKept<<endl;
	Zone2* pFinalZone(dt.createZone(vFinalT));

	{
		// Visualization
		pFinalZone->showGeomview("finalResult.list",Visualizer3::CORANGE);
	}

	cout<<"Writing final.obj"<<endl;
	pFinalZone->writeObj("final.obj");



	{
		// Visualization goodT/badT
		vector<Triangle2*> vAllT,vGoodT,vBadT;
		dt.getTrianglePointers(vAllT);
		pFinalZone->getTriangles(vGoodT);
		sort(vAllT.begin(),vAllT.end());
		sort(vGoodT.begin(),vGoodT.end());
		set_difference(vAllT.begin(),vAllT.end(),vGoodT.begin(),vGoodT.end(),back_inserter(vBadT));

		Visualizer3 v("bad_triangles.list");
		v.writeTriangles(vGoodT,Visualizer3::CGREEN,false);
		v.writeTriangles(vBadT,Visualizer3::CRED,false);

	}

	return 0;
}



