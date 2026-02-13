#ifndef PERFORMANCE_EVALUATOR_HPP
#define PERFORMANCE_EVALUATOR_HPP

#include <string>
#include <vector>

#include <bso/spatial_design/ms_N_building.hpp>
#include <bso/structural_design/sd_model.hpp>
#include <bso/building_physics/bp_model.hpp>
#include <bso/utilities/data_point.hpp>

namespace bso { namespace optimizers { namespace scdp {

class performance_evaluator {
public:
    static void getPerformancePerSpace(
        std::vector<utilities::data_point>& performances, 
        spatial_design::ms_N_building& ms, 
        structural_design::sd_model& sd, 
        building_physics::bp_model& bp, 
        const std::string& modificationType, 
        const std::string& performanceDivisionType,
        bool verbose);

    static void normalizeData(
        std::vector<utilities::data_point>& performances,
        utilities::data_point& utopianPoint,
        utilities::data_point& distopianPoint,
        bool verbose);

    static void sortDataPointsHighestToLowest(
        std::vector<utilities::data_point>& performances,
        const std::string& modificationType,
        const int iterationNr,
        const std::vector<double>& objectiveWeights,
        bool verbose);
};

}}} // namespace bso::optimizers::scdp

#include <bso/optimizers/scdp/performance_evaluator.cpp>

#endif // PERFORMANCE_EVALUATOR_HPP
