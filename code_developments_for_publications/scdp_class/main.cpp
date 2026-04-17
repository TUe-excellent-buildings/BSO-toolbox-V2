#include <exception>
#include <iostream>

#include <bso/spatial_design/ms_N_building.hpp>
#include <bso/spatial_design/cf_building.hpp>
#include <bso/utilities/geometry.hpp>
#include <bso/utilities/data_point.hpp>
#include <bso/utilities/clustering.hpp>
#include <bso/utilities/data_export.hpp>
#include <bso/structural_design/sd_model.hpp>
#include <bso/building_physics/bp_model.hpp>
#include <bso/daylight/dl_model.hpp>
#include <bso/grammar/grammar.hpp>
#include <bso/visualization/visualization.hpp>

#include <bso/optimizers/scdp/scdp_config.hpp>
#include <bso/optimizers/scdp/scdp_optimizer.hpp>

int main(int argc, char* argv[])
{
    try
    {
        bso::optimizers::scdp::scdp_config config(argc, argv);
        bso::optimizers::scdp::scdp_optimizer optimizer(config);
        optimizer.runOptimization();

        if(config.getVerbose()){std::cout << "Optimization completed successfully." << std::endl;}
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "SCDP optimizer failed: " << e.what() << std::endl;
        return 1;
    }
}
