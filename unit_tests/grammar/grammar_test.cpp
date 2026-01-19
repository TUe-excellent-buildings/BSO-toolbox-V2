#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE "grammar_tests"
#endif

#include <boost/test/included/unit_test.hpp>

#include <bso/spatial_design/ms_building.hpp>
#include <bso/spatial_design/ms_N_building.hpp>
#include <bso/spatial_design/cf_building.hpp>
#include <bso/structural_design/sd_model.hpp>
#include <bso/building_physics/bp_model.hpp>
#include <bso/grammar/grammar.hpp>

#include <unit_tests/grammar/rule_set/vertex_property_test.cpp>
#include <unit_tests/grammar/rule_set/line_property_test.cpp>
#include <unit_tests/grammar/rule_set/rectangle_property_test.cpp>
#include <unit_tests/grammar/rule_set/triangle_property_test.cpp>
#include <unit_tests/grammar/rule_set/cuboid_property_test.cpp>
#include <unit_tests/grammar/rule_set/triprism_property_test.cpp>

#include <unit_tests/grammar/rule_set/point_property_test.cpp>
#include <unit_tests/grammar/rule_set/edge_property_test.cpp>
#include <unit_tests/grammar/rule_set/surface_property_test.cpp>
#include <unit_tests/grammar/rule_set/space_property_test.cpp>

#include <unit_tests/grammar/rule_set/sd_rule_set/sd_vertex_rule_test.cpp>
#include <unit_tests/grammar/rule_set/sd_rule_set/sd_line_rule_test.cpp>
#include <unit_tests/grammar/rule_set/sd_rule_set/sd_rectangle_rule_test.cpp>
#include <unit_tests/grammar/rule_set/sd_rule_set/sd_triangle_rule_test.cpp>

#include <unit_tests/grammar/rule_set/bp_rule_set/bp_rectangle_rule_test.cpp>
#include <unit_tests/grammar/rule_set/bp_rule_set/bp_triangle_rule_test.cpp>
#include <unit_tests/grammar/rule_set/bp_rule_set/bp_space_rule_test.cpp>

/*
BOOST_TEST()
BOOST_REQUIRE_THROW(function, std::domain_error)
BOOST_REQUIRE(!s[8].dominates(s[9]) && !s[9].dominates(s[8]))
BOOST_CHECK_EQUAL_COLLECTIONS(a.begin(), a.end(), b.begin(), b.end());
*/

namespace grammar_test {
using namespace bso::grammar;

BOOST_AUTO_TEST_SUITE( grammar_test )

	BOOST_AUTO_TEST_CASE( initialization_1 )
	{
		bso::spatial_design::ms_building ms("grammar/ms_test_1.txt");
		bso::spatial_design::cf_building cf(ms);

		bso::grammar::grammar gram(cf);
	}
	
	BOOST_AUTO_TEST_CASE( initialization_2 )
	{
		bso::spatial_design::ms_N_building ms("grammar/ms_N_test_1.txt");
		bso::spatial_design::cf_building cf(ms);

		bso::grammar::grammar gram(cf);
	}
	
	BOOST_AUTO_TEST_CASE( run_compare )
	{
		// This boost test is meant to verify the generation and results of the sd and bp simulations
		// for NOBSD representation with the already proven orthogonal BSD.
		// The important difference between the two representations are the generation of the conformal geometry where the simulation is based upon.
		// Although the inserted design is orthogonal, the non-orthogonal conformal model and the orthogonal conformal model are compared for this design.
		// Meaning that if the result of the simulations for the sd and bp simulation are the same in either representation the simulations for the NOBSD is setup correctly.
		
		// Setup for conformal geometry difference
		bso::spatial_design::ms_building ms1("grammar/design_1");
		bso::spatial_design::cf_building cf1(ms1);
		
		bso::spatial_design::ms_N_building ms2("grammar/design_N_1");
		bso::spatial_design::cf_building cf2(ms2);

		bso::grammar::grammar gram1(cf1);
		bso::grammar::grammar gram2(cf2);
		
		// SD analysis check
		bso::structural_design::sd_model SD1 = gram1.sd_grammar <bso::grammar::DEFAULT_SD_GRAMMAR >(std::string("grammar/settings/sd_settings.txt"));
		SD1.analyze();
		
		bso::structural_design::sd_model SD2 = gram2.sd_grammar <bso::grammar::DEFAULT_SD_GRAMMAR >(std::string("grammar/settings/sd_settings.txt"));
		SD2.analyze();
		
		// perform Checks:
		bool check1 = false;
		double tolerance1 =  SD1.getTotalResults().mTotalStrainEnergy*0.001;
		
		if(SD1.getTotalResults().mTotalStrainEnergy-SD2.getTotalResults().mTotalStrainEnergy < tolerance1 && SD1.getTotalResults().mTotalStrainEnergy-SD2.getTotalResults().mTotalStrainEnergy > -tolerance1) check1 = true;
		
		BOOST_REQUIRE(check1);
		
		// BP analysis check:
		bso::building_physics::bp_model BP1 = gram1.bp_grammar <bso::grammar::DEFAULT_BP_GRAMMAR >(std::string("grammar/settings/bp_settings.txt"));
		BP1.simulatePeriods("runge_kutta_dopri5",1e-3,1e-3);
		
		bso::building_physics::bp_model BP2 = gram2.bp_grammar <bso::grammar::DEFAULT_BP_GRAMMAR >(std::string("grammar/settings/bp_settings.txt"));
		BP2.simulatePeriods("runge_kutta_dopri5",1e-3,1e-3);
		
		// perform Checks:
		bool check2 = true;
		double tolerance2 =  10;
		
		if(BP1.getTotalResults().mTotalHeatingEnergy-BP2.getTotalResults().mTotalHeatingEnergy < tolerance2 && BP1.getTotalResults().mTotalHeatingEnergy-BP2.getTotalResults().mTotalHeatingEnergy > -tolerance2) check2 = true;
		
		BOOST_REQUIRE(check2);
	}

