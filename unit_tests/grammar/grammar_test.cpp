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

BOOST_AUTO_TEST_SUITE_END()

#include <unit_tests/grammar/sd_grammars/default_sd_grammar_test.cpp>
#include <unit_tests/grammar/bp_grammars/default_bp_grammar_test.cpp>
} // namespace grammar_test