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

void ms_N_building::shiftFillPlane(const bso::utilities::geometry::vertex& location,
		const bso::utilities::geometry::vector& shiftDirection, double shiftAmount, const double& tol /*= 1e-3*/)
{	
	if(shiftDirection.z() > 0)
	{
		std::stringstream errorMessage;
				errorMessage << "There is no valid shift amount defined." << std::endl
										<< "The inserted shift amount is: " << shiftAmount << std::endl
										<< "(bso/spatial_design/ms_N_building.cpp)." << std::endl;
				throw std::invalid_argument(errorMessage.str());
	}
	
	// normalize the shiftDirection for convenience of later use
	auto n = shiftDirection.normalized();
	
	// Find spaces to shift
	std::vector<ms_space*> notSelectedSpaces;
	std::vector<ms_space*> partialSpaces;
	auto selection = this->selectSpacesGeometrically(location, shiftDirection, notSelectedSpaces, partialSpaces,true,true);
	if(selection.size() == 0 || notSelectedSpaces.size() == 0)
	{
		std::stringstream errorMessage;
				errorMessage << "There are no spaces found to be shifted or no spaces found to not be shifted. " << std::endl
										<< "the nr of spaces to be shifted are: " << selection.size() << "and the nr of spaces not be be shifted" << notSelectedSpaces.size() << std::endl
										<< "Inserted a different plane, not of location: " << location << "and shiftDirection" << shiftDirection << std::endl
										<< "(bso/spatial_design/ms_N_building.cpp)." << std::endl;
				throw std::invalid_argument(errorMessage.str());
	}

	// Identify toughing & coplanar quads and store the levels of the floor for later use:
	std::vector<bso::utilities::geometry::quadrilateral> toughingSQuads;
	std::vector<bso::utilities::geometry::quadrilateral> toughingNSQuads;
	std::vector<double> allLevels;
	std::vector<double> allLevelsS;
	std::vector<double> allLevelsSN;
	for (auto& i : selection)
	{
		bso::utilities::geometry::quad_hexahedron spaceGeomS = i->getGeometry();
		std::vector<bso::utilities::geometry::quadrilateral> QuadS = spaceGeomS.getQuadrilateral();
		for (auto& j : notSelectedSpaces)
		{
			bso::utilities::geometry::quad_hexahedron spaceGeomNS = j->getGeometry();
			std::vector<bso::utilities::geometry::quadrilateral> QuadNS = spaceGeomNS.getQuadrilateral();
			for(const auto k: QuadS)
			{
				if(k.getNormal().isVertical()) continue; // only consider vertical planes
				std::vector<bso::utilities::geometry::line_segment> linesk = k.getLines();
				for(const auto t: QuadNS)
				{
					if(k.getNormal().isVertical()) continue; // only consider vertical planes
					if(!k.isCoplanar(t[0], tol) || !k.isCoplanar(t[2], tol)) continue; // the ID 0 and 2 vertices are opposing vertices therefore if hold then quads are coPlanar
					bool alreadyAdd = false;
					std::vector<bso::utilities::geometry::line_segment> linest = t.getLines();
					for(const auto l1: linest)
					{
						for(const auto l2: linesk)
						{
							if(l1.intersectsWith(l2, tol))
							{
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
									toughingSQuads.push_back(k);
									allLevels.push_back(k[0].z());
									allLevelsS.push_back(allLevels.back());
									allLevels.push_back(k[2].z());
									allLevelsS.push_back(allLevels.back());

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
									toughingNSQuads.push_back(t);
									allLevels.push_back(t[0].z());
									allLevelsSN.push_back(allLevels.back());
									allLevels.push_back(t[2].z());
									allLevelsSN.push_back(allLevels.back());
									alreadyAdd =  true;
								}
								if(alreadyAdd) break;
							}
						}
						if(alreadyAdd) break;
					}
				}
			}
		}
	}

	// Sort vector, bring all duplicates to end of vector plus get cut off point, then erase duplicates from the end of vector allLevels:
	sort(allLevels.begin(), allLevels.end()); 
	auto it = unique(allLevels.begin(), allLevels.end()); 
	allLevels.erase(it, allLevels.end()); 

	// Sort vector, bring all duplicates to end of vector plus get cut off point, then erase duplicates from the end of vector allLevelsS:
	sort(allLevelsS.begin(), allLevelsS.end()); 
	auto itS = unique(allLevelsS.begin(), allLevelsS.end()); 
	allLevelsS.erase(itS, allLevelsS.end()); 

	// Sort vector, bring all duplicates to end of vector plus get cut off point, then erase duplicates from the end of vector allLevelsSN:
	sort(allLevelsSN.begin(), allLevelsSN.end()); 
	auto itSN = unique(allLevelsSN.begin(), allLevelsSN.end()); 
	allLevelsSN.erase(itSN, allLevelsSN.end()); 

	// Pick the smallest elevation of either selected or non-selected to determine to which elevation the fill area will be filled.
	bool elevationFillSelected = true;
	if(allLevelsS.back() > allLevelsSN.back()) elevationFillSelected = false;

	std::cout << "toughingSQuads: " << std::endl;
	for(const auto i: toughingSQuads) std::cout << i << std::endl;
	std::cout << "toughingNSQuads: " << std::endl;
	for(const auto i: toughingNSQuads) std::cout << i << std::endl;


	// Find the lines for a polygon_non_convex of each elevation:
	std::vector<std::vector<bso::utilities::geometry::line_segment>> polygons;
	std::vector<std::vector<bso::utilities::geometry::line_segment>> polygonsS;
	std::vector<std::vector<bso::utilities::geometry::line_segment>> polygonsSN;
	polygons.resize(allLevels.size()-1);
	polygonsS.resize(allLevels.size()-1);
	polygonsSN.resize(allLevels.size()-1);

	for(int i=0; i< toughingSQuads.size(); i++)
	{
		std::vector<bso::utilities::geometry::line_segment> lines = toughingSQuads[i].getLines();
		for(int j=0; j<allLevels.size()-1; j++)
		{
			if((toughingSQuads[i][0]).z() >= allLevels[j] && (toughingSQuads[i][0]).z() < allLevels[j+1])
			{
				polygons[j].push_back(lines[0]);
		 		polygonsS[j].push_back(lines[0]);
			}
		}
	}
	for(int i=0; i< toughingNSQuads.size(); i++)
	{
		std::vector<bso::utilities::geometry::line_segment> lines = toughingNSQuads[i].getLines();
		for(int j=0; j<allLevels.size()-1; j++)
		{
			if((toughingNSQuads[i][0]).z() >= allLevels[j] && (toughingNSQuads[i][0]).z() < allLevels[j+1])
			{
				polygons[j].push_back(lines[0]);
				polygonsSN[j].push_back(lines[0]);
			}
		}
	}

	std::cout << "\n polygons: " << std::endl;
	int count = 0;
	for(const auto i: polygons)
	{
		std::cout << "elevation " << count << std::endl;
		for(const auto j: i) std::cout << j << std::endl;
		count ++;
	} 

	std::cout << "\n polygonsS: " << std::endl;
	count = 0;
	for(const auto i: polygonsS)
	{
		std::cout << "elevation " << count << std::endl;
		for(const auto j: i) std::cout << j << std::endl;
		count ++;
	} 

	std::cout << "\n polygonsSN: " << std::endl;
	count = 0;
	for(const auto i: polygonsSN)
	{
		std::cout << "elevation " << count << std::endl;
		for(const auto j: i) std::cout << j << std::endl;
		count ++;
	} 

	// Shift spaces
	for (auto& i : selection)
	{
		// Define the new pVertex vector of considered space
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


	// remove lines which have no duplicate at the other (either selected or nonSelected) side
	std::vector<std::vector<bso::utilities::geometry::line_segment>> selectedLines;
	std::vector<std::vector<bso::utilities::geometry::line_segment>> nonSelectedLines;
	selectedLines.resize(allLevels.size()-1);
	nonSelectedLines.resize(allLevels.size()-1);
	
	std::vector<std::vector<bso::utilities::geometry::vertex>> allVertices;
	allVertices.resize(allLevels.size()-1);
	for(int i=0; i<polygons.size(); i++)
	{
		for(auto j: polygons[i])
		{
			allVertices[i].push_back(j[0]);
			allVertices[i].push_back(j[1]);
		}
	}
	for(int i=0; i<allLevels.size()-1; i++)
	{
		bso::utilities::splitLines(allVertices[i], polygonsS[i], tol);
		bso::utilities::splitLines(allVertices[i], polygonsSN[i], tol);

		for(int j=0; j< polygonsS[i].size(); j++)
		{
			int index=0;
			while(index < polygonsSN[i].size())
			{
				if((polygonsS[i][j]).isSameAs(polygonsSN[i][index], tol)) break;
				index++;
			}
			if(index < polygonsSN[i].size())
			{ // Duplicate of line found so keep
				// shift line since spaces will be shifted to this position:
				bso::utilities::geometry::vertex v1 = (polygonsS[i][j])[0] + n * shiftAmount;
				bso::utilities::geometry::vertex v2 = (polygonsS[i][j])[1] + n * shiftAmount;

				bso::utilities::geometry::line_segment l = {v1,v2};
				selectedLines[i].push_back(l);
			}
		}
		for(int j=0; j< polygonsSN[i].size(); j++)
		{
			int index=0;
			while(index < polygonsS[i].size())
			{
				if((polygonsSN[i][j]).isSameAs(polygonsS[i][index], tol)) break;
				index++;
			}
			if(index < polygonsS[i].size())
			{ // Duplicate of line found so keep
				nonSelectedLines[i].push_back((polygonsSN[i][j]));
			}
		}
	}
	
	// sort lines to form a chain of lines
	for(auto linesAtLevel: selectedLines)
	{
		if(linesAtLevel.size() <= 1) continue;
		bool reverse = false;
		bool done = false;
		int i=0;
		int condition = linesAtLevel.size();
		int vID = 1;
		while(!done)
		{	
			bool anyConnected = false;
			// find nex lineAtLevel which is connected
			if(i+1 == linesAtLevel.size())
			{
				anyConnected = true;
				break;
			}
			for(int j=i+1; j < linesAtLevel.size(); j++)
			{
				if(linesAtLevel[i][vID].isSameAs(linesAtLevel[j][1], tol))
				{
					bso::utilities::geometry::line_segment temp;
					temp = linesAtLevel[i+1];
					linesAtLevel[i+1] = {linesAtLevel[j][1],linesAtLevel[j][0]};
					linesAtLevel[j] = temp;
					anyConnected = true;
					break;
				} 
				else if (linesAtLevel[i][vID].isSameAs(linesAtLevel[j][0], tol))
				{
					if(j != i+1)
					{
						bso::utilities::geometry::line_segment temp;
						temp = linesAtLevel[i+1];
						linesAtLevel[i+1] = linesAtLevel[j];
						linesAtLevel[j] = temp;
					}
					anyConnected = true;
					break;
				}
			}
			if(!anyConnected)
			{
				if(vID = 1)
				{
					// shift the content of the vector to the back of the vector and reverse iteration order
					int shift = condition-1 - i;
					std::vector<bso::utilities::geometry::line_segment> temp = linesAtLevel;
					
					int s = shift;
					for(int count = 0; count < linesAtLevel.size(); count++)
					{
						linesAtLevel[s]= temp[count];
						s ++;
						if(s == linesAtLevel.size()) {s = 0;}
					}
					
					vID = 0;
					i = shift;
					reverse = true;
				}
				else if(vID = 0)
				{
					// start new line
					done = true;
				}

			}
			if(i >= linesAtLevel.size()) {done = true;}
			else if(i < 0) {done = true;}
			else if(!reverse) {i++;}
			else {i--;}
		}
	}

	count = 0;
	for(auto linesAtLevel: selectedLines)
	{
		std::cout << "ordered selected lines at elevation sorted: " << count << std::endl;
		for(const auto j: linesAtLevel)
		{
			std::cout << j << std::endl;
		}
		count++;
	}
	
	for(auto& linesAtLevel: nonSelectedLines)
	{
		if(linesAtLevel.size() <= 1) continue;
		bool reverse = false;
		bool done = false;
		int i=0;
		int condition = linesAtLevel.size();
		int vID = 1;
		while(!done)
		{	
			bool anyConnected = false;
			// find nex lineAtLevel which is connected
			if(i+1 == linesAtLevel.size())
			{
				anyConnected = true;
				break;
			}
			for(int j=i+1; j < linesAtLevel.size(); j++)
			{
				if(linesAtLevel[i][vID].isSameAs(linesAtLevel[j][1], tol))
				{
					bso::utilities::geometry::line_segment temp;
					temp = linesAtLevel[i+1];
					linesAtLevel[i+1] = {linesAtLevel[j][1],linesAtLevel[j][0]};
					linesAtLevel[j] = temp;
					anyConnected = true;
					break;
				} 
				else if (linesAtLevel[i][vID].isSameAs(linesAtLevel[j][0], tol))
				{
					if(j != i+1)
					{
						bso::utilities::geometry::line_segment temp;
						temp = linesAtLevel[i+1];
						linesAtLevel[i+1] = linesAtLevel[j];
						linesAtLevel[j] = temp;
					}
					anyConnected = true;
					break;
				}
			}
			if(!anyConnected)
			{
				if(vID == 1)
				{
					// shift the content of the vector to the back of the vector and reverse iteration order
					int shift = condition-1 - i;
					std::vector<bso::utilities::geometry::line_segment> temp = linesAtLevel;
					
					int s = shift;
					for(int count = 0; count < linesAtLevel.size(); count++)
					{
						linesAtLevel[s]= temp[count];
						s ++;
						if(s == linesAtLevel.size()) {s = 0;}
					}
					
					vID = 0;
					i = shift;
					reverse = true;
				}
				else if(vID == 0)
				{
					// should start new line in future when surfaces are not continues
					done = true;
				}

			}
			if(i >= linesAtLevel.size()) {done = true;}
			else if(i < 0) {done = true;}
			else if(!reverse) {i++;}
			else {i--;}
		}
	}

	count = 0;
	for(auto linesAtLevel: nonSelectedLines)
	{
		std::cout << "ordered non Selected lines at elevation sorted: " << count << std::endl;
		for(const auto j: linesAtLevel)
		{
			std::cout << j << std::endl;
		}
		count++;
	}


	// get vertices of lines to form a polygon-non-convex
	std::vector<std::vector<bso::utilities::geometry::vertex>> polygonsV;
	polygonsV.resize(allLevels.size()-1);	
	for(int i=0; i<polygonsV.size(); i++)
	{
		if(selectedLines[i].size() < 1 || nonSelectedLines[i].size() < 1)
		{
			continue; // need points on both sides to be a minumum of two
		} 
		
		polygonsV[i].push_back(selectedLines[i][0][0]); 
		for(int j=0; j<selectedLines[i].size(); j++)
		{
			polygonsV[i].push_back(selectedLines[i][j][1]);
		}
		
		// Check if line from each side intersect and if the lines should be inserted in reverse order
		bso::utilities::geometry::line_segment l1 = {{polygonsV[i].back()}, {nonSelectedLines[i][0][0]}};
		bso::utilities::geometry::line_segment l2 = {{polygonsV[i][0]}, {(nonSelectedLines[i].back())[1]}};
		if(l1.intersectsWith(l2, tol))
		{
			polygonsV[i].push_back((nonSelectedLines[i].back())[1]); 

			for(int j=nonSelectedLines[i].size()-1; j>=0; j--)
			{
				polygonsV[i].push_back(nonSelectedLines[i][j][0]);
			}
		}
		else 
		{
			polygonsV[i].push_back(nonSelectedLines[i][0][0]); 
			for(int j=0; j<nonSelectedLines[i].size(); j++)
			{
				polygonsV[i].push_back(nonSelectedLines[i][j][1]);
			}
		}
		
	}

	// make the polygon_non_convex and make quads out of them per elevation
	std::vector<std::vector<bso::utilities::geometry::quadrilateral>> quadsPerLevel;
	//quadsPerLevel.resize(allLevels.size()-1);
	for(int i=0; i<polygonsV.size(); i++)
	{
		if(polygonsV[i].size() == 0) continue; // needs to be defined different in the future
		bso::utilities::geometry::polygon_non_convex polyTemp(polygonsV[i]);
		std::cout << "polyTemp is: " << polyTemp << std::endl;

		std::vector<bso::utilities::geometry::quadrilateral> quadsTemp;
		polyTemp.convertToQuads(quadsTemp);
		quadsPerLevel.push_back(quadsTemp);
	}

	// Define new spaces in the cap formed by the shifting of the selected spaces
	for(int i=0; i<quadsPerLevel.size(); i++)
	{
		// elevation height
		bso::utilities::geometry::vector lowVector = {0,0,allLevels[i]};
		bso::utilities::geometry::vector hightVector = {0,0,allLevels[i+1]};
		
		for(const auto j: quadsPerLevel[i])
		{
			// Define vertices of new space
			std::vector <utilities::geometry::vertex*> vertexes;
			for(const auto v: j) 
			{
				vertexes.push_back(new utilities::geometry::vertex);
				*(vertexes.back()) = v + lowVector;

				vertexes.push_back(new utilities::geometry::vertex);
				*(vertexes.back()) = v + hightVector;
			}

			// Define new space
			int newID = mSpaces.back()->getID();
			newID++;
			mSpaces.push_back(new ms_space());
			mSpaces.back()->setID(newID);
			mSpaces.back()->setPVertex(vertexes);
		}
	}

	

	// Check if shifted spaces and non-shifted spaces really leaf a capp to fill
	




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

std::vector <bso::utilities::geometry::quadrilateral> ms_N_building::getQuadAtZ0(const double mTol)
{
	std::vector <bso::utilities::geometry::quadrilateral> quadAtZ0 = getQuadAtZ0(mSpaces,mTol);
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

std::vector <utilities::geometry::line_segment> ms_N_building::getLinesAtZ0(const double mTol)
{
	std::vector <utilities::geometry::line_segment> linesAtZ = getLinesAtZ0(mSpaces,mTol);
	return linesAtZ;
} //getLinesAtZ0

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