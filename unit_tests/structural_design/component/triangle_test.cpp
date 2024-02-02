#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE "sd_triangle_component"
#endif

#include <boost/test/included/unit_test.hpp>

#include <bso/structural_design/component/triangle.hpp>

#include <sstream>

/*
BOOST_TEST()
BOOST_REQUIRE_THROW(function, std::domain_error)
BOOST_REQUIRE(!s[8].dominates(s[9]) && !s[9].dominates(s[8]))
BOOST_CHECK_EQUAL_COLLECTIONS(a.begin(), a.end(), b.begin(), b.end());
*/

namespace component_test {
using namespace bso::structural_design::component;

BOOST_AUTO_TEST_SUITE( sd_triangle_component )
	
	BOOST_AUTO_TEST_CASE( initialize_from_container )
	{
		point p1(1,{0,0,0});
		point p2(1,{4000,0,0});
		point p3(1,{2000,4000,0});
		std::vector<point> pl1 = {p1, p2, p3};
		triangle t1(pl1);
		
		BOOST_REQUIRE(p1.isSameAs(t1[0]));
		BOOST_REQUIRE(p2.isSameAs(t1[1]));
		BOOST_REQUIRE(p3.isSameAs(t1[2]));
	}

	BOOST_AUTO_TEST_CASE( initialize_from_initializer_list )
	{
		point p1(1,{0,0,0});
		point p2(1,{4000,0,0});
		point p3(1,{2000,4000,0});
		triangle t1({p1, p2, p3});
		
		BOOST_REQUIRE(p1.isSameAs(t1[0]));
		BOOST_REQUIRE(p2.isSameAs(t1[1]));
		BOOST_REQUIRE(p3.isSameAs(t1[2]));
	}
	
	BOOST_AUTO_TEST_CASE( flat_shell )
	{
		point p1(1,{0,0,0});
		point p2(1,{4000,0,0});
		point p3(1,{2000,4000,0});
		triangle t1({p1, p2, p3});
		
		structure st1("flat_shell",{{"thickness",100},{"poisson",0.3},{"E",1e5}});
		t1.addStructure(st1);
		
		BOOST_REQUIRE(!t1.hasTruss());
		BOOST_REQUIRE(!t1.hasBeam());
		BOOST_REQUIRE( t1.hasFlatShell());
		BOOST_REQUIRE(!t1.hasQuadHexahedron());
		
		BOOST_REQUIRE(t1.getStructures()[0].thickness() == 100);
		BOOST_REQUIRE(t1.getStructures()[0].poisson() == 0.3);
		BOOST_REQUIRE(t1.getStructures()[0].E() == 1e5);
	}
	
	BOOST_AUTO_TEST_CASE( invalid_structure )
	{
		point p1(1,{0,0,0});
		point p2(1,{4000,0,0});
		point p3(1,{2000,4000,0});
		triangle t1({p1, p2, p3});
		
		structure st1("truss",{{"A",100},{"E",1e5}});
		structure st2("beam",{{"width",100},{"height",400},{"poisson",0.3},{"E",1e5}});
		structure st3("quad_hexahedron",{{"poisson",0.3},{"E",1e5}});
		
		BOOST_REQUIRE_THROW(t1.addStructure(st1),std::invalid_argument);
		BOOST_REQUIRE_THROW(t1.addStructure(st2),std::invalid_argument);
		BOOST_REQUIRE_THROW(t1.addStructure(st3),std::invalid_argument);
	}
	
