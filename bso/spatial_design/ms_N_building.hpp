#ifndef MS_N_BUILDING_HPP
#define MS_N_BUILDING_HPP

#include <vector>
#include <map>
#include <utility>
#include <bso/spatial_design/ms_space.hpp>
#include <bso/spatial_design/sc_building.hpp>

namespace bso { namespace spatial_design {

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
	unsigned int getLastSpaceID() const;
	double getVolume() const;
	double getFloorArea() const;
	double getAverageHorizontalRibSize() const;
	double getSpaceVolumeRatio() const;
	double getSpaceFloorAreaRatio() const;
	double getAverageRecDif() const;
	bool hasFloatingSpaces(std::vector<ms_space*>& floatingSpaces, 
		const double tol = 1e-3) const; 
	bool isFragmented(std::vector<ms_space*>& spacesNonBiggestFragment, 
		const double tol = 1e-3) const;
	// bool areSpacesOverlapping(std::vector<ms_space*>& spacesNotOverlapping,
	// 	const double tol = 1e-3) const;

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
	void getMostHeadingDirection(bso::utilities::geometry::vector& mostHeading, bso::utilities::geometry::vector& secondMostHeading, int amountOfHeadingCatagories = 6, const double& tol = 1e-3);

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
	
	// New functions:
	//void splitBuilding(double minimumHeight, double minimumWidth, double minimumDepth); // under development
	//void enforceSpaceSize(double minimumHeight, double minimumWidth, double minimumDepth);
	void duplicateElevation(double atZFromGround = 0, double elevationHeight = 3000, bool duplicateBelowZLevel = true);
	void shiftFillPlaneCentre(double shiftAmount, bool shiftInMostHeadingDirection = false, int amountOfHeadingCatagories = 6, const double& tol = 1e-3);
	void shiftFillPlane(const bso::utilities::geometry::vertex& location,
		const bso::utilities::geometry::vector& shiftDirection, double shiftAmount, const double& tol = 1e-3);
	void extrude(std::vector<ms_space*> spacePtrs, const bso::utilities::geometry::vertex& location, 
		const bso::utilities::geometry::vector& shiftDirection, double shiftAmount, const double& tol = 1e-3);
	void enforceAverageSpaceSize(double minElevationHeight = 3000, const double& tol = 1e-3);
	//void sweep(const bso::utilities::geometry::vertex& location,
	//	const std::vector<std::pair<unsigned int, double> >& distances =
	//	{{0,1.0},{1,0.0},{2,0.0}}, const double& tol = 1e-3);
	//void cutOff(const bso::utilities::geometry::vertex& location,
	//	const bso::utilities::geometry::vector& direction, const double& tol = 1e-3);
	//void rotate();
	//void rotate(bso::utilities::vector axis);
	//void rotate(std::vector<ms_space*> mSpaces);
	//void rotate(std::vector<ms_space*> mSpaces, bso::utilities::vector axis);
	//void stretchAt();
	void replicateSplits(std::vector<ms_space*> spaces); // under development
	//void recenterBuilding();

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
	
	// Translational function to Leiden representation via the geometry conformal model
	void addBoundary();
	
	bool operator == (const ms_N_building& rhs);
	bool operator != (const ms_N_building& rhs);
	
	friend std::ostream& operator<< (std::ostream& stream, const ms_N_building& building);
};

} // namespace spatial_design
} // namespace bso

#include <bso/spatial_design/ms_N_building.cpp>

#endif // MS_N_BUILDING_HPP