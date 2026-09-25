#ifndef VIS_MS_BUILDING_HPP
#define VIS_MS_BUILDING_HPP

#include <bso/visualization/models/model_base.hpp>
#include <bso/visualization/utilities/add_geometry.hpp>
#include <bso/visualization/bsp/bsp.hpp>
#include <bso/spatial_design/ms_building.hpp>
#include <bso/spatial_design/ms_N_building.hpp>
#include <bso/utilities/geometry/quadrilateral.hpp>
#include <bso/utilities/data_point.hpp>
#include <bso/utilities/clustering.hpp>

#include <boost/algorithm/string.hpp>
#include <utility>
#include <fstream>
#include <limits>

#ifdef SD_MODEL_HPP
#include <bso/structural_design/sd_model.hpp>
#endif // SD_MODEL_HPP

#ifdef BP_MODEL_HPP
#include <bso/building_physics/bp_model.hpp>
#endif // BP_MODEL_HPP

#ifdef DL_MODEL_HPP
#include <bso/daylight/dl_model.hpp>
#endif // DL_MODEL_HPP

#include <bitset>

namespace bso { namespace visualization
{

inline bool& showMSModelLabels()
{
	static bool enabled = true;
	return enabled;
}

inline bool& showMSModelLegend()
{
	return showModelLegend();
}

// Helper functions for creating overlay planes for MS models in any of the visualization contexts (e.g., MS building, MS N building, etc.)
inline bool& showMSModelOverlayPlanes() { return showModelOverlayPlanes(); }
inline std::vector<bso::utilities::geometry::quadrilateral>& msModelOverlayPlanes() { return modelOverlayPlanes(); }
inline void setMSModelOverlayPlanes(const std::vector<bso::utilities::geometry::quadrilateral>& planes) { setModelOverlayPlanes(planes); }
inline void clearMSModelOverlayPlanes() { clearModelOverlayPlanes(); }

inline bso::utilities::geometry::quadrilateral makeMSModelOverlayPlane(
	const bso::utilities::geometry::vertex& location,
	bso::utilities::geometry::vector normal,
	const double halfSize = 30000.0)
{
	return makeModelOverlayPlane(location, normal, halfSize);
}
inline void setMSModelOverlayPlane(
	const bso::utilities::geometry::vertex& location,
	bso::utilities::geometry::vector normal,
	const double halfSize = 30000.0)
{
	setModelOverlayPlane(location, normal, halfSize);
}

class MS_Model : public model_base
{
private:
	std::string mTitle;
	std::list<polygon*> polygons;
	std::list<label*>   labels;

	polygon_props  pprops;
	polygon_props  ppropsHighlight;
	polygon_props  ppropsHighlight1;
	polygon_props  ppropsHighlight2;
	polygon_props  ppropsHighlight3;
	polygon_props  ppropsHighlight4;
	polygon_props  ppropsSurfaceType; 
	polygon_props  ppropsDefault; 
	std::vector<polygon_props> performanceProps;
	std::vector<polygon_props> clusterProps;
	std::vector<std::pair<int, polygon_props>> spaceIDProps;

	line_props     lprops;
	label_props    lbprops;
	random_bsp     *pbsp;
	bool mLegendHasUnchanged = false;
	bool mLegendHasNew = false;
	bool mLegendHasShifted = false;
public:
	MS_Model(const spatial_design::ms_building& ms, const std::string& type = "spaces",
					 const std::string& title = "ms_building", const double& linewidth = 1.0, const float& transparency = 0.3f);
	MS_Model(const spatial_design::ms_N_building& msN, const std::string& type = "spaces",
					 const std::string& title = "ms_N_building", const double& linewidth = 1.0, const float& transparency = 0.3f);
	MS_Model(const utilities::clustering& clus, const std::string& title = "ms_N_clustering", const double& linewidth = 1.0, const float& transparency = 0.3f);
	MS_Model(const std::vector<utilities::data_point>& spaceData, const std::vector<double>& resultWeightFactor, float transparency =1.0, const std::string& title = "ms_N_performances", const double& linewidth = 1.0);

