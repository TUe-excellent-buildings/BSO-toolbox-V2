#ifndef MS_N_BUILDING_CPP
#define MS_N_BUILIDNG_CPP

#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <utility>
#include <stdexcept>
#include <exception>
#include <boost/algorithm/string.hpp>
#include <bitset>

namespace bso { namespace spatial_design {

ms_N_building::ms_N_building()
{ // empty constructor
	mLastSpaceID = 0;
} // ms_building() (empty constructor)

ms_N_building::ms_N_building(std::string fileName)
:	insertFileName(fileName)
{ // initilization by string or text file
	mLastSpaceID = 0;
	std::ifstream input;
	if (!fileName.empty()) input.open(fileName.c_str());
	
	if (fileName.empty() || !input.is_open())
	{
		std::stringstream errorMessage;
		errorMessage << "Could not initialize an MS_N building spatial design with the following input file:" << std::endl
								 << ((fileName.empty())? "no input file given" : fileName) << std::endl
								 << "(bso/spatial_design/ms_N_building.cpp). " << std::endl;
		throw std::invalid_argument(errorMessage.str());
	}
	
	std::string line;

	while (!input.eof()) // Parse the input file line by line
	{
		getline(input,line); // get next line from the file
		try
		{
			boost::algorithm::trim(line); // remove white space from start and end of line (to see if it is an empty line, removes any incidental white space)
			if (line == "") //skip empty lines (tokenizer does not like it)
			{
				continue; // continue to next line
			}
			else if (line.substr(0,2) == "N," || line.substr(0,2) == "n,")
			{
				mSpaces.push_back(new ms_space(line));																  
			}
			else if (line.substr(0,2) == "R," || line.substr(0,2) == "r,") /* R definition method is not allowed*/
			{
				std::stringstream errorMessage;
				errorMessage << "Encountered an 'R' or 'r' definition in the MS input file, this is not allowed. See line:" << std::endl
										<< line << std::endl
										<< "(bso/spatial_design/ms_N_building.cpp)." << std::endl;
				throw std::invalid_argument(errorMessage.str());
			}
			else
			{
				continue;
			}
		}
		catch(std::exception& e)
		{
			std::stringstream errorMessage;
			errorMessage << "Encountered an error while parsing the following line of an MS input file:" << std::endl
									 << line << std::endl
									 << "(bso/spatial_design/ms_N_building.cpp). Got the following error: " << std::endl
									 << e.what() << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
	}
	if (mSpaces.size() == 0)
	{
		std::stringstream errorMessage;
		errorMessage << "Could not initialize an MS_N building spatial design with the following input file:" << std::endl
								 << ((fileName.empty())? "the file did not contain a valid definition of a space" : fileName) << std::endl
								 << "(bso/spatial_design/ms_N_building.cpp). " << std::endl;
		throw std::invalid_argument(errorMessage.str());
	}
	
	mLastSpaceID = this->getLastSpaceID();
	this->checkValidity();
} // ms_N_building() (constructor using input file)

ms_N_building::ms_N_building(const ms_N_building& rhs)
{ // copy constructor
	mLastSpaceID = rhs.mLastSpaceID;
	for (auto i : rhs.mSpaces)
	{
		mSpaces.push_back(new ms_space(*i));
	}
	
	insertFileName = rhs.getInsertFileName();
	this->checkValidity();
} // ms_N_building() (copy constructor)

ms_N_building::~ms_N_building()
{ // destructor
	for (auto i : mSpaces) delete i;
} // ~ms_N_building()

void ms_N_building::checkValidity() const
{
	
} // checkValidity()

void ms_N_building::writeToFile(std::string fileName) const
{
	std::ofstream output;
	if (!fileName.empty()) output.open(fileName.c_str());
	
	if (fileName.empty() || !output.is_open())
	{
		std::stringstream errorMessage;
		errorMessage << "Could not open the following file to write an ms_N building spatial design to:" << std::endl
								 << ((fileName.empty())? "no input file given" : fileName) << std::endl
								 << "(bso/spatial_design/ms_N_building.cpp). " << std::endl;
		throw std::invalid_argument(errorMessage.str());
	}
	
	output << *this;
} // writeToFile()

std::vector<ms_space*> ms_N_building::getSpacePtrs() const
{
	return mSpaces;
} //getSpacePtrs()

ms_space* ms_N_building::getSpacePtr(const ms_space& space) const
{
	for (auto i : mSpaces)
	{
		if (/*i == &space ||*/ *i == space)
		{
			return i;
		}
	}
	
	// if this part of code is reached, the space was not found in the preceding for loop
	std::stringstream errorMessage;
	errorMessage << "Could not find the following space in an ms_N building spatial design:" << std::endl
							 << space << std::endl
							 << "(bso/spatial_design/ms_N_building.cpp). " << std::endl;
	throw std::runtime_error(errorMessage.str());
} // getSpacePtr()

ms_space* ms_N_building::getSpacePtr(const ms_space* spacePtr) const
{
	return getSpacePtr(*spacePtr);
}

unsigned int ms_N_building::getLastSpaceID() const
{
	for (auto i : mSpaces)
	{
		if (mLastSpaceID < i->getID()) mLastSpaceID = i->getID();
	}
	return mLastSpaceID;
} // getLastSpaceID()

double ms_N_building::getVolume() const
{
	double volume = 0;
	for (auto i : mSpaces) volume += i->getVolume();
		
	return volume;
} // getVolume()

double ms_N_building::getFloorArea() const
{
	double area = 0;
	for (const auto& i : mSpaces) area += i->getFloorArea();
	return area;
}

/*
std::vector<ms_space*> ms_N_building::selectSpacesGeometrically(
		const bso::utilities::geometry::vertex& location,
		const bso::utilities::geometry::vector& direction,
		const bool includePartialSpaces) const
{
	std::vector<ms_space*> spaceSelection;
	for (const auto& i : mSpaces)
	{
		bool allVerticesSelected = true;
		bool oneOrMoreVerticesSelected = false;
		for (const auto& j : i->getGeometry())
		{
			double checkValue = direction.dot(j - location);
			if (checkValue >= -1e-3)
			{ // check if point j lies behind the plane defined by the location and the direction
				if (checkValue >= 1e-3) oneOrMoreVerticesSelected = true;
			}
			else
			{
				allVerticesSelected = false;
			}
		}
		if (allVerticesSelected) spaceSelection.push_back(i);
		else if (oneOrMoreVerticesSelected && includePartialSpaces) spaceSelection.push_back(i);
	}
	return spaceSelection;
}
*/

void ms_N_building::setZZero()
{
	this->resetOrigin({2});
} // setZZero()

void ms_N_building::resetOrigin(const std::vector<unsigned int>& indices /*= {0,1,2}*/)
{
	/*
	utilities::geometry::vector coordDifference = {0,0,0};
	for (const auto& index : indices)
	{
		double min = (*((mSpaces[0]->getVertices())[0]))(index); // set an initial value to the minimum
	
		// find the minimum value of the z-coordinates in the building
		for (auto i : mSpaces) 
		{
			std::vector <utilities::geometry::vertex*> vertices = i->getVertices();
			for(const auto j: i->getVertices())
			{
				if (min > (*j)(index)) min = (*j)(index);
			}
		}			

		coordDifference(index) -= min;
	}
	
	if (coordDifference(2) > 0) coordDifference(2) = 0;
	
	for (auto i : mSpaces) 
	{
		// Define space new coördinates
		std::vector <utilities::geometry::vertex*> vertices = i->getVertices();
		for(int j=0; j< vertices.size(); j++)
		{
			*vertices[j] = *vertices[j] + coordDifference;
		}

		// Set space vertices
		i->setPVertex(vertices);
	}
	*/
	// ms_building origonal code for resertOrgin:
	utilities::geometry::vector coordDifference = {0,0,0};
	for (const auto& index : indices)
	{
		double min = mSpaces[0]->getCoordinates()(index); // set an initial value to the minimum
	
		// find the minimum value of the z-coordinates in the building
		for (auto i : mSpaces) 
		{
			double coord = i->getCoordinates()(index);
			if (min > coord) min = coord;
		}			

		coordDifference(index) -= min;
	}
	
	if (coordDifference(2) > 0) coordDifference(2) = 0;
	
	for (auto i : mSpaces) i->setCoordinates(i->getCoordinates() + coordDifference);
	//this->snapOn({{0,1},{1,1},{2,1}});
	
} // resetOrigin()

void ms_N_building::addSpace(const ms_space& space)
{
	if(space.getSDefMethod() == "N" || space.getSDefMethod() == "n")
	{
		try
		{
			mSpaces.push_back(new ms_space(space));
			checkValidity();
		}
		catch(std::exception& e)
		{
			std::stringstream errorMessage;
			errorMessage << "Could not add the following space to an ms_N building spatial design: " << std::endl
									<< space << std::endl
									<< "(bso/spatial_design/ms_N_building.cpp). Got the following error message: " << std::endl
									<< e.what() << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
	}
	else
	{
		std::stringstream errorMessage;
				errorMessage << "Encountered an unknown or not allowed definition method in the MS input file, this is not allowed. See def indicator:" << std::endl
										<< space.getSDefMethod() << std::endl
										<< "(bso/spatial_design/ms_N_building.cpp)." << std::endl;
				throw std::invalid_argument(errorMessage.str());
	}
} // addSpace

void ms_N_building::deleteSpace(ms_space* spacePtr)
{
	if (std::find(mSpaces.begin(), mSpaces.end(), spacePtr) == mSpaces.end()) spacePtr = getSpacePtr(spacePtr);
	try
	{
		mSpaces.erase(std::remove(mSpaces.begin(), mSpaces.end(), spacePtr), mSpaces.end());
	}
	catch(std::exception& e)
	{
		std::stringstream errorMessage;
		errorMessage << "Could not delete the following space from an ms_N building spatial design: " << std::endl
								 << *spacePtr << std::endl
								 << "(bso/spatial_design/ms_N_building.cpp). Got the following error message: " << std::endl
								 << e.what() << std::endl;
		throw std::invalid_argument(errorMessage.str());
	}
} // deleteSpace()

void ms_N_building::deleteSpace(ms_space& space)
{
	this->deleteSpace(this->getSpacePtr(space));
}

/*
void ms_N_building::cutOff(const bso::utilities::geometry::vertex& location,
		const bso::utilities::geometry::vector& direction, const double& tol)
{
	// normalize the direction for convenience of later use
	auto n = direction.normalized();
	// check if the cutting plane is orhtogonal
	bool isOrthogonal = false;
	for (unsigned int i = 0; i < 3; ++i)
	{
		bso::utilities::geometry::vector check = {0,0,0};
		check(i) = 1;
		if (check.isParallel(n, tol))
		{
			isOrthogonal = true;
			break;
		}
	}
	if (!isOrthogonal)
	{
		std::stringstream errorMessage;
		errorMessage << "\nError, could not cut-off part of the building in following location:\n"
								 << location << ", and the following direction:\n"
								 << direction << ". Direction is not orthogonal.\n"
								 << "(bso/spatial_design/ms_N_building.cpp)" << std::endl;
		throw std::invalid_argument(errorMessage.str());
	}
	
	// remove spaces that are completely included by the cut-off plane
	auto selection = this->selectSpacesGeometrically(location,n,false);
	for (auto& i : selection) this->deleteSpace(i);
	
	// select spaces that are cut by the cut-off plane
	selection = this->selectSpacesGeometrically(location,n,true);
	for (auto& i : selection)
	{// cut each space at the cutting plane
		bso::utilities::geometry::vertex coords = i->getCoordinates();
		bso::utilities::geometry::vector tempSum = coords + i->getDimensions();
		bso::utilities::geometry::vector vec1 = coords - location;

		if (n.dot(vec1) < 0)
		{
			vec1 = tempSum - location;
			double distance = n.dot(vec1);
			tempSum -= n* distance;
		}
		else
		{
			double distance = n.dot(vec1);
			coords -= n * distance;
		}
		
		i->setCoordinates(coords);
		i->setDimensions(tempSum - coords);
	}
}
*/
/*
void ms_N_building::sweep(const bso::utilities::geometry::vertex& location,
		const std::vector<std::pair<unsigned int, double> >& distances 
		, const double& tol)
{
	bso::utilities::geometry::vertex coords;
	bso::utilities::geometry::vector tempSum;
	
	unsigned int axis;
	double distance;
	bool checkDouble[3] = {false};
	
	for (auto i : distances)
	{
		axis = i.first;
		if (axis != 0 && axis != 1 && axis != 2)
		{
			std::stringstream errorMessage;
			errorMessage << "Trying to sweep an ms_N building spatial design over the \n"
									 << "following non-existent axis: " << axis << std::endl
									 << "(bso/spatial_design/ms_N_building.cpp)." << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
		
		if (checkDouble[axis])
		{
			std::stringstream errorMessage;
			errorMessage << "Defined a scaling factor to scale and ms_N building \n"
									 << "spatial design twice for the same axis, axis: " << axis << std::endl
									 << "(bso/spatial_design/ms_N_building.cpp)." << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
		else checkDouble[axis] = true;
	}
	
	for (auto i : mSpaces)
	{
		coords = i->getCoordinates();
		tempSum = coords + i->getDimensions();

		for (auto j : distances)
		{
			axis = j.first;
			distance = j.second;
			
			if (distance < 0)
			{
				if (coords(axis)  + tol < location(axis)) coords(axis)  += distance;
				if (tempSum(axis) + tol < location(axis)) tempSum(axis) += distance;
			}
			else 
			{
				if (coords(axis)  - tol > location(axis)) coords(axis)  += distance;
				if (tempSum(axis) - tol > location(axis)) tempSum(axis) += distance;
			}
		}

		i->setCoordinates(coords);
		i->setDimensions(tempSum - coords);
	}
} // sweep()
*/

void ms_N_building::scale(const std::vector<std::pair<unsigned int, double> >& scales /*= {{0,sqrt(2.0)},{1,sqrt(2.0)}}*/)
{ // Seems to be up to date however the unit test for this one is not correct yet.
	std::vector<utilities::geometry::vertex*> vertices;
	
	unsigned int axis;
	double n;
	bool checkDouble[3] = {false};
	
	for (auto i : scales)
	{
		axis = i.first;
		if (axis != 0 && axis != 1 && axis != 2)
		{
			std::stringstream errorMessage;
			errorMessage << "Trying to scale an ms_N building spatial design over the \n"
									 << "following non-existent axis: " << axis << std::endl
									 << "(bso/spatial_design/ms_N_building.cpp)." << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
		
		if (checkDouble[axis])
		{
			std::stringstream errorMessage;
			errorMessage << "Defined a scaling factor to scale and ms_N building \n"
									 << "spatial design twice for the same axis, axis: " << axis << std::endl
									 << "(bso/spatial_design/ms_N_building.cpp)." << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
		else checkDouble[axis] = true;
	}
	
	for (auto i : mSpaces)
	{
		vertices = i->getVertices();
		for (int k=0; k < vertices.size(); k++)
		{
			for (auto j : scales)
			{
				axis = j.first;
				n = j.second;

				(*vertices[k])(axis) *= n;
			}
		}
		
		i->setPVertex(vertices);		
	}
} // scale()

void ms_N_building::splitSpace(ms_space* spacePtr, const std::vector<std::pair<unsigned int, unsigned int> >& splits /*= {{0,2},{1,2}}*/)
{ // note not up to date yet	
	this->getLastSpaceID();
	utilities::geometry::vertex coords  = spacePtr->getCoordinates();
	utilities::geometry::vector tempSum = coords + spacePtr->getDimensions();
	
	std::vector<double> splitValues[3];
	for (unsigned int i = 0; i < 3; i++)
	{
		splitValues[i] = std::vector<double>();
		splitValues[i].push_back(coords(i));
		splitValues[i].push_back(tempSum(i));
	}
	
	unsigned int axis, div;
	bool checkDouble[3] = {false};
	
	for (auto i : splits)
	{
		axis = i.first;
		div = i.second;
		
		if (axis != 0 && axis != 1 && axis != 2)
		{
			std::stringstream errorMessage;
			errorMessage << "Trying to split space \"" << spacePtr->getID() << "\" over a non-existing axis: " << axis << std::endl
									 << "(bso/spatial_design/ms_N_building.cpp)." << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
		
		if (div < 2)
		{
			std::stringstream errorMessage;
			errorMessage << "Trying to split space \"" << spacePtr->getID() << "\" into less than two new spaces: n_split = " << div << std::endl
									 << "(bso/spatial_design/ms_N_building.cpp)." << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
		
		if (checkDouble[axis])
		{
			std::stringstream errorMessage;
			errorMessage << "Defined a split for space \"" << spacePtr->getID() << "\" twice for the same axis, axis: " << axis << std::endl
									 << "(bso/spatial_design/ms_N_building.cpp)." << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
		else checkDouble[axis] = true;
		
		double delta = floor((splitValues[axis][1] - splitValues[axis][0])/div);

		for (unsigned int i = 0; i < div-1; i++)
		{
			double nextCoord;
			nextCoord = splitValues[axis][i] + delta;
			splitValues[axis].insert(splitValues[axis].begin() + i + 1,nextCoord);
		}
	}
	
	ms_space* temp;
	for (unsigned int i = 0; i < splitValues[0].size() - 1; i++)
	{
		for (unsigned int j = 0; j < splitValues[1].size() - 1; j++)
		{
			for (unsigned int k = 0; k < splitValues[2].size() - 1; k++)
			{
				temp = new ms_space(*spacePtr);
				temp->setID(++mLastSpaceID);
				coords << splitValues[0][i], splitValues[1][j], splitValues[2][k];
				temp->setCoordinates(coords);
				tempSum << splitValues[0][i+1], splitValues[1][j+1], splitValues[2][k+1];
				temp->setDimensions(tempSum - coords);
				mSpaces.push_back(temp);
			}
		}
	}
	
	deleteSpace(spacePtr);
} // splitSpace()

void ms_N_building::splitSpace(ms_space& space, const std::vector<std::pair<unsigned int, unsigned int> >& splits /*= {{0,2},{1,2}}*/)
{
	this->splitSpace(this->getSpacePtr(space), splits);
} // splitSpace()

void ms_N_building::snapOn(const std::vector<std::pair<unsigned int, double> >& snaps /*= {{0,10},{1,10}}*/)
{
	utilities::geometry::vertex coords;
	utilities::geometry::vector tempSum;
	
	unsigned int axis;
	double n;
	bool checkDouble[3] = {false};
	
	for (auto i : snaps)
	{
		axis = i.first;
		if (axis != 0 && axis != 1 && axis != 2)
		{
			std::stringstream errorMessage;
			errorMessage << "Trying to snap-on an ms_N building spatial to a grid over the following non-existent axis: " << axis << std::endl
									 << "(bso/spatial_design/ms_N_building.cpp)." << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
		
		if (checkDouble[axis])
		{
			std::stringstream errorMessage;
			errorMessage << "Defined a snap-on for an ms_N building spatial design twice for the same axis, axis: " << axis << std::endl
									 << "(bso/spatial_design/ms_N_building.cpp)." << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
		else checkDouble[axis] = true;
	}
	
	for (auto i : mSpaces)
	{
		coords = i->getCoordinates();
		tempSum = coords + i->getDimensions();
		
		for (auto j : snaps)
		{
			axis = j.first;
			n = j.second;

			coords(axis)  = round(coords(axis)  / n) * n;
			tempSum(axis) = round(tempSum(axis) / n) * n;
		}
		
		i->setCoordinates(coords);
		i->setDimensions(tempSum - coords);
	}
} // snapOn()

int ms_N_building::addBoundary() {
	double min_x, min_y, max_x, max_y, min_z, max_z = 0.0;
	int i = 0;
	std::string msBoundary;
	// Look through each pre-existing space
	std::for_each(mSpaces.begin(),mSpaces.end(),[&](const auto& elem) { 
		// Get bounding vertices of a given space
		std::vector<utilities::geometry::vertex*> v = elem->getVertices();
	
		
		// Iterate over each vertex (point)
		std::for_each(v.begin(),v.end(),[&](const auto& vert) {
			// Set a counter
			i=0;
			std::for_each(vert->begin(),vert->end(),[&](double coordinate) {
				if(!i && coordinate < min_x) min_x = coordinate;
				if(!i && coordinate > max_x) max_x = coordinate;

				if(i==1 && coordinate < min_y) min_y = coordinate;
				if(i==1 && coordinate > max_y) max_y = coordinate;

				if(i==2 && coordinate < min_z) min_z = coordinate;
				if(i==2 && coordinate > max_z) max_z = coordinate;

				i++;
			});
		});

	});
	msBoundary = "N,0," + 
			std::to_string(min_x) + "," + std::to_string(min_y) + "," + std::to_string(min_z) + "," +
			std::to_string(min_x) + "," + std::to_string(min_y) + "," + std::to_string(max_z) + "," +
			std::to_string(min_x) + "," + std::to_string(max_y) + "," + std::to_string(min_z) + "," +
			std::to_string(min_x) + "," + std::to_string(max_y) + "," + std::to_string(max_z) + "," +
			std::to_string(max_x) + "," + std::to_string(min_y) + "," + std::to_string(min_z) + "," +
			std::to_string(max_x) + "," + std::to_string(min_y) + "," + std::to_string(max_z) + "," +
			std::to_string(max_x) + "," + std::to_string(max_y) + "," + std::to_string(min_z) + "," +
			std::to_string(max_x) + "," + std::to_string(max_y) + "," + std::to_string(max_z);
	mSpaces.push_back(new ms_space(msBoundary));

	return 0;
}

bool ms_N_building::hasFloatingSpaces(std::vector<ms_space*>& floatingSpaces, 
	const double tol /*= 1e-3*/) const
{
	std::vector<bso::utilities::geometry::quad_hexahedron> groundedSpaceGeoms;
	floatingSpaces = mSpaces;
	bool hasFloatingSpaces = false;
	bool addedSpaceToGroundedSpaceGeoms = true; // initial
	while (addedSpaceToGroundedSpaceGeoms)
	{
		addedSpaceToGroundedSpaceGeoms = false;
		auto spaceIt = floatingSpaces.begin();
		while (spaceIt != floatingSpaces.end())
		{
			auto spaceGeom = (*spaceIt)->getGeometry();

			// check if this space has any vertex with a z-coordinate at or below zero
			bool isGrounded = false;
			for (const auto& j : spaceGeom)
			{
				if (j(2) < abs(tol))
				{
					isGrounded = true;
					break;
				}
			}
			if (isGrounded)
			{
				groundedSpaceGeoms.push_back(spaceGeom);
				addedSpaceToGroundedSpaceGeoms = true;
				floatingSpaces.erase(spaceIt);
				continue;
			}
			
			// check if this space is connected to any of the grounded spaces
			bool isConnectedToGroundedSpace = false;
			for (const auto& i : groundedSpaceGeoms)
			{
				for (const auto& j : spaceGeom)
				{
					if (i.isInside(j,tol))
					{ // if any vertex j of the space is inside a grounded space
						isConnectedToGroundedSpace = true;
						break;
					}
					for (const auto& k : i.getPolygons())
					{
						if (k->isInside(j,tol))
						{ // if any vertex j of the space is inside the surface of a grounded space
							isConnectedToGroundedSpace = true;
							break;
						}
					}
					if (isConnectedToGroundedSpace) break;
					for (const auto& k : i.getLines())
					{
						if (k.isOnLine(j,tol))
						{ // if any vertex j of the space is on a line segment of a grounded space
							isConnectedToGroundedSpace = true;
							break;
						}
					}
					if (isConnectedToGroundedSpace) break;
					for (const auto& k : i)
					{
						if (k.isSameAs(j,tol))
						{ // if any vertex j of the space is collocated with a vertex of a grounded space
							isConnectedToGroundedSpace = true;
							break;
						}
					}
					if (isConnectedToGroundedSpace) break;
				}
				for (const auto& j : i)
				{
					if (spaceGeom.isInside(j,tol))
					{ // if any vertex j of a grounded space i is inside the space
						isConnectedToGroundedSpace = true;
						break;
					}
				}
				if (isConnectedToGroundedSpace) break;
			}
			
			if (isConnectedToGroundedSpace)
			{
				groundedSpaceGeoms.push_back(spaceGeom);
				addedSpaceToGroundedSpaceGeoms = true;
				floatingSpaces.erase(spaceIt);
				continue;
			}
			
			++spaceIt;
		}
	}
	
	
	return (floatingSpaces.size() != 0);
} // hasFloatingSpaces()

bool ms_N_building::operator == (const ms_N_building& rhs)
{
	if (mSpaces.size() != rhs.mSpaces.size()) return false;
	
	// check if rhs contains the same spaces, assuming they may be in different order
	std::vector<ms_space*> cmp = rhs.mSpaces; // copy the vector containing the spaces to safe computation time, matches can be removed from this
	std::reverse(cmp.begin(), cmp.end()); // reverse the vector, since items will be removed we want to iterate from the back, however if vectors are same order we want to benefit from that to safe computation time

	for (auto i : mSpaces)
	{
		bool found = false;
		for (unsigned int j = cmp.size(); j > 0; j--)
		{
			if (*i == *(cmp[j - 1]))
			{
				found = true;
				cmp.erase(cmp.begin() + j - 1);
				break;
			}
		}
		if (!found) return false;
	}
	return true;
}

bool ms_N_building::operator != (const ms_N_building& rhs)
{
	return !(*this == rhs);
}

std::ostream& operator <<(std::ostream& stream, const ms_N_building& building)
{
	bool first = true;
	for (auto i : building.getSpacePtrs())
	{
		if (!first) stream << std::endl;
		else first = false;
		stream << *i;
	}
	
	return stream;
} // << operator

} // namespace spatial_design
} // namespace bso

#endif // ms_N_BUILDING_CPP