	BOOST_AUTO_TEST_CASE(getSpacesOnBoundary_returns_boundary_spaces)
	{
		// Create a simple 3x3 grid of spaces with three elevations
		// In this case, there is a space fully inclosed by other spaces,
		// a space fully inclosed by spaces and the ground,
		// and a space inclosed by spaces and its roof by air.
		std::string s1 = "N,1, 0,0,0, 1,0,0, 0,1,0, 1,1,0, 0,0,1, 1,0,1, 0,1,1, 1,1,1";
		std::string s2 = "N,2, 1,0,0, 2,0,0, 1,1,0, 2,1,0, 1,0,1, 2,0,1, 1,1,1, 2,1,1";
		std::string s3 = "N,3, 2,0,0, 3,0,0, 2,1,0, 3,1,0, 2,0,1, 3,0,1, 2,1,1, 3,1,1";
		std::string s4 = "N,4, 0,1,0, 1,1,0, 0,2,0, 1,2,0, 0,1,1, 1,1,1, 0,2,1, 1,2,1";
		std::string s5 = "N,5, 1,1,0, 2,1,0, 1,2,0, 2,2,0, 1,1,1, 2,1,1, 1,2,1, 2,2,1";
		std::string s6 = "N,6, 2,1,0, 3,1,0, 2,2,0, 3,2,0, 2,1,1, 3,1,1, 2,2,1, 3,2,1";
		std::string s7 = "N,7, 0,2,0, 1,2,0, 0,3,0, 1,3,0, 0,2,1, 1,2,1, 0,3,1, 1,3,1";
		std::string s8 = "N,8, 1,2,0, 2,2,0, 1,3,0, 2,3,0, 1,2,1, 2,2,1, 1,3,1, 2,3,1";
		std::string s9 = "N,9, 2,2,0, 3,2,0, 2,3,0, 3,3,0, 2,2,1, 3,2,1, 2,3,1, 3,3,1";

		std::string s10 = "N,10, 0,0,1, 1,0,1, 0,1,1, 1,1,1, 0,0,2, 1,0,2, 0,1,2, 1,1,2";
		std::string s11 = "N,11, 1,0,1, 2,0,1, 1,1,1, 2,1,1, 1,0,2, 2,0,2, 1,1,2, 2,1,2";
		std::string s12 = "N,12, 2,0,1, 3,0,1, 2,1,1, 3,1,1, 2,0,2, 3,0,2, 2,1,2, 3,1,2";
		std::string s13 = "N,13, 0,1,1, 1,1,1, 0,2,1, 1,2,1, 0,1,2, 1,1,2, 0,2,2, 1,2,2";
		std::string s14 = "N,14, 1,1,1, 2,1,1, 1,2,1, 2,2,1, 1,1,2, 2,1,2, 1,2,2, 2,2,2";
		std::string s15 = "N,15, 2,1,1, 3,1,1, 2,2,1, 3,2,1, 2,1,2, 3,1,2, 2,2,2, 3,2,2";
		std::string s16 = "N,16, 0,2,1, 1,2,1, 0,3,1, 1,3,1, 0,2,2, 1,2,2, 0,3,2, 1,3,2";
		std::string s17 = "N,17, 1,2,1, 2,2,1, 1,3,1, 2,3,1, 1,2,2, 2,2,2, 1,3,2, 2,3,2";
		std::string s18 = "N,18, 2,2,1, 3,2,1, 2,3,1, 3,3,1, 2,2,2, 3,2,2, 2,3,2, 3,3,2";

		std::string s19 = "N,19, 0,0,2, 1,0,2, 0,1,2, 1,1,2, 0,0,3, 1,0,3, 0,1,3, 1,1,3";
		std::string s20 = "N,20, 1,0,2, 2,0,2, 1,1,2, 2,1,2, 1,0,3, 2,0,3, 1,1,3, 2,1,3";
		std::string s21 = "N,21, 2,0,2, 3,0,2, 2,1,2, 3,1,2, 2,0,3, 3,0,3, 2,1,3, 3,1,3";
		std::string s22 = "N,22, 0,1,2, 1,1,2, 0,2,2, 1,2,2, 0,1,3, 1,1,3, 0,2,3, 1,2,3";
		std::string s23 = "N,23, 1,1,2, 2,1,2, 1,2,2, 2,2,2, 1,1,3, 2,1,3, 1,2,3, 2,2,3";
		std::string s24 = "N,24, 2,1,2, 3,1,2, 2,2,2, 3,2,2, 2,1,3, 3,1,3, 2,2,3, 3,2,3";
		std::string s25 = "N,25, 0,2,2, 1,2,2, 0,3,2, 1,3,2, 0,2,3, 1,2,3, 0,3,3, 1,3,3";
		std::string s26 = "N,26, 1,2,2, 2,2,2, 1,3,2, 2,3,2, 1,2,3, 2,2,3, 1,3,3, 2,3,3";
		std::string s27 = "N,27, 2,2,2, 3,2,2, 2,3,2, 3,3,2, 2,2,3, 3,2,3, 2,3,3, 3,3,3";

		std::string s28 = "N,28, 10,0,0, 11,0,0, 10,1,0, 11,1,0, 10,0,1, 11,0,1, 10,1,1, 11,1,1"; // isolated space
		std::string s29 = "N,29, 3,0,0,	4,0,0,	4,1,0,	3,1,0,	3,0,1,	4,0,1,	4,1,1,	3,1,1"; // overhanging space

		bso::spatial_design::ms_space sp1(s1), sp2(s2), sp3(s3), sp4(s4), sp5(s5),
			sp6(s6), sp7(s7), sp8(s8), sp9(s9), sp10(s10),
			sp11(s11), sp12(s12), sp13(s13), sp14(s14), sp15(s15),
			sp16(s16), sp17(s17), sp18(s18), sp19(s19), sp20(s20), 
			sp21(s21), sp22(s22), sp23(s23), sp24(s24), sp25(s25), 
			sp26(s26), sp27(s27), sp28(s28), sp29(s29);

		bso::spatial_design::ms_N_building msb;
		msb.addSpace(sp1);
		msb.addSpace(sp2);
		msb.addSpace(sp3);
		msb.addSpace(sp4);
		msb.addSpace(sp5);
		msb.addSpace(sp6);
		msb.addSpace(sp7);
		msb.addSpace(sp8);
		msb.addSpace(sp9);
		msb.addSpace(sp10);
		msb.addSpace(sp11);
		msb.addSpace(sp12);
		msb.addSpace(sp13);
		msb.addSpace(sp14);
		msb.addSpace(sp15);
		msb.addSpace(sp16);
		msb.addSpace(sp17);
		msb.addSpace(sp18);
		msb.addSpace(sp19);
		msb.addSpace(sp20);
		msb.addSpace(sp21);
		msb.addSpace(sp22);
		msb.addSpace(sp23);
		msb.addSpace(sp24);
		msb.addSpace(sp25);
		msb.addSpace(sp26);
		msb.addSpace(sp27);
		msb.addSpace(sp28);
		msb.addSpace(sp29);
		
		// Convert to a conformal building
		bso::spatial_design::cf_building cf(msb);

		// Execute grammar for their if-ten rules sets which determine exterior vs. interior rectangles
		bso::grammar::grammar gram(cf);

		// Get spaces that lie on the exterior boundary (via external rectangles)
		auto boundarySpaces = gram.getSpacesOnBoundary();
		auto boundarySpacesInclGround = gram.getSpacesOnBoundary(true);

		BOOST_REQUIRE(boundarySpaces.size() == 26);
		BOOST_REQUIRE(boundarySpacesInclGround.size() == 28);
	}


BOOST_AUTO_TEST_SUITE_END()

#include <unit_tests/grammar/sd_grammars/default_sd_grammar_test.cpp>
#include <unit_tests/grammar/bp_grammars/default_bp_grammar_test.cpp>
} // namespace grammar_test