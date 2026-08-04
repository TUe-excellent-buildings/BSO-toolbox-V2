#ifndef MS_N_BUILDING_HPP
#define MS_N_BUILDING_HPP

#include <vector>
#include <map>
#include <utility>
#include <bso/spatial_design/ms_space.hpp>
#include <bso/spatial_design/sc_building.hpp>

namespace bso { namespace spatial_design {

struct shift_fill_plane_debug_data
{
	std::vector<utilities::geometry::quad_hexahedron> selectedSpaces;
	std::vector<utilities::geometry::quad_hexahedron> notSelectedSpaces;
	std::vector<utilities::geometry::quad_hexahedron> partialSpaces;

	std::vector<utilities::geometry::quadrilateral> toughingSQuadsPreDedup;
	std::vector<utilities::geometry::quadrilateral> toughingNSQuadsPreDedup;
	std::vector<utilities::geometry::quadrilateral> toughingSQuadsPostDedup;
	std::vector<utilities::geometry::quadrilateral> toughingNSQuadsPostDedup;
	std::vector<utilities::geometry::quadrilateral> extrudingQuadsPostSplit;

	std::vector<utilities::geometry::line_segment> linesAtFloorFlattened;
	std::vector<utilities::geometry::quad_hexahedron> linesAtFloorReferenceSpaces;

	std::vector<utilities::geometry::quad_hexahedron> shiftedSpaces;
};

struct extrude_debug_data
{
	std::vector<utilities::geometry::quad_hexahedron> selectedSpaces;
	std::vector<utilities::geometry::quad_hexahedron> notSelectedSpaces;
	std::vector<utilities::geometry::quad_hexahedron> partialSpaces;

	std::vector<utilities::geometry::quadrilateral> fullyOnExtrusionSideQuads;
	std::vector<utilities::geometry::quadrilateral> partiallyOnExtrusionSideQuads;
	std::vector<utilities::geometry::quadrilateral> notOnExtrusionSideQuads;

	std::vector<utilities::geometry::quad_hexahedron> testSpaces;
	std::vector<utilities::geometry::quad_hexahedron> validTestSpaces;
	std::vector<utilities::geometry::quad_hexahedron> rejectedTestSpaces;
};

struct shift_fill_plane_centre_debug_data
{
	bso::utilities::geometry::vertex location;
	bso::utilities::geometry::vector mostAzimuth;
	bso::utilities::geometry::vector secondMostAzimuth;
	bso::utilities::geometry::vector shiftDirection;

	std::vector<utilities::geometry::quad_hexahedron> selectedSpaces;
	std::vector<utilities::geometry::quad_hexahedron> notSelectedSpaces;
	std::vector<utilities::geometry::quad_hexahedron> partialSpaces;

	bool usedShiftSelectedSide = false;
	bool usedShiftNonSelectedSide = false;
	bool usedExtrude = false;
	bool flippedExtrudeDirection = false;

	bso::utilities::geometry::vector appliedDirection;

	std::string selectedSideShiftError;
	std::string nonSelectedSideShiftError;

	shift_fill_plane_debug_data selectedSideShiftDebug;
	shift_fill_plane_debug_data nonSelectedSideShiftDebug;
	extrude_debug_data extrudeDebug;
};

class ms_N_building
{
private:
	std::vector<ms_space*> mSpaces;
	mutable unsigned int mLastSpaceID;
	void checkValidity() const;
	std::string insertFileName;
public:
	ms_N_building(); // empty constructor
	ms_N_building(std::string fileName); // initialization by string or text file
	ms_N_building(const ms_N_building& rhs); // copy constructor
	~ms_N_building(); // destructor
	
