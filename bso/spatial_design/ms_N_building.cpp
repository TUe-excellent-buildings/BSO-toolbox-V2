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
//#include <bso/utilities/geometry.hpp>
#include <bso/utilities/delaunay.hpp>

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
	for (auto& i : rhs.mSpaces)
	{
		mSpaces.push_back(new ms_space(*i));
	}
	
	insertFileName = rhs.getInsertFileName();
	this->checkValidity();
} // ms_N_building() (copy constructor)

ms_N_building::~ms_N_building()
{ // destructor
	for (auto& i : mSpaces) delete i;
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
							 << " " << std::endl 
							 << "The following spaces are in the model: " << std::endl;
							 for(auto i: mSpaces)
							 {
								errorMessage << *i << std::endl;
							 }

	errorMessage <<	"(bso/spatial_design/ms_N_building.cpp). " << std::endl;
	throw std::runtime_error(errorMessage.str());
} // getSpacePtr()

ms_space* ms_N_building::getSpacePtr(const ms_space* spacePtr) const
{
	return getSpacePtr(*spacePtr);
}

ms_space* ms_N_building::getSpacePtrWithID(unsigned int id) const
{
	ms_space* spacePtr = nullptr;
	
	for (auto i : mSpaces)
	{
		if (i->getID() == id) return i;
	}
	
	std::stringstream errorMessage;
	errorMessage << "Could not find the following space in an ms_N building spatial design:" << std::endl
							 << "ID: " << id << std::endl
							 << "(bso/spatial_design/ms_N_building.cpp). " << std::endl;
	throw std::runtime_error(errorMessage.str());

	return spacePtr;
} // getSpacePtrWithID()

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
	this->resetOrigin({2});
} // setZZero()

