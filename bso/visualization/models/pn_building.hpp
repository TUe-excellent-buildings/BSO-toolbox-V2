#ifndef VIS_PN_BUILDING_HPP
#define VIS_PN_BUILDING_HPP

#include <bso/visualization/models/model_base.hpp>
#include <bso/visualization/bsp/bsp.hpp>

#include <boost/algorithm/string.hpp>
#include <utility>

#ifdef SD_MODEL_HPP
#include <bso/structural_design/sd_model.hpp>
#endif // SD_MODEL_HPP

#ifdef BP_MODEL_HPP
#include <bso/building_physics/bp_model.hpp>
#endif // BP_MODEL_HPP

#include <bitset>

namespace bso { namespace visualization
{

class PN_BUILDING_Model : public model_base
{
private:
	std::string mTitle;
	std::list<polygon*> polygons;
	std::list<label*>   labels;

	polygon_props  pprops;
	polygon_props  ppropsSurfaceType; 
	polygon_props  ppropsDefault; 
	std::vector<std::pair<int, polygon_props>> spaceIDProps;

	line_props     lprops;
	line_props	   lprops_boundary;
	label_props    lbprops;
	random_bsp     *pbsp;
public:
   	/*
	The following constructor contain the ability for defining different colors per space defined in a file 
	(first input contains the file name for this input)
	A description of how this file should look like is shown bellow:
	
	# space with ID zero is considered the standard color and will overrule the other standards defined.
	# this standard color will be used for all spaces without a defined color.
	# To define a color of a space insert the ID number of the space followed by four times the four rgba value's.
	# See link: https://www.w3schools.com/css/css_colors_rgb.asp for explanation rgba value's.
	# In the BSO the rgba value's are defined as a ratio. Where 1.0 equals the maximum value.

	#	space_ID	ambient(rgba)		diffuse(rgba)		specular(rgba)	emission(rgba)	shininess(float value)	translucent(bool)		twoSided(bool)
	0,	0.1,0.5,0.1,0.3,	0.2,1.0,0.2,0.3,	0.2,1.0,0.2,0.3,	0.0,0.0,0.0,0.0,	60.0,	true,	true
	1,	0.1,0.5,0.1,0.3,	0.2,1.0,0.2,0.3,	0.2,1.0,0.2,0.3,	0.0,0.0,0.0,0.0,	60.0,	true,	true
	2,	1.0,0.1,0.1,1.0,	1.0,0.2,0.2,1.0,	1.0,0.2,0.2,1.0,	0.0,0.0,0.0,0.0,	60.0,	true,	true
	*/
	PN_BUILDING_Model(const std::string& colorFileName, const std::string& PNFileName, 
						const std::string& title = "pn_building", const double& linewidth = 1.0);

	~PN_BUILDING_Model();
	
