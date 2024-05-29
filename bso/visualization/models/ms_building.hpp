#ifndef VIS_MS_BUILDING_HPP
#define VIS_MS_BUILDING_HPP

#include <bso/visualization/models/model_base.hpp>
#include <bso/visualization/bsp/bsp.hpp>
#include <bso/spatial_design/ms_building.hpp>
#include <bso/spatial_design/ms_N_building.hpp>

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

class MS_Model : public model_base
{
private:
	std::string mTitle;
	std::list<polygon*> polygons;
	std::list<label*>   labels;

	polygon_props  pprops;
	polygon_props  ppropsSurfaceType; 
	std::vector<std::pair<int, polygon_props>> spaceIDProps;

	line_props     lprops;
	label_props    lbprops;
	random_bsp     *pbsp;
public:
	MS_Model(const spatial_design::ms_building& ms, const std::string& type = "spaces",
					 const std::string& title = "ms_building", const double& linewidth = 1.0);
	MS_Model(const spatial_design::ms_N_building& msN, const std::string& type = "spaces",
					 const std::string& title = "ms_N_building", const double& linewidth = 1.0);
	MS_Model(const std::string& fileName, const spatial_design::ms_N_building& msN, const std::string& type = "spaces",
					 const std::string& title = "ms_N_building", const double& linewidth = 1.0);
	~MS_Model();
	
	void render(const camera &cam) const;
	const std::string get_description();
}; // MS_Model

MS_Model::MS_Model(const spatial_design::ms_building& ms,
						const std::string& type /*= "spaces"*/, const std::string& title /*= "ms_building"*/,
						const double& linewidth /*= 1.0*/)
{
	lprops.width = linewidth;
	mTitle = title;
	pprops.ambient = rgba(0.1f, 0.5f, 0.1f, 0.3f);
	pprops.diffuse = rgba(0.2f, 1.0f, 0.2f, 0.3f);
	pprops.specular = rgba(0.2f, 1.0f, 0.2f, 0.3f);
	pprops.emission = rgba(0.0f, 0.0f, 0.0f, 0.0f);
	pprops.shininess = 60.0f;
	pprops.translucent = true;
	pprops.twosided = true;

	ppropsSurfaceType.ambient = rgba(1.0f, 1.0f, 0.04f, 0.4f);
	ppropsSurfaceType.diffuse = rgba(1.0f, 1.0f, 0.04f, 0.4f);
	ppropsSurfaceType.specular = rgba(0.04f, 0.04f, 0.04f, 1.0f);
	ppropsSurfaceType.emission = rgba(0.04f, 0.04f, 0.04f, 1.0f);
	ppropsSurfaceType.shininess = 60.0;
	ppropsSurfaceType.translucent = true;
	ppropsSurfaceType.twosided = true;
	
	for (const auto& i : ms)
	{
		namespace geom = bso::utilities::geometry;
	
		geom::quad_hexahedron spaceGeometry = i->getGeometry();
		std::stringstream centerLabel;
		centerLabel << i->getID();
		
		if (type == "spaces" || type == "" )
		{
			this->addPolyhedron(polygons, &spaceGeometry, &pprops, &lprops);
			if (type == "") centerLabel.str(std::string());
		}
		else if (type == "surface_type")
		{
			std::vector<std::string> surfaceTypes;
			if (!(i->getSurfaceTypes(surfaceTypes)))
			{
				std::stringstream errorMessage;
				errorMessage << "\nError in visualizing ms_buildings.\n"
										 << "Could not get surface types from space."
										 << "(bso/visualization/visualization/models/ms_building.hpp)" << std::endl;
				throw std::runtime_error(errorMessage.str());
			}
			std::swap(surfaceTypes[0],surfaceTypes[2]);
			std::swap(surfaceTypes[4],surfaceTypes[5]);
			for (unsigned int j = 0; j < 6; ++j)
			{
				centerLabel.str(std::string());
				centerLabel << surfaceTypes[j];
				auto tempSurface = spaceGeometry.getPolygons()[j];
				auto surfaceCenter = tempSurface->getCenter();
				std::vector<geom::vertex> surfaceVertices;
				for (unsigned int k = 0; k < 4; ++k)
				{
					surfaceVertices.push_back((*tempSurface)[k] + 0.2*(surfaceCenter - (*tempSurface)[k]));
				}
				geom::quadrilateral surfaceGeometry(surfaceVertices);
				this->addPolygon(polygons,&surfaceGeometry,&ppropsSurfaceType,&lprops,0.0);
				this->addLabel(labels,&lbprops,centerLabel.str(),surfaceGeometry.getCenter());
			}
			centerLabel.str(std::string());
		}
		else if (type == "space_type")
		{
			this->addPolyhedron(polygons, &spaceGeometry, &ppropsSurfaceType, &lprops);
			std::string spaceType;
			if (!(i->getSpaceType(spaceType)))
			{
				std::stringstream errorMessage;
				errorMessage << "\nError in visualizing ms_buildings.\n"
										 << "Could not get space type from space."
										 << "(bso/visualization/visualization/models/ms_building.hpp)" << std::endl;
				throw std::runtime_error(errorMessage.str());
			}
			centerLabel << " - " << spaceType;
		}
		else
		{
			std::stringstream errorMessage;
			errorMessage << "\nError in visualizing ms_buildings.\n"
									 << "Did not recognize type of visualization: " << type << "\n"
									 << "Options are: spaces; surface_type; space_type\n" 
									 << "(bso/visualization/visualization/models/ms_building.hpp)" << std::endl;
			throw std::runtime_error(errorMessage.str());
		}
		
		//Add the space indixes to the visualization models
		this->addLabel(labels,&lbprops,centerLabel.str(),spaceGeometry.getCenter());
	}

	pbsp = new random_bsp(polygons);
} //

