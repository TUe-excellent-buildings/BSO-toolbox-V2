#ifndef VIS_DL_MODEL_HPP
#define VIS_DL_MODEL_HPP

#include <bso/daylight/dl_model.hpp>
#include <bso/visualization/models/model_base.hpp>
#include <bso/visualization/bsp/bsp.hpp>
#include <bso/utilities/geometry.hpp>

#include <cstdlib>
#include <cmath>
#include <algorithm>
// #include <memory>
// #include <vector>

namespace bso { namespace visualization
{

class DL_Model : public model_base
{
public:
	DL_Model(const bso::daylight::dl_model& dl, const std::string& type, const std::string title = "dl_model");
	~DL_Model();
	void render(const camera &cam) const;
	const std::string get_description();
	//bool key_pressed(int key);
	void addBoundaryLinesForSpace(bso::spatial_design::conformal::cf_space* cf_space);

protected:

private:
    std::string mTitle;
    std::list<polygon*> polygons;
	std::list<label*> labels;

    polygon_props pprops_window;
    polygon_props pprops_wall;
	polygon_props pprops_Space;

	polygon_props pprops_north;
	polygon_props pprops_east;
	polygon_props pprops_south;
	polygon_props pprops_west;

	label_props lbprops;

	line_props lprops_space_boundary;
	line_props lprops_rectangles;

