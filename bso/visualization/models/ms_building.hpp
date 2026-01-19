#ifndef VIS_MS_BUILDING_HPP
#define VIS_MS_BUILDING_HPP

#include <bso/visualization/models/model_base.hpp>
#include <bso/visualization/bsp/bsp.hpp>
#include <bso/spatial_design/ms_building.hpp>
#include <bso/spatial_design/ms_N_building.hpp>
#include <bso/utilities/data_point.hpp>
#include <bso/utilities/clustering.hpp>

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
	polygon_props  ppropsDefault; 
	std::vector<polygon_props> performanceProps;
	std::vector<polygon_props> clusterProps;
	std::vector<std::pair<int, polygon_props>> spaceIDProps;

	line_props     lprops;
	label_props    lbprops;
	random_bsp     *pbsp;
public:
	MS_Model(const spatial_design::ms_building& ms, const std::string& type = "spaces",
					 const std::string& title = "ms_building", const double& linewidth = 1.0);
	MS_Model(const spatial_design::ms_N_building& msN, const std::string& type = "spaces",
					 const std::string& title = "ms_N_building", const double& linewidth = 1.0);
	MS_Model(const utilities::clustering& clus, const std::string& title = "ms_N_clustering", const double& linewidth = 1.0);
	MS_Model(const std::vector<utilities::data_point>& spaceData, const std::vector<double>& resultWeightFactor, float transparency =1.0, const std::string& title = "ms_N_performances", const double& linewidth = 1.0);

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
	
	
	lprops.width = linewidth;
	mTitle = title;
	pprops.ambient = rgba(0.1f, 0.5f, 0.1f, 0.3f);
	pprops.diffuse = rgba(0.2f, 1.0f, 0.2f, 0.3f);
	pprops.specular = rgba(0.2f, 1.0f, 0.2f, 0.3f);
	pprops.emission = rgba(0.0f, 0.0f, 0.0f, 0.0f);
	pprops.shininess = 60.0f;
	pprops.translucent = true;
	pprops.twosided = true;

	for (const auto& i : msN)
	{
		namespace geom = bso::utilities::geometry;
		geom::quad_hexahedron spaceGeometry = i->getGeometry();
		std::stringstream centerLabel;

		// define default:
		bool defaultDefined = false;
		for(const auto i: spaceIDProps)
		{
			if(i.first == 0)
			{
				ppropsDefault = i.second;
				defaultDefined = true;
				break;
			}
		}
		
		if (type == "spaces" || type == "" )
		{
			int spaceID = i->getID();
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
			if(found)
			{
				this->addPolyhedron(polygons, &spaceGeometry, &(spaceIDProps[index].second), &lprops);
			}
			else if(defaultDefined)
			{
				this->addPolyhedron(polygons, &spaceGeometry, &ppropsDefault, &lprops);
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
		
		//Add the space indexes to the visualization models
		this->addLabel(labels,&lbprops,centerLabel.str(),spaceGeometry.getCenter());
	}

	pbsp = new random_bsp(polygons);
} //

MS_Model::MS_Model(const utilities::clustering& clus, const std::string& title /*= "ms_N_building"*/, const double& linewidth /*= 1.0*/)
{
	// Get and check initial data for validity for this function
	const auto clusters = clus.getClusters();
	double clusterSize = clusters.size(); // defined as double for later calculations
	if(clusterSize == 0)
	{
		std::stringstream errorMessage;
			errorMessage << "\nError in visualizing ms_N_buildings clusters.\n"
									 << "The amount of inserted clusters is zero, therefore there is nothing to visualize: " << clusterSize << "\n"
									 << "Options are: spaces; surface_type; space_type\n" 
									 << "(bso/visualization/visualization/models/ms_N_building.hpp)" << std::endl;
			throw std::runtime_error(errorMessage.str());
	}

	// Initiate variables
	lprops.width = linewidth;
	mTitle = title;
	float f1, f2, f3, f4 = 0.3;

	// Define color for cluster and insert the spaces of that cluster in that color
	polygon_props temp;
	clusterProps.resize(clusterSize);
	for (double j = 0; j < clusterSize; j++) // defined as double for later calculations
	{
		// the clusterSize is transformed to as wide possible range of colors.
		double range;
		range = j/(clusterSize-1);

		// Define rgb color value's according to range(i.e. percentage) values
		double factor;
		if(range > 0.75 && range <= 1.0) // red till yellow
		{
			factor = 1-(range-0.75)/0.25;
			f1=1.0;
			f2=1.0*factor;
			f3=0;
		}
		else if(range >= 0.5 && range <= 0.75) // yellow till green
		{
			factor = (range-0.5)/0.25;
			f1=1.0*factor;
			f2=1.0;
			f3=0;
		}
		else if(range >= 0.25 && range <= 0.5) // green till aqua
		{
			factor = 1-(range-0.25)/0.25;
			f1=0;
			f2=1.0;
			f3=1.0*factor;
		}
		else if(range >= 0 && range <= 0.25) // aqua till blue
		{
			factor = (range)/0.25;
			f1=0;
			f2=1.0*factor;
			f3=1.0;
		}
		else
		{
			std::stringstream errorMessage;
			errorMessage << "\nError in visualizing ms_N_buildings clusters.\n"
									 << "The range value is not within the expected range of 0 to 1: " << range << "\n"
									 << "Therefor f1, f2, and f3 could not be defined properly\n"
									 << "(bso/visualization/visualization/models/ms_N_building.hpp)" << std::endl;
			throw std::runtime_error(errorMessage.str());
		}
				
		temp.ambient = rgba(f1, f2, f3, f4);
		temp.diffuse = rgba(f1, f2, f3, f4);							
		temp.specular = rgba(f1, f2, f3, f4);							
		temp.emission = rgba(0.0f, 0.0f, 0.0f, 0.0f);						
		temp.shininess = 60.0f;
		temp.translucent = true;
		temp.twosided = true;

		// Add new color to clusterProps
		clusterProps[j] = temp; // The data clusterProps should already be full with the amount of element required since otherwise the data is rearranged and we can't send it trough pointer reference to visualization

		// Send geometry from cluster to visualization with just generated according color
		namespace geom = bso::utilities::geometry;
		for (const auto& i : clusters[j].getData())
		{
			geom::quad_hexahedron spaceGeometry = i->getMSpacePtrs().back()->getGeometry();
			std::stringstream centerLabel;
			centerLabel << i->getMSpacePtrs().back()->getID();

			this->addPolyhedron(polygons, &spaceGeometry, &clusterProps[j], &lprops);
		}
	}

	pbsp = new random_bsp(polygons);
} //

MS_Model::MS_Model(const std::vector<utilities::data_point>& spaceData, const std::vector<double>& resultWeightFactor, float transparency /*=1.0*/, const std::string& title /*= "ms_N_building"*/, const double& linewidth /*= 1.0*/)
{
	// Check prerequicatate:
	int spaceDataSize = spaceData.size();
	if(spaceDataSize == 0)
	{
		std::stringstream errorMessage;
			errorMessage << "\nError in visualizing ms_N_buildings performance.\n"
									 << "The amount of inserted performances is zero, therefore there is nothing to visualize: " << spaceDataSize << "\n"
									 << "(bso/visualization/visualization/models/ms_N_building.hpp)" << std::endl;
			throw std::runtime_error(errorMessage.str());
	}
	double sumW = 0;
	for(unsigned int j = 0; j < resultWeightFactor.size(); ++j){sumW = sumW + resultWeightFactor[j];}
	if(sumW <= 1+0.000000001 && sumW >= 1+0.000000001)
	{
		std::stringstream errorMessage;
			errorMessage << "\nError in visualizing ms_N_buildings performance.\n"
									 << "The values (i.e. percentages) inserted in the resultWeightFactor did not sum up to a 1,\n"
									 << "Therefore does toe values not represent percentages and can it not be used for visualization.\n"
									 << "The found sum of the values is: "<< sumW << ".\n"
									 << "(bso/visualization/visualization/models/ms_N_building.hpp)" << std::endl;
			throw std::runtime_error(errorMessage.str());
	}

	// initiate standard data:
	lprops.width = linewidth;
	mTitle = title;

	// Generate color for each performance and add space to visualization:
	float f1, f2, f3, f4 = transparency;
	performanceProps.resize(spaceData.size());
	for(long unsigned int i=0; i<spaceData.size(); i++)
	{
		// check if the spaceData has the same amount of numbers stored then the provided resultWeightFactor, if not error:
		if(spaceData[i].size() != resultWeightFactor.size())
		{
			std::stringstream errorMessage;
			errorMessage << "\nError in visualizing ms_N_buildings performance.\n"
									 << "The inserted resultWeightFactor does not have the same dimension as the data,\n"
									 << "therefore can the data not be weighted for correct visualizations\n"
									 << "the dimensions were for the data point: " << spaceData[i].size() << " and for the resultWeightFactor " <<  resultWeightFactor.size() << "\n"
									 << "(bso/visualization/visualization/models/ms_N_building.hpp)" << std::endl;
			throw std::runtime_error(errorMessage.str());
		}
		
		// Modify the date with the resultWeight to get a clear scale from 1 till 0 value's despite incorporating more then one kind of data, i.e. sd and bp
		utilities::data_point dataWeighted = spaceData[i];
		for(unsigned int j = 0; j < (spaceData[i]).size(); ++j)
		{
			dataWeighted(j) = (spaceData[i])(j) * resultWeightFactor[j];
		}

		// Sum the performances of all the objectives:
		double performance = 0;
		for(unsigned int j = 0; j < dataWeighted.size(); ++j) performance = performance + dataWeighted(j);

		// Define rgb color value's according to performance
		double factor;
		if(performance > 0.5 && performance <= (1.0+0.0001)) // red till yellow
		{
			factor = 1-(performance-0.5)/0.5;
			f1=1.0;
			f2=1.0*factor;
			f3=0;
		}
		else if(performance >= (0-0.001) && performance <= 0.5) // yellow till green
		{
			factor = (performance)/0.5;
			f1=1.0*factor;
			f2=1.0;
			f3=0;
		}
		else
		{
			std::stringstream errorMessage;
			errorMessage << "\nError in visualizing ms_N_buildings performance.\n"
									 << "The inserted spaceData contains results outside the range of 0 till 1,\n"
									 << "as a result, the colors can not be defined correctly. \n"
									 << "the found value is: " << performance <<". \n"
									 << "please normalize the data prior to importing the data in this function. \n"
									 << "(bso/visualization/visualization/models/ms_N_building.hpp)" << std::endl;
			throw std::runtime_error(errorMessage.str());
		}
		
		// Generate color for specific performance:
		polygon_props temp;
		temp.emission = rgba(0.0f, 0.0f, 0.0f, 0.0f);						
		temp.shininess = 60.0f;
		temp.translucent = true;
		temp.twosided = true;
		temp.ambient = rgba(f1, f2, f3, f4);
		temp.diffuse = rgba(f1, f2, f3, f4);							
		temp.specular = rgba(f1, f2, f3, f4);
		
		// Add new to performanceProps
		performanceProps[i] = temp;

		// Send space to visualization with corrsponding color:
		namespace geom = bso::utilities::geometry;
		geom::quad_hexahedron spaceGeometry = (spaceData[i]).getMSpacePtrs().back()->getGeometry();
		std::stringstream centerLabel;
		centerLabel << (spaceData[i]).getMSpacePtrs().back()->getID();
		this->addPolyhedron(polygons, &spaceGeometry, &(performanceProps[i]), &lprops);
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
