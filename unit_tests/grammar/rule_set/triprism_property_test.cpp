#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE "grammar_triprism_property_test"
#endif

#include <bso/grammar/rule_set/triprism_property.hpp>

/*
BOOST_TEST()
BOOST_REQUIRE_THROW(function, std::domain_error)
BOOST_REQUIRE_NO_THROW(function)
BOOST_REQUIRE(!s[8].dominates(s[9]) && !s[9].dominates(s[8]))
BOOST_CHECK_EQUAL_COLLECTIONS(a.begin(), a.end(), b.begin(), b.end());
*/

namespace grammar_rule_set_test {
using namespace bso::grammar::rule_set;

BOOST_AUTO_TEST_SUITE( grammar_triprism_property_test )

BOOST_AUTO_TEST_CASE( initialization )
{
	namespace cf = bso::spatial_design::conformal;
	bso::spatial_design::ms_N_building msModel("grammar/ms_N_test_2.txt");
	bso::spatial_design::cf_building cfModel(msModel);
	cf::cf_geometry_model* geomModel = &cfModel;

	#if defined(BSO_USE_FADE)
	auto tripPtr1 = geomModel->addTriPrism({
		{0,0,0},{3000,0,0},{3000,3000,0},
		{0,0,3000},{3000,0,3000},{3000,3000,3000}});
	#endif
	#if defined(BSO_USE_CGAL)
	auto tripPtr1 = geomModel->addTriPrism({
		{0,0,0},{3000,0,0},{0,3000,0},
		{0,0,3000},{3000,0,3000},{0,3000,3000}});
	#endif

	BOOST_REQUIRE_NO_THROW(triprism_property tp1(tripPtr1));
	triprism_property tp1(tripPtr1);
	BOOST_REQUIRE(tp1.getTriprismPtr() == tripPtr1);
	BOOST_REQUIRE(tp1.isInSpace());
	
	BOOST_REQUIRE(!tp1.isVertexProperty());
	BOOST_REQUIRE(!tp1.isLineProperty());
	BOOST_REQUIRE(!tp1.isRectangleProperty());
	BOOST_REQUIRE(!tp1.isTriangleProperty());
	BOOST_REQUIRE( tp1.isTriprismProperty());
	BOOST_REQUIRE(!tp1.isCuboidProperty());
	BOOST_REQUIRE(!tp1.isPointProperty());
	BOOST_REQUIRE(!tp1.isEdgeProperty());
	BOOST_REQUIRE(!tp1.isSurfaceProperty());
	BOOST_REQUIRE(!tp1.isSpaceProperty());
}

BOOST_AUTO_TEST_CASE( get_space_type )
{
	namespace cf = bso::spatial_design::conformal;
	bso::spatial_design::ms_N_building msModel("grammar/ms_N_test_2.txt");
	bso::spatial_design::cf_building cfModel(msModel);
	cf::cf_geometry_model* geomModel = &cfModel;
	auto tripPtr1 = geomModel->addTriPrism({
		{0,0,0},{3000,0,0},{3000,3000,0},
		{0,0,3000},{3000,0,3000},{3000,3000,3000}});
	auto tripPtr2 = geomModel->addTriPrism({
		{3000,0,0},{6000,0,0},{3000,3000,0},
		{3000,0,3000},{6000,0,3000},{3000,3000,3000}});

	triprism_property tp1(tripPtr1);
	triprism_property tp2(tripPtr2);

	BOOST_REQUIRE(tp1.getSpaceType() == "");
	BOOST_REQUIRE(tp2.getSpaceType() == "T");
}

BOOST_AUTO_TEST_CASE( is_above_or_below_ground_surfaces )
{
	namespace cf = bso::spatial_design::conformal;
	bso::spatial_design::ms_N_building msModel("grammar/ms_N_test_2.txt");
	bso::spatial_design::cf_building cfModel(msModel);
	cf::cf_geometry_model* geomModel = &cfModel;
	auto tripPtr1 = geomModel->addTriPrism({
		{0,0,0},{3000,0,0},{3000,3000,0},
		{0,0,3000},{3000,0,3000},{3000,3000,3000}});
	auto tripPtr2 = geomModel->addTriPrism({
		{3000,0,3000},{6000,0,3000},{3000,3000,3000},
		{3000,0,6000},{6000,0,6000},{3000,3000,6000}});
	auto tripPtr3 = geomModel->addTriPrism({
		{6000,0,-3000},{9000,0,-3000},{6000,3000,-3000},
		{6000,0,0},{9000,0,0},{6000,3000,0}});

	triprism_property tp1(tripPtr1);
	triprism_property tp2(tripPtr2);
	triprism_property tp3(tripPtr3);

	BOOST_REQUIRE( tp1.isAboveGroundSurface());
	BOOST_REQUIRE( tp2.isAboveGroundSurface());
	BOOST_REQUIRE(!tp3.isAboveGroundSurface());
	
	BOOST_REQUIRE(!tp1.isBelowGroundSurface());
	BOOST_REQUIRE(!tp2.isBelowGroundSurface());
	BOOST_REQUIRE( tp3.isBelowGroundSurface());
}
	
BOOST_AUTO_TEST_SUITE_END()
} // namespace grammar_rule_set_test