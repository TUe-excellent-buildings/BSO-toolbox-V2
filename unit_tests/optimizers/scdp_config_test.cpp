#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE "scdp_config"
#endif

#include <boost/test/included/unit_test.hpp>

#include <bso/optimizers/scdp/scdp_config.hpp>

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

BOOST_AUTO_TEST_SUITE_END()
