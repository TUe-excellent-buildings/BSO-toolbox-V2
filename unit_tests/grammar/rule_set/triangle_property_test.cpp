#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE "grammar_triangle_property_test"
#endif

#include <bso/grammar/rule_set/triangle_property.hpp>

/* Please note, in the geometry conformal model are no triangular walls found. Hence, only the triangular floors are considered in this boost check*/

/*
BOOST_TEST()
BOOST_REQUIRE_THROW(function, std::domain_error)
BOOST_REQUIRE_NO_THROW(function)
BOOST_REQUIRE(!s[8].dominates(s[9]) && !s[9].dominates(s[8]))
BOOST_CHECK_EQUAL_COLLECTIONS(a.begin(), a.end(), b.begin(), b.end());
*/

namespace grammar_rule_set_test {
using namespace bso::grammar::rule_set;

BOOST_AUTO_TEST_SUITE( grammar_triangle_property_test )

BOOST_AUTO_TEST_CASE( initialization )
{
	namespace cf = bso::spatial_design::conformal;
	bso::spatial_design::ms_N_building msModel("grammar/ms_N_test_2.txt");
	bso::spatial_design::cf_building cfModel(msModel);
	cf::cf_geometry_model* geomModel = &cfModel;
	auto triPtr = geomModel->addTriangle({{0,0,0},{3000,0,0},{3000,3000,0}});

	BOOST_REQUIRE_NO_THROW(triangle_property tp1(triPtr));
	triangle_property tp1(triPtr);
	BOOST_REQUIRE(tp1.getTrianglePtr() == triPtr);
	BOOST_REQUIRE(tp1.getOrientation().isCodirectional({0,0,1}));
	
	BOOST_REQUIRE(!tp1.isVertexProperty());
	BOOST_REQUIRE(!tp1.isLineProperty());
	BOOST_REQUIRE(!tp1.isRectangleProperty());
	BOOST_REQUIRE( tp1.isTriangleProperty());
	BOOST_REQUIRE(!tp1.isCuboidProperty());
	BOOST_REQUIRE(!tp1.isTriprismProperty());
	BOOST_REQUIRE(!tp1.isPointProperty());
	BOOST_REQUIRE(!tp1.isEdgeProperty());
	BOOST_REQUIRE(!tp1.isSurfaceProperty());
	BOOST_REQUIRE(!tp1.isSpaceProperty());
}

BOOST_AUTO_TEST_CASE( space_and_surface_types )
{ 
	namespace cf = bso::spatial_design::conformal;
	bso::spatial_design::ms_N_building msModel("grammar/ms_N_test_2.txt");
	bso::spatial_design::cf_building cfModel(msModel);
	cf::cf_geometry_model* geomModel = &cfModel;

	auto  triPtr2 = geomModel->addTriangle({{3000,0,0},{6000,0,0},{3000,3000,0}});

	triangle_property rp2( triPtr2);
	
	typedef std::pair<std::string, std::string> type_set;

	BOOST_REQUIRE(rp2.getSpaceTypes() == type_set("G","T"));

	BOOST_REQUIRE(rp2.getSurfaceTypes() == type_set("F","G"));
}

BOOST_AUTO_TEST_CASE( is_floor_check )
{
	namespace cf = bso::spatial_design::conformal;
	bso::spatial_design::ms_N_building msModel("grammar/ms_N_test_2.txt");
	bso::spatial_design::cf_building cfModel(msModel);
	cf::cf_geometry_model* geomModel = &cfModel;
	auto triPtr1 = geomModel->addTriangle({{6000,0,-3000},{9000,3000,-3000},{6000,3000,-3000}});
	auto triPtr1b = geomModel->addTriangle({{6000,0,-3000},{9000,0,-3000},{9000,3000,-3000}});
	auto triPtr2 = geomModel->addTriangle({{0,0,3000},{3000,3000,3000},{0,3000,3000}});
	auto triPtr2b = geomModel->addTriangle({{0,0,3000},{3000,0,3000},{3000,3000,3000}});
	auto triPtr3 = geomModel->addTriangle({{9000,0,6000},{12000,3000,6000},{9000,3000,6000}});
	auto triPtr3b = geomModel->addTriangle({{9000,0,6000},{12000,0,6000},{12000,3000,6000}});
	auto triPtr4 = geomModel->addTriangle({{6000,0,-3000},{9000,0,0},{6000,0,0}});
	auto triPtr4b = geomModel->addTriangle({{6000,0,-3000},{9000,0,-3000},{9000,0,0}});
	auto triPtr5 = geomModel->addTriangle({{6000,0,3000},{6000,3000,6000},{6000,0,6000}});
	auto triPtr5b = geomModel->addTriangle({{6000,0,3000},{6000,3000,3000},{6000,3000,6000}});

	triangle_property rp1(triPtr1);
	triangle_property rp2(triPtr2);
	triangle_property rp3(triPtr3);
	triangle_property rp4(triPtr4);
	triangle_property rp5(triPtr5);
	triangle_property rp1b(triPtr1b);
	triangle_property rp2b(triPtr2b);
	triangle_property rp3b(triPtr3b);
	triangle_property rp4b(triPtr4b);
	triangle_property rp5b(triPtr5b);
	
	BOOST_REQUIRE( rp1.isFloor());
	BOOST_REQUIRE( rp2.isFloor());
	BOOST_REQUIRE( rp3.isFloor());
	BOOST_REQUIRE(!rp4.isFloor());
	BOOST_REQUIRE(!rp5.isFloor());
	BOOST_REQUIRE( rp1b.isFloor());
	BOOST_REQUIRE( rp2b.isFloor());
	BOOST_REQUIRE( rp3b.isFloor());
	BOOST_REQUIRE(!rp4b.isFloor());
	BOOST_REQUIRE(!rp5b.isFloor());
}

BOOST_AUTO_TEST_CASE( is_space_separating_check )
{ 
	namespace cf = bso::spatial_design::conformal;
	bso::spatial_design::ms_N_building msModel("grammar/ms_N_test_2.txt");
	bso::spatial_design::cf_building cfModel(msModel);
	cf::cf_geometry_model* geomModel = &cfModel;
	auto triPtr1 = geomModel->addTriangle({{6000,0,-3000},{9000,0,-3000},{6000,3000,-3000}});
	auto triPtr2 = geomModel->addTriangle({{0,0,3000},{3000,3000,3000},{0,3000,3000}});
	auto triPtr3 = geomModel->addTriangle({{9000,0,6000},{12000,0,6000},{9000,3000,6000}});
	auto triPtr7 = geomModel->addTriangle({{12000,0,3000},{15000,0,3000},{12000,3000,3000}});

	triangle_property rp1(triPtr1);
	triangle_property rp2(triPtr2);
	triangle_property rp3(triPtr3);
	triangle_property rp7(triPtr7);
	
	BOOST_REQUIRE( rp1.isSpaceSeparating());
	BOOST_REQUIRE( rp2.isSpaceSeparating());
	BOOST_REQUIRE( rp3.isSpaceSeparating());
	BOOST_REQUIRE( rp7.isSpaceSeparating());
}

BOOST_AUTO_TEST_CASE( is_external_check )
{
	namespace cf = bso::spatial_design::conformal;
	bso::spatial_design::ms_N_building msModel("grammar/ms_N_test_2.txt");
	bso::spatial_design::cf_building cfModel(msModel);
	cf::cf_geometry_model* geomModel = &cfModel;
	auto triPtr1 = geomModel->addTriangle({{6000,0,-3000},{9000,0,-3000},{6000,3000,-3000}});
	auto triPtr2 = geomModel->addTriangle({{0,0,3000},{3000,3000,3000},{0,3000,3000}});
	auto triPtr3 = geomModel->addTriangle({{9000,0,6000},{9000,3000,6000},{9000,0,9000}});
	auto triPtr7 = geomModel->addTriangle({{12000,0,3000},{15000,0,3000},{12000,3000,3000}});

	triangle_property rp1(triPtr1);
	triangle_property rp2(triPtr2);
	triangle_property rp3(triPtr3);
	triangle_property rp7(triPtr7);
	
	BOOST_REQUIRE( rp1.isExternal());
	BOOST_REQUIRE(!rp2.isExternal());
	BOOST_REQUIRE(!rp3.isExternal());
	BOOST_REQUIRE( rp7.isExternal());
}

BOOST_AUTO_TEST_CASE( is_roof_or_overhang_check )
{
	namespace cf = bso::spatial_design::conformal;
	bso::spatial_design::ms_N_building msModel("grammar/ms_N_test_2.txt");
	bso::spatial_design::cf_building cfModel(msModel);
	cf::cf_geometry_model* geomModel = &cfModel;
	auto triPtr1 = geomModel->addTriangle({{0,0,6000},{3000,3000,6000},{0,3000,6000}});
	auto triPtr2 = geomModel->addTriangle({{6000,0,-3000},{9000,3000,-3000},{6000,3000,-3000}});
	auto triPtr3 = geomModel->addTriangle({{6000,-1000,0},{9000,0,0},{6000,0,0}});
	auto triPtr3b = geomModel->addTriangle({{6000,-1000,0},{9000,-1000,0},{9000,0,0}});
	auto triPtr4 = geomModel->addTriangle({{12000,0,3000},{15000,0,3000},{12000,3000,3000}});

	triangle_property rp1(triPtr1);
	triangle_property rp2(triPtr2);
	triangle_property rp3(triPtr3);
	triangle_property rp3b(triPtr3b);
	triangle_property rp4(triPtr4);

	BOOST_REQUIRE( rp1.isRoof());
	BOOST_REQUIRE(!rp2.isRoof());
	BOOST_REQUIRE( rp3.isRoof());
	BOOST_REQUIRE( rp3b.isRoof());
	BOOST_REQUIRE(!rp4.isRoof());
	
	BOOST_REQUIRE(!rp1.isOverhang());
	BOOST_REQUIRE(!rp2.isOverhang());
	BOOST_REQUIRE(!rp3.isOverhang());
	BOOST_REQUIRE(!rp3b.isOverhang());
	BOOST_REQUIRE( rp4.isOverhang());
}

BOOST_AUTO_TEST_CASE( is_above_or_below_ground_check )
{
	namespace cf = bso::spatial_design::conformal;
	bso::spatial_design::ms_N_building msModel("grammar/ms_N_test_2.txt");
	bso::spatial_design::cf_building cfModel(msModel);
	cf::cf_geometry_model* geomModel = &cfModel;
	auto triPtr1 = geomModel->addTriangle({{0,0,6000},{3000,3000,6000},{0,3000,6000}});
	auto triPtr2 = geomModel->addTriangle({{6000,0,0},{9000,0,0},{600,3000,0}});
	auto triPtr4 = geomModel->addTriangle({{6000,0,-3000},{9000,0,-3000},{6000,3000,-3000}});

	triangle_property rp1(triPtr1);
	triangle_property rp2(triPtr2);
	triangle_property rp4(triPtr4);

	BOOST_REQUIRE( rp1.isAboveGroundSurface());
	BOOST_REQUIRE(!rp2.isAboveGroundSurface());
	BOOST_REQUIRE(!rp4.isAboveGroundSurface());
	
	BOOST_REQUIRE(!rp1.isBelowGroundSurface());
	BOOST_REQUIRE(!rp2.isBelowGroundSurface());
	BOOST_REQUIRE( rp4.isBelowGroundSurface());
}

BOOST_AUTO_TEST_SUITE_END()
} // namespace grammar_rule_set_test