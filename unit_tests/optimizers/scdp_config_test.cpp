#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE "scdp_config"
#endif

#include <boost/test/included/unit_test.hpp>

#include <bso/optimizers/scdp/scdp_config.hpp>

#include "argv_builder.hpp"

BOOST_AUTO_TEST_SUITE(scdp_config_test)

BOOST_AUTO_TEST_CASE(parses_minimal_required_args_and_checks_defaults)
{
    ArgvBuilder argv({"prog", "-i", "design_1", "-m", "sd"});
    bso::optimizers::scdp::scdp_config config(argv.argc(), argv.argv());

    BOOST_CHECK_EQUAL(config.getInputFile(), "design_1");
    BOOST_CHECK_EQUAL(config.getModification(), "sd");
    BOOST_CHECK_EQUAL(config.getOutputAll(), false);
    BOOST_CHECK_EQUAL(config.getOutputBest(), false);
}

BOOST_AUTO_TEST_CASE(parses_output_flags)
{
    ArgvBuilder argv({"prog", "-i", "design_1", "-m", "sd", "-o", "verbose", "result_line", "all"});
    bso::optimizers::scdp::scdp_config config(argv.argc(), argv.argv());

    BOOST_CHECK_EQUAL(config.getVerbose(), true);
    BOOST_CHECK_EQUAL(config.getResultLine(), true);
    BOOST_CHECK_EQUAL(config.getOutputAll(), true);
    BOOST_CHECK_EQUAL(config.getOutputBest(), false);
}

BOOST_AUTO_TEST_CASE(rejects_invalid_modification)
{
    ArgvBuilder argv({"prog", "-i", "design_1", "-m", "invalid"});
    BOOST_REQUIRE_THROW(bso::optimizers::scdp::scdp_config config(argv.argc(), argv.argv()), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(rejects_all_and_best_with_result_line)
{
    ArgvBuilder argv({"prog", "-i", "design_1", "-m", "sd", "-o", "result_line", "all", "best"});
    BOOST_REQUIRE_THROW(bso::optimizers::scdp::scdp_config config(argv.argc(), argv.argv()), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(rejects_invalid_performance_division)
{
    ArgvBuilder argv({"prog", "-i", "design_1", "-m", "sd", "-p", "not_a_division"});
    BOOST_REQUIRE_THROW(bso::optimizers::scdp::scdp_config config(argv.argc(), argv.argv()), std::invalid_argument);
}

// Simulation flag validation tests
BOOST_AUTO_TEST_CASE(rejects_sd_modification_without_sd_simulation)
{
    // ERROR: sd modification with only bp simulation
    ArgvBuilder argv({"prog", "-i", "design_304", "-m", "sd", "-sim", "bp"});
    BOOST_REQUIRE_THROW(bso::optimizers::scdp::scdp_config config(argv.argc(), argv.argv()), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(rejects_bp_modification_without_bp_simulation)
{
    // ERROR: bp modification with only sd simulation
    ArgvBuilder argv({"prog", "-i", "design_304", "-m", "bp", "-sim", "sd"});
    BOOST_REQUIRE_THROW(bso::optimizers::scdp::scdp_config config(argv.argc(), argv.argv()), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(rejects_weighted_modification_without_both_simulations)
{
    // ERROR: weighted modification with only sd simulation
    ArgvBuilder argv1({"prog", "-i", "design_304", "-m", "weighted", "-sim", "sd"});
    BOOST_REQUIRE_THROW(bso::optimizers::scdp::scdp_config config1(argv1.argc(), argv1.argv()), std::invalid_argument);
    
    // ERROR: weighted modification with only bp simulation
    ArgvBuilder argv2({"prog", "-i", "design_304", "-m", "weighted", "-sim", "bp"});
    BOOST_REQUIRE_THROW(bso::optimizers::scdp::scdp_config config2(argv2.argc(), argv2.argv()), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(rejects_sdbp_modification_without_both_simulations)
{
    // ERROR: sdbp modification with only sd simulation
    ArgvBuilder argv({"prog", "-i", "design_304", "-m", "sdbp", "-sim", "sd"});
    BOOST_REQUIRE_THROW(bso::optimizers::scdp::scdp_config config(argv.argc(), argv.argv()), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(rejects_bpsd_modification_without_both_simulations)
{
    // ERROR: bpsd modification with only bp simulation
    ArgvBuilder argv({"prog", "-i", "design_304", "-m", "bpsd", "-sim", "bp"});
    BOOST_REQUIRE_THROW(bso::optimizers::scdp::scdp_config config(argv.argc(), argv.argv()), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(accepts_sd_modification_with_sd_simulation)
{
    // OK: sd modification with sd simulation
    ArgvBuilder argv({"prog", "-i", "design_304", "-m", "sd", "-sim", "sd"});
    bso::optimizers::scdp::scdp_config config(argv.argc(), argv.argv());
    BOOST_CHECK_EQUAL(config.getModification(), "sd");
    BOOST_CHECK_EQUAL(config.getRunSDSimulation(), true);
    BOOST_CHECK_EQUAL(config.getRunBPSimulation(), false);
}

BOOST_AUTO_TEST_CASE(accepts_bp_modification_with_bp_simulation)
{
    // OK: bp modification with bp simulation
    ArgvBuilder argv({"prog", "-i", "design_304", "-m", "bp", "-sim", "bp"});
    bso::optimizers::scdp::scdp_config config(argv.argc(), argv.argv());
    BOOST_CHECK_EQUAL(config.getModification(), "bp");
    BOOST_CHECK_EQUAL(config.getRunSDSimulation(), false);
    BOOST_CHECK_EQUAL(config.getRunBPSimulation(), true);
}

BOOST_AUTO_TEST_CASE(accepts_weighted_modification_with_both_simulations)
{
    // OK: weighted modification with both simulations
    ArgvBuilder argv({"prog", "-i", "design_304", "-m", "weighted", "-sim", "sd", "bp"});
    bso::optimizers::scdp::scdp_config config(argv.argc(), argv.argv());
    BOOST_CHECK_EQUAL(config.getModification(), "weighted");
    BOOST_CHECK_EQUAL(config.getRunSDSimulation(), true);
    BOOST_CHECK_EQUAL(config.getRunBPSimulation(), true);
}

BOOST_AUTO_TEST_CASE(accepts_sdbp_modification_with_both_simulations_explicit)
{
    // OK: sdbp modification with explicit sd bp
    ArgvBuilder argv({"prog", "-i", "design_304", "-m", "sdbp", "-sim", "sd", "bp"});
    bso::optimizers::scdp::scdp_config config(argv.argc(), argv.argv());
    BOOST_CHECK_EQUAL(config.getModification(), "sdbp");
    BOOST_CHECK_EQUAL(config.getRunSDSimulation(), true);
    BOOST_CHECK_EQUAL(config.getRunBPSimulation(), true);
}

BOOST_AUTO_TEST_CASE(accepts_weighted_modification_without_sim_flag_defaults_to_both)
{
    // OK: weighted modification without -sim flag (defaults to both)
    ArgvBuilder argv({"prog", "-i", "design_304", "-m", "weighted"});
    bso::optimizers::scdp::scdp_config config(argv.argc(), argv.argv());
    BOOST_CHECK_EQUAL(config.getModification(), "weighted");
    BOOST_CHECK_EQUAL(config.getRunSDSimulation(), true);
    BOOST_CHECK_EQUAL(config.getRunBPSimulation(), true);
}

BOOST_AUTO_TEST_SUITE_END()