	// Building spatial design analysis functions:
	const std::string getInsertFileName() const{return insertFileName;}																	 
	void writeToFile(std::string fileName) const;
	std::vector<ms_space*> getSpacePtrs() const;
	ms_space* getSpacePtr(const ms_space& space) const;
	ms_space* getSpacePtr(const ms_space* spacePtr) const; // the spacePtr that was passed may belong to another instance of ms_building
	ms_space* getSpacePtrWithID(unsigned int id) const; // the spacePtr that was passed may belong to another instance of ms_building
	unsigned int getLastSpaceID() const;
	double getVolume() const;
	double getFloorArea() const;
	int getNrElevations(const double mTol = 1e-3) const;
	double getAverageHorizontalRibSize() const;
	double getSpaceVolumeRatio() const;
	void getSpaceVolumeBoxPlotValues(double& median, double& firstQuartile, double& thirdQuartile, double& average, std::vector<double>& outliers) const;
	double getSpaceFloorAreaRatio() const;
	std::vector<double> getSpaceFloorAreaBoxPlotValues(double& median, double& firstQuartile, double& thirdQuartile, double& average, std::vector<double>& outliers) const;
	double getAverageRecDif() const;
	bool hasFloatingSpaces(std::vector<ms_space*>& floatingSpaces, 
		const double tol = 1e-3) const; 
	bool isFragmented(std::vector<ms_space*>& spacesNonBiggestFragment, 
		const double tol = 1e-3) const;
	bool getOverlappingSpaces(std::vector<ms_space*>& overlappingSpaces, 
		std::vector<ms_space*>& notOverlappingSpaces,
	 	const double tol = 1e-3) const;

	// Get the NOBSD representation as a certain geometry:
	void getNOBSDAtGround(std::vector<std::pair<bso::spatial_design::ms_space*, utilities::geometry::quad_hexahedron>>& spacePair, 
		std::vector<double>& allLevels, const double mTol = 1e-3);
	void getNOBSDAtGround(std::vector<std::pair<bso::spatial_design::ms_space*, utilities::geometry::quad_hexahedron>>& spaceAtGroundV, const double mTol = 1e-3);
	void getSpacesAtGround(std::vector<bso::spatial_design::ms_space*> spacePtr, std::vector<std::pair<bso::spatial_design::ms_space*, utilities::geometry::quad_hexahedron>>& spacePair, 
		std::vector<double>& allLevels, const double mTol = 1e-3);
	void getDelaunayVariables(std::vector <utilities::geometry::vertex>& delaunayPoints, std::vector <utilities::geometry::line_segment>& delaunayLines,
		std::vector<std::pair<bso::spatial_design::ms_space*, utilities::geometry::quad_hexahedron>>& spacePair, const double mTol = 1e-3); // Note: this functions uses the results from getXAtGround to retrieve the needed geometry information from.
	std::vector <bso::utilities::geometry::quadrilateral> getQuadAtZ0(std::vector<bso::spatial_design::ms_space*> spaces, const double mTol = 1e-3);
	std::vector <bso::utilities::geometry::line_segment> getLinesAtZ0(std::vector<bso::spatial_design::ms_space*> spaces, const double mTol = 1e-3);
	bso::utilities::geometry::vertex getCentreBuilding();
	void getMostAzimuthDirection(bso::utilities::geometry::vector& mostAzimuth, bso::utilities::geometry::vector& secondMostAzimuth, int amountOfAzimuthCategories = 6);