MS_Model::MS_Model(const spatial_design::ms_N_building& msN,
						const std::string& type /*= "spaces"*/, const std::string& title /*= "ms_building"*/,
						const double& linewidth /*= 1.0*/)
{
	lprops.width = linewidth;
	mTitle = title;
	pprops.ambient = rgba(0.1f, 0.5f, 0.1f, 0.3f);
	pprops.diffuse = rgba(0.2f, 1.0f, 0.2f, 0.3f);
	pprops.specular = rgba(0.2f, 1.0f, 0.2f, 0.3f);
	pprops.emission = rgba(0.0f, 0.0f, 0.0f, 0.0f);
	pprops.shininess = 60.0f;
	pprops.translucent = true;
	pprops.twosided = true;

	ppropsSurfaceType.ambient = rgba(1.0f, 1.0f, 0.04f, 0.4f);
	ppropsSurfaceType.diffuse = rgba(1.0f, 1.0f, 0.04f, 0.4f);
	ppropsSurfaceType.specular = rgba(0.04f, 0.04f, 0.04f, 1.0f);
	ppropsSurfaceType.emission = rgba(0.04f, 0.04f, 0.04f, 1.0f);
	ppropsSurfaceType.shininess = 60.0;
	ppropsSurfaceType.translucent = true;
	ppropsSurfaceType.twosided = true;
	
	for (const auto& i : msN)
	{
		namespace geom = bso::utilities::geometry;
		geom::quad_hexahedron spaceGeometry = i->getGeometry();
		std::stringstream centerLabel;
		centerLabel << i->getID();
		
		if (type == "spaces" || type == "" )
		{
			this->addPolyhedron(polygons, &spaceGeometry, &pprops, &lprops);
			if (type == "") centerLabel.str(std::string());
		}
		else if (type == "surface_type")
		{
			std::vector<std::string> surfaceTypes;
			if (!(i->getSurfaceTypes(surfaceTypes)))
			{
				std::stringstream errorMessage;
				errorMessage << "\nError in visualizing ms_N_buildings.\n"
										 << "Could not get surface types from space."
										 << "(bso/visualization/visualization/models/ms_N_building.hpp)" << std::endl;
				throw std::runtime_error(errorMessage.str());
			}
			std::swap(surfaceTypes[0],surfaceTypes[2]);
			std::swap(surfaceTypes[4],surfaceTypes[5]);
			for (unsigned int j = 0; j < 6; ++j)
			{
				centerLabel.str(std::string());
				centerLabel << surfaceTypes[j];
				auto tempSurface = spaceGeometry.getPolygons()[j];
				auto surfaceCenter = tempSurface->getCenter();
				std::vector<geom::vertex> surfaceVertices;
				for (unsigned int k = 0; k < 4; ++k)
				{
					surfaceVertices.push_back((*tempSurface)[k] + 0.2*(surfaceCenter - (*tempSurface)[k]));
				}
				geom::quadrilateral surfaceGeometry(surfaceVertices);
				this->addPolygon(polygons,&surfaceGeometry,&ppropsSurfaceType,&lprops,0.0);
				this->addLabel(labels,&lbprops,centerLabel.str(),surfaceGeometry.getCenter());
			}
			centerLabel.str(std::string());
		}
		else if (type == "space_type")
		{
			this->addPolyhedron(polygons, &spaceGeometry, &ppropsSurfaceType, &lprops);
			std::string spaceType;
			if (!(i->getSpaceType(spaceType)))
			{
				std::stringstream errorMessage;
				errorMessage << "\nError in visualizing ms_N_buildings.\n"
										 << "Could not get space type from space."
										 << "(bso/visualization/visualization/models/ms_N_building.hpp)" << std::endl;
				throw std::runtime_error(errorMessage.str());
			}
			centerLabel << " - " << spaceType;
		}
		else
		{
			std::stringstream errorMessage;
			errorMessage << "\nError in visualizing ms_N_buildings.\n"
									 << "Did not recognize type of visualization: " << type << "\n"
									 << "Options are: spaces; surface_type; space_type\n" 
									 << "(bso/visualization/visualization/models/ms_N_building.hpp)" << std::endl;
			throw std::runtime_error(errorMessage.str());
		}
		
		//Add the space indixes to the visualization models
		this->addLabel(labels,&lbprops,centerLabel.str(),spaceGeometry.getCenter());
	}

	pbsp = new random_bsp(polygons);
} //