	/*
	The following constructor contain the ability for defining different colors per space defined in a file 
	(first input contains the file name for this input)
	A description of how this file should look like is shown bellow:
	
	# space with ID zero is considered the standard color.
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
	bool key_pressed(int key) override;
	const std::string get_description();
}; // MS_Model

MS_Model::MS_Model(const spatial_design::ms_building& ms,
						const std::string& type /*= "spaces"*/, const std::string& title /*= "ms_building"*/,
						const double& linewidth /*= 1.0*/, const float& transparency /*= 0.3f*/)
{
	// Default green color for spaces
	lprops.width = linewidth;
	lbprops.textcolor = rgba(0.0f, 0.0f, 0.0f, 1.0f);
	mTitle = title;
	pprops.ambient = rgba(0.1f, 0.5f, 0.1f, transparency);
	pprops.diffuse = rgba(0.2f, 1.0f, 0.2f, transparency);
	pprops.specular = rgba(0.2f, 1.0f, 0.2f, transparency);
	pprops.emission = rgba(0.0f, 0.0f, 0.0f, 0.0f);
	pprops.shininess = 60.0f;
	pprops.translucent = true;
	pprops.twosided = true;

	// orange/gold highlight with transparency, spaces with marker "highlightInVisualization" will be visualized with this color
	ppropsHighlight.ambient = rgba(1.0f, 0.65f, 0.2f, transparency);
	ppropsHighlight.diffuse = rgba(1.0f, 0.65f, 0.2f, transparency);
	ppropsHighlight.specular = rgba(1.0f, 0.65f, 0.2f, transparency);
	ppropsHighlight.emission = rgba(0.0f, 0.0f, 0.0f, 0.0f);
	ppropsHighlight.shininess = 60.0f;
	ppropsHighlight.translucent = true;
	ppropsHighlight.twosided = true;

	// Clear blue highlight with transparency, spaces with marker "highlightInVisualization1" will be visualized with this color
	ppropsHighlight1.ambient = rgba(0.2f, 0.2f, 1.0f, transparency);
	ppropsHighlight1.diffuse = rgba(0.2f, 0.2f, 1.0f, transparency);
	ppropsHighlight1.specular = rgba(0.2f, 0.2f, 1.0f, transparency);
	ppropsHighlight1.emission = rgba(0.0f, 0.0f, 0.0f, 0.0f);
	ppropsHighlight1.shininess = 60.0f;
	ppropsHighlight1.translucent = true;
	ppropsHighlight1.twosided = true;

	// Magenta highlight with transparency, spaces with marker "highlightInVisualization2" will be visualized with this color
	ppropsHighlight2.ambient = rgba(1.0f, 0.2f, 0.8f, transparency);
	ppropsHighlight2.diffuse = rgba(1.0f, 0.2f, 0.8f, transparency);
	ppropsHighlight2.specular = rgba(1.0f, 0.2f, 0.8f, transparency);
	ppropsHighlight2.emission = rgba(1.0f, 0.2f, 0.8f, 0.3f);
	ppropsHighlight2.shininess = 60.0f;
	ppropsHighlight2.translucent = true;
	ppropsHighlight2.twosided = true;

	// yellow highlight with transparency, spaces with marker "highlightInVisualization3" will be visualized with this color
	ppropsHighlight3.ambient = rgba(1.0f, 1.0f, 0.2f, transparency);
	ppropsHighlight3.diffuse = rgba(1.0f, 1.0f, 0.2f, transparency);
	ppropsHighlight3.specular = rgba(1.0f, 1.0f, 0.2f, transparency);
	ppropsHighlight3.emission = rgba(0.0f, 0.0f, 0.0f, 0.0f);
	ppropsHighlight3.shininess = 60.0f;
	ppropsHighlight3.translucent = true;
	ppropsHighlight3.twosided = true;

	// bright red highlight with transparency, spaces with marker "highlightInVisualization4" will be visualized with this color
	ppropsHighlight4.ambient = rgba(1.0f, 0.2f, 0.2f, transparency);
	ppropsHighlight4.diffuse = rgba(1.0f, 0.2f, 0.2f, transparency);
	ppropsHighlight4.specular = rgba(1.0f, 0.2f, 0.2f, transparency);
	ppropsHighlight4.emission = rgba(0.0f, 0.0f, 0.0f, 0.0f);
	ppropsHighlight4.shininess = 60.0f;
	ppropsHighlight4.translucent = true;
	ppropsHighlight4.twosided = true;

	// standard color for surface types, bright yellow with transparency
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
		const bool isShifted = i->highlightInVisualization3;
		const bool isNew = (!isShifted && i->highlightInVisualization1);
		const bool isUnchanged = !(i->highlightInVisualization || i->highlightInVisualization1 ||
			i->highlightInVisualization2 || i->highlightInVisualization3 || i->highlightInVisualization4);
		if (isShifted) mLegendHasShifted = true;
		if (isNew) mLegendHasNew = true;
		if (isUnchanged) mLegendHasUnchanged = true;
		std::stringstream centerLabel;
		centerLabel << i->getID();
		
		if (type == "spaces" || type == "" )
		{
			if(i->highlightInVisualization)
			{
				this->addPolyhedron(polygons, &spaceGeometry, &ppropsHighlight, &lprops);
				if (type == "") centerLabel.str(std::string());
				
			}
			else if(i->highlightInVisualization1)
			{
				this->addPolyhedron(polygons, &spaceGeometry, &ppropsHighlight1, &lprops);
				if (type == "") centerLabel.str(std::string());
			}
			else if(i->highlightInVisualization2)
			{
				this->addPolyhedron(polygons, &spaceGeometry, &ppropsHighlight2, &lprops);
				if (type == "") centerLabel.str(std::string());
			}
			else if(i->highlightInVisualization3)
			{
				this->addPolyhedron(polygons, &spaceGeometry, &ppropsHighlight3, &lprops);
				if (type == "") centerLabel.str(std::string());
			}
			else if(i->highlightInVisualization4)
			{
				this->addPolyhedron(polygons, &spaceGeometry, &ppropsHighlight4, &lprops);
				if (type == "") centerLabel.str(std::string());
			}
			else
			{
				this->addPolyhedron(polygons, &spaceGeometry, &pprops, &lprops);
				if (type == "") centerLabel.str(std::string());
			}
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
		
		//Add the space indexes to the visualization models
		this->addLabel(labels,&lbprops,centerLabel.str(),spaceGeometry.getCenter());
	}

	pbsp = new random_bsp(polygons);
} //

MS_Model::MS_Model(const spatial_design::ms_N_building& msN,
						const std::string& type /*= "spaces"*/, const std::string& title /*= "ms_building"*/,
						const double& linewidth /*= 1.0*/, const float& transparency /*= 0.3f*/)
{
	lprops.width = linewidth;
	lbprops.textcolor = rgba(0.0f, 0.0f, 0.0f, 1.0f);
	mTitle = title;

	// Clear green highlight with transparency, default color of spaces
	pprops.ambient = rgba(0.1f, 0.5f, 0.1f, transparency);
	pprops.diffuse = rgba(0.2f, 1.0f, 0.2f, transparency);
	pprops.specular = rgba(0.2f, 1.0f, 0.2f, transparency);
	pprops.emission = rgba(0.0f, 0.0f, 0.0f, 0.0f);
	pprops.shininess = 60.0f;
	pprops.translucent = true;
	pprops.twosided = true;

	// orange/gold highlight with transparency, spaces with marker "highlightInVisualization" will be visualized with this color
	ppropsHighlight.ambient = rgba(1.0f, 0.65f, 0.2f, transparency);
	ppropsHighlight.diffuse = rgba(1.0f, 0.65f, 0.2f, transparency);
	ppropsHighlight.specular = rgba(1.0f, 0.65f, 0.2f, transparency);
	ppropsHighlight.emission = rgba(0.0f, 0.0f, 0.0f, 0.0f);
	ppropsHighlight.shininess = 60.0f;
	ppropsHighlight.translucent = true;
	ppropsHighlight.twosided = true;

	// Clear blue highlight with transparency, spaces with marker "highlightInVisualization1" will be visualized with this color
	ppropsHighlight1.ambient = rgba(0.2f, 0.2f, 1.0f, transparency);
	ppropsHighlight1.diffuse = rgba(0.2f, 0.2f, 1.0f, transparency);
	ppropsHighlight1.specular = rgba(0.2f, 0.2f, 1.0f, transparency);
	ppropsHighlight1.emission = rgba(0.0f, 0.0f, 0.0f, 0.0f);
	ppropsHighlight1.shininess = 60.0f;
	ppropsHighlight1.translucent = true;
	ppropsHighlight1.twosided = true;

	// Magenta highlight with transparency, spaces with marker "highlightInVisualization2" will be visualized with this color
	ppropsHighlight2.ambient = rgba(1.0f, 0.2f, 0.8f, transparency);
	ppropsHighlight2.diffuse = rgba(1.0f, 0.2f, 0.8f, transparency);
	ppropsHighlight2.specular = rgba(1.0f, 0.2f, 0.8f, transparency);
	ppropsHighlight2.emission = rgba(1.0f, 0.2f, 0.8f, 0.0f);
	ppropsHighlight2.shininess = 60.0f;
	ppropsHighlight2.translucent = true;
	ppropsHighlight2.twosided = true;

	// yellow highlight with transparency, spaces with marker "highlightInVisualization3" will be visualized with this color
	ppropsHighlight3.ambient = rgba(1.0f, 1.0f, 0.2f, transparency);
	ppropsHighlight3.diffuse = rgba(1.0f, 1.0f, 0.2f, transparency);
	ppropsHighlight3.specular = rgba(1.0f, 1.0f, 0.2f, transparency);
	ppropsHighlight3.emission = rgba(0.0f, 0.0f, 0.0f, 0.0f);
	ppropsHighlight3.shininess = 60.0f;
	ppropsHighlight3.translucent = true;
	ppropsHighlight3.twosided = true;

	// bright red highlight with transparency, spaces with marker "highlightInVisualization4" will be visualized with this color
	ppropsHighlight4.ambient = rgba(1.0f, 0.2f, 0.2f, transparency);
	ppropsHighlight4.diffuse = rgba(1.0f, 0.2f, 0.2f, transparency);
	ppropsHighlight4.specular = rgba(1.0f, 0.2f, 0.2f, transparency);
	ppropsHighlight4.emission = rgba(0.0f, 0.0f, 0.0f, 0.0f);
	ppropsHighlight4.shininess = 60.0f;
	ppropsHighlight4.translucent = true;
	ppropsHighlight4.twosided = true;

	// standard color for surface types, bright yellow with transparency
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
		const bool isShifted = i->highlightInVisualization3;
		const bool isNew = (!isShifted && i->highlightInVisualization1);
		const bool isUnchanged = !(i->highlightInVisualization || i->highlightInVisualization1 ||
			i->highlightInVisualization2 || i->highlightInVisualization3 || i->highlightInVisualization4);
		if (isShifted) mLegendHasShifted = true;
		if (isNew) mLegendHasNew = true;
		if (isUnchanged) mLegendHasUnchanged = true;
		std::stringstream centerLabel;
		centerLabel << i->getID();
		
		if (type == "spaces" || type == "" )
		{
			if(i->highlightInVisualization4) // red-color first so it overrides the other highlights when multiple highlights are true for one space.
			{
				this->addPolyhedron(polygons, &spaceGeometry, &ppropsHighlight4, &lprops);
				if (type == "") centerLabel.str(std::string());
			}
			else if(i->highlightInVisualization)
			{
				this->addPolyhedron(polygons, &spaceGeometry, &ppropsHighlight, &lprops);
				if (type == "") centerLabel.str(std::string());
			}
			else if(i->highlightInVisualization1)
			{
				this->addPolyhedron(polygons, &spaceGeometry, &ppropsHighlight1, &lprops);
				if (type == "") centerLabel.str(std::string());
			}
			else if(i->highlightInVisualization2)
			{
				this->addPolyhedron(polygons, &spaceGeometry, &ppropsHighlight2, &lprops);
				if (type == "") centerLabel.str(std::string());
			}
			else if(i->highlightInVisualization3)
			{
				this->addPolyhedron(polygons, &spaceGeometry, &ppropsHighlight3, &lprops);
				if (type == "") centerLabel.str(std::string());
			}
			else
			{
				this->addPolyhedron(polygons, &spaceGeometry, &pprops, &lprops);
				if (type == "") centerLabel.str(std::string());
			}
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

MS_Model::MS_Model(const utilities::clustering& clus, const std::string& title /*= "ms_N_building"*/, const double& linewidth /*= 1.0*/, const float& transparency /*= 0.3f*/)
{
	// Get and check initial data for validity for this function
	lbprops.textcolor = rgba(0.0f, 0.0f, 0.0f, 1.0f);
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
	float f1, f2, f3, f4 = transparency;

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
	lbprops.textcolor = rgba(0.0f, 0.0f, 0.0f, 1.0f);
	int spaceDataSize = spaceData.size();
	if(spaceDataSize == 0)
	{
		std::stringstream errorMessage;
			errorMessage << "\nError in visualizing ms_N_buildings performance.\n"
									 << "The amount of inserted performances is zero, therefore there is nothing to visualize: " << spaceDataSize << "\n"
									 << "(bso/visualization/visualization/models/ms_N_building.hpp)" << std::endl;
			throw std::runtime_error(errorMessage.str());
	}
	if(spaceData[0].size() != resultWeightFactor.size())
	{
		std::stringstream errorMessage;
			errorMessage << "\nError in visualizing ms_N_buildings performance.\n"
									 << "The inserted dimention of the data_point and the amount of inserted weight factors do not match,\n"
									 << "therefore the data can not be weighted correctly for visualization: " << spaceData[0].size() << " and " << resultWeightFactor.size() << "\n"
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

		// Define rgb color value's according to performance,
		// with 0.5 as the threshold between red and green, 
		//and 1.0 as the maximum value for red and marker of the worst performance.
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

// Write a colour settings file that reproduces the same per-space colours
// as the performance-based MS_Model constructor above. The resulting file
// can be read by the existing MS_Model(const std::string&, const spatial_design::ms_N_building&, ...)
// constructor (and the corresponding visualize overload) to recreate the
// visualization without recomputing the performance values.
inline void write_msN_performance_visualization_to_file(
	const std::string& fileName,
	const std::vector<utilities::data_point>& spaceData,
	const std::vector<double>& resultWeightFactor,
	float transparency = 1.0f)
{
	if (spaceData.empty())
	{
		std::stringstream errorMessage;
		errorMessage << "\nError in writing ms_N_buildings performance visualization file.\n"
						 << "The amount of inserted performances is zero, therefore there is nothing to write.\n"
						 << "(bso/visualization/visualization/models/ms_N_building.hpp)" << std::endl;
		throw std::runtime_error(errorMessage.str());
	}
	if (spaceData[0].size() != resultWeightFactor.size())
	{
		std::stringstream errorMessage;
		errorMessage << "\nError in writing ms_N_buildings performance visualization file.\n"
						 << "The inserted dimension of the data_point and the amount of inserted weight factors do not match,\n"
						 << "therefore the data can not be weighted correctly for visualization.\n"
						 << "The dimensions are: " << spaceData[0].size() << " and " << resultWeightFactor.size() << "\n"
						 << "(bso/visualization/visualization/models/ms_N_building.hpp)" << std::endl;
		throw std::runtime_error(errorMessage.str());
	}

	std::ofstream output(fileName.c_str());
	if (!output.is_open())
	{
		std::stringstream errorMessage;
		errorMessage << "Could not open file to write ms_N_building performance visualization: "
						 << (fileName.empty() ? std::string("<empty file name>") : fileName) << "\n"
						 << "(bso/visualization/visualization/models/ms_N_building.hpp)" << std::endl;
		throw std::invalid_argument(errorMessage.str());
	}

	// Header comments describing the format, matching the existing colour-input convention.
	output << "# ms_N_building performance visualization colour settings" << std::endl;
	output << "# space_ID\tambient(rgba)\t\tdiffuse(rgba)\t\tspecular(rgba)\temission(rgba)\tshininess(float value)\ttranslucent(bool)\t\ttwoSided(bool)" << std::endl;

	float f1, f2, f3, f4 = transparency;
	for (std::size_t i = 0; i < spaceData.size(); ++i)
	{
		if (spaceData[i].size() != resultWeightFactor.size())
		{
			std::stringstream errorMessage;
			errorMessage << "\nError in writing ms_N_buildings performance visualization file.\n"
						 << "The inserted resultWeightFactor does not have the same dimension as the data,\n"
						 << "therefore can the data not be weighted for correct visualizations.\n"
						 << "The dimensions were for the data point: " << spaceData[i].size()
						 << " and for the resultWeightFactor " << resultWeightFactor.size() << "\n"
						 << "(bso/visualization/visualization/models/ms_N_building.hpp)" << std::endl;
			throw std::runtime_error(errorMessage.str());
		}

		utilities::data_point dataWeighted = spaceData[i];
		for (unsigned int j = 0; j < spaceData[i].size(); ++j)
		{
			dataWeighted(j) = (spaceData[i])(j) * resultWeightFactor[j];
		}

		double performance = 0.0;
		for (unsigned int j = 0; j < dataWeighted.size(); ++j)
		{
			performance += dataWeighted(j);
		}

		double factor;
		if (performance > 0.5 && performance <= (1.0 + 0.0001)) // red till yellow
		{
			factor = 1 - (performance - 0.5) / 0.5;
			f1 = 1.0f;
			f2 = static_cast<float>(1.0 * factor);
			f3 = 0.0f;
		}
		else if (performance >= (0 - 0.001) && performance <= 0.5) // yellow till green
		{
			factor = (performance) / 0.5;
			f1 = static_cast<float>(1.0 * factor);
			f2 = 1.0f;
			f3 = 0.0f;
		}
		else
		{
			std::stringstream errorMessage;
			errorMessage << "\nError in writing ms_N_buildings performance visualization file.\n"
						 << "The inserted spaceData contains results outside the range of 0 till 1,\n"
						 << "as a result, the colors can not be defined correctly.\n"
						 << "The found value is: " << performance << ".\n"
						 << "Please normalize the data prior to importing the data in this function.\n"
						 << "(bso/visualization/visualization/models/ms_N_building.hpp)" << std::endl;
			throw std::runtime_error(errorMessage.str());
		}

		int spaceID = spaceData[i].getMSpacePtrs().back()->getID();

		// Write one line following the existing colour settings format.
		output << spaceID << ", "
				 << f1 << "," << f2 << "," << f3 << "," << f4 << ", "
				 << f1 << "," << f2 << "," << f3 << "," << f4 << ", "
				 << f1 << "," << f2 << "," << f3 << "," << f4 << ", "
				 << 0.0f << "," << 0.0f << "," << 0.0f << "," << 0.0f << ", "
				 << 60.0f << ", "
				 << "true, true" << std::endl;
	}
}

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
		lbprops.textcolor = rgba(0.0f, 0.0f, 0.0f, 1.0f);
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
		for(const auto& i: spaceIDProps)
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
			long unsigned int index = 0;
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

	if (showMSModelLabels())
	{
		for (auto lbit = labels.begin(); lbit != labels.end(); lbit++) (*lbit)->render();
	}

	if (showMSModelOverlayPlanes() && !msModelOverlayPlanes().empty())
	{
		glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT | GL_LIGHTING_BIT);
		glDisable(GL_LIGHTING);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(1.0f, 0.15f, 0.15f, 0.45f);
		for (const auto& quad : msModelOverlayPlanes())
		{
			double minX = std::numeric_limits<double>::infinity();
			double minY = std::numeric_limits<double>::infinity();
			double maxX = -std::numeric_limits<double>::infinity();
			double maxY = -std::numeric_limits<double>::infinity();
			double zAccum = 0.0;
			int count = 0;
			for (const auto& v : quad)
			{
				if (v.x() < minX) minX = v.x();
				if (v.y() < minY) minY = v.y();
				if (v.x() > maxX) maxX = v.x();
				if (v.y() > maxY) maxY = v.y();
				zAccum += v.z();
				++count;
			}
			if (count == 0) continue;
			double zPlane = zAccum / count;

			glBegin(GL_QUADS);
			glVertex3d(minX, minY, zPlane);
			glVertex3d(maxX, minY, zPlane);
			glVertex3d(maxX, maxY, zPlane);
			glVertex3d(minX, maxY, zPlane);
			glEnd();
		}
		glPopAttrib();
	}

	if (showMSModelLegend())
	{
		int vp[4];
		glGetIntegerv(GL_VIEWPORT, vp);
		const int viewportWidth = vp[2];
		const int viewportHeight = vp[3];

		const int pad = 10;
		const int boxW = 16;
		const int boxH = 10;
		const int rowGap = 5;
		const int titleHeight = 14;
		const int textXOffset = boxW + 8;
		const int panelW = 260;
		struct LegendItem
		{
			float r, g, b, a;
			std::string text;
		};
		std::vector<LegendItem> items;
		if (mLegendHasShifted) items.push_back({1.0f, 1.0f, 0.2f, 0.3f, "Shifted space"});
		if (mLegendHasNew) items.push_back({0.2f, 0.2f, 1.0f, 0.3f, "New space"});
		if (mLegendHasUnchanged) items.push_back({0.2f, 1.0f, 0.2f, 0.3f, "unchanged space"});
		if (items.empty()) items.push_back({0.2f, 1.0f, 0.2f, 0.3f, "unchanged space"});

		const int panelH = titleHeight + 12 + static_cast<int>(items.size()) * (boxH + rowGap) + pad;
		const int panelX = pad;
		const int panelY = viewportHeight - panelH - pad;

		auto drawText = [](int x, int y, const std::string& text)
		{
			glRasterPos3i(x, y, 0);
			for (char c : text) glutBitmapCharacter(GLUT_BITMAP_8_BY_13, c);
		};

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0.0, (GLdouble)viewportWidth, 0.0, (GLdouble)viewportHeight, 0.0, 1.0);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_CURRENT_BIT);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glColor4f(0.0f, 0.0f, 0.0f, 0.55f);
		glBegin(GL_QUADS);
		glVertex3i(panelX, panelY, 0);
		glVertex3i(panelX + panelW, panelY, 0);
		glVertex3i(panelX + panelW, panelY + panelH, 0);
		glVertex3i(panelX, panelY + panelH, 0);
		glEnd();

		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		drawText(panelX + pad, panelY + panelH - titleHeight, "Space color legend");

		for (int idx = 0; idx < static_cast<int>(items.size()); ++idx)
		{
			int rowY = panelY + panelH - titleHeight - 10 - (idx + 1) * (boxH + rowGap);
			glColor4f(items[idx].r, items[idx].g, items[idx].b, items[idx].a);
			glBegin(GL_QUADS);
			glVertex3i(panelX + pad, rowY, 0);
			glVertex3i(panelX + pad + boxW, rowY, 0);
			glVertex3i(panelX + pad + boxW, rowY + boxH, 0);
			glVertex3i(panelX + pad, rowY + boxH, 0);
			glEnd();

			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			drawText(panelX + pad + textXOffset, rowY + 1, items[idx].text);
		}

		glPopAttrib();
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
	}

	glPopAttrib();
}

bool MS_Model::key_pressed(int key)
{
	switch (key)
	{
	case 'g':
	case 'G':
		showMSModelLegend() = !showMSModelLegend();
		return true;
	case 'i':
	case 'I':
	case 'l':
	case 'L':
		showMSModelLabels() = !showMSModelLabels();
		return true;
	}

	return false;
}

} // namespace visualization
} // namespace bso

#endif // VIS_MS_BUILDING_HPP