	// Representation adjustment functions:
	void setZZero(); 
	void resetOrigin(const std::vector<unsigned int>& indices = {0,1,2});
	void addSpace(const ms_space& space); 
	void deleteSpace(ms_space* spacePtr);
	void deleteSpace(ms_space& space);
	void scale(const std::vector<std::pair<unsigned int, double> >& scales = {{0,sqrt(2.0)},{1,sqrt(2.0)}}); 
	void splitSpace(ms_space* spacePtr, const std::vector<std::pair<unsigned int, unsigned int> >& splits = {{0,2},{1,2}}); // split perpendicular to inserted axis; x-axis=0, y-axis=1, and z-axis=2 
	void splitSpace(ms_space& space, const std::vector<std::pair<unsigned int, unsigned int> >& splits = {{0,2},{1,2}}); // split perpendicular to inserted axis; x-axis=0, y-axis=1, and z-axis=2
	void splitSpace(ms_space* spacePtr, int splitAmount, bso::utilities::geometry::line_segment& lineToSplit); //split perpendicular to inserted line of space
	void splitSpace(ms_space& space, int splitAmount, bso::utilities::geometry::line_segment& lineToSplit); //split perpendicular to inserted line of space
	void duplicateElevation(double atZFromGround = 0, double elevationHeight = 3000, bool duplicateBelowZLevel = true, const double& tol = 1e-3); // duplicate either all spaces above or below a certain z-level by a certain elevation height in z-direction and moves the above spaces up to create a new elevation.
	void shiftFillPlaneCentre(double shiftAmount, bool shiftInMostHeadingDirection = false, int amountOfHeadingCatagories = 6, bool verbose = false,
		const double& tol = 1e-3, shift_fill_plane_centre_debug_data* debugData = nullptr);
	std::vector<bso::spatial_design::ms_space*> shiftFillPlane(const bso::utilities::geometry::vertex& location,
		const bso::utilities::geometry::vector& shiftDirection, double shiftAmount, const double& tol = 1e-3, bool verbose = false,
		shift_fill_plane_debug_data* debugData = nullptr);
	std::vector<bso::spatial_design::ms_space*> shiftFillPlane(const bso::utilities::geometry::vertex& location,
		const bso::utilities::geometry::vector& shiftDirection, double shiftAmount, const bool extrudeFromSelectedSide, double minimalSpaceAngle = 20.0,
		const double& tol = 1e-3, bool verbose = false, shift_fill_plane_debug_data* debugData = nullptr);
	void extrude(const bso::utilities::geometry::vertex& location, const bso::utilities::geometry::vector& extrudeDirection, 
		double extrudeAmount, bool verbose = false, const double& tol = 1e-3, extrude_debug_data* debugData = nullptr);
	void extrude(std::vector<ms_space*> spacePtrs, const bso::utilities::geometry::vertex& location, 
		const bso::utilities::geometry::vector& extrudeDirection, double extrudeAmount, bool verbose = false, 
		const double& tol = 1e-3, extrude_debug_data* debugData = nullptr);
	void enforceAverageSpaceSize(double minElevationHeight = 3000, const double& tol = 1e-3, bool verbose = false);

	// Space selection functions:
	std::vector<ms_space*> selectSpacesGeometrically(
		const bso::utilities::geometry::vertex& location,
		const bso::utilities::geometry::vector& normalOfPlane,
		const bool includePartialSpaces = false,
		const bool returnNormalDefinedSide = false) const;
	std::vector<ms_space*> selectSpacesGeometrically(
		const bso::utilities::geometry::vertex& location,
		const bso::utilities::geometry::vector& normalOfPlane,
		std::vector<ms_space*>& notSelectedSpaces,
		std::vector<ms_space*>& partialSpaces,
		const bool includePartialSpaces = false,
		const bool returnNormalDefinedSide = false) const;

	const auto begin() const {return mSpaces.begin();}
	const auto end() const {return mSpaces.end();}
	
	// Translational function to Leiden
	void addBoundary(); // Add a boundary line and uses the conformal making process for making a Prism-Net representation
	std::vector<utilities::geometry::triangular_prism> convertMSToPN(std::string fileName, double mTol = 0.001); // Uses less triangles and more spread to make a prism-net (PN) representation from Leiden
	
	bool operator == (const ms_N_building& rhs);
	bool operator != (const ms_N_building& rhs);
	ms_N_building& operator=(const ms_N_building& rhs); // Assignment operator, needed for proper copy of mSpaces when assigning content of an object to another object
	//ms_N_building& operator=(ms_N_building&& rhs) noexcept; // Move assignment operator

	friend std::ostream& operator<< (std::ostream& stream, const ms_N_building& building);
};

} // namespace spatial_design
} // namespace bso

#include <bso/spatial_design/ms_N_building.cpp>

#endif // MS_N_BUILDING_HPP