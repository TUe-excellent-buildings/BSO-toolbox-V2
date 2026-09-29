#ifndef BSO_DAYLIGHT_SPACE_HPP
#define BSO_DAYLIGHT_SPACE_HPP

#include <string>


namespace bso { namespace daylight {

	class space : public bso::daylight::properties::space_properties
	{
	private:
		unsigned int mSpaceID = 0;    
		
		// Geometry elements (as defined in the conformal model) belonging to this space.
		std::vector<> ceilingRectangles;
		std::vector<> ceilingTriangles;
		std::vector<> floorRectangles;
		std::vector<> floorTriangles;
		std::vector<> wallRectangles;
		std::vector<> windowRectangles;
		
		bso::utilities::geometry::quadrilateral floorSurface; // for grid generation, we need the floor surface as one quadrilateral, so we can apply the NEN grid generation rules on it.
		std::vector<> gridPoints; // Stores the generated grid points for this space, which are generated in the function generateGridNEN() and can be used for visualization and for the daylight calculation.
		double p_meter = 0.0; // grid point density per mm, calculated during generateGridNEN()

	public:
		space() = default;
		space(const unsigned int spaceID, const bool isExternal = false, const bool mIsOpen = false);

		void generateGridNEN(const double ReferenceHeight, const double space_margin, const double maxGridSize);
		
		unsigned int getSpaceID() const {return mSpaceID;}
		double getPmeter() const {return p_meter;}
		const std::vector<>& getGridPoints() const {return gridPoints;}

		void setSpaceID(const unsigned int spaceID) {mSpaceID = spaceID;}
	};

} // namespace daylight
} // namespace bso

#include <bso/daylight/geometry/space.cpp>

#endif // BSO_DAYLIGHT_SPACE_HPP
