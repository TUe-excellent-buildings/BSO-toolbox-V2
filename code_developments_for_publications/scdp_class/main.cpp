#include <exception>
#include <iostream>

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