	void render(const camera &cam) const;
	const std::string get_description();
}; // PN_BUILDING_Model

PN_BUILDING_Model::PN_BUILDING_Model(const std::string& colorFileName, const std::string& PNFileName, 
	const std::string& title /*= "pn_building"*/, const double& linewidth /*= 1.0*/)
{
	// Read the color input file
	std::ifstream colorInput;
	if (!colorFileName.empty()) colorInput.open(colorFileName.c_str());
	if (colorFileName.empty() || !colorInput.is_open())
	{
		std::stringstream errorMessage;
		errorMessage << "Could not initialize a visualization of MS_N building spatial design with the following colour input file:" << std::endl
					<< ((colorFileName.empty())? "no input file given" : colorFileName) << std::endl
					<< "(bso/visualization/ms_building.hpp). " << std::endl;
		throw std::invalid_argument(errorMessage.str());
	}
	std::string line;
	while (!colorInput.eof()) // Parse the colorInput file line by line
	{
		getline(colorInput,line); // get next line from the file
		try
		{
			boost::algorithm::trim(line); // remove white space from start and end of line (to see if it is an empty line, removes any incidental white space)
			if (line == "") //skip empty lines (tokenizer does not like it)
			{
				continue; // continue to next line
			}
			else if(line.substr(0,1) == "#")
			{
				continue;
			}
			else
			{
				boost::char_separator<char> sep(","); // defines what separates tokens in a string
				typedef boost::tokenizer< boost::char_separator<char> > t_tokenizer; // settings for the boost::tokenizer
				t_tokenizer tokens(line, sep); // this is where the tokenized line will be stored
				t_tokenizer::iterator token = tokens.begin(); // set iterator to first token
				int number_of_tokens = std::distance( tokens.begin(), tokens.end()); // count the number of tokens int the line

				try
				{
					int ID = utilities::trim_and_cast_int(*(token));
					polygon_props temp;

					if(number_of_tokens != 20)
					{
						std::stringstream errorMessage;
						errorMessage << std::endl
												<< "An invalid amount of input argument was encountered when trying to read the visualization settings" << std::endl
												<< "When trying to parse the following input line: " << std::endl
												<< line << std::endl
												<< "The number_of_tokens are: " << number_of_tokens << std::endl
												<< "(bso/visualizations/ms_building.hpp)" << std::endl;
						throw std::invalid_argument(errorMessage.str());
					}

					float f1, f2, f3, f4;

					f1 = utilities::trim_and_cast_float(*(++token));
					f2 = utilities::trim_and_cast_float(*(++token));
					f3 = utilities::trim_and_cast_float(*(++token));
					f4 = utilities::trim_and_cast_float(*(++token));
					temp.ambient = rgba(f1, f2, f3, f4);

					f1 = utilities::trim_and_cast_float(*(++token));
					f2 = utilities::trim_and_cast_float(*(++token));
					f3 = utilities::trim_and_cast_float(*(++token));
					f4 = utilities::trim_and_cast_float(*(++token));
					temp.diffuse = rgba(f1, f2, f3, f4);							

					f1 = utilities::trim_and_cast_float(*(++token));
					f2 = utilities::trim_and_cast_float(*(++token));
					f3 = utilities::trim_and_cast_float(*(++token));
					f4 = utilities::trim_and_cast_float(*(++token));
					temp.specular = rgba(f1, f2, f3, f4);							

					f1 = utilities::trim_and_cast_float(*(++token));
					f2 = utilities::trim_and_cast_float(*(++token));
					f3 = utilities::trim_and_cast_float(*(++token));
					f4 = utilities::trim_and_cast_float(*(++token));
					temp.emission = rgba(f1, f2, f3, f4);							

					temp.shininess = utilities::trim_and_cast_float(*(++token));

					temp.translucent = utilities::trim_and_cast_bool(*(++token));

					temp.twosided = utilities::trim_and_cast_bool(*(++token));

					// Add new to spaceIDProps
					std::pair<int, polygon_props> pairTemp (ID, temp);
					spaceIDProps.push_back(pairTemp);
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
			}
		}
		catch(std::exception& e)
		{
			std::stringstream errorMessage;
			errorMessage << "Encountered an error while parsing the following line of a visualization ms_N_building input file:" << std::endl
							<< line << std::endl
							<< "(bso/visualization/ms_building.hpp). Got the following error: " << std::endl
							<< e.what() << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
	}

	// Read the PN building file
	std::ifstream PNInput;
	if (!PNFileName.empty()) PNInput.open(PNFileName.c_str());
	if (PNFileName.empty() || !PNInput.is_open())
	{
		std::stringstream errorMessage;
		errorMessage << "Could not initialize a visualization of MS_N building spatial design with the following colour input file:" << std::endl
					<< ((PNFileName.empty())? "no input file given" : PNFileName) << std::endl
					<< "(bso/visualization/ms_building.hpp). " << std::endl;
		throw std::invalid_argument(errorMessage.str());
	}
	std::vector<std::pair<int, std::vector<bso::utilities::geometry::triangular_prism>>> prismPerSpaceID;
	while (!PNInput.eof()) // Parse the colorInput file line by line
	{
		getline(PNInput,line); // get next line from the file
		try
		{
			boost::algorithm::trim(line); // remove white space from start and end of line (to see if it is an empty line, removes any incidental white space)
			if (line == "") //skip empty lines (tokenizer does not like it)
			{
				continue; // continue to next line
			}
			else if(line.substr(0,1) == "#") // skip comments
			{
				continue;
			}
			else if(line.substr(0,2) == "0,") // Do not take into acount the bounding box and its following triangular prims
			{
				break;
			}
			else if(line.substr(0,2) == "{{") // Define the triangular prims within a space
			{
				// tokenize the line settings
				boost::char_separator<char> sep(",{}"); // defines what separates tokens in a string
				typedef boost::tokenizer< boost::char_separator<char> > t_tokenizer; // settings for the boost::tokenizer
				t_tokenizer tokens(line, sep); // this is where the tokenized line will be stored
				t_tokenizer::iterator token = tokens.begin(); // set iterator to first token
				int number_of_tokens = std::distance( tokens.begin(), tokens.end()); // count the number of tokens int the line

				// Tokenize these kinds of lines: {{0,0,0},{4000,0,0},{0,5000,0},{0,0,3000},{4000,0,3000},{0,5000,3000}}
				// Tokenize these kinds of lines: {{0	,	0	,	0	},{4000,0,0},{0,5000,0},{0,0,3000},{4000,0,3000},{0,5000,3000}}
				utilities::geometry::vertex temp;
				std::vector<utilities::geometry::vertex> triPrismV;
				if(number_of_tokens >= 18) // avoid tokens which are non-existing
				{
					for(int i=0; i<6; i++)
					{
						triPrismV.push_back(utilities::geometry::vertex());	
						(triPrismV.back())(0) = utilities::trim_and_cast_double(*(token));
						token++;
						(triPrismV.back())(1) = utilities::trim_and_cast_double(*(token));
						token++;
						(triPrismV.back())(2) = utilities::trim_and_cast_double(*(token));	
						token++;
					}
				}

				prismPerSpaceID.back().second.push_back(triPrismV);
			}
			else // start a new space
			{
				int spaceID;

				// tokenize the line settings
				boost::char_separator<char> sep(",{}"); // defines what separates tokens in a string
				typedef boost::tokenizer< boost::char_separator<char> > t_tokenizer; // settings for the boost::tokenizer
				t_tokenizer tokens(line, sep); // this is where the tokenized line will be stored
				t_tokenizer::iterator token = tokens.begin(); // set iterator to first token

				try
				{
					spaceID = utilities::trim_and_cast_int(*token);
				}
				catch(const std::exception& e)
				{
					std::stringstream errorMessage;
						errorMessage << std::endl
												<< "Was not able to read the first character as an integer value. " << std::endl
												<< "(bso/visualizations/pn_building.hpp)" << std::endl;
						throw std::invalid_argument(errorMessage.str());
				}
				prismPerSpaceID.push_back(std::make_pair(spaceID, std::vector<bso::utilities::geometry::triangular_prism>()));
			}
		}
		catch(std::exception& e)
		{
			std::stringstream errorMessage;
			errorMessage << "Encountered an error while parsing the following line of a visualization ms_N_building input file:" << std::endl
							<< line << std::endl
							<< "(bso/visualization/pn_building.cpp). Got the following error: " << std::endl
							<< e.what() << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
	}

	lprops.width = linewidth;
	mTitle = title;
	pprops.ambient = rgba(0.1f, 0.5f, 0.1f, 0.3f);
	pprops.diffuse = rgba(0.2f, 1.0f, 0.2f, 0.3f);
	pprops.specular = rgba(0.2f, 1.0f, 0.2f, 0.3f);
	pprops.emission = rgba(0.0f, 0.0f, 0.0f, 0.0f);
	pprops.shininess = 60.0f;
	pprops.translucent = true;
	pprops.twosided = true;

	for (const auto& i : prismPerSpaceID)
	{
		namespace geom = bso::utilities::geometry;
		//geom::quad_hexahedron spaceGeometry = i->getGeometry();
		std::stringstream centerLabel;

		// define default:
		bool defaultDefined = false;
		for(const auto& props: spaceIDProps)
		{
			if(props.first == 0)
			{
				ppropsDefault = props.second;
				defaultDefined = true;
				break;
			}
		}

		int spaceID = i.first;
		bool found = false;
		int index = 0;
		while(index < spaceIDProps.size())
		{
			if(spaceIDProps[index].first == spaceID)
			{
				found = true;
				break;
			}
			index++;
		}

		for(const auto& triPrismOfSpace: i.second)
		{
			if(found)
			{
				this->addPolyhedron(polygons, &triPrismOfSpace, &(spaceIDProps[index].second), &lprops);
			}
			else if(defaultDefined)
			{
				this->addPolyhedron(polygons, &triPrismOfSpace, &ppropsDefault, &lprops);
			}
			else
			{
				this->addPolyhedron(polygons, &triPrismOfSpace, &pprops, &lprops);
			}
		}
	}

	pbsp = new random_bsp(polygons);
} //

PN_BUILDING_Model::~PN_BUILDING_Model()
{
	delete pbsp;

	for (std::list<polygon*>::iterator pit = polygons.begin();
			 pit != polygons.end(); pit++) delete *pit;

	for (std::list<label*>::iterator lbit = labels.begin();
			 lbit != labels.end(); lbit++) delete *lbit;
}

const std::string PN_BUILDING_Model::get_description()
{
	return mTitle;
}

void PN_BUILDING_Model::render(const camera &cam) const
{
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	pbsp->render_btf(cam);

	for (auto lbit = labels.begin(); lbit != labels.end(); lbit++) (*lbit)->render();

	glPopAttrib();
}

} // namespace visualization
} // namespace bso

#endif // VIS_PN_BUILDING_HPP
