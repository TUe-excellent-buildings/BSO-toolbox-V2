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

void ms_N_building::setZZero()
{
	//this->resetOrigin({2});
} // setZZero()

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

void ms_N_building::scale(const std::vector<std::pair<unsigned int, double> >& scales /*= {{0,sqrt(2.0)},{1,sqrt(2.0)}}*/)
{ 
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
{ 
	//Gather information
	this->getLastSpaceID(); // this statement make sure that an error 
	bso::utilities::geometry::quad_hexahedron geometry = spacePtr->getGeometry();
	
	// Initiate value's to be used for new space definition
	bool firstFloorDone = false;
	std::vector<unsigned int> splitsdiv; // store the split value's in order of axis x,y, and z
	splitsdiv.resize(3);
	splitsdiv[0] = 1;//1 is the minimum value of splits since it will uphold the origonal space
	splitsdiv[1] = 1;//1 is the minimum value of splits since it will uphold the origonal space
	splitsdiv[2] = 1;//1 is the minimum value of splits since it will uphold the origonal space
	bso::utilities::geometry::vector heightV; //heigh vector for definition of vertices in the z-direction
	std::vector<bso::utilities::geometry::line_segment> axisL; // stores the x-axis first and y-axis as second
	axisL.resize(2);
	axisL[0] = {{0,0,0},{0,1,1}}; // to be as close to axis {1,0,0}; hence this far off initiation is used to check if the axis is found
	axisL[1] = {{0,0,0},{1,0,1}}; // to be as close to axis {0,1,0}; hence this far off initiation is used to check if the axis is found
	std::vector<bso::utilities::geometry::line_segment> axisLOposite; // stores the opposite to the first found x-axis first and same for the y-axis as second
	axisLOposite.resize(2);

	// Determine ground and top floor
	for (auto i: geometry.getPolygons())
	{
		if(!i->getNormal().isVertical()) continue; // skip non floors
		
		// Only find the heightV of the second found floor then break for the rest of the for-loop
		if(firstFloorDone == true)
		{
			for(int j=0; j < 4; j++)
			{
				if(axisL[0][0].x()==((*i)[j]).x() && axisL[0][0].y()==((*i)[j]).y())
				{
					heightV = ((*i)[j]) - axisL[0][0];
				}
			}
			break;
		}
		firstFloorDone = true;

		//find axis 0 and axis 1
		bso::utilities::geometry::vector axis0v = (axisL[0]).getVector(), axis1v = (axisL[1]).getVector();
		for(auto j: i->getLines())
		{
			std::pair<double,double> heading = (j.getVector()).getHeading();

			if(abs((heading.first)-90) < abs(((axis0v.getHeading()).first)-90) && abs((heading.first)-90) < abs(((axis0v.getHeading()).first)-270) || abs((heading.first)-270) < abs(((axis0v.getHeading()).first)-270) && abs((heading.first)-270) < abs(((axis0v.getHeading()).first)-90)) 
			{
				axisL[0] = j;
				axis0v = j.getVector();
				continue;
			}			
			else if(abs((heading.first)) < abs(((axis1v.getHeading()).first)) && abs((heading.first)) < abs(((axis1v.getHeading()).first)-180) || abs((heading.first)-180) < abs(((axis1v.getHeading()).first)-180) && abs((heading.first)-180) < abs(((axis1v.getHeading()).first)))
			{
				axisL[1] = j;
				axis1v = j.getVector();
				continue;
			}
			else{/*do nothing*/}	
		}

		// check if axis are found
		bso::utilities::geometry::line_segment axis0check = {{0,0,0},{0,1,1}}, axis1check = {{0,0,0},{1,0,1}};
		if (axisL[0] == axis0check || axisL[1] == axis1check)
		{
			std::stringstream errorMessage;
				errorMessage << "Trying to find the axis 0 and 1 of space \"" << spacePtr->getID() << "\" however, this was not found" << std::endl
										<< "the found axis0 is: " <<axisL[0] << " axis1 is: " << axisL[1] << std::endl
										<< "(bso/spatial_design/ms_N_building.cpp)." << std::endl;
				throw std::invalid_argument(errorMessage.str());
		}

		// find opposite line to axis0 and axis1 on the considered floor
		for(auto j: i->getLines())
		{
			if((axisL[0])[0] != j[0] && (axisL[0])[1] != j[0] && (axisL[0])[0] != j[1] && (axisL[0])[1] != j[1]) axisLOposite[0] = j;
			else if((axisL[1])[0] != j[0] && (axisL[1])[1] != j[0] && (axisL[1])[0] != j[1] && (axisL[1])[1] != j[1]) axisLOposite[1] = j;
			else {/*do nothing*/}
		}

		// make sure the axisL and axisLoposite are in the x+ and y+ direction of their vector definition
		if((axisL[0]).getVector().x() <= 0) axisL[0] = {{(axisL[0])[1]},(axisL[0])[0]};
		if((axisLOposite[0]).getVector().x() <= 0) axisLOposite[0] = {{(axisLOposite[0])[1]},(axisLOposite[0])[0]};
		if((axisL[1]).getVector().y() <= 0) axisL[1] = {{(axisL[1])[1]},(axisL[1])[0]};
		if((axisLOposite[1]).getVector().y() <= 0) axisLOposite[1] = {{(axisLOposite[1])[1]},(axisLOposite[1])[0]};
		
		// make sure the first vertex (with index 0) of the x-axis is the same as the first vertex (with index 0) of the y-axis; Hence this allows for consistend definition of vertices at the space definition stage.
		if((axisL[0])[0] != (axisL[1])[0])
		{
			if((axisL[0])[0] == (axisL[1])[1])
			{
				axisL[1] = {{(axisL[1])[1]},(axisL[1])[0]}; // flip y-axis
				axisLOposite[1] = {{(axisLOposite[1])[1]},(axisLOposite[1])[0]}; // flip opposite y-axis as well
			}
			else // switch axisL with axisLOposite
			{ 
				bso::utilities::geometry::line_segment switchL = axisL[1];
				axisL[1] = axisLOposite[1];
				axisLOposite[1] = switchL;
				if((axisL[0])[0] == (axisL[1])[1]) 
				{
					axisL[1] = {{(axisL[1])[1]},(axisL[1])[0]}; // flip y-axis
					axisLOposite[1] = {{(axisLOposite[1])[1]},(axisLOposite[1])[0]}; // flip opposite y-axis as well
				}
			}
		}


		// check inserted split values and store them in order of axis x-,y-, and z-
		unsigned int axis, div;
		bool checkDouble[3] = {false};
		for (auto i : splits)
		{
			axis = i.first;
			div = i.second;
			
			// First check if the inserted split values are viable
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
			
			splitsdiv[axis] = div; // store split values in order of axis x-,y-,z- by position index of the vector container
		}
	}

	// Define a matrix of vertices in the x-y-z direction of the new spaces; Note: these value's are calculated ones, thereby ensures exactly the same vertices for defining the new spaces
	std::vector<std::vector<std::vector<bso::utilities::geometry::vertex>>> Vmatrix;
	Vmatrix.resize(splitsdiv[0]+1);
	for(int i=0; i<splitsdiv[0]+1; i++) //x-axis
	{
		Vmatrix[i].resize(splitsdiv[1]+1); //resize to fit the amound of vertixes to be found in the next for-loop
		
		bso::utilities::geometry::vertex tempV1, tempV2;
		tempV1 = (axisL[0])[0] + (((axisL[0]).getVector())/(splitsdiv[0]))*i;
		tempV2 = (axisLOposite[0])[0] + (((axisLOposite[0]).getVector())/(splitsdiv[0]))*i;
		bso::utilities::geometry::line_segment xL = {tempV1,tempV2};

		for(int j=0; j<splitsdiv[1]+1; j++) // y-axis
		{
			Vmatrix[i][j].resize(splitsdiv[2]+1); //resize to fit the amound of vertixes to be found in the next for-loop

			for(int k=0; k<splitsdiv[2]+1; k++) // z-axis
			{
				Vmatrix[i][j][k] = xL[0] + ((xL).getVector())/(splitsdiv[1])*j + heightV/(splitsdiv[2])*k;
			}
		}
	}

	// Generate and add the new spaces
	ms_space* temp;
	for(int i=0; i<splitsdiv[0]; i++) //x-axis
	{
		for(int j=0; j<splitsdiv[1]; j++) // y-axis
		{
			for(int k=0; k<splitsdiv[2]; k++) // z-axis
			{
				temp = new ms_space(*spacePtr);
				temp->setID(++mLastSpaceID);
				
				std::vector <utilities::geometry::vertex*> vertexes;
				for(int t=0; t<8; t++) vertexes.push_back(new utilities::geometry::vertex);

				*vertexes[0] = Vmatrix[i][j][k];
				*vertexes[1] = Vmatrix[i+1][j][k];
				*vertexes[2] = Vmatrix[i][j+1][k];
				*vertexes[3] = Vmatrix[i+1][j+1][k];
				*vertexes[4] = Vmatrix[i][j][k+1];
				*vertexes[5] = Vmatrix[i+1][j][k+1];
				*vertexes[6] = Vmatrix[i][j+1][k+1];
				*vertexes[7] = Vmatrix[i+1][j+1][k+1];
				
				temp->setPVertex(vertexes);
				mSpaces.push_back(temp);
			}
		}
	}
	deleteSpace(spacePtr); // Delete the origonal space
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