	BOOST_AUTO_TEST_CASE( mesh_1 )
	{
		point p1(1,{0,0,0});
		point p2(1,{4000,0,0});
		point p3(1,{2000,4000,0});
		triangle t1({p1, p2, p3});
		bso::utilities::geometry::vertex center = t1.getCenter();
		
		structure st1("flat_shell",{{"thickness",100},{"poisson",0.3},{"E",1e5}});
		t1.addStructure(st1);
		
		std::vector<point*> pointStore;
		t1.mesh(2,pointStore);

		BOOST_REQUIRE(t1.getElementPoints().size() == 3); // amount of elements
		BOOST_REQUIRE(t1.getMeshedPoints().size() == 7); // amount of points in mesh

		BOOST_REQUIRE(t1.getMeshedPoints()[0]->isSameAs({0,  0,  0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[1]->isSameAs({2000,0,  0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[2]->isSameAs({4000,0,  0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[3]->isSameAs({3000,2000,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[4]->isSameAs({2000,4000,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[5]->isSameAs({1000,2000,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[6]->isSameAs({2000,1333.333333,0})); // center
		
		t1.clearMesh();
		BOOST_REQUIRE(t1.getElements().size() == 0);
		BOOST_REQUIRE(t1.getMeshedPoints().size() == 0);
	}

	BOOST_AUTO_TEST_CASE( mesh_2 )
	{
		point p1(1,{0,0,0});
		point p2(1,{4000,0,0});
		point p3(1,{2000,4000,0});
		triangle t1({p1, p2, p3});
		bso::utilities::geometry::vertex center = t1.getCenter();

		structure st1("flat_shell",{{"thickness",100},{"poisson",0.3},{"E",1e5}});
		t1.addStructure(st1);
		
		std::vector<point*> pointStore;
		t1.mesh(4,pointStore);

		BOOST_REQUIRE(t1.getElementPoints().size() == 12); // amount of elements
		BOOST_REQUIRE(t1.getMeshedPoints().size() == 19); // amount of points in mesh

		//row 1
		BOOST_REQUIRE(t1.getMeshedPoints()[0]->isSameAs({0,0,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[1]->isSameAs({1000,0,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[2]->isSameAs({2000,0,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[3]->isSameAs({3000,0,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[4]->isSameAs({4000,0,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[5]->isSameAs({3500,1000,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[6]->isSameAs({3000,2000,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[7]->isSameAs({2500,3000,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[8]->isSameAs({2000,4000,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[9]->isSameAs({1500,3000,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[10]->isSameAs({1000,2000,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[11]->isSameAs({500,1000,0}));

		//row 2
		BOOST_REQUIRE(t1.getMeshedPoints()[12]->isSameAs({1000,666.666666,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[13]->isSameAs({2000,666.666666,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[14]->isSameAs({3000,666.666666,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[15]->isSameAs({2500,1666.666666,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[16]->isSameAs({2000,2666.666666,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[17]->isSameAs({1500,1666.666666,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[18]->isSameAs({2000,1333.333333,0})); // center

		t1.clearMesh();
		BOOST_REQUIRE(t1.getElements().size() == 0);
		BOOST_REQUIRE(t1.getMeshedPoints().size() == 0);
	}

	BOOST_AUTO_TEST_CASE( mesh_3 )
	{
		point p1(1,{0,0,0});
		point p2(1,{6000,0,0});
		point p3(1,{3000,6000,0});
		triangle t1({p1, p2, p3});
		bso::utilities::geometry::vertex center = t1.getCenter();
		
		structure st1("flat_shell",{{"thickness",100},{"poisson",0.3},{"E",1e5}});
		t1.addStructure(st1);
		
		std::vector<point*> pointStore;
		t1.mesh(6,pointStore);

		BOOST_REQUIRE(t1.getElementPoints().size() == 27); // amount of elements
		BOOST_REQUIRE(t1.getMeshedPoints().size() == 37); // amount of points in mesh

		//row 1
		BOOST_REQUIRE(t1.getMeshedPoints()[0]->isSameAs({0,0,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[1]->isSameAs({1000,0,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[2]->isSameAs({2000,0,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[3]->isSameAs({3000,0,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[4]->isSameAs({4000,0,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[5]->isSameAs({5000,0,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[6]->isSameAs({6000,0,0}));

		BOOST_REQUIRE(t1.getMeshedPoints()[7]->isSameAs({5500,1000,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[8]->isSameAs({5000,2000,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[9]->isSameAs({4500,3000,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[10]->isSameAs({4000,4000,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[11]->isSameAs({3500,5000,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[12]->isSameAs({3000,6000,0}));

		BOOST_REQUIRE(t1.getMeshedPoints()[13]->isSameAs({2500,5000,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[14]->isSameAs({2000,4000,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[15]->isSameAs({1500,3000,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[16]->isSameAs({1000,2000,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[17]->isSameAs({500,1000,0}));

		//row 2
		BOOST_REQUIRE(t1.getMeshedPoints()[18]->isSameAs({1000,666.666666,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[19]->isSameAs({2000,666.666666,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[20]->isSameAs({3000,666.666666,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[21]->isSameAs({4000,666.666666,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[22]->isSameAs({5000,666.666666,0}));

		BOOST_REQUIRE(t1.getMeshedPoints()[23]->isSameAs({4500,1666.666666,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[24]->isSameAs({4000,2666.666666,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[25]->isSameAs({3500,3666.666666,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[26]->isSameAs({3000,4666.666666,0}));

		BOOST_REQUIRE(t1.getMeshedPoints()[27]->isSameAs({2500,3666.666666,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[28]->isSameAs({2000,2666.666666,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[29]->isSameAs({1500,1666.666666,0}));

		//row 3
		BOOST_REQUIRE(t1.getMeshedPoints()[30]->isSameAs({2000,1333.333333,  0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[31]->isSameAs({3000,1333.333333,  0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[32]->isSameAs({4000,1333.333333,  0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[33]->isSameAs({3500,2333.333333,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[34]->isSameAs({3000,3333.333333,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[35]->isSameAs({2500,2333.333333,0}));
		BOOST_REQUIRE(t1.getMeshedPoints()[36]->isSameAs({3000,2000,0})); // center

		
		t1.clearMesh();
		BOOST_REQUIRE(t1.getElements().size() == 0);
		BOOST_REQUIRE(t1.getMeshedPoints().size() == 0);
	}

	BOOST_AUTO_TEST_CASE( mesh_invalid )
	{
		point p1(1,{0,0,0});
		point p2(1,{4000,0,0});
		point p3(1,{2000,4000,0});
		triangle t1({p1, p2, p3});
		
		structure st1("flat_shell",{{"thickness",100},{"poisson",0.3},{"E",1e5}});
		t1.addStructure(st1);
		
		std::vector<point*> pointStore;
		BOOST_REQUIRE_THROW(t1.mesh(1,pointStore),std::invalid_argument);
		BOOST_REQUIRE_THROW(t1.mesh(3,pointStore),std::invalid_argument);
		BOOST_REQUIRE_THROW(t1.mesh(5,pointStore),std::invalid_argument);
		BOOST_REQUIRE_THROW(t1.mesh(7,pointStore),std::invalid_argument);
	}
	
	BOOST_AUTO_TEST_CASE( constraint_test)
	{
		point p1(1,{0,0,0});
		point p2(1,{1,0,0});
		point p3(1,{1,1,0});
		triangle t1({p1, p2, p3});
		
		structure st1("flat_shell",{{"thickness",100},{"poisson",0.3},{"E",1e5}});
		t1.addStructure(st1);
		
		constraint c1(4);
		t1.addConstraint(c1);
		
		std::vector<point*> pointStore;
		t1.mesh(2,pointStore);
		
		for (auto& i : pointStore)
		{
			for (const auto& j : i->getConstraints())
			{
				BOOST_REQUIRE(j.DOF() == 4);
			}
		}
	}
	
BOOST_AUTO_TEST_SUITE_END()
} // namespace component_test