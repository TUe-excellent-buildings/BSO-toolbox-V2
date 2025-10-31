#ifndef MS_SPACE_HPP
#define MS_SPACE_HPP

#include <vector>
#include <string>
#include <bso/utilities/geometry.hpp>

namespace bso { namespace spatial_design {

class ms_space
{
private:
	utilities::geometry::vertex mCoordinates; // {x,y,z}; is only in use with the R-insertion method
	utilities::geometry::vector mDimensions; // {width,depth,height}; is only in use with the R-insertion method
	std::vector <utilities::geometry::vertex*> pVertex; //Contains all eight vertices to represent the non-orthogonal space in {x,y,z}. Note: is not in use during R-definition of space.
	
	unsigned int mID; // Identification number of the space
	std::string sDefMethod; // A space definition method; orthogonal ('R' or 'r') or non-orthogonal ('N' or 'n')
	std::string mSpaceType; // a type which can be defined by a user and which can subsequently be assigned to the space
	std::vector<std::string> mSurfaceTypes; // each surface can also be assigned a user defined type. Convention: {+y,+x,-y,-x,+z,-z}/{north,east,south,west,top,bottom}
	void reset(); // resets the space to initial values
	bool checkValidity() const; // check if the space is valid
public:
	ms_space(); // empty constructor
	ms_space(unsigned int id, utilities::geometry::vertex coords, utilities::geometry::vector dim, const std::string& spaceType = std::string(), const std::vector<std::string>& 	surfaceTypes = std::vector<std::string>()); // constructor with initialization
	ms_space(unsigned int id, std::vector <utilities::geometry::vertex*> pVert, const std::string& spaceType = std::string(), const std::vector<std::string>& surfaceTypes = std::vector<std::string>()); // constructor with initialization																																													 
	ms_space(std::string line); // constructor from input line
	ms_space(const ms_space& rhs); // copy constructor
	~ms_space(); // destructor
	
	void setPVertex(const std::vector <utilities::geometry::vertex*> vertexes);
	void setCoordinates(const utilities::geometry::vertex& coords);
	void setDimensions(const utilities::geometry::vector& dims);
	void setID(const unsigned int& id);
	
	unsigned int getID() const; // returns the ID of the space
	utilities::geometry::vertex getCoordinates() const; // return the coordinates of the space
	utilities::geometry::vector getDimensions() const; // return the dimensions of the space
	std::vector<utilities::geometry::vertex*> getVertices() const; // return the vertices of the space
	const std::string getSDefMethod() const { return sDefMethod;}										
	utilities::geometry::quad_hexahedron getGeometry() const; // return the geometry of the space
	bool getSpaceType(std::string& spaceType) const; // returns TRUE if space type is assigned FALSE if not. Assigns the space type to the argument that is passed by reference
	bool getSurfaceTypes(std::vector<std::string>& surfaceTypes) const; // returns TRUE is surface types are assigned FALSE if not. Assigns the surface types to the argument that is passed by reference
	double getVolume() const; // return the volume of the space in *1e-9 m3, so in case of mm3 it returns m3
	double getFloorArea() const;
	double getSurfaceArea() const;
	
	bool operator == (const ms_space& rhs) const;
	bool operator != (const ms_space& rhs) const;
	
	friend std::ostream& operator<< (std::ostream& stream, const ms_space& space);
};

} // namespace spatial_design
} // namespace bso

#include <bso/spatial_design/ms_space.cpp>

#endif // MS_SPACE_HPP