	random_bsp *pbsp;

}; // class DL_MODEL


DL_Model::DL_Model(const bso::daylight::dl_model& dl, const std::string& type, const std::string title /*= "dl_model"*/)
{
	namespace geom = bso::utilities::geometry;
	mTitle = title;

	//polygon properties for windows
	pprops_window.ambient = rgba(0.3f, 0.5f, 0.7f, 0.3f);
	pprops_window.diffuse = rgba(0.2f, 0.6f, 0.8f, 0.3f);
	pprops_window.specular = rgba(0.4f, 0.7f, 0.8f, 0.3f);
	pprops_window.emission = rgba(0.0f, 0.0f, 0.0f, 0.3f);
    pprops_window.shininess = 60.0f;
    pprops_window.translucent = true;
    pprops_window.twosided = true;

	//polygon properties for walls/floors/ceilings
	pprops_wall.ambient = rgba(0.5f, 0.5f, 0.5f, 0.3f);
	pprops_wall.diffuse = rgba(0.6f, 0.6f, 0.6f, 0.3f);
	pprops_wall.specular = rgba(0.7f, 0.7f, 0.7f, 0.3f);
	pprops_wall.emission = rgba(0.0f, 0.0f, 0.0f, 0.3f);
    pprops_wall.shininess = 60.0f;
    pprops_wall.translucent = false;
    pprops_wall.twosided = true;

	//boundary lines
	lprops_space_boundary.width = 5.0f;
	lprops_rectangles.width = 0.1f;

	//polygon properties for nort (RED)
	pprops_north.ambient = rgba(1.0f, 0.0f, 0.0f, 1.0f);
    pprops_north.shininess = 60.0f;
    pprops_north.translucent = false;
    pprops_north.twosided = true;

	//polygon properties for east (yellow)
	pprops_east.ambient = rgba(1.0f, 1.0f, 0.0f, 1.0f);
    pprops_east.shininess = 60.0f;
    pprops_east.translucent = false;
    pprops_east.twosided = true;

	//polygon properties for south (black)
	pprops_south.ambient = rgba(0.0f, 0.0f, 0.0f, 1.0f);
    pprops_south.shininess = 60.0f;
    pprops_south.translucent = false;
    pprops_south.twosided = true;

	//polygon properties for west (blue)
	pprops_west.ambient = rgba(0.0f, 0.0f, 1.0f, 1.0f);
    pprops_west.shininess = 60.0f;
    pprops_west.translucent = false;
    pprops_west.twosided = true;


	if (type == "")
	{
		//loop trough the rectangles in the daylight model
		for (const auto& map : dl.getWindowProperties()) //std::vector<std::map<bso::spatial_design::conformal::cf_rectangle*, bool>> mRectangleWindowYN;
		{
			for (const auto& [cfRec,isWindow] : map) //for loop uses structure binding (C++17 feuature) to unpack each key-value from the map
			{

				std::vector<geom::vertex> rectangleVertices;
				for (const auto& vertex : *cfRec) 
				{
					rectangleVertices.push_back(cfRec->getCenter() + vertex - cfRec->getCenter());
				}

				geom::quadrilateral rectangleGeometry(rectangleVertices);

				if (isWindow==true) 
				{
					this->addPolygon(polygons, &rectangleGeometry, &pprops_window, &lprops_rectangles, 0.0);
				}
				else if (isWindow==false)
				{
					this->addPolygon(polygons, &rectangleGeometry, &pprops_wall, &lprops_rectangles, 0.0);
				}

			} //map [rect,isWindow] loop
		} //getWindowProperties()
		for (const auto& i : dl.getTriangles()) //std::vector<bso::spatial_design::conformal::cf_triangle*> mTriangles;
		{
			std::vector<geom::vertex> triangleVertices;
			for (const auto& vertex : *i) 
			{
				triangleVertices.push_back(i->getCenter() + vertex - i->getCenter());
			}

			geom::triangle triangleGeometry(triangleVertices);

			this->addPolygon(polygons, &triangleGeometry, &pprops_wall, &lprops_rectangles, 0.0);
		} //getTriangles()

		//boundary lines per space
		pprops_Space.ambient = rgba(1.0f, 1.0f, 0.04f, 0.0f);
		pprops_Space.diffuse = rgba(1.0f, 1.0f, 0.04f, 0.0f);
		pprops_Space.specular = rgba(0.04f, 0.04f, 0.04f, 0.0f);
		pprops_Space.emission = rgba(0.04f, 0.04f, 0.04f, 0.0f);
		for (const auto& i : dl.getSpacePropertiesQH())
		{
			this->addLabel(labels,&lbprops,std::to_string(i->getSpaceID()),i->getCenter()); // space numbers
			this->addBoundaryLinesForSpace(i); //lines
		}
	}
		
	pbsp = new random_bsp(polygons); 

} //ctor DL_MODEL()

DL_Model::~DL_Model()
{
	delete pbsp;

	for (std::list<polygon*>::iterator pit = polygons.begin();
				pit != polygons.end(); pit++)
			delete *pit;

	for (std::list<label*>::iterator lbit = labels.begin();
				lbit != labels.end(); lbit++)
			delete *lbit;

} //dctor DL_MODEL()

void DL_Model::render(const camera &cam) const
{
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	pbsp->render_btf(cam);
	std::list<label*>::const_iterator lbit;
	for (lbit = labels.begin(); lbit != labels.end(); lbit++)
	{
		(*lbit)->render();
	}
	glPopAttrib();
}

const std::string DL_Model::get_description() {
    return mTitle;
}

// bool DL_Model::key_pressed(int key)
// {
// 		switch (key)
// 		{
// 		case 't':
// 		case 'T':
// 			//toggle geometry translucency
// 			pprops_window.translucent = !pprops_window.translucent;
// 			pprops_wall.translucent = !pprops_wall.translucent;
// 			return true;
// 		};
// 	return false;
// }//

void DL_Model::addBoundaryLinesForSpace(bso::spatial_design::conformal::cf_space* cf_space)
{
	std::vector<bso::utilities::geometry::line_segment> cf_line_segments;
	cf_line_segments  = cf_space->getLines();

    for (const auto& line_segment : cf_line_segments)
    {
		this->addLineSegment(polygons, &line_segment, &pprops_Space, &lprops_space_boundary);
    }
}

} // namespace Visualisation
} // namespace BSO


#endif // VIS_DL_MODEL_HPP
