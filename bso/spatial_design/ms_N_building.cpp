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

#include <bso/utilities/delaunay.hpp>
#include <bso/utilities/geometry/polygon_non_convex.hpp>

namespace bso { namespace spatial_design {

ms_N_building::ms_N_building()
{ // empty constructor
	mLastSpaceID = 0;
} // ms_building() (empty constructor)

ms_N_building::ms_N_building(std::string fileName)
:	insertFileName(fileName)
{ // initialization by string or text file
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
	deleteSpace(spacePtr); // Delete the original space
} // splitSpace()

void ms_N_building::splitSpace(ms_space& space, const std::vector<std::pair<unsigned int, unsigned int> >& splits /*= {{0,2},{1,2}}*/)
{
	this->splitSpace(this->getSpacePtr(space), splits);
} // splitSpace()

void ms_N_building::splitSpace(ms_space* spacePtr, int splitAmount, bso::utilities::geometry::line_segment& lineToSplit) //split inserted line of space into 'splitamount'of section wherefrom splitamount of spaces are defined
{
	// The inserted line may be closes to a certain axis. 
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

void ms_N_building::splitSpace(ms_space& space, int splitAmount, bso::utilities::geometry::line_segment& lineToSplit) //split the longest line of the space in two
{
	this->splitSpace(this->getSpacePtr(space), splitAmount, lineToSplit);
} // splitSpace()

void ms_N_building::duplicateElevation(double atZFromGround, double elevationHeight, bool duplicateBelowZLevel)
{
	if(elevationHeight <= 0)
	{
		std::stringstream errorMessage;
				errorMessage << "Can not add an elevation of height zero or a negative value." << std::endl
										<< "The inserted elevationHeight is: " << elevationHeight << std::endl
										<< "(bso/spatial_design/ms_N_building.cpp)." << std::endl;
				throw std::invalid_argument(errorMessage.str());
	}
	
	// Find spaces which gets intersected, connected (a horizontal Quad of space coincides with atZFromGround),
	// or need to be shifted to be able to add an elevation
	std::vector<int> intersectedSpaces;
	std::vector<int> connectedSpaces;
	std::vector<int> connectedSpacesTop;
	std::vector<int> toShiftSpaces;
	for (int i=0; i<mSpaces.size(); i++)
	{
		bso::utilities::geometry::quad_hexahedron spaceGeom = mSpaces[i]->getGeometry();
		bso::utilities::geometry::vertex center = spaceGeom.getCenter();
		bso::utilities::geometry::vertex checkVertex = {center.x(),center.y(),atZFromGround};
		if(spaceGeom.isInside(checkVertex))
		{
			intersectedSpaces.push_back(i);
			continue;
		}
		else if(spaceGeom.isInsideOrOn(checkVertex)) // only consider the spaces bellow the atZFromGround lever as a connected space
		{
			if(center.z() < atZFromGround){connectedSpaces.push_back(i);} // store connected spaces bellow which is default setting
			else{connectedSpacesTop.push_back(i);} // store connected spaces above
		}
		if(center.z() > atZFromGround)
		{
			toShiftSpaces.push_back(i);
		}
	}

	// switch connectedSpaces content since this is used for duplication depending on if the space above or bellow the Z line needs to be duplicated.
	if(!duplicateBelowZLevel){connectedSpaces = connectedSpacesTop;}
	else if(connectedSpaces.size() == 0 && connectedSpacesTop.size() > 0){connectedSpaces = connectedSpacesTop;} // this ensure that at least one of the two is used if the default setting was not sufficient to avoid the error message

	// Error message if the inserted atZFromGround is invalid
	if(intersectedSpaces.size() == 0 && connectedSpaces.size() == 0)
	{
		std::stringstream errorMessage;
				errorMessage << "The inserted elevation is not at a height where any spaces are currently defined." << std::endl
										<< "The inserted z-coordinate at where the elevation should be added is: " << atZFromGround << std::endl
										<< "(bso/spatial_design/ms_N_building.cpp)." << std::endl;
				throw std::invalid_argument(errorMessage.str());
	}

	// Shift the to shift spaces
	for(const int i: toShiftSpaces)
	{
		// Define the new pVertex vector of considered space
		std::vector <utilities::geometry::vertex*> pVert;
		utilities::geometry::vertex temp;
		for(const auto j: mSpaces[i]->getVertices())
		{
			pVert.push_back(new utilities::geometry::vertex);							
			temp(0) = j->x();
			temp(1) = j->y();
			temp(2) = j->z() + elevationHeight;
			*pVert.back() = temp;
		}
		// Replace the pVertex of the considered space
		mSpaces[i]->setPVertex(pVert);
	}

	// Define the new to add spaces regarding connected spaces to the atZFromGround level
	for(const int i: connectedSpaces)
	{
		bso::utilities::geometry::quad_hexahedron spaceGeom = mSpaces[i]->getGeometry();
		for(const utilities::geometry::quadrilateral j: spaceGeom.getQuadrilateral())
		{
			// Define new space with horizontal Quad from the original connected space which is located bellow the atZFromGround level
			if(j.getNormal().isVertical())
			{
				// Define ID of new space
				int newID = mSpaces.back()->getID();
				newID++;

				// Define all vertices of new space
				std::vector <utilities::geometry::vertex*> pVert;
				utilities::geometry::vertex temp;
				for(const auto k: j)
				{
					pVert.push_back(new utilities::geometry::vertex);							
					temp(0) = k.x();
					temp(1) = k.y();
					temp(2) = atZFromGround;
					*pVert.back() = temp;		
					
					pVert.push_back(new utilities::geometry::vertex);
					temp(2) = atZFromGround + elevationHeight;
					*pVert.back() = temp;
				}

				// Add new space
				mSpaces.push_back(new ms_space());
				mSpaces.back()->setID(newID);
				mSpaces.back()->setPVertex(pVert);
				break;
			}
		}
	}

	// Duplicate the layer of spaces which is intersected above the intersected space with space height of elevationHeight
	for(const int i: intersectedSpaces)
	{
		int count = 0;
		utilities::geometry::quadrilateral horizontalQuad1;
		bso::utilities::geometry::quad_hexahedron spaceGeom = mSpaces[i]->getGeometry();
		for(const auto j: spaceGeom.getQuadrilateral())
		{
			// Define new space with horizontal Quad original space
			if(j.getNormal().isVertical())
			{
				if(count == 0) // store the first found floor and then find the second
				{
					horizontalQuad1 = j;
					count++;
					continue;
				}

				utilities::geometry::quadrilateral topFloor;
				if(horizontalQuad1[0].z() > j[0].z()) {topFloor = horizontalQuad1;}
				else topFloor = j;

				// Define ID of new space
				int newID = mSpaces.back()->getID();
				newID++;

				// Define all vertices of new space
				std::vector <utilities::geometry::vertex*> pVert;
				utilities::geometry::vertex temp;
				for(const auto k: topFloor)
				{
					pVert.push_back(new utilities::geometry::vertex);							
					temp(0) = k.x();
					temp(1) = k.y();
					temp(2) = k.z();
					*pVert.back() = temp;		
					
					pVert.push_back(new utilities::geometry::vertex);
					temp(2) = k.z() + elevationHeight;
					*pVert.back() = temp;
				}

				// Add new space
				mSpaces.push_back(new ms_space());
				mSpaces.back()->setID(newID);
				mSpaces.back()->setPVertex(pVert);
				break;
			}
		}
	}
} //duplicateElevation()

void ms_N_building::shiftFillPlaneCentre(double shiftAmount, bool shiftInMostHeadingDirection /*= false*/, int amountOfHeadingCatagories /* = 6 */, const double& tol /*= 1e-3*/)
{
	bso::utilities::geometry::vertex location = this->getCentreBuilding();
	bso::utilities::geometry::vector shiftDirection;

	// Define shiftDirection:
	// Get headings azimuth from all horizontal lines in the building	
	std::vector <double> horizontalHeadings;
	for (const auto i : mSpaces)
	{
		//Get original space geometry
		utilities::geometry::quad_hexahedron spaceGeom = i->getGeometry();
		
		// Add lines if horizontal to linesAtZ
		for (const auto& k : spaceGeom.getLines())
		{
			bso::utilities::geometry::vector lineV = k.getVector();
			if(!lineV.isVertical()) 
			{				
				horizontalHeadings.push_back(lineV.getHeading().first);
			}
		}
	}

	// There are x amount of azimuth catagories considered and dispatched all horizontal lines in these
	// this figures out which heading direction is most frequent and which are second most frequent
	//double amountOfHeadingCatagories = 6; //should be even to have two matching opposing sides
	
	//Generate headingCategories:
	std::vector<std::pair<double,double>> headingCategories;
	double sectionSize = 180/(amountOfHeadingCatagories);
	std::pair<double,double> temp (360-sectionSize/2,0+sectionSize/2);
	headingCategories.push_back(temp);
	for(int i=0; i<amountOfHeadingCatagories; i++)
	{
		temp = std::make_pair(temp.first,temp.first+sectionSize);
		headingCategories.push_back(temp);
	}

	//Dispatch buildings found horizontal line headings into Categories:
	std::vector<std::vector<double>> headingInCategories; 
	headingInCategories.resize(amountOfHeadingCatagories);
	for(double i: horizontalHeadings)
	{
		for(int j=0; j<amountOfHeadingCatagories; j++)
		{
			if(headingCategories[j].first <= i && headingCategories[j].second > i)
			{
				headingInCategories[j].push_back(i);
				break;
			}
			double oppositeFirst = headingCategories[j].first+180;
			double oppositeSecond = headingCategories[j].second+180;
			if(oppositeFirst > 360) {oppositeFirst = oppositeFirst -360;}
			if(oppositeSecond > 360) {oppositeSecond = oppositeSecond -360;}
			if(oppositeFirst <= i && oppositeSecond > i)
			{
				double flipHeading = i-180;
				if(flipHeading < 0){flipHeading = 360+i-180;}
				headingInCategories[j].push_back(flipHeading);
				break;
			}
		}
	}

	// Find most frequent heading category in building
	int biggestCategory = 0;
	int categorySize = 0;
	for(int i=0; i<amountOfHeadingCatagories; i++)
	{
		if(headingInCategories[i].size() > categorySize)
		{
			biggestCategory = i;
			categorySize = headingInCategories[i].size();
		}
	}

	// Are there headings found more often then take that heading, otherwise use the middle of the heading category
	double headingFrequent;
	std::vector<std::pair<double,int>> headingFrequency;
	for(const auto i: headingInCategories[biggestCategory])
	{
		bool found = false;
		for(auto j: headingFrequency)
		{
			if(j.first == i)
			{
				(j.second)++;
				found = true;
				break;
			}
		}
		if(!found)
		{
			std::pair<double,int> temp (i,1);
			headingFrequency.push_back(temp);
		}
	}

	int IDMost = headingFrequency.size() + 1;
	int most = 1;
	for(int i=0; i<headingFrequency.size(); i++)
	{
		if((headingFrequency[i]).second > most)
		{
			most = (headingFrequency[i]).second;
			IDMost = i;
		}
	}
	double ShiftDirectionPerpendicularHeading;
	if(most == 1) // use default middle heading from category if no more then one time a heading is observed
	{
		ShiftDirectionPerpendicularHeading = biggestCategory * categorySize + 0;
	}
	else // use the heading most frequent
	{
		ShiftDirectionPerpendicularHeading = (headingFrequency[IDMost]).first;
	}

	// define the shiftDirection from ShiftDirectionPerpendicularHeading
	std::pair<double,double> headingV (ShiftDirectionPerpendicularHeading, 0);
	bso::utilities::geometry::vector perpendicularV;
	perpendicularV.setHeading(headingV);
	shiftDirection = {perpendicularV.y(),-perpendicularV.x(),0};

	std::cout << "the found location/centre of the building is: " << location << std::endl;
	std::cout << "the heading found is with ID: " << IDMost << " and is found : " << most << " amount of times. " << std::endl;
	std::cout << "the most found frequent Heading is: " << ShiftDirectionPerpendicularHeading << std::endl;
	std::cout << "this heading defined in a vector is: " << perpendicularV << std::endl;
	std::cout << "the found shiftDirection is: " << shiftDirection << std::endl;

	this->shiftFillPlane(location, shiftDirection, shiftAmount, tol);
} // shiftFillPlaneCentre()

void ms_N_building::shiftFillPlane(const bso::utilities::geometry::vertex& location,
		const bso::utilities::geometry::vector& shiftDirection, double shiftAmount, const double& tol /*= 1e-3*/)
{	
	if(shiftDirection.z() > 0 || shiftDirection.x() <= tol && shiftDirection.x() >= -tol && shiftDirection.y() <= tol && shiftDirection.y() >= -tol)
	{
		std::stringstream errorMessage;
				errorMessage << "There is no valid shift Direction defined." << std::endl
										<< "The inserted shift Direction is: " << shiftDirection << std::endl
										<< "(bso/spatial_design/ms_N_building.cpp)." << std::endl;
				throw std::invalid_argument(errorMessage.str());
	}
	if(shiftAmount <= 0)
	{
		std::stringstream errorMessage;
				errorMessage << "The shift amount should be larger then zero." << std::endl
										<< "The inserted shift amount is: " << shiftAmount << std::endl
										<< "(bso/spatial_design/ms_N_building.cpp)." << std::endl;
				throw std::invalid_argument(errorMessage.str());
	}
	
	// normalize the shiftDirection for convenience of later use
	auto n = shiftDirection.normalized();
	bso::utilities::geometry::vector perpendicularToN = {n.y(),-n.x(),0};
	bso::utilities::geometry::vector locationZ0 = {location.x(),location.y(),0};
	bso::utilities::geometry::line_segment planeZ0line = {locationZ0, locationZ0+perpendicularToN};
	
	// Find spaces to shift
	std::vector<ms_space*> notSelectedSpaces;
	std::vector<ms_space*> partialSpaces;
	std::vector<bool> shiftIssueSpaces;
	auto selection = this->selectSpacesGeometrically(location, shiftDirection, notSelectedSpaces, partialSpaces,true,true);
	
	std::vector<ms_space*> selectionShiftIssue;
	if(selection.size() == 0 || notSelectedSpaces.size() == 0)
	{
		std::stringstream errorMessage;
				errorMessage << "There are no spaces found to be shifted or no spaces found to not be shifted. " << std::endl
										<< "the nr of spaces to be shifted are: " << selection.size() << "and the nr of spaces not be be shifted" << notSelectedSpaces.size() << std::endl
										<< "Inserted a different plane, not of location: " << location << "and shiftDirection" << shiftDirection << std::endl
										<< "(bso/spatial_design/ms_N_building.cpp)." << std::endl;
				throw std::invalid_argument(errorMessage.str());
	}

	// Identify toughing & coplanar quads between selected and non-selected spaces and store the levels of the floor for later use:
	std::vector<bso::utilities::geometry::quadrilateral> toughingSQuads;
	std::vector<bso::utilities::geometry::quadrilateral> toughingNSQuads;
	std::vector<bso::utilities::geometry::quadrilateral> toughingSHQuads;
	std::vector<bso::utilities::geometry::quadrilateral> toughingNSHQuads;
	std::vector<double> allLevels;
	for (int i=0; i<selection.size(); i++)
	{
		bool spaceBreak = false;
		bso::utilities::geometry::quad_hexahedron spaceGeomS = selection[i]->getGeometry();
		std::vector<bso::utilities::geometry::quadrilateral> QuadS = spaceGeomS.getQuadrilateral();
		for (auto& j : notSelectedSpaces)
		{
			bso::utilities::geometry::quad_hexahedron spaceGeomNS = j->getGeometry();
			std::vector<bso::utilities::geometry::quadrilateral> QuadNS = spaceGeomNS.getQuadrilateral();
			for(const auto k: QuadS)
			{
				//std::cout << "QuadS is: " << k << std::endl;
				bool QuadSisVertical = k.getNormal().isVertical(); // if true then a horizontal plane is considered
				
				std::vector<bso::utilities::geometry::line_segment> linesk = k.getLines();
				for(const auto t: QuadNS)
				{
					//std::cout << "QuadNS is: " << k << std::endl;
					if(QuadSisVertical && t.getNormal().isVertical()) // both are horizontal planes
					{
						if(!k.isCoplanar(t[0], tol) || !k.isCoplanar(t[2], tol)) continue; // the ID 0 and 2 vertices are opposing vertices therefore if hold then quads are coPlanar
						bool anyIsInside = false;
						for(const auto v: t)
						{
							if(k.isInside(v, tol))
							{
								anyIsInside = true;
								break;
							}
						}
						if(!anyIsInside)
						{
							for(const auto v: k)
							{
								if(t.isInside(v, tol))
								{
									anyIsInside = true;
									break;
								}
							}
						}
						if(anyIsInside)
						{
							bool alreadyAdd = false;
							bool doubleInsertQ1 = false;
							for (const auto& m : toughingSQuads)
							{// check if quad k or t already exist in toughingSQuads
								if (k.isSameAs(m, tol))
								{
									doubleInsertQ1 = true;
								}
							}
							if(!doubleInsertQ1)
							{//if agree, then add to toughingSQuads
								toughingSHQuads.push_back(k);
								allLevels.push_back(k[0].z());
								allLevels.push_back(k[2].z());

								alreadyAdd =  true;
							}
							bool doubleInsertQ2 = false;
							for (const auto& m : toughingNSQuads)
							{// check if quad k or t already exist in toughingNSQuads
								if (t.isSameAs(m, tol))
								{
									doubleInsertQ2 = true;
								}
							}
							if(!doubleInsertQ2)
							{//if agree, then add to toughingNSQuads
								toughingNSHQuads.push_back(t);
								allLevels.push_back(t[0].z());
								allLevels.push_back(t[2].z());

								alreadyAdd =  true;
							}
							if(alreadyAdd) break;
						}
					}
					else
					{					
						if(QuadSisVertical) {continue;} // only consider two vertical planes in the following
						if(!k.isCoplanar(t[0], tol) || !k.isCoplanar(t[2], tol)) continue; // the ID 0 and 2 vertices are opposing vertices therefore if hold then quads are coPlanar
						bool alreadyAdd = false;
						bool AddIfSweepIsOke = false;

						// check for identical quads since the line intersection check in the else statement would not notice this case.
						if(k.isSameAs(t,tol))
						{
							bool doubleInsertQ1 = false;
							for (const auto& m : toughingSQuads)
							{// check if quad k already exist in toughingSQuads
								if (k.isSameAs(m, tol))
								{
									doubleInsertQ1 = true;
								}
							}
							if(!doubleInsertQ1)
							{// if agree, then check and add to toughingSQuads
								// check if quad can be sweeped without interfering with non moving spaces
								bool sweepQuadWillIntersectSpace = false;

								bso::utilities::geometry::vertex pointOnPlaneZ0line0 = planeZ0line.getPointClosestTo(k[0]);
								bso::utilities::geometry::vertex pointOnPlaneZ0line1 = planeZ0line.getPointClosestTo(k[1]);
								bso::utilities::geometry::vertex cornerAtZ00 = {k[0].x(),k[0].y(),0};
								bso::utilities::geometry::vertex cornerAtZ01 = {k[1].x(),k[1].y(),0};
								bso::utilities::geometry::line_segment bottumLine = {cornerAtZ00,cornerAtZ01};
								if(!bottumLine.isOnLine(pointOnPlaneZ0line0, tol) && !bottumLine.isOnLine(pointOnPlaneZ0line1, tol))
								{
									bso::utilities::geometry::line_segment checkLine0 = {pointOnPlaneZ0line0, cornerAtZ00};
									bso::utilities::geometry::line_segment checkLine1 = {pointOnPlaneZ0line1, cornerAtZ01};
									
									for(const auto l: ((spaceGeomNS.getQuadrilateral())[4]).getLines())
									{
										bso::utilities::geometry::line_segment lz0 = {{l[0].x(),l[0].y(),0},{l[1].x(),l[1].y(),0}};
										bso::utilities::geometry::vertex intersectionPoint;
										if(checkLine0.intersectsWith(lz0, intersectionPoint, tol))
										{
											if(!intersectionPoint.isSameAs(checkLine0[0],tol) && !intersectionPoint.isSameAs(checkLine0[1],tol))
											{
												sweepQuadWillIntersectSpace = true;
												break;
											}
										}
										if(checkLine1.intersectsWith(lz0, intersectionPoint, tol))
										{
											if(!intersectionPoint.isSameAs(checkLine1[0],tol) && !intersectionPoint.isSameAs(checkLine1[1],tol))
											{
												sweepQuadWillIntersectSpace = true;
												break;
											}
										}
									}
								}
								
								if(!sweepQuadWillIntersectSpace)
								{
									toughingSQuads.push_back(k);
									allLevels.push_back(k[0].z());
									allLevels.push_back(k[2].z());
								}
								else
								{
									// Do not add quad to toughingQuads and add the space to spaces which need more check for shifting.
									std::cout << "space i is: " << *selection[i] << "and will be removed from to shift spaces. " << std::endl;
									selection[i]->highlightInVisualization = true;
									selectionShiftIssue.push_back(selection[i]);
									spaceBreak = true;
									selection.erase (selection.begin()+i);
									if(i=!0) i--;
									break;
								}
								alreadyAdd =  true;
							}
							bool doubleInsertQ2 = false;
							for (const auto& m : toughingNSQuads)
							{// check if t already exist in toughingNSQuads
								if (t.isSameAs(m, tol))
								{
									doubleInsertQ2 = true;
								}
							}
							if(!doubleInsertQ2)
							{//if agree, then: 
								// check if quad can be sweeped without interfering with non moving spaces
								bool sweepQuadWillIntersectSpace = false;
								
								bso::utilities::geometry::vertex pointOnPlaneZ0line0 = planeZ0line.getPointClosestTo(k[0]);
								bso::utilities::geometry::vertex pointOnPlaneZ0line1 = planeZ0line.getPointClosestTo(k[1]);
								bso::utilities::geometry::vertex cornerAtZ00 = {k[0].x(),k[0].y(),0};
								bso::utilities::geometry::vertex cornerAtZ01 = {k[1].x(),k[1].y(),0};
								bso::utilities::geometry::line_segment bottumLine = {cornerAtZ00,cornerAtZ01};
								if(!bottumLine.isOnLine(pointOnPlaneZ0line0, tol) && !bottumLine.isOnLine(pointOnPlaneZ0line1, tol))
								{
									bso::utilities::geometry::line_segment checkLine0 = {pointOnPlaneZ0line0, cornerAtZ00};
									bso::utilities::geometry::line_segment checkLine1 = {pointOnPlaneZ0line1, cornerAtZ01};
									
									for(const auto l: ((spaceGeomNS.getQuadrilateral())[4]).getLines())
									{
										bso::utilities::geometry::line_segment lz0 = {{l[0].x(),l[0].y(),0},{l[1].x(),l[1].y(),0}};
										bso::utilities::geometry::vertex intersectionPoint;
										if(checkLine0.intersectsWith(lz0, intersectionPoint, tol))
										{
											if(!intersectionPoint.isSameAs(checkLine0[0],tol) && !intersectionPoint.isSameAs(checkLine0[1],tol))
											{
												sweepQuadWillIntersectSpace = true;
												break;
											}
										}
										if(checkLine1.intersectsWith(lz0, intersectionPoint, tol))
										{
											if(!intersectionPoint.isSameAs(checkLine1[0],tol) && !intersectionPoint.isSameAs(checkLine1[1],tol))
											{
												sweepQuadWillIntersectSpace = true;
												break;
											}
										}
									}
								}
								
								if(!sweepQuadWillIntersectSpace)
								{
									toughingNSQuads.push_back(t);
									allLevels.push_back(t[0].z());
									allLevels.push_back(t[2].z());
								}
								else
								{
									// Do not add quad to toughingQuads and add the space to spaces which need more check for shifting.
									std::cout << "space i is: " << *selection[i] << "and will be removed from to shift spaces. " << std::endl;
									selection[i]->highlightInVisualization = true;
									selectionShiftIssue.push_back(selection[i]);
									spaceBreak = true;
									selection.erase (selection.begin()+i);
									if(i=!0) i--;

									// check scenario's:
										//1. is the shiftAmount big enough to avoid overlap? (Note: the earlier check performed does not check for this case)

										//2. is not shifting an option? and if so, which quads should be then take into account, if any?



									break;
								}
								alreadyAdd =  true;
							}
							if(alreadyAdd || spaceBreak) break;
						}
						else
						{
							std::vector<bso::utilities::geometry::line_segment> linest = t.getLines();
							for(const auto l1: linest)
							{
								for(const auto l2: linesk)
								{
									if(l1.intersectsWith(l2, tol))
									{
										bool doubleInsertQ1 = false;
										for (const auto& m : toughingSQuads)
										{// check if quad k already exist in toughingSQuads
											if (k.isSameAs(m, tol))
											{
												doubleInsertQ1 = true;
											}
										}
										if(!doubleInsertQ1)
										{// if agree, then check and add to toughingSQuads
											// check if quad can be sweeped without interfering with non moving spaces
											bool sweepQuadWillIntersectSpace = false;

											bso::utilities::geometry::vertex pointOnPlaneZ0line0 = planeZ0line.getPointClosestTo(k[0]);
											bso::utilities::geometry::vertex pointOnPlaneZ0line1 = planeZ0line.getPointClosestTo(k[1]);
											bso::utilities::geometry::vertex cornerAtZ00 = {k[0].x(),k[0].y(),0};
											bso::utilities::geometry::vertex cornerAtZ01 = {k[1].x(),k[1].y(),0};
											bso::utilities::geometry::line_segment bottumLine = {cornerAtZ00,cornerAtZ01};
											if(!bottumLine.isOnLine(pointOnPlaneZ0line0, tol) && !bottumLine.isOnLine(pointOnPlaneZ0line1, tol))
											{
												bso::utilities::geometry::line_segment checkLine0 = {pointOnPlaneZ0line0, cornerAtZ00};
												bso::utilities::geometry::line_segment checkLine1 = {pointOnPlaneZ0line1, cornerAtZ01};
												
												for(const auto l: ((spaceGeomNS.getQuadrilateral())[4]).getLines())
												{
													bso::utilities::geometry::line_segment lz0 = {{l[0].x(),l[0].y(),0},{l[1].x(),l[1].y(),0}};
													bso::utilities::geometry::vertex intersectionPoint;
													if(checkLine0.intersectsWith(lz0, intersectionPoint, tol))
													{
														if(!intersectionPoint.isSameAs(checkLine0[0],tol) && !intersectionPoint.isSameAs(checkLine0[1],tol))
														{
															sweepQuadWillIntersectSpace = true;
															break;
														}
													}
													if(checkLine1.intersectsWith(lz0, intersectionPoint, tol))
													{
														if(!intersectionPoint.isSameAs(checkLine1[0],tol) && !intersectionPoint.isSameAs(checkLine1[1],tol))
														{
															sweepQuadWillIntersectSpace = true;
															break;
														}
													}
												}
											}
											
											if(!sweepQuadWillIntersectSpace)
											{
												toughingSQuads.push_back(k);
												allLevels.push_back(k[0].z());
												allLevels.push_back(k[2].z());
											}
											else
											{
												// Do not add quad to toughingQuads and add the space to spaces which need more check for shifting.
												//std::cout << "space i is: " << *selection[i] << "and will be removed from to shift spaces. " << std::endl;
												selection[i]->highlightInVisualization = true;
												selectionShiftIssue.push_back(selection[i]);
												spaceBreak = true;
												selection.erase (selection.begin()+i);
												if(i=!0) i--;
												break;
											}
											alreadyAdd =  true;
										}
										bool doubleInsertQ2 = false;
										for (const auto& m : toughingNSQuads)
										{// check if t already exist in toughingNSQuads
											if (t.isSameAs(m, tol))
											{
												doubleInsertQ2 = true;
											}
										}
										if(!doubleInsertQ2)
										{//if agree, then: 
											// check if quad can be sweeped without interfering with non moving spaces
											bool sweepQuadWillIntersectSpace = false;
											
											bso::utilities::geometry::vertex pointOnPlaneZ0line0 = planeZ0line.getPointClosestTo(k[0]);
											bso::utilities::geometry::vertex pointOnPlaneZ0line1 = planeZ0line.getPointClosestTo(k[1]);
											bso::utilities::geometry::vertex cornerAtZ00 = {k[0].x(),k[0].y(),0};
											bso::utilities::geometry::vertex cornerAtZ01 = {k[1].x(),k[1].y(),0};
											bso::utilities::geometry::line_segment bottumLine = {cornerAtZ00,cornerAtZ01};
											if(!bottumLine.isOnLine(pointOnPlaneZ0line0, tol) && !bottumLine.isOnLine(pointOnPlaneZ0line1, tol))
											{
												bso::utilities::geometry::line_segment checkLine0 = {pointOnPlaneZ0line0, cornerAtZ00};
												bso::utilities::geometry::line_segment checkLine1 = {pointOnPlaneZ0line1, cornerAtZ01};
												
												for(const auto l: ((spaceGeomNS.getQuadrilateral())[4]).getLines())
												{
													bso::utilities::geometry::line_segment lz0 = {{l[0].x(),l[0].y(),0},{l[1].x(),l[1].y(),0}};
													bso::utilities::geometry::vertex intersectionPoint;
													if(checkLine0.intersectsWith(lz0, intersectionPoint, tol))
													{
														if(!intersectionPoint.isSameAs(checkLine0[0],tol) && !intersectionPoint.isSameAs(checkLine0[1],tol))
														{
															sweepQuadWillIntersectSpace = true;
															break;
														}
													}
													if(checkLine1.intersectsWith(lz0, intersectionPoint, tol))
													{
														if(!intersectionPoint.isSameAs(checkLine1[0],tol) && !intersectionPoint.isSameAs(checkLine1[1],tol))
														{
															sweepQuadWillIntersectSpace = true;
															break;
														}
													}
												}
											}
											
											if(!sweepQuadWillIntersectSpace)
											{
												toughingNSQuads.push_back(t);
												allLevels.push_back(t[0].z());
												allLevels.push_back(t[2].z());
											}
											else
											{
												// Do not add quad to toughingQuads and add the space to spaces which need more check for shifting.
												//std::cout << "space i is: " << *selection[i] << "and will be removed from to shift spaces. " << std::endl;
												selection[i]->highlightInVisualization = true;
												selectionShiftIssue.push_back(selection[i]);
												spaceBreak = true;
												selection.erase (selection.begin()+i);
												if(i=!0) i--;

												// check scenario's:
												//1. is the shiftAmount big enough to avoid overlap? (Note: the earlier check performed does not check for this case)

												//2. is not shifting an option? and if so, which quads should be then take into account, if any?




												break;
											}
											alreadyAdd =  true;
										}
										if(alreadyAdd || spaceBreak) break;
									}
								}
								if(alreadyAdd || spaceBreak) break;
							}
						}
					}
					if(spaceBreak) break;
				}
				if(spaceBreak) break;
			}
			if(spaceBreak) break;
		}
	}

	if(selection.size() == 0 || notSelectedSpaces.size() == 0)
	{
		std::stringstream errorMessage;
				errorMessage << "There are no spaces found to be shifted after searching for Quads. " << std::endl
										<< "Meaning that spaces where removed from being shifted because they would run into non-moving spaces " << std::endl
										<< "the nr of spaces to be shifted are: " << selection.size() << "and the nr of spaces not be be shifted" << notSelectedSpaces.size() << std::endl
										<< "Inserted a different plane, not of location: " << location << "and shiftDirection" << shiftDirection << std::endl
										<< "(bso/spatial_design/ms_N_building.cpp)." << std::endl;
				throw std::invalid_argument(errorMessage.str());
	}

	// Sort vector, bring all duplicates to end of vector plus get cut off point, then erase duplicates from the end of vector allLevels:
	sort(allLevels.begin(), allLevels.end()); 
	auto it = unique(allLevels.begin(), allLevels.end()); 
	allLevels.erase(it, allLevels.end()); 

	// Determine the bottom lines of each toughing quad and add to linesAtFloor:
	std::vector<std::vector<bso::utilities::geometry::line_segment>> linesAtFloor;
	linesAtFloor.resize(allLevels.size()-1);

	for(int i=0; i< toughingSQuads.size(); i++)
	{
		std::vector<bso::utilities::geometry::line_segment> lines = toughingSQuads[i].getLines();
		for(int j=0; j<allLevels.size()-1; j++)
		{
			if(allLevels[j] >= (toughingSQuads[i][0]).z() && allLevels[j] < (toughingSQuads[i][2]).z())// toughingSQuads[i][2] represent top level quad and toughingSQuads[i][0] represent bottom level quad
			{
				bso::utilities::geometry::line_segment lineAtLevel = {{lines[0][0].x(),lines[0][0].y(),allLevels[j]},{lines[0][1].x(),lines[0][1].y(),allLevels[j]}};
				linesAtFloor[j].push_back(lineAtLevel);
			}
		}
	}
	for(int i=0; i< toughingNSQuads.size(); i++)
	{
		std::vector<bso::utilities::geometry::line_segment> lines = toughingNSQuads[i].getLines();
		for(int j=0; j<allLevels.size()-1; j++)
		{
			if(allLevels[j] >= (toughingNSQuads[i][0]).z() && allLevels[j] < (toughingNSQuads[i][2]).z())// toughingNSQuads[i][2] represent top level quad and toughingNSQuads[i][0] represent bottom level quad
			{
				bso::utilities::geometry::line_segment lineAtLevel = {{lines[0][0].x(),lines[0][0].y(),allLevels[j]},{lines[0][1].x(),lines[0][1].y(),allLevels[j]}};
				linesAtFloor[j].push_back(lineAtLevel);
			}
		}
	}

	// Look into all unique selectionShiftIssue spaces
	for(auto& i : selectionShiftIssue)
	{

	}

	// Shift spaces
	for (auto& i : selection)
	{
		// Define the new pVertex vector of considered space
		i->highlightInVisualization1 = true;
		bso::utilities::geometry::quad_hexahedron spaceGeom = i->getGeometry();
		std::vector <utilities::geometry::vertex*> pVert;
		utilities::geometry::vertex temp;
		for(const auto j: spaceGeom.getVertices())
		{
			pVert.push_back(new utilities::geometry::vertex);							
			temp = j + n * shiftAmount;
			*pVert.back() = temp;
		}
		// Replace the pVertex of the considered space
		i->setPVertex(pVert);
	}

	// first split lines at all the vertices then remove lines which have no duplicate at the other (either selected or nonSelected) side 
	// to remove the non toughing sections of the found quads
	std::vector<std::vector<bso::utilities::geometry::line_segment>> lines;
	lines.resize(allLevels.size()-1);
	
	std::vector<std::vector<bso::utilities::geometry::vertex>> allVertices;
	allVertices.resize(allLevels.size()-1);
	for(int i=0; i<linesAtFloor.size(); i++)
	{
		for(auto j: linesAtFloor[i])
		{
			allVertices[i].push_back(j[0]);
			allVertices[i].push_back(j[1]);
		}
	}
	for(int i=0; i<allLevels.size()-1; i++)
	{
		bso::utilities::splitLines(allVertices[i], linesAtFloor[i], false, tol);

		for(int j=0; j< linesAtFloor[i].size(); j++)
		{
			int index=0;
			while(index < linesAtFloor[i].size())
			{
				if(j == index)
				{
					index++;
					continue;
				}
				if((linesAtFloor[i][j]).isSameAs(linesAtFloor[i][index], tol)) break;
				index++;
			}
			if(index < linesAtFloor[i].size())
			{ // Duplicate of line found so keep one of them
				linesAtFloor[i].erase(linesAtFloor[i].begin() + index);
			}
			else
			{ // No duplicate found so erase the considered line
				linesAtFloor[i].erase(linesAtFloor[i].begin() + j);
				j--;
			}
		}
	}

	
			
	// Generate lines opposing side i.e. shift the line so they are on the position of the shifted spaces and remove the lines invalid for space generation
	std::vector<std::vector<bso::utilities::geometry::line_segment>> linesOpposite;
	linesOpposite.resize(allLevels.size()-1);
	for(int i=0; i< linesAtFloor.size(); i++)
	{
		for(int j=0; j<linesAtFloor[i].size(); j++)
		{
			bso::utilities::geometry::vertex v1 = (linesAtFloor[i][j])[0] + n * shiftAmount;
			bso::utilities::geometry::vertex v2 = (linesAtFloor[i][j])[1] + n * shiftAmount;
			bso::utilities::geometry::line_segment l = {v1,v2};
			
			// determine if line wil form a valid space:
			bool noSeparation = false;
			bso::utilities::geometry::vector vec1 = linesAtFloor[i][j].getVector();
			bso::utilities::geometry::vector vec2 = -1*vec1;
			if(vec1.isCodirectional(n)) {noSeparation = true;}
			else if(vec2.isCodirectional(n)) {noSeparation = true;}

			if(noSeparation)
			{ // if not, remove line
				linesAtFloor[i].erase(linesAtFloor[i].begin() + j);
				if(j!=0) j--;
			}
			else
			{ // if yes, add line to opposing so that the pair of the lines wil form a space
				linesOpposite[i].push_back(l);
			}
		}
	}
	
	// Define spaces between the pairs of lines and linesOpposites
	for(int i=0; i<linesAtFloor.size(); i++)
	{
		// elevation height
		bso::utilities::geometry::vector hightVector = {0,0,allLevels[i+1]-allLevels[i]};
		
		for(int j=0; j< linesAtFloor[i].size(); j++)
		{
			// Define vertices of new space
			std::vector <utilities::geometry::vertex*> vertexes;

			vertexes.push_back(new utilities::geometry::vertex);
			*vertexes.back() = linesAtFloor[i][j][0];
			vertexes.push_back(new utilities::geometry::vertex);
			*vertexes.back() = linesAtFloor[i][j][1];
			vertexes.push_back(new utilities::geometry::vertex);
			*vertexes.back() = linesOpposite[i][j][0];
			vertexes.push_back(new utilities::geometry::vertex);
			*vertexes.back() = linesOpposite[i][j][1];
			vertexes.push_back(new utilities::geometry::vertex);
			*vertexes.back() = linesAtFloor[i][j][0]+hightVector;
			vertexes.push_back(new utilities::geometry::vertex);
			*vertexes.back() = linesAtFloor[i][j][1]+hightVector;
			vertexes.push_back(new utilities::geometry::vertex);
			*vertexes.back() = linesOpposite[i][j][0]+hightVector;
			vertexes.push_back(new utilities::geometry::vertex);
			*vertexes.back() = linesOpposite[i][j][1]+hightVector;
		
			// Define new space
			int newID = mSpaces.back()->getID();
			newID++;
			mSpaces.push_back(new ms_space());
			mSpaces.back()->setID(newID);
			mSpaces.back()->setPVertex(vertexes);
			std::cout << "space added: " << *mSpaces.back() << std::endl;
		}
	}

	// Check if shifted spaces and non-shifted spaces really leave a capp to fill
	



} // shiftFillCutPlane

std::vector<ms_space*> ms_N_building::selectSpacesGeometrically(
		const bso::utilities::geometry::vertex& location,
		const bso::utilities::geometry::vector& normalOfPlane,
		std::vector<ms_space*>& notSelectedSpaces,
		std::vector<ms_space*>& partialSpaces,
		const bool includePartialSpaces /*= false*/,
		const bool returnNormalDefinedSide /*= false*/) const
{
	bso::utilities::geometry::vector normal = normalOfPlane;
	if(!returnNormalDefinedSide){normal = {-normalOfPlane.x(),-normalOfPlane.y(),-normalOfPlane.z()};}
	
	std::vector<ms_space*> spaceSelection;
	for (const auto& i : mSpaces)
	{
		bool allVerticesSelected = true;
		bool oneOrMoreVerticesSelected = false;
		bool partialSelection = false;
		for (const auto& j : i->getGeometry())
		{
			double checkValue = normal.dot(j - location);
			if (checkValue >= -1e-3)
			{ // check if point j lies behind the plane defined by the location and the direction
				if (checkValue >= 1e-3) oneOrMoreVerticesSelected = true;
			}
			else {allVerticesSelected = false;}
			if (checkValue >= -1e-3 && checkValue <= 1e-3) partialSelection = true;
		}
		if (allVerticesSelected) spaceSelection.push_back(i);
		else if (oneOrMoreVerticesSelected)
		{
			if(includePartialSpaces) spaceSelection.push_back(i);
			partialSpaces.push_back(i);
		}
		else notSelectedSpaces.push_back(i);
		if(partialSelection) partialSpaces.push_back(i);
	}
	return spaceSelection;
} // selectSpacesGeometrically

std::vector<ms_space*> ms_N_building::selectSpacesGeometrically(
		const bso::utilities::geometry::vertex& location,
		const bso::utilities::geometry::vector& normalOfPlane,
		const bool includePartialSpaces /*= false*/,
		const bool returnNormalDefinedSide /*= false*/) const
{
	std::vector<ms_space*> notSelectedSpaces; // empty variable
	std::vector<ms_space*> partialSpaces; // empty variable
	std::vector<ms_space*> spaceSelection = selectSpacesGeometrically(location, normalOfPlane, notSelectedSpaces, partialSpaces, includePartialSpaces, returnNormalDefinedSide);
	return spaceSelection;
} // selectSpacesGeometrically

void ms_N_building::replicateSplits(std::vector<ms_space*> spaces)
{
	std::vector<std::pair<bso::spatial_design::ms_space*, utilities::geometry::quad_hexahedron>> spaceAtGroundV;
	std::vector<double> allLevels;
	std::vector <utilities::geometry::line_segment> linesAtZ;

	// These functions get the above defined information
	double mTol = 0.001;
	std::vector<bso::utilities::geometry::vertex> delauneyPoints;
	std::vector<bso::utilities::geometry::line_segment> delauneyLines;

	
	// for(auto i: spaces)
	// {
		
	// 	utilities::geometry::quad_hexahedron spaceGeom = i->getGeometry();
	// 	std::vector<utilities::geometry::line_segment> linesToConsider;
	// 	for(const auto j: linesAtZ)
	// 	{
	// 		if(spaceGeom.isInsideOrOn(j[0]) || spaceGeom.isInsideOrOn(j[1])) linesToConsider.push_back(j);
	// 	}

	// 	if(linesToConsider.size() == 0)
	// 	{

	// 	}
	// 	else if(linesToConsider.size() == 0)
	// 	{

	// 	}
	// }



}//replicateSplits()


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

void ms_N_building::getSpacesAtGround(std::vector<bso::spatial_design::ms_space*> spacePtr, std::vector<std::pair<bso::spatial_design::ms_space*, utilities::geometry::quad_hexahedron>>& spaceAtGroundV, std::vector<double>& allLevels, const double mTol)
{
	for (const auto i : spacePtr)
	{
		//Get original space geometry
		utilities::geometry::quad_hexahedron spaceGeom = i->getGeometry();
					
		//Find the highest and lowest vertex to add to allLevels and for the definition of the shifting vector
		utilities::geometry::vertex lowVertex = spaceGeom[0];
		utilities::geometry::vertex highVertex = spaceGeom[0];
		for (const auto j: spaceGeom)
		{
			if(lowVertex.z() > j.z()) lowVertex = j;
			else if(highVertex.z() < j.z()) highVertex = j;
		}			
		allLevels.push_back(highVertex.z());
		allLevels.push_back(lowVertex.z());

		//Generate the at ground level space geometry
		std::vector<utilities::geometry::vertex> vertices_1;
		utilities::geometry::vertex shift = {0,0,lowVertex.z()};
		if(lowVertex.z() < mTol && lowVertex.z() > -mTol) for(auto k: spaceGeom){vertices_1.push_back(k);}
		else for(auto k: spaceGeom){vertices_1.push_back(k-shift);}
		utilities::geometry::quad_hexahedron atGround = utilities::geometry::quad_hexahedron(vertices_1, mTol);

		std::pair<bso::spatial_design::ms_space*, utilities::geometry::quad_hexahedron> spaceGeomPair (i,atGround);
		spaceAtGroundV.push_back(spaceGeomPair);
	}
	
	// Sort vector, bring all duplicates to end of vector plus get cut off point, then erase duplicates from the end of vector allLevels:
	sort(allLevels.begin(), allLevels.end()); 
	auto it = unique(allLevels.begin(), allLevels.end()); 
	allLevels.erase(it, allLevels.end()); 
} //getNOBSDAtGround

void ms_N_building::getNOBSDAtGround(std::vector<std::pair<bso::spatial_design::ms_space*, utilities::geometry::quad_hexahedron>>& spaceAtGroundV, std::vector<double>& allLevels, const double mTol)
{
	for (const auto i : mSpaces)
	{
		//Get original space geometry
		utilities::geometry::quad_hexahedron spaceGeom = i->getGeometry();
					
		//Find the highest and lowest vertex to add to allLevels and for the definition of the shifting vector
		utilities::geometry::vertex lowVertex = spaceGeom[0];
		utilities::geometry::vertex highVertex = spaceGeom[0];
		for (const auto j: spaceGeom)
		{
			if(lowVertex.z() > j.z()) lowVertex = j;
			else if(highVertex.z() < j.z()) highVertex = j;
		}			
		allLevels.push_back(highVertex.z());
		allLevels.push_back(lowVertex.z());

		//Generate the at ground level space geometry
		std::vector<utilities::geometry::vertex> vertices_1;
		utilities::geometry::vertex shift = {0,0,lowVertex.z()};
		if(lowVertex.z() < mTol && lowVertex.z() > -mTol) for(auto k: spaceGeom){vertices_1.push_back(k);}
		else for(auto k: spaceGeom){vertices_1.push_back(k-shift);}
		utilities::geometry::quad_hexahedron atGround = utilities::geometry::quad_hexahedron(vertices_1, mTol);

		std::pair<bso::spatial_design::ms_space*, utilities::geometry::quad_hexahedron> spaceGeomPair (i,atGround);
		spaceAtGroundV.push_back(spaceGeomPair);
	}
	
	// Sort vector, bring all duplicates to end of vector plus get cut off point, then erase duplicates from the end of vector allLevels:
	sort(allLevels.begin(), allLevels.end()); 
	auto it = unique(allLevels.begin(), allLevels.end()); 
	allLevels.erase(it, allLevels.end()); 
} //getNOBSDAtGround

void ms_N_building::getNOBSDAtGround(std::vector<std::pair<bso::spatial_design::ms_space*, utilities::geometry::quad_hexahedron>>& spaceAtGroundV, const double mTol)
{
	std::vector<double> allLevels; // create empty variable
	getNOBSDAtGround(spaceAtGroundV, allLevels, mTol);
} //getNOBSDAtGround

std::vector <bso::utilities::geometry::quadrilateral> ms_N_building::getQuadAtZ0(std::vector<bso::spatial_design::ms_space*> spaces, const double mTol)
{
	std::vector <bso::utilities::geometry::quadrilateral> quadAtZ0;
	for (const auto i : spaces)
	{
		//Get original space geometry
		utilities::geometry::quad_hexahedron spaceGeom = i->getGeometry();

		for (const auto& k : spaceGeom.getQuadrilateral())
		{
			if(!k.getNormal().isVertical()) continue; // only consider horizontal floors

			//Define shift and define at z=0 quads
			std::vector<utilities::geometry::vertex> vertices_1;
			utilities::geometry::vertex shift = {0,0,k[0].z()};
			if(shift.z() < mTol && shift.z() > -mTol) for(auto l: k){vertices_1.push_back(l);}
			else for(auto l: k){vertices_1.push_back(l-shift);}
			utilities::geometry::quadrilateral quadAtGround = utilities::geometry::quadrilateral(vertices_1, mTol);

			
			bool doubleInsert = false;
			for (const auto& m : quadAtZ0)
			{// check if quad k already exist in quadAtZ0
				if (quadAtGround.isSameAs(m,mTol))
				{
					doubleInsert = true;
				}
			}
			if(doubleInsert == false)
			{//if agree, then add to quadAtZ0
				quadAtZ0.push_back(quadAtGround);
			}
		}
	}
	
	return quadAtZ0;
} //getQuadAtZ0

std::vector <utilities::geometry::line_segment> ms_N_building::getLinesAtZ0(std::vector<bso::spatial_design::ms_space*> spaces, const double mTol)
{
	std::vector <utilities::geometry::line_segment> linesAtZ;
	for (const auto i : spaces)
	{
		//Get original space geometry
		utilities::geometry::quad_hexahedron spaceGeom = i->getGeometry();
		
		// Add the unique lines of each space at Z to linesAtZ
		for (const auto& k : spaceGeom.getLines())
		{
			utilities::geometry::vertex Ltemp1 = {k[0].x(),k[0].y(),0};
			utilities::geometry::vertex Ltemp2 = {k[1].x(),k[1].y(),0};
			if(Ltemp1.isSameAs(Ltemp2, mTol)) {continue;} //only allow non-vertical lines
			utilities::geometry::line_segment Ltemp {{Ltemp1},{Ltemp2}}; 
			
			bool doubleInsert = false;
			for (const auto& m : linesAtZ)
			{// check if line k already exist in linesAtZ
				if (m.isSameAs(Ltemp, mTol))
				{
					doubleInsert = true;
				}
			}
			if(doubleInsert == false)
			{//if both agree, then add to linesAtZ
				linesAtZ.push_back(Ltemp);
			}
		}
	}
	return linesAtZ;
} //getLinesAtZ0

bso::utilities::geometry::vertex ms_N_building::getCentreBuilding()
{
	bso::utilities::geometry::vertex centreOfBuilding = {0,0,0};
	for(const auto i: mSpaces)
	{
		centreOfBuilding = centreOfBuilding + i->getGeometry().getCenter();
	}
	centreOfBuilding = centreOfBuilding/mSpaces.size();
	return centreOfBuilding;
} //getCentreBuilding()

void ms_N_building::getDelaunayVariables(std::vector <utilities::geometry::vertex>& delaunayPoints, std::vector <utilities::geometry::line_segment>& delaunayLines, std::vector<std::pair<bso::spatial_design::ms_space*, utilities::geometry::quad_hexahedron>>& spacePair, const double mTol)
{
	for(const auto j: spacePair)
	{
		// Add all unique cornerpoints at z=0 of spaceAtGround to the delaunayPoints
		for(auto v: j.second)
		{
			utilities::geometry::vertex temp = {v.x(),v.y(),0};
			bool doubleInsert = false;
			for (const auto& m : delaunayPoints)
			{// check if temp already exist in delaunayPoints
				if (m == temp)
				{
					doubleInsert = true;
				}
			}
			if(doubleInsert == false)
			{//if not, then add to delaunayPoints
				delaunayPoints.push_back(temp);
			}
		}

		// Add the unique lines of each space to delaunayLines
		for (const auto& k : (j.second).getLines())
		{
			utilities::geometry::vertex Ltemp1 = {k[0].x(),k[0].y(),0};
			utilities::geometry::vertex Ltemp2 = {k[1].x(),k[1].y(),0};
			utilities::geometry::line_segment Ltemp {{Ltemp1},{Ltemp2}}; //Note the line is newly defined for the case that some round of error occur during the shifting of the lines of the spacesAtGround[i]
			
			bool doubleInsert = false;
			for (const auto& m : delaunayLines)
			{// check if line k already exist in delaunayLines
				if (m.isSameAs(Ltemp, mTol))
				{
					doubleInsert = true;
				}
			}
			if(Ltemp1.isSameAs(Ltemp2, mTol))
			{//Check if line has different end points
				doubleInsert = true;
			}
			if(doubleInsert == false)
			{//if both agree, then add to delaunayLines
				delaunayLines.push_back(Ltemp);
			}
			
			//Find line-line intersections and add to delaunayPoints, then during splitLines() the lines will be split at this delaunayPoint
			//Note: no line-rectangle intersection check needed since these are accounted for by the addition of the cornerpoints
			utilities::geometry::vertex pIntersection;
			for(auto q: spacePair)
			{	
				if((q.second).isSameAs((j.second), mTol)){continue;}
				for (const auto& l : (q.second).getLines())
				{
					if (k.intersectsWith(l, pIntersection, mTol))
					{
						//consider the translated to ground floor vertex of the space
						utilities::geometry::vertex temp = {pIntersection.x(),pIntersection.y(),0};
						
						bool doubleInsert = false;
						for (const auto& m : delaunayPoints)
						{// check if temp already exist in delaunayPoints
							if (m.isSameAs(temp, mTol))
							{
								doubleInsert = true;
							}
						}
						
						if(doubleInsert == false)
						{//if not, then add to delaunayPoints
							delaunayPoints.push_back(temp);
						}
					}
				}
			}
		}
	}

	bso::utilities::splitLines(delaunayPoints, delaunayLines);
} // getDelaunayVariables

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