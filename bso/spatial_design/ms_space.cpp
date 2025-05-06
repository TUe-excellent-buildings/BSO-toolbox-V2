#ifndef MS_SPACE_CPP
#define MS_SPACE_CPP

#include <sstream>
#include <string>
#include <vector>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <exception>
#include <stdexcept>
#include <bitset>
#include <iomanip> // allows for std::setprecision use

#include <bso/utilities/trim_and_cast.hpp>

namespace bso { namespace spatial_design {

ms_space::ms_space()
{
	this->reset();
} // ms_space()

ms_space::ms_space(unsigned int id, utilities::geometry::vertex coords, utilities::geometry::vector dim, const std::string& spaceType /*= std::string()*/, const std::vector<std::string>& surfaceTypes /*= std::vector<std::string>()*/)
{
	// Store the inserted information in object
	sDefMethod = "R";
	mCoordinates = coords;
	mDimensions = dim;
	mID = id;
	mSpaceType = spaceType;
	mSurfaceTypes = surfaceTypes;
	
	try
	{
		checkValidity();
	}
	catch(std::exception& e)
	{
		std::stringstream errorMessage;
		errorMessage << std::endl
								 << "The following arguments initialized an invalid ms_space:" << std::endl
								 << "ID: " << id << std::endl
								 << "Coordinates: " << coords.transpose() << std::endl
								 << "Dimensions: " << dim.transpose() << std::endl;
		if (spaceType != "") errorMessage << "spaceType: " << id << std::endl;
		if (!surfaceTypes.empty())
		{
			errorMessage << "SurfaceTypes: ";
			for (auto i = mSurfaceTypes.begin(); i != mSurfaceTypes.end(); i++)
			{
				errorMessage << *i;
				if (std::next(i,1) != mSurfaceTypes.end()) errorMessage << ",";
 			}
			errorMessage << std::endl;
		}			
		errorMessage << "(bso/spatial_design/ms_space.cpp). Got the following error:" << std::endl
								 << e.what() << std::endl;
		throw std::invalid_argument(errorMessage.str());
	}
} // ms_space()

ms_space::ms_space(unsigned int id, std::vector <utilities::geometry::vertex*> pVert, const std::string& spaceType /*= std::string()*/, const std::vector<std::string>& surfaceTypes /*= std::vector<std::string>()*/)
{
	sDefMethod = "N";
	pVertex = pVert;
	mID = id;
	mSpaceType = spaceType;
	mSurfaceTypes = surfaceTypes;
	
	try
	{
		checkValidity();
	}
	catch(std::exception& e)
	{
		std::stringstream errorMessage;
		errorMessage << std::endl
								 << "The following arguments initialized an invalid ms_space:" << std::endl
								 << "ID: " << id << std::endl
								 << "Vertices: " ;
								 for(const auto i: pVert){errorMessage << i << "  ";}
								 errorMessage << std::endl;
		if (spaceType != "") errorMessage << "spaceType: " << id << std::endl;
		if (!surfaceTypes.empty())
		{
			errorMessage << "SurfaceTypes: ";
			for (auto i = mSurfaceTypes.begin(); i != mSurfaceTypes.end(); i++)
			{
				errorMessage << *i;
				if (std::next(i,1) != mSurfaceTypes.end()) errorMessage << ",";
 			}
			errorMessage << std::endl;
		}			
		errorMessage << "(bso/spatial_design/ms_space.cpp). Got the following error:" << std::endl
								 << e.what() << std::endl;
		throw std::invalid_argument(errorMessage.str());
	}
} // ms_space()

ms_space::ms_space(std::string line)
{	
	// tokenize the line
	boost::char_separator<char> sep(","); // defines what separates tokens in a string
	typedef boost::tokenizer< boost::char_separator<char> > t_tokenizer; // settings for the boost::tokenizer
	t_tokenizer tokens(line, sep); // this is where the tokenized line will be stored
	t_tokenizer::iterator token = tokens.begin(); // set iterator to first token
	int number_of_tokens = std::distance( tokens.begin(), tokens.end()); // count the number of tokens int the line
	--number_of_tokens; // This removes the insertion method indication token for the later on switch statements
	
	try
	{//Check for a correct insertion method, if correct define sDefMethod.
		sDefMethod = utilities::trim_and_cast_char(*(token));
		if(sDefMethod != "N" && sDefMethod != "n" && sDefMethod != "R" && sDefMethod != "r")
		{
			std::stringstream errorMessage;
			errorMessage << std::endl
									 << "and sDefMethod is: " << sDefMethod;
			throw std::invalid_argument(errorMessage.str());
		}
	}
	catch (std::exception& e)
	{
		std::stringstream errorMessage;
			errorMessage << std::endl
									 << "Could not parse the following line to initialize an ms_space:" << std::endl
									 << line << std::endl
									 << "since there is no valid insertion method defined at the start of the line. " << std::endl
									 << "This should either be a 'R', 'r', 'N', or 'n', separated by a ',' from the vectors or coordinates and dimensions. " << std::endl
									 << e.what() << std::endl
									 << "(bso/spatial_design/ms_space.cpp). " << std::endl;
			throw std::invalid_argument(errorMessage.str());
	}	
	
	
	try 
	{// store values in string line in mDimensions and mCoordinates and or pVertex with first value from line variable as mID
		mID = utilities::trim_and_cast_int(*(++token));
		
		if (sDefMethod == "N" || sDefMethod == "n")
		{// handle the non-orthogonal cases
			utilities::geometry::vertex temp;
			
			if(number_of_tokens >= 25) // avoid tokens which are non-existing
			{
				for(int i=0; i<8; i++)
				{
					pVertex.push_back(new utilities::geometry::vertex);
							
					temp(0) = utilities::trim_and_cast_double(*(++token));
					temp(1) = utilities::trim_and_cast_double(*(++token));
					temp(2) = utilities::trim_and_cast_double(*(++token));
					
					*pVertex[i] = temp;		
				}
			}
			
			switch (number_of_tokens)
			{
				case 25: // 8 corner polyhedron; 
				{
					
					break;
				}
				case 26: // 8 corner polyhedron; a space type defined
				{				
					mSpaceType = *(++token);
					boost::algorithm::trim(mSpaceType);		
					break;
				}
				case 31: // 8 corner polyhedron; only surface types have been defined (Note: break is skiped to continue to case 32)
				case 32: // 8 corner polyhedron; both a space type and surface types have been defined
				{
					if (number_of_tokens == 32)
					{
						mSpaceType = *(++token);
						boost::algorithm::trim(mSpaceType);	
					}
					
					mSurfaceTypes.clear();
					std::string temp;
					
					for (unsigned int i = 0; i < 6; i++)
					{
						temp = *(++token);
						boost::algorithm::trim(temp);
						mSurfaceTypes.push_back(temp);
					}
					
					break;
				}
				default:
				{
				std::stringstream errorMessage;
				errorMessage << std::endl
										 << "An invalid amount of input argument was encountered when trying to initialize the " << sDefMethod << " method defined space with ID " << mID << "." << std::endl
										 << "When trying to parse the following input line: " << std::endl
										 << line << std::endl
										 << "The determined number_of_tokens are: " << number_of_tokens << std::endl
										 << "(bso/spatial_design/ms_space.cpp)" << std::endl;
				throw std::invalid_argument(errorMessage.str());
				
				break;
				}			
			}
		} 
		else if (sDefMethod == "R" || sDefMethod == "r") //number_of_tokens = 7
		{
			if(number_of_tokens >= 7)
			{
				// Read the Dimensioning vector en Coordinate vertex from the R/r insertion method
				utilities::geometry::vertex temp;
				temp(0) = utilities::trim_and_cast_double(*(++token));
				temp(1) = utilities::trim_and_cast_double(*(++token));
				temp(2) = utilities::trim_and_cast_double(*(++token));
				mDimensions = temp;
				temp(0) = utilities::trim_and_cast_double(*(++token));
				temp(1) = utilities::trim_and_cast_double(*(++token));
				temp(2) = utilities::trim_and_cast_double(*(++token));
				mCoordinates = temp;
			}
			
			switch (number_of_tokens)
			{ // handle the tokens for the orthogonal cases
				case 7: // orthogonal rectangle; no space nor surface types have been defined
				{
					break;
				} 
				case 8: // orthogonal rectangle; only a space type has been defined
				{
					mSpaceType = *(++token);
					boost::algorithm::trim(mSpaceType);			
					break; 
				}
				case 13: // orthogonal rectangle; only surface types have been defined (Note: break is skiped to continue to case 14)
				case 14: // orthogonal rectangle; both a space type and surface types have been defined
				{
					if (number_of_tokens == 14)
					{
						mSpaceType = *(++token);
						boost::algorithm::trim(mSpaceType);
					}
					
					mSurfaceTypes.clear();
					std::string temp;
					
					for (unsigned int i = 0; i < 6; i++)
					{
						temp = *(++token);
						boost::algorithm::trim(temp);
						mSurfaceTypes.push_back(temp);
					}

					break;
				}
				default:
				{
					std::stringstream errorMessage;
					errorMessage << std::endl
											 << "An invalid amount of input argument was encountered when trying to initialize the " << sDefMethod << " method defined space with ID " << mID << "." << std::endl
											 << "When trying to parse the following input line: " << std::endl
											 << line << std::endl
											 << "The determined number_of_tokens are: " << number_of_tokens << std::endl										   
											 << "(bso/spatial_design/ms_space.cpp)" << std::endl;
					throw std::invalid_argument(errorMessage.str());
					
					break;
				}
			} // switch statement
		} // if statement for sDefMethod == "R" or "r"
	}
	catch (std::exception& e)
	{
		std::stringstream errorMessage;
		errorMessage << std::endl
								 << "Could not parse the following line to initialize an ms_space:" << std::endl
								 << line << std::endl
								 << "(bso/spatial_design/ms_space.cpp). Got the following error:" << std::endl
								 << e.what() << std::endl;
		throw std::invalid_argument(errorMessage.str());
	}
	
	try
	{
		checkValidity();
	}
	catch(std::exception& e)
	{
		std::stringstream errorMessage;
		errorMessage << std::endl
								 << "Parsing the following line to initialize leads to an invalid ms_space:" << std::endl
								 << line << std::endl
								 << "(bso/spatial_design/ms_space.cpp). Got the following error:" << std::endl
								 << e.what() << std::endl;
		throw std::invalid_argument(errorMessage.str());
	}
} // ms_space()

ms_space::ms_space(const ms_space& rhs)
{
	mCoordinates  = rhs.mCoordinates;
	mDimensions   = rhs.mDimensions;
	mID           = rhs.mID;
	mSpaceType    = rhs.mSpaceType;
	mSurfaceTypes = rhs.mSurfaceTypes;
	pVertex		  = rhs.pVertex; 
	sDefMethod	  = rhs.sDefMethod;
	
	try
	{
		checkValidity();
	}
	catch(std::exception& e)
	{
		std::stringstream errorMessage;
		errorMessage << std::endl
									<< "Encountered invalid space when copying the space with the following ID:" << rhs.mID << std::endl
									<< "(bso/spatial_design/ms_space.cpp). Got the following error:" << std::endl
									<< e.what() << std::endl;
		throw std::invalid_argument(errorMessage.str());
	}
} // ms_space()

ms_space::~ms_space()
{
	
} // ~ms_space()

void ms_space::reset()
{
	mCoordinates = {0,0,0};
	mDimensions = {0,0,0};
	pVertex.clear();
	mID = 0;
	sDefMethod = "";
	mSpaceType = "";
	mSurfaceTypes.clear();
} // reset()

bool ms_space::checkValidity() const
{
	bool check = false;
	if(sDefMethod == "R" | sDefMethod == "r")
	{
		bool check = mDimensions.minCoeff() < 0;
		if (check)
		{
			std::stringstream errorMessage;
			errorMessage << std::endl
									 << "Found a space with a negative dimension: " << mDimensions.transpose() << std::endl
									 << "Found this in space with ID: " << mID << std::endl
									 << "(bso/spatial_design/ms_space.hpp)" << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
	}
	else if (sDefMethod == "N" | sDefMethod == "n")
	{
		try
		{
			bso::utilities::geometry::quad_hexahedron(pVertex);
		}
		catch(std::exception& e)
		{
			std::stringstream errorMessage;
			errorMessage << std::endl
									 << "Found a space which does not define a valid quad-hexahedron: " << std::endl
									 << "Found this in space with ID: " << mID << std::endl
									 << "Got the following error: " << std::endl
									 << e.what() << std::endl
									 << "(bso/spatial_design/ms_space.hpp)" << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
	}
	else
	{
		std::stringstream errorMessage;
			errorMessage << std::endl
									 << "Could not perform a checkValidity check since no valid insertion method is defined for this space" << std::endl
									 << "Found this in space with ID: " << mID << std::endl
									 << "(bso/spatial_design/ms_space.hpp)" << std::endl;
			throw std::invalid_argument(errorMessage.str());
	}
	return check;
} // checkValidity()

void ms_space::setPVertex(const std::vector <utilities::geometry::vertex*> vertexes)
{
	try
	{
		pVertex = vertexes;
		checkValidity();
	}
	catch(std::exception& e)
	{
		std::stringstream errorMessage;
		errorMessage << "Could not set the vertices to space with ID \"" << mID << "\": the following vertices: " << std::endl;
								 for(const auto i: vertexes) errorMessage << i << "; ";
								 errorMessage << "" << std::endl
								 << "(bso/spatial_design/ms_space.hpp). Got the following error: " << std::endl
								 << e.what() << std::endl;
		throw std::invalid_argument(errorMessage.str());
	}
} // setPVertex()

void ms_space::setCoordinates(const utilities::geometry::vertex& coords)
{
	if (sDefMethod == "R" || sDefMethod == "r")
	{
		try
		{
			mCoordinates = coords;
			checkValidity();
		}
		catch(std::exception& e)
		{
			std::stringstream errorMessage;
			errorMessage << "Could not set the following coordinates to space with ID \"" << mID << "\": " << coords.transpose() << std::endl
									<< "(bso/spatial_design/ms_space.hpp). Got the following error: " << std::endl
									<< e.what() << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
	}
	else
	{
		std::stringstream errorMessage;
			errorMessage << "This function can not be used in combination with any other insertion method then R/r." << std::endl
									<< "(bso/spatial_design/ms_space.hpp). " << std::endl;
			throw std::invalid_argument(errorMessage.str());
	}
} // setCoordinates()

void ms_space::setDimensions(const utilities::geometry::vector& dims)
{
	if (sDefMethod == "R" || sDefMethod == "r")
	{
		try
		{
			mDimensions = dims;
			checkValidity();
		}
		catch(std::exception& e)
		{
			std::stringstream errorMessage;
			errorMessage << "Could not set the following dimensions to space with ID \"" << mID << "\": " << dims.transpose() << std::endl
									<< "(bso/spatial_design/ms_space.hpp). Got the following error: " << std::endl
									<< e.what() << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
	}	
	else
	{
		std::stringstream errorMessage;
			errorMessage << "This function can not be used in combination with anny other insertion method then R/r." << std::endl
									<< "(bso/spatial_design/ms_space.hpp). " << std::endl;
			throw std::invalid_argument(errorMessage.str());
	}
} // setDimensions()

void ms_space::setID(const unsigned int& id)
{
	mID = id;
} // setID()

unsigned int ms_space::getID() const
{
	return mID;
} // getID()

utilities::geometry::vertex ms_space::getCoordinates() const
{
	return mCoordinates;
} // getCoordinates

utilities::geometry::vector ms_space::getDimensions() const
{
	return mDimensions;
} // getDimensions

std::vector<utilities::geometry::vertex*> ms_space::getVertices() const
{
	std::vector<utilities::geometry::vertex*> vertices;

	if (sDefMethod == "R" || sDefMethod == "r")
	{
		std::vector<utilities::geometry::vertex> cornerPoints;
		cornerPoints.reserve(8);
		vertices.reserve(8);
		std::vector<unsigned int> vertexOrder = {0,1,3,2,4,5,7,6};
		for (const auto& i : vertexOrder)
		{
			auto tempPoint = mCoordinates;
			for (unsigned int j = 0; j < 3; j++)
			{
				if (std::bitset<3>(i)[j]) tempPoint[j] += mDimensions[j];
			}
			cornerPoints.push_back(tempPoint);
			vertices.push_back(&cornerPoints.back());
		}
	}
	else if (sDefMethod == "N" || sDefMethod == "n")
	{
		vertices = pVertex;
	}
	return vertices;
} //getVertices()

utilities::geometry::quad_hexahedron ms_space::getGeometry() const
{ // return the geometry of the space
	std::vector<utilities::geometry::vertex> cornerPoints;
	
	if (sDefMethod == "R" || sDefMethod == "r")
	{
		cornerPoints.reserve(8);
		std::vector<unsigned int> vertexOrder = {0,1,3,2,4,5,7,6};
		for (const auto& i : vertexOrder)
		{
			auto tempPoint = mCoordinates;
			for (unsigned int j = 0; j < 3; j++)
			{
				if (std::bitset<3>(i)[j]) tempPoint[j] += mDimensions[j];
			}
			cornerPoints.push_back(tempPoint);
		}
	}
	else if (sDefMethod == "N" || sDefMethod == "n")
	{
		for (int i=0; i<pVertex.size(); i++) cornerPoints.push_back(*pVertex[i]);
	}

	return bso::utilities::geometry::quad_hexahedron(cornerPoints);
} // getGeometry


bool ms_space::getSpaceType(std::string& spaceType) const
{
	spaceType = mSpaceType;
	return (mSpaceType != "");
} // getSpaceType()

bool ms_space::getSurfaceTypes(std::vector<std::string>& surfaceTypes) const
{
	surfaceTypes = mSurfaceTypes;
	return (!mSurfaceTypes.empty());
} // getSurfaceTypes

double ms_space::getVolume() const
{
	if(sDefMethod == "N" || sDefMethod == "n"){return this->getGeometry().getVolume()*1e-9;}
	if(sDefMethod == "R" || sDefMethod == "r") {return mDimensions.prod()*1e-9;}
	else {return this->getGeometry().getVolume()*1e-9;}
} // getVolume()

double ms_space::getFloorArea() const
{
	double area = 0;
	if(sDefMethod == "R" || sDefMethod == "r")
	{
		area = mDimensions(0)*mDimensions(1);
	}
	else
	{
		std::vector<bso::utilities::geometry::vertex> pVertex = this->getGeometry().getVertices();
		if(pVertex.size() != 8)
		{
			std::stringstream errorMessage;
			errorMessage << std::endl
									 << "Did not recieve eight vertices. Therefore, the area calculation of the space can not be performed." << std::endl
									 << "(bso/spatial_design/ms_space.hpp)" << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}

		std::vector<bso::utilities::geometry::vertex> VertexH1;
		std::vector<bso::utilities::geometry::vertex> VertexH2;
		VertexH1.push_back(pVertex[0]);
		for(int i=1; i < 8; i++)
		{
			if(VertexH1[0].z() == pVertex[i].z())
			{
				VertexH1.push_back(pVertex[i]);
			}
			else VertexH2.push_back(pVertex[i]);
		}
		if(VertexH1.size() != 4 || VertexH2.size() != 4)
		{
			std::stringstream errorMessage;
			errorMessage << std::endl
									 << "Could not find four vertices on the same z level to calculate the area of the horizontal polygon" << std::endl
									 << "the following was found: the first plane had " << VertexH1.size() << " amound of vertices and the" << std::endl
									 << "second plane had " << VertexH2.size() << " amound of vertices" << std::endl
									 << "(bso/spatial_design/ms_space.hpp)" << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}

		// Define the area of the space on the lowest horizontal polygon (ref for area calculation: https://www.themathdoctors.org/polygon-coordinates-and-areas/)
		if(VertexH1[0].z() < VertexH2[0].z())
		{
			area = 0.5*(abs((VertexH1[0].x()*VertexH1[1].y()-VertexH1[1].x()*VertexH1[0].y())+(VertexH1[1].x()*VertexH1[2].y()-VertexH1[2].x()*VertexH1[1].y())+(VertexH1[2].x()*VertexH1[3].y()-VertexH1[3].x()*VertexH1[2].y())+(VertexH1[3].x()*VertexH1[0].y()-VertexH1[0].x()*VertexH1[3].y())));
		}
		else
		{
			area = 0.5*(abs((VertexH2[0].x()*VertexH2[1].y()-VertexH2[1].x()*VertexH2[0].y())+(VertexH2[1].x()*VertexH2[2].y()-VertexH2[2].x()*VertexH2[1].y())+(VertexH2[2].x()*VertexH2[3].y()-VertexH2[3].x()*VertexH2[2].y())+(VertexH2[3].x()*VertexH2[0].y()-VertexH2[0].x()*VertexH2[3].y())));
		}
	}
	return area;
} // getFloorArea()

double ms_space::getSurfaceArea() const
{
	double area = 0;
	if(sDefMethod == "R" || sDefMethod == "r")
	{
		area = mDimensions(0)*mDimensions(1)*2;
		area += mDimensions(1)*mDimensions(2)*2;
		area += mDimensions(0)*mDimensions(2)*2;
	}
	else
	{
		std::vector<bso::utilities::geometry::quadrilateral> Quads = this->getGeometry().getQuadrilateral();
		for(const auto& i: Quads)
		{
			area += i.getArea();
		}
	}
	return area;
} // getSurfaceArea()

bool ms_space::operator == (const ms_space& rhs) const
{
	if (mID != rhs.mID) return false;
	if (mCoordinates != rhs.mCoordinates) return false;
	if (mDimensions != rhs.mDimensions) return false;
	if (pVertex.size() != rhs.pVertex.size()) return false;
	for (unsigned int i = 0; i < pVertex.size(); i++)
	{	
		if (!(*(pVertex[i])).isSameAs(*(rhs.pVertex[i]))) 
		{
			return false;
		}
	}
	if (sDefMethod != rhs.sDefMethod) return false;
	if (mSpaceType != rhs.mSpaceType) return false;
	if (mSurfaceTypes.size() != rhs.mSurfaceTypes.size()) return false;
	for (unsigned int i = 0; i < mSurfaceTypes.size(); i++)
	{
		if (mSurfaceTypes[i] != rhs.mSurfaceTypes[i]) return false;
	}
	return true;
}

bool ms_space::operator != (const ms_space& rhs) const
{
	return !(*this == rhs);
}

std::ostream& operator <<(std::ostream& stream, const ms_space& space)
{
	if(space.getSDefMethod() == "R")
	{
		stream << "R"
					 << "," << space.getID()
					 << "," << space.getDimensions()(0)
					 << "," << space.getDimensions()(1)
					 << "," << space.getDimensions()(2)
					 << "," << space.getCoordinates()(0)
					 << "," << space.getCoordinates()(1)
					 << "," << space.getCoordinates()(2);
		std::string tempStr;
		if (space.getSpaceType(tempStr)) stream << "," << tempStr;
		std::vector<std::string> tempVec;
		if (space.getSurfaceTypes(tempVec)) for (auto i : tempVec) stream << "," << i;
	}
	else if(space.getSDefMethod()== "N")
	{
		stream << "N" << "," << space.getID() << std::setprecision(std::numeric_limits<double>::max_digits10);
		
		for(const auto& i: space.getVertices())
		{
			// The std::setprecision statement makes sure that the output file does not lose precision in value,
			// This is important when a scaled design needs to be inputted again.
			stream << ",	";
			stream << (*i) (0); 
			stream << "," << (*i) (1);
			stream << "," << (*i) (2);
		}
		std::string tempStr;
		if (space.getSpaceType(tempStr)) stream << "," << tempStr;
		std::vector<std::string> tempVec;
		if (space.getSurfaceTypes(tempVec)) for (auto i : tempVec) stream << "," << i;
	}
	else
	{
		std::stringstream errorMessage;
			errorMessage << std::endl
									 << "Could not perform << operator since no valid insertion method is defined" << std::endl
									 << "(bso/spatial_design/ms_space.hpp)" << std::endl;
			throw std::invalid_argument(errorMessage.str());
	}
	return stream;
}

} // namespace spatial_design
} // namespace bso

#endif // MS_SPACE_CPP