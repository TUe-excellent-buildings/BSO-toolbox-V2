#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE "output_manager"
#endif

#include <boost/test/included/unit_test.hpp>

#include <bso/optimizers/scdp/output_manager.hpp>
#include <bso/optimizers/scdp/scdp_config.hpp>
#include <bso/spatial_design/ms_N_building.hpp>
#include <bso/utilities/data_point.hpp>

#include <filesystem>
#include <initializer_list>
#include <string>
#include <vector>

namespace {
struct ArgvBuilder {
    std::vector<std::string> args;
    std::vector<char*> cargs;

    ArgvBuilder(std::initializer_list<const char*> init)
    {
        args.reserve(init.size());
        cargs.reserve(init.size());
        for (const auto* item : init) {
            args.emplace_back(item);
        }
        for (auto& item : args) {
            cargs.push_back(&item[0]);
        }
    }

    int argc() const { return static_cast<int>(args.size()); }
    char** argv() { return cargs.data(); }
};
} // namespace

BOOST_AUTO_TEST_SUITE(output_manager_test)

BOOST_AUTO_TEST_CASE(set_and_get_result_dir)
{
    ArgvBuilder argv({"prog", "-i", "design_1", "-m", "sd"});
    bso::optimizers::scdp::scdp_config config(argv.argc(), argv.argv());
    bso::optimizers::scdp::output_manager manager(config);

    manager.setResultDir("results/test/");
    BOOST_CHECK_EQUAL(manager.getResultDir(), "results/test/");
}

BOOST_AUTO_TEST_CASE(create_result_dir_sets_path)
{
    ArgvBuilder argv({"prog", "-i", "design_1", "-m", "sd"});
    bso::optimizers::scdp::scdp_config config(argv.argc(), argv.argv());
    bso::optimizers::scdp::output_manager manager(config);

    const bool existed_before = std::filesystem::exists("results");

    std::string dir;
    bool created = manager.createResultDir(dir, false);

    BOOST_CHECK_EQUAL(created, true);
    BOOST_CHECK_EQUAL(dir, "results/");

    if (!existed_before) {
        std::error_code ec;
        std::filesystem::remove("results", ec);
    }
}

BOOST_AUTO_TEST_CASE(output_data_with_no_outputs_does_not_throw)
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
        config,
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
