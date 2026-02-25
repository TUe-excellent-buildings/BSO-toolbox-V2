#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE "output_manager"
#endif

#include <boost/test/included/unit_test.hpp>

#include <bso/optimizers/scdp/output_manager.hpp>
#include <bso/optimizers/scdp/scdp_config.hpp>
#include <bso/spatial_design/ms_N_building.hpp>
#include <bso/utilities/data_point.hpp>

#include <filesystem>

#include "argv_builder.hpp"

namespace {
void cleanupResultsDir()
{
    std::error_code ec;
    std::filesystem::remove_all("results", ec);
}
} // namespace

struct OutputManagerFixture {
    OutputManagerFixture()
    {
        // Clean up before each test to ensure fresh state
        cleanupResultsDir();
    }
    
    ~OutputManagerFixture()
    {
        // Clean up after each test
        cleanupResultsDir();
    }
};

BOOST_AUTO_TEST_SUITE(output_manager_test)

BOOST_FIXTURE_TEST_CASE(set_and_get_result_dir, OutputManagerFixture)
{
    ArgvBuilder argv({"prog", "-i", "design_1", "-m", "sd", "--resultDir", "test_subdir"});
    bso::optimizers::scdp::scdp_config config(argv.argc(), argv.argv());
    bso::optimizers::scdp::output_manager manager(config);

    // Result dir is set automatically in constructor if config has --resultDir flag
    // Just check that it's not empty
    BOOST_CHECK(!manager.getResultDir().empty());
}

BOOST_FIXTURE_TEST_CASE(result_dir_created_via_constructor_when_flag_set, OutputManagerFixture)
{
    ArgvBuilder argv({"prog", "-i", "design_1", "-m", "sd", "--resultDir", "test_subdir"});
    bso::optimizers::scdp::scdp_config config(argv.argc(), argv.argv());

    bso::optimizers::scdp::output_manager manager(config);
    
    // Check that result directory was set
    BOOST_CHECK(!manager.getResultDir().empty());
    BOOST_CHECK(std::filesystem::exists("results"));
}

BOOST_FIXTURE_TEST_CASE(output_data_with_no_outputs_does_not_throw, OutputManagerFixture)
{
    ArgvBuilder argv({"prog", "-i", "design_1", "-m", "sd"});
    bso::optimizers::scdp::scdp_config config(argv.argc(), argv.argv());
    bso::optimizers::scdp::output_manager manager(config);

    std::vector<bso::spatial_design::ms_N_building> designs;
    std::vector<bso::utilities::data_point> performances;
    std::vector<std::string> axes = {"sd", "bp"};
    std::vector<double> loads;
    std::vector<double> exterior_area;
    std::string modification = "sd";
    std::string design_name = "design_1";
    std::string delete_method = "off";

    bso::optimizers::scdp::OptimizationOutput output{
        designs,
        performances,
        axes,
        loads,
        exterior_area,
        modification,
        design_name,
        delete_method,
        0,
        0,
        0
    };

    BOOST_REQUIRE_NO_THROW(manager.outputData(output));
}

BOOST_AUTO_TEST_SUITE_END()
