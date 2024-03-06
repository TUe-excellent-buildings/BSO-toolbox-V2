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
	ms_N_building(std::string fileName); // initilization by string or text file
	ms_N_building(const ms_N_building& rhs); // copy constructor
	~ms_N_building(); // destructor
	
	// Space analysis functions
	const std::string getInsertFileName() const{return insertFileName;}																	 
	void writeToFile(std::string fileName) const;
	std::vector<ms_space*> getSpacePtrs() const;
	ms_space* getSpacePtr(const ms_space& space) const;
	ms_space* getSpacePtr(const ms_space* spacePtr) const; // the spacePtr that was passed may belong to another instance of ms_building
	unsigned int getLastSpaceID() const;
	double getVolume() const;
	double getFloorArea() const;
	bool hasFloatingSpaces(std::vector<ms_space*>& floatingSpaces, 
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
	void addBoundary();
	
	bool operator == (const ms_N_building& rhs);
	bool operator != (const ms_N_building& rhs);
	
	friend std::ostream& operator<< (std::ostream& stream, const ms_N_building& building);
};

} // namespace spatial_design
} // namespace bso

#include <bso/spatial_design/ms_N_building.cpp>

#endif // MS_N_BUILDING_HPP