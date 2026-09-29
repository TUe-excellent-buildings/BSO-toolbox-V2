#ifndef BSO_DAYLIGHT_SIMULATIONS_GRID_POINT_HPP
#define BSO_DAYLIGHT_SIMULATIONS_GRID_POINT_HPP

namespace bso::daylight::simulations {

class GridPoint
{
private:
    bso::utilities::geometry::vertex gridPoint; // x, y, z coordinates of the grid point
    unsigned int spaceID; // ID of the space this grid point belongs to
    unsigned int ruleOfGridTxt; // To locate the rule in the grid.txt file that corresponds to this grid point, so we can link the results of the simulation to this grid point and its space.
    
    // Results of daylight simulation at this grid point
    bool generatedResults = false; // flag to check if results have been generated for this grid point
    double DFValue = 0.0; // Daylight Factor []
    double illuminance = 0.0; // Illuminance []
public:
    GridPoint();
    GridPoint(const bso::utilities::geometry::vertex& point);
    ~GridPoint() = default;

    // Set results after simulation
    void setResults();

    // Getter functions
    bso::utilities::geometry::vertex getGridPoint() const { return gridPoint; }
    unsigned int getSpaceID() const { return spaceID; }
    unsigned int getRuleOfGridTxt() const { return ruleOfGridTxt; }
    double getDFValue() const { return DFValue; }
    double getIlluminance() const { return illuminance; }
};

} // namespace bso::daylight::simulations

#include <bso/daylight/simulations/grid_point.cpp>

#endif // BSO_DAYLIGHT_SIMULATIONS_GRID_POINT_HPP