void ms_N_building::resetOrigin(const std::vector<unsigned int>& indices /*= {0,1,2}*/)
{
	utilities::geometry::vector coordDifference = {0,0,0};
	for (const auto& index : indices)
	{
		utilities::geometry::vertex initalV = (*(mSpaces[0]->getVertices())[0]);
		double min = initalV(index); // set an initial value to the minimum
	
		// find the minimum value of the x,y, or z-coordinates in the building
		for (auto i : mSpaces) 
		{
			for (auto j : i->getVertices())
			{
				if(min > (*j)(index)) min = (*j)(index);
			}
		}			

		coordDifference(index) -= min;
	}
	
	if (coordDifference(2) > 0) coordDifference(2) = 0;
	
	utilities::geometry::vertex temp;
	for (auto i : mSpaces)
	{
		std::vector<utilities::geometry::vertex*> newVP;
		for (int j=0; j<8; j++)
		{
			newVP.push_back(new utilities::geometry::vertex);
			temp = 	*((i->getVertices())[j]) + coordDifference;	
			*newVP[j] = temp;
		}
		i->setPVertex(newVP);
	} 
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
			else if(abs((heading.first)) < abs(((axis1v.getHeading()).first)) && abs((heading.first)) < abs(((axis1v.getHeading()).first)-180) && abs((heading.first)) < abs(((axis1v.getHeading()).first)-360)|| abs((heading.first)-180) < abs(((axis1v.getHeading()).first)-180) && abs((heading.first)-180) < abs(((axis1v.getHeading()).first)) && abs((heading.first)-180) < abs(((axis1v.getHeading()).first)-360) || abs((heading.first)-360) < abs(((axis1v.getHeading()).first)-360) && abs((heading.first)-360) < abs(((axis1v.getHeading()).first)-180) && abs((heading.first)-360) < abs(((axis1v.getHeading()).first)))
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
										<< "the space is: " << *spacePtr << std::endl
										<< "the lines and its headings of space " << spacePtr->getID() << " are as follows: " << std::endl;
										for(auto j: i->getLines())
										{
											errorMessage << j << " heading: <" << (j.getVector()).getHeading().first << " : " << (j.getVector()).getHeading().second << ">" << std::endl;
										}
						   errorMessage << "(bso/spatial_design/ms_N_building.cpp)." << std::endl;
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

void ms_N_building::splitSpace(ms_space* spacePtr, int splitAmount, bso::utilities::geometry::line_segment& lineToSplit) //split inserted line of space into 'splitamount'of section wherefrom splitamount of spaces are defined
{
	// The inserted line may be closes to a centain axis. 
	// However, it does not mean that the found axisL in the splitSpace function is the same as the axis of the inserted line since another line of the space may be closer to this axis. 
	// Therefore, this function will make sure the inserted line is split and not another line which is find closer to the axis of the inserted line
	
	std::pair<double,double> heading = lineToSplit.getVector().getHeading();
	int axis;
	if(heading.first >= 45 && heading.first < 135 || heading.first >= 225 && heading.first < 315) axis = 0;
	else axis = 1;
	
	bso::utilities::geometry::quad_hexahedron geometry = spacePtr->getGeometry();
	std::vector<bso::utilities::geometry::line_segment> axisL; // stores the x-axis first and y-axis as second
	axisL.resize(2);
	axisL[0] = {{0,0,0},{0,1,1}}; // to be as close to axis {1,0,0}; hence this far off initiation is used to check if the axis is found
	axisL[1] = {{0,0,0},{1,0,1}}; // to be as close to axis {0,1,0}; hence this far off initiation is used to check if the axis is found


	int count = 0;
	for (auto i: geometry.getPolygons())
	{
		if(!i->getNormal().isVertical()) continue; // skip non floors

		bool correctFloor = false;
		for(auto j: i->getLines()) 
		{
			if(j == lineToSplit)
			{
				correctFloor = true;
			}
		}
		if(correctFloor == false)
		{
			count++;
			if(count == 2)
			{
				std::stringstream errorMessage;
				errorMessage << "The inserted line has not been found to be part of either the top or botum floor. " << std::endl
										<< "Therefore, the inserted line has been found invalid. "<< std::endl
										<< "The inserted line was: " << lineToSplit << std::endl
										<< "(bso/spatial_design/ms_N_building.cpp)." << std::endl;
				throw std::invalid_argument(errorMessage.str());
			}
			continue;
		}

		//find axis 0 and axis 1 as it would be in the splitSpace function
		bso::utilities::geometry::vector axis0v = (axisL[0]).getVector(), axis1v = (axisL[1]).getVector();
		for(auto j: i->getLines())
		{
			heading = (j.getVector()).getHeading();

			if(abs((heading.first)-90) < abs(((axis0v.getHeading()).first)-90) && abs((heading.first)-90) < abs(((axis0v.getHeading()).first)-270) || abs((heading.first)-270) < abs(((axis0v.getHeading()).first)-270) && abs((heading.first)-270) < abs(((axis0v.getHeading()).first)-90)) 
			{
				axisL[0] = j;
				axis0v = j.getVector();
				continue;
			}			
			else if(abs((heading.first)) < abs(((axis1v.getHeading()).first)) && abs((heading.first)) < abs(((axis1v.getHeading()).first)-180) && abs((heading.first)) < abs(((axis1v.getHeading()).first)-360)|| abs((heading.first)-180) < abs(((axis1v.getHeading()).first)-180) && abs((heading.first)-180) < abs(((axis1v.getHeading()).first)) && abs((heading.first)-180) < abs(((axis1v.getHeading()).first)-360) || abs((heading.first)-360) < abs(((axis1v.getHeading()).first)-360) && abs((heading.first)-360) < abs(((axis1v.getHeading()).first)-180) && abs((heading.first)-360) < abs(((axis1v.getHeading()).first)))
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
										<< "the space is: " << *spacePtr << std::endl
										<< "the lines and its headings of space " << spacePtr->getID() << " are as follows: " << std::endl;
										for(auto j: i->getLines())
										{
											errorMessage << j << " heading: <" << (j.getVector()).getHeading().first << " : " << (j.getVector()).getHeading().second << ">" << std::endl;
										}
						   errorMessage << "(bso/spatial_design/ms_N_building.cpp)." << std::endl;
				throw std::invalid_argument(errorMessage.str());
		}

		// Check if the inserted line is already found as an axis. If so, adjust the axis variable acordingly
		if(axisL[axis] == lineToSplit) break;
		else if(axis == 0)
		{
			if(axisL[1] == lineToSplit)
			{
				axis = 1;
				break;
			}
		}
		else if (axis == 1)
		{
			if(axisL[0] == lineToSplit)
			{
				axis = 0;
				break;
			}
		}

		std::vector<bso::utilities::geometry::line_segment> axisLOposite; // stores the opposite to the first found x-axis first and same for the y-axis as second
		axisLOposite.resize(2);

		// find opposite line to axis0 and axis1 on the considered floor as it would be in the splitSpace function
		for(auto j: i->getLines())
		{
			if((axisL[0])[0] != j[0] && (axisL[0])[1] != j[0] && (axisL[0])[0] != j[1] && (axisL[0])[1] != j[1]) axisLOposite[0] = j;
			else if((axisL[1])[0] != j[0] && (axisL[1])[1] != j[0] && (axisL[1])[0] != j[1] && (axisL[1])[1] != j[1]) axisLOposite[1] = j;
			else {/*do nothing*/}
		}

		// Check if the inserted line is found as an axisLOposite. If so, adjust the axis variable acordingly
		if(axisLOposite[axis] == lineToSplit) break;
		else if(axis == 0)
		{
			if(axisLOposite[1] == lineToSplit)
			{
				axis = 1;
				break;
			}
		}
		else if (axis == 1)
		{
			if(axisLOposite[0] == lineToSplit)
			{
				axis = 0;
				break;
			}
		}
		else
		{
			/*This option should not be reached since it should already be picked up the first error message of checking the considere line if it is part of either floors*/
			std::stringstream errorMessage;
				errorMessage << "The inserted line has not been found in the axis determination phase. " << std::endl
										<< "The inserted line was: " << lineToSplit << std::endl
										<< "(bso/spatial_design/ms_N_building.cpp)." << std::endl;
				throw std::invalid_argument(errorMessage.str());
		}
	}

	// Send the correct variables to the splitSpace function
	std::vector<std::pair<unsigned int, unsigned int> > splits;
	std::pair<unsigned int, unsigned int> inputSplits = {axis,splitAmount};
	splits.push_back(inputSplits);
	this->splitSpace(spacePtr, splits);
} // splitSpace()

void ms_N_building::splitSpace(ms_space& space, int splitAmount, bso::utilities::geometry::line_segment& lineToSplit) //split the longests line of the space in two
{
	this->splitSpace(this->getSpacePtr(space), splitAmount, lineToSplit);
} // splitSpace()

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

void ms_N_building::addBoundary() 
{
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
} // addBoundary() 


std::vector<utilities::geometry::triangular_prism> ms_N_building::convertMSToPN(std::string fileName, double mTol) //PN stands for prism-net representation from Leiden
{
	this->addBoundary();
	
	// Declarations for needed information per space
	std::vector<double> allLevels; // stores all the floor levels
	std::vector<utilities::geometry::vertex> highV;
	std::vector<utilities::geometry::vertex> LowV;

	// First define all levels in the building and make for each level a pair containing the level and an empty std::vector<utilities::geometry::quad_hexahedron>
	for (const auto& i : mSpaces)
	{
		// Get quad_hexahedron geometry
		utilities::geometry::quad_hexahedron spaceGeom = i->getGeometry();

		//Find the highest and lowest vertex
		utilities::geometry::vertex lowVertex = spaceGeom[0];
		utilities::geometry::vertex highVertex = spaceGeom[0];
		for (const auto j: spaceGeom)
		{
			if(lowVertex.z() > j.z())
			{//find the lowest vertex of SpaceGeomTemp_1 for the shifting of the space geometry
				lowVertex = j;
			}
			else if(highVertex.z() < j.z())
			{
				highVertex = j;
			}
		}
		highV.push_back(highVertex);
		LowV.push_back(lowVertex);

		// Determine all the floor levels in the building spatial designs
		bool highVUnique = true;
		bool lowVUnique = true;
		for(double j: allLevels)
		{
			if(highVertex.z()<=(j+mTol) && highVertex.z()>=(j-mTol))
			{
				highVUnique = false;
			}
			if(lowVertex.z()<=(j+mTol) && lowVertex.z()>=(j-mTol))
			{
				lowVUnique = false;
			}
		}
		if(highVUnique){allLevels.push_back(highVertex.z());}
		if(lowVUnique){allLevels.push_back(lowVertex.z());}
	}

	// Sort allLevels for later use in triangular prism cell definition.
	sort(allLevels.begin(), allLevels.end());
	
	std::vector<std::pair<double,std::vector<utilities::geometry::vertex>>> delaunayPointsAtLevel;
	std::vector<std::pair<double,std::vector<utilities::geometry::line_segment>>> delaunayLinesAtLevel;
	
		
	for(int IDlevel=0; IDlevel<allLevels.size()-1; IDlevel++)
	{	
		delaunayPointsAtLevel.push_back(std::make_pair(allLevels[IDlevel],std::vector<utilities::geometry::vertex>()));
		delaunayLinesAtLevel.push_back(std::make_pair(allLevels[IDlevel],std::vector<utilities::geometry::line_segment>()));

		for (int i=0; i<mSpaces.size(); i++)
		{
			// Get quad_hexahedron geometry
			utilities::geometry::quad_hexahedron spaceGeom = mSpaces[i]->getGeometry();

			// j.first > LowV[IDSpace].z()-mTol && j.first < highV[IDSpace].z()-mTol
			if(allLevels[IDlevel] >= LowV[i].z()-mTol && allLevels[IDlevel] < highV[i].z()-mTol || i == (mSpaces.size()-1)) // NOTE: last space is the bounding box added in function addBoundary(), so variables need to be included at every elevation to get a correct delaunay triangulation
			//if(LowV[i].z()-mTol <= allLevels[IDlevel] && LowV[i].z()+mTol > allLevels[IDlevel] || i == (mSpaces.size()-1)) // NOTE: last space is the bounding box, so variables need to be included to get a correct delaunay triangulation per elevation
			{
				// Add all unique cornerpoints spaceGEOM to the delaunayPoints
				for(const auto& j: spaceGeom)
				{
					utilities::geometry::vertex temp = {j.x(),j.y(),allLevels[IDlevel]};

					bool doubleInsert = false;
					for (const auto& m : delaunayPointsAtLevel.back().second)
					{// check if temp already exist in delauneyPoints
						if (m == temp)
						{
							doubleInsert = true;
						}
					}
					if(doubleInsert == false)
					{//if not, then add to delaunayPoints
						delaunayPointsAtLevel.back().second.push_back(temp);
					}
				}

				for (const auto& k : spaceGeom.getLines())
				{
					// Add the unique lines of each spaceGEOM to delaunayLines
					utilities::geometry::vertex Ltemp1 = {k[0].x(),k[0].y(),allLevels[IDlevel]};
					utilities::geometry::vertex Ltemp2 = {k[1].x(),k[1].y(),allLevels[IDlevel]};
					utilities::geometry::line_segment Ltemp {{Ltemp1},{Ltemp2}}; //Note the line is newly defined for the case that some round of error occur during the shifting of the lines of the spacesAtGround[i]
					
					bool doubleInsert = false;
					for (const auto& m : delaunayLinesAtLevel.back().second)
					{// Check if line k already exist in delauneyPoints
						if (m.isSameAs(Ltemp, mTol))
						{
							doubleInsert = true;
						}
					}
					if(Ltemp1.isSameAs(Ltemp2, mTol))
					{// Check if line has different end points
						doubleInsert = true;
					}
					if(doubleInsert == false)
					{// If both agree, then add to delaunayLines
						delaunayLinesAtLevel.back().second.push_back(Ltemp);
					}
				}
			}
		}
	}

	// Split lines at intersection vertices and remove duplicated lines
	for(unsigned int i=0; i<allLevels.size()-1; i++)
	{
		bso::utilities::splitLines(delaunayPointsAtLevel[i].second, delaunayLinesAtLevel[i].second, mTol);// this prevent the generation of triangles on one line or triangles containing duplicate vertices
	}

	// Send all vertices and constrain lines to the delaunay triangulation, receive all triangles, and project them to the correct level for later use
	std::vector<std::pair<double,std::vector<utilities::geometry::triangle>>> delaunayTriAtLevel;
	for(unsigned int i=0; i<allLevels.size()-1; i++)
	{
		std::vector<utilities::geometry::triangle> trianglesTemp = bso::utilities::delaunay(delaunayPointsAtLevel[i].second, delaunayLinesAtLevel[i].second, mTol);
		std::vector<utilities::geometry::triangle> triAtLevel;
		// project triangles to the correct level
		for(auto& tri: trianglesTemp)
		{
			utilities::geometry::triangle temp = {{tri[0].x(),tri[0].y(),allLevels[i]}, {tri[1].x(),tri[1].y(),allLevels[i]}, {tri[2].x(),tri[2].y(),allLevels[i]}};
			triAtLevel.push_back(temp);
		}
		delaunayTriAtLevel.push_back(std::make_pair(allLevels[i],triAtLevel));
	}

	// Initiate output file stream
	std::ofstream output;
	if (!fileName.empty()) output.open(fileName.c_str());
	
	if (fileName.empty() || !output.is_open())
	{
		std::stringstream errorMessage;
		errorMessage << "Could not open the following file to write a conformal model design to:" << std::endl
								<< ((fileName.empty())? "no input file given" : fileName) << std::endl
								<< "(bso/spatial_design/conformal/cf_building_model.cpp). " << std::endl;
		throw std::invalid_argument(errorMessage.str());
	}

	// initialize delaunayTriAtLevelInSpace with all bool set to false reflecting the nr. triangles at each level as in delaunayTriAtLevel
	std::vector<std::pair<double,std::vector<bool>>> delaunayTriAtLevelInSpace;
	delaunayTriAtLevelInSpace.resize(delaunayTriAtLevel.size());
	for(unsigned int i=0; i<delaunayTriAtLevel.size(); i++)
	{
		delaunayTriAtLevelInSpace[i].first = delaunayTriAtLevel[i].first;
		delaunayTriAtLevelInSpace[i].second.resize(delaunayTriAtLevel[i].second.size(), false);
	}

	// Identify which triangles should be used to generate a triangular prism cell in the space and generate that space and accompanying geometric entities
	std::vector<utilities::geometry::triangular_prism> triPrismBoundingBox;
	for(int IDSpace=0; IDSpace<mSpaces.size(); IDSpace++)
	{
		// Get spaceGEOM
		utilities::geometry::quad_hexahedron spaceGeom = mSpaces[IDSpace]->getGeometry();
		
		// Find the triangles in the space
		std::vector<std::pair<double,std::vector<utilities::geometry::triangle>>> triInSpaceAtLevel;
		for(unsigned int j=0; j<delaunayTriAtLevel.size(); j++)
		{
			if(!(delaunayTriAtLevel[j].first > LowV[IDSpace].z()-mTol && delaunayTriAtLevel[j].first < highV[IDSpace].z()-mTol)) continue; // triangle is not defined within the level bounds of the considered space, so continue.
			triInSpaceAtLevel.push_back(std::make_pair(delaunayTriAtLevel[j].first,std::vector<utilities::geometry::triangle>()));

			for(unsigned int idT=0; idT<(delaunayTriAtLevel[j].second).size(); idT++) 
			{
				bool allNodesInside = true;
				for(const auto k: (delaunayTriAtLevel[j].second)[idT]) // iterate trough the vertices of the triangle
				{
					if(!spaceGeom.isInsideOrOn(k, mTol))
					{
						// Additional checks for prevention malfunctioning, which has been previously observed
						// Note; the tolerance of 0.001 is applied differently in each geometry check. 
						// Meaning that in the isOnLine check, the considered vertices onLine are not by definition within +0.001 and -0.001 from the line in distance.
						// However, within the isSameAs check are the vertices within 0.001 from each other
						// For this reason are all possible geometry checks applied.
						
						bool anyTrue = false;
						for(const auto l: spaceGeom.getLines())
						{
							if(l.isOnLine(k, mTol))
							{
								anyTrue = true;
							}
						}

						for(const auto l : spaceGeom)
						{
							if(l.isSameAs(k, mTol))
							{
								anyTrue = true;
							}
						}

						if(!anyTrue)
						{
							allNodesInside = false;
						}
					}
				}
				if(allNodesInside)
				{
					((triInSpaceAtLevel.back()).second).push_back((delaunayTriAtLevel[j].second)[idT]);
					if(IDSpace != mSpaces.size()-1) {delaunayTriAtLevelInSpace[j].second[idT] = true;} // set the triangle as used
				}
			}
		}
		
		for(const auto j: triInSpaceAtLevel)
		{		
			if(j.second.size() < 2)
			{
				std::stringstream errorMessage;
				errorMessage << "The amount of triCell found are to little to form a space fully filled with triPrism cells.\n"
										<< "The minumum required amount of triCells are 2 and the inserted amount is: " << j.second.size() << "\n"
										<< "Meaning that there are tricell(s) missing. \n"
										<< "The possibly found triCell consist of the following: " << std::endl;
											for(const auto i: j.second)
											{
												errorMessage << i << std::endl;
											}
						errorMessage << "the considered space is: " << *mSpaces[IDSpace] << "\n"
										<< "(bso/spatial_design/ms_N_building.cpp)" << std::endl;
							throw std::invalid_argument(errorMessage.str());
			}
		}
		
		// Define all six vertices of the triangular prism and store in triPrism
		std::vector<utilities::geometry::triangular_prism> triPrism;
		for(unsigned int IDtriAtLevel = 0; IDtriAtLevel<triInSpaceAtLevel.size(); IDtriAtLevel++)
		{
			for(const auto tri: triInSpaceAtLevel[IDtriAtLevel].second)
			{
				std::vector<utilities::geometry::vertex> triPrismV;
				triPrismV.reserve(6);
				
				//Generate triangular prism
				triPrismV.clear();
				for(const auto k: tri)
				{// bottom row prisms
					utilities::geometry::vertex high = {0,0,0};
					if(IDtriAtLevel+1 >= triInSpaceAtLevel.size()){high = {k.x(), k.y(), highV[IDSpace].z()};}
					else{high = {k.x(), k.y(), triInSpaceAtLevel[IDtriAtLevel+1].first};}
					triPrismV.push_back(k);
					triPrismV.push_back(high);
				}
				triPrism.push_back(utilities::geometry::triangular_prism(triPrismV, mTol));
			}
		}

		if(IDSpace == mSpaces.size()-1) // last space is the bounding box, thus contains all triangular prims defined for the PN representation.
		{
			triPrismBoundingBox = triPrism;
			break;
		}

		// Write the found triangular prisms in space to text file
		output << mSpaces[IDSpace]->getID() << ", " << spaceGeom << std::endl;
		for(const auto& TP: triPrism)
		{
			output << TP << std::endl;
		}
	}

	// Write the bounding box triangles not belonging to a space to the text file:
	// Get spaceGEOM
	utilities::geometry::quad_hexahedron spaceGeom = mSpaces[mSpaces.size()-1]->getGeometry();

	// Define all six vertices of the triangular prism and store in triPrism
	std::vector<utilities::geometry::triangular_prism> triPrism;
	for(unsigned int IDtriAtLevel = 0; IDtriAtLevel<delaunayTriAtLevel.size(); IDtriAtLevel++)
	{
		int IDCount = 0;
		for(const auto tri: delaunayTriAtLevel[IDtriAtLevel].second)
		{
			// Only continue with the triangles that are not used in the previous spaces, i.e. outside of spaces and only existing in the bounding box
			bool inSpace = (delaunayTriAtLevelInSpace[IDtriAtLevel].second)[IDCount];
			IDCount++;
			if(inSpace == true) continue;			

			std::vector<utilities::geometry::vertex> triPrismV;
			triPrismV.reserve(6);
			
			//Generate triangular prism
			triPrismV.clear();
			for(const auto k: tri)
			{// bottom row prisms
				utilities::geometry::vertex high = {0,0,0};
				if(IDtriAtLevel+1 >= delaunayTriAtLevel.size()){high = {k.x(), k.y(), highV[mSpaces.size()-1].z()};}
				else{high = {k.x(), k.y(), delaunayTriAtLevel[IDtriAtLevel+1].first};}
				triPrismV.push_back(k);
				triPrismV.push_back(high);
			}
			triPrism.push_back(utilities::geometry::triangular_prism(triPrismV, mTol));
		}
	}

	// Write the found triangular prisms in space to text file
	output << mSpaces[mSpaces.size()-1]->getID() << ", " << spaceGeom << std::endl;
	for(const auto& TP: triPrism)
	{
		output << TP << std::endl;
	}

	output.close(); // close the output file stream
	return triPrismBoundingBox; // return the triPrism defined in the bounding box for later on visualizations
}


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