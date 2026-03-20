#ifndef BSO_DAYLIGHT_SIMULATIONS_GRID_POINT_CPP
#define BSO_DAYLIGHT_SIMULATIONS_GRID_POINT_CPP

namespace bso::daylight::simulations {

GridPoint::GridPoint()
{

}

GridPoint::GridPoint(const bso::utilities::geometry::vertex& point)
    : gridPoint(point)
{

}

void GridPoint::setResults()
{
    // Implementation for setting simulation results
    


    // Results generated for this grid point
    generatedResults = true;
}

} // namespace bso::daylight::simulations

#endif // BSO_DAYLIGHT_SIMULATIONS_GRID_POINT_HPP