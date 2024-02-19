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
	
	const std::string getInsertFileName() const{return insertFileName;}																	 
	void writeToFile(std::string fileName) const; // Check function!
	std::vector<ms_space*> getSpacePtrs() const;
	ms_space* getSpacePtr(const ms_space& space) const;
	ms_space* getSpacePtr(const ms_space* spacePtr) const; // the spacePtr that was passed may belong to another instance of ms_building
	unsigned int getLastSpaceID() const;
	double getVolume() const;
	double getFloorArea() const;
	
	//std::vector<ms_space*> selectSpacesGeometrically(
		//const bso::utilities::geometry::vertex& location,
		//const bso::utilities::geometry::vector& direction,
		//const bool includePartialSpaces = false) const; // Not updated for NOBSD and might not be since it is part of the space bound adjustments.
		
	void setZZero(); // Check function!
	void resetOrigin(const std::vector<unsigned int>& indices = {0,1,2}); // Not updated for NOBSD and might not be since it is part of the space bound adjustments.
	void addSpace(const ms_space& space); // Check function!
	void deleteSpace(ms_space* spacePtr);
	void deleteSpace(ms_space& space);
	//void cutOff(const bso::utilities::geometry::vertex& location,
		//const bso::utilities::geometry::vector& direction, const double& tol = 1e-3); // Not updated for NOBSD and might not be since it is part of the space bound adjustments.
	//void sweep(const bso::utilities::geometry::vertex& location,
		//const std::vector<std::pair<unsigned int, double> >& distances =
		//{{0,1.0},{1,0.0},{2,0.0}}, const double& tol = 1e-3); // Not updated for NOBSD and might not be since it is part of the space bound adjustments.
	void scale(const std::vector<std::pair<unsigned int, double> >& scales = {{0,sqrt(2.0)},{1,sqrt(2.0)}}); // Check function!
	void splitSpace(ms_space* spacePtr, const std::vector<std::pair<unsigned int, unsigned int> >& splits = {{0,2},{1,2}}); // Check function!
	void splitSpace(ms_space& space, const std::vector<std::pair<unsigned int, unsigned int> >& splits = {{0,2},{1,2}}); // Check function!
	void snapOn(const std::vector<std::pair<unsigned int, double> >& snaps = {{0,10},{1,10}}); // Check function!
	
	int addBoundary();

	const auto begin() const {return mSpaces.begin();}
	const auto end() const {return mSpaces.end();}
	
	//bool hasOverlappingSpaces(std::multimap<ms_space*,ms_space*>& overlappingSpaces,
		//const double tol = 1e-3) const; // Not updated for NOBSD and might not be since it is part of the space bound adjustments.
	bool hasFloatingSpaces(std::vector<ms_space*>& floatingSpaces, 
		const double tol = 1e-3) const; // Check function!
	
	bool operator == (const ms_N_building& rhs);
	bool operator != (const ms_N_building& rhs);

	// operator sc_building() const; // convert MS to SC; Not updated for NOBSD
	
	friend std::ostream& operator<< (std::ostream& stream, const ms_N_building& building);
};

} // namespace spatial_design
} // namespace bso

#include <bso/spatial_design/ms_N_building.cpp>

#endif // MS_N_BUILDING_HPP