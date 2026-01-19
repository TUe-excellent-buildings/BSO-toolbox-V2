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
	
	// Building spatial design analysis functions
	const std::string getInsertFileName() const{return insertFileName;}																	 
	void writeToFile(std::string fileName) const;
	std::vector<ms_space*> getSpacePtrs() const;
	ms_space* getSpacePtr(const ms_space& space) const;
	ms_space* getSpacePtr(const ms_space* spacePtr) const; // the spacePtr that was passed may belong to another instance of ms_building
	ms_space* getSpacePtrWithID(unsigned int id) const; // the spacePtr that was passed may belong to another instance of ms_building
	unsigned int getLastSpaceID() const;
	double getVolume() const;
	double getFloorArea() const; 
	int getNrElevations(const double mTol = 1e-3) const; // returns the number of distinct floor levels in the building, mTol is the tolerance for merging elevation levels
	double getAverageHorizontalRibSize() const; // returns the average horizontal rib size of all spaces in the building
	double getSpaceVolumeRatio() const; // returns the ratio between the smallest and largest space volume in the building
	void getSpaceVolumeBoxPlotValues(double& median, double& firstQuartile, double& thirdQuartile, double& average, std::vector<double>& outliers);// returns box plot values of space volumes in the building; space volume is times *1e-9, i.e. mm3 to m3
	double getSpaceFloorAreaRatio() const; // returns the ratio between the smallest and largest space floor area in the building
	std::vector<double> getSpaceFloorAreaBoxPlotValues(double& median, double& firstQuartile, double& thirdQuartile, double& average, std::vector<double>& outliers);
	double getAverageRecDif() const; // provide average recDif value of all spaces; Note: recDif is a measure of how rectangular a quadrilateral is, see utilities/geometry/quadrilateral.hpp for more information
	bool hasFloatingSpaces(std::vector<ms_space*>& floatingSpaces, 
		const double tol = 1e-3) const; 
	bool isFragmented(std::vector<ms_space*>& spacesNonBiggestFragment, 
			const double tol = 1e-3) const;

	// Representation adjustment functions
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