MS_Model::MS_Model(const std::string& fileName, const spatial_design::ms_N_building& msN, 
						const std::string& type /*= "spaces"*/, const std::string& title /*= "ms_building"*/,
						const double& linewidth /*= 1.0*/)
{
	std::ifstream input;
	if (!fileName.empty()) input.open(fileName.c_str());
	
	if (fileName.empty() || !input.is_open())
	{
		std::stringstream errorMessage;
		errorMessage << "Could not initialize a visualization of MS_N building spatial design with the following colour input file:" << std::endl
								 << ((fileName.empty())? "no input file given" : fileName) << std::endl
								 << "(bso/visualization/ms_building.hpp). " << std::endl;
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
					std::cout << ID << std::endl;
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
					std::cout << f1 << " ; "<< f2 << " ; "<< f3 << " ; " << f4 << std::endl;
					temp.ambient = rgba(f1, f2, f3, f4);
				
					f1 = utilities::trim_and_cast_float(*(++token));
					f2 = utilities::trim_and_cast_float(*(++token));
					f3 = utilities::trim_and_cast_float(*(++token));
					f4 = utilities::trim_and_cast_float(*(++token));
					std::cout << f1 << " ; "<< f2 << " ; "<< f3 << " ; " << f4 << std::endl;
					temp.diffuse = rgba(f1, f2, f3, f4);							
				
					f1 = utilities::trim_and_cast_float(*(++token));
					f2 = utilities::trim_and_cast_float(*(++token));
					f3 = utilities::trim_and_cast_float(*(++token));
					f4 = utilities::trim_and_cast_float(*(++token));
					std::cout << f1 << " ; "<< f2 << " ; "<< f3 << " ; " << f4 << std::endl;
					temp.specular = rgba(f1, f2, f3, f4);							
				
					f1 = utilities::trim_and_cast_float(*(++token));
					f2 = utilities::trim_and_cast_float(*(++token));
					f3 = utilities::trim_and_cast_float(*(++token));
					f4 = utilities::trim_and_cast_float(*(++token));
					std::cout << f1 << " ; "<< f2 << " ; "<< f3 << " ; " << f4 << std::endl;
					temp.emission = rgba(f1, f2, f3, f4);							
				
					temp.shininess = utilities::trim_and_cast_float(*(++token));
					std::cout << temp.shininess << std::endl;
				
					temp.translucent = utilities::trim_and_cast_bool(*(++token));
					std::cout << temp.translucent << std::endl;
				
					temp.twosided = utilities::trim_and_cast_bool(*(++token));
					std::cout << temp.translucent << std::endl;

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
	
	
	lprops.width = linewidth;
	mTitle = title;
	pprops.ambient = rgba(0.1f, 0.5f, 0.1f, 0.3f);
	pprops.diffuse = rgba(0.2f, 1.0f, 0.2f, 0.3f);
	pprops.specular = rgba(0.2f, 1.0f, 0.2f, 0.3f);
	pprops.emission = rgba(0.0f, 0.0f, 0.0f, 0.0f);
	pprops.shininess = 60.0f;
	pprops.translucent = true;
	pprops.twosided = true;

	ppropsSurfaceType.ambient = rgba(1.0f, 1.0f, 0.04f, 0.4f);
	ppropsSurfaceType.diffuse = rgba(1.0f, 1.0f, 0.04f, 0.4f);
	ppropsSurfaceType.specular = rgba(0.04f, 0.04f, 0.04f, 1.0f);
	ppropsSurfaceType.emission = rgba(0.04f, 0.04f, 0.04f, 1.0f);
	ppropsSurfaceType.shininess = 60.0;
	ppropsSurfaceType.translucent = true;
	ppropsSurfaceType.twosided = true;
	
	for (const auto& i : msN)
	{
		namespace geom = bso::utilities::geometry;
		geom::quad_hexahedron spaceGeometry = i->getGeometry();
		std::stringstream centerLabel;
		centerLabel << i->getID();

		// define default:
		polygon_props SpaceColorDefault;
		bool found = false;
		for(const auto i: spaceIDProps)
		{
			if(i.first == 0)
			{
				SpaceColorDefault = i.second;
				found = true;
				break;
			}
		}
		if(!found)
		{
			std::stringstream errorMessage;
			errorMessage << "\nError in visualizing ms_N_buildings.\n"
									 << "Did not receive a default color for a space: " << type << "\n"
									 << "(bso/visualization/visualization/models/ms_N_building.hpp)" << std::endl;
			throw std::runtime_error(errorMessage.str());
		}
		
		if (type == "spaces" || type == "" )
		{
			polygon_props colorSpace;
			int spaceID = i->getID();
			found = false;
			for(const auto i: spaceIDProps)
			{
				if(i.first == spaceID)
				{
					colorSpace = i.second;
					std::cout << SpaceColorDefault.shininess << std::endl;
					found = true;
					break;
				}
			}
			if(found)
			{
				this->addPolyhedron(polygons, &spaceGeometry, &colorSpace, &lprops);
			}
			else
			{
				this->addPolyhedron(polygons, &spaceGeometry, &pprops, &lprops);
			}
			if (type == "") centerLabel.str(std::string());
		}
		else
		{
			std::stringstream errorMessage;
			errorMessage << "\nError in visualizing ms_N_buildings.\n"
									 << "Did not recognize type of visualization: " << type << "\n"
									 << "Options are: spaces\n" 
									 << "(bso/visualization/visualization/models/ms_N_building.hpp)" << std::endl;
			throw std::runtime_error(errorMessage.str());
		}
		
		//Add the space indixes to the visualization models
		this->addLabel(labels,&lbprops,centerLabel.str(),spaceGeometry.getCenter());
	}

	pbsp = new random_bsp(polygons);
} //


MS_Model::~MS_Model()
{
	delete pbsp;

	for (std::list<polygon*>::iterator pit = polygons.begin();
			 pit != polygons.end(); pit++) delete *pit;

	for (std::list<label*>::iterator lbit = labels.begin();
			 lbit != labels.end(); lbit++) delete *lbit;
}

const std::string MS_Model::get_description()
{
	return mTitle;
}

void MS_Model::render(const camera &cam) const
{
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	pbsp->render_btf(cam);

	for (auto lbit = labels.begin(); lbit != labels.end(); lbit++) (*lbit)->render();

	glPopAttrib();
}

} // namespace visualization
} // namespace bso

#endif // VIS_MS_BUILDING_HPP
