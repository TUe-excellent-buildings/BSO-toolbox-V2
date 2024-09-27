#ifndef VIS_GEOMETRY_HPP
#define VIS_GEOMETRY_HPP

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

class GEOMETRY_Model : public model_base
{
private:
	std::string mTitle;
	std::list<polygon*> polygons;
	std::list<label*>   labels;

	polygon_props  pprops;
	polygon_props  ppropsSurfaceType; 

	line_props     lprops;
	line_props	   lprops_boundary;
	label_props    lbprops;
	random_bsp     *pbsp;
public:
	GEOMETRY_Model(const std::vector<bso::utilities::geometry::quad_hexahedron> QHs,
						const std::vector<bso::utilities::geometry::triangular_prism> triPrisms, 
						const std::vector<bso::utilities::geometry::quadrilateral> quads, 
						const std::vector<bso::utilities::geometry::triangle> triangles,
						const std::vector<bso::utilities::geometry::line_segment> lines,
						const std::string& title = "geometry_model",
						const double& linewidth = 1.0);

	~GEOMETRY_Model();
	
	void render(const camera &cam) const;
	const std::string get_description();
}; // GEOMETRY_Model

GEOMETRY_Model::GEOMETRY_Model(const std::vector<bso::utilities::geometry::quad_hexahedron> QHs,
						const std::vector<bso::utilities::geometry::triangular_prism> triPrisms, 
						const std::vector<bso::utilities::geometry::quadrilateral> quads, 
						const std::vector<bso::utilities::geometry::triangle> triangles,
						const std::vector<bso::utilities::geometry::line_segment> lines,
						const std::string& title /*= "geometry_model"*/,
						const double& linewidth /*= 1.0*/)
{
	lprops.width = linewidth;
	lprops_boundary.color = rgba(1.0f, 0.0f, 0.0f, 1.0f);
	lprops_boundary.width = 2.0f;
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
	
	for (const bso::utilities::geometry::quad_hexahedron& i : QHs)
	{
		this->addPolyhedron(polygons, &i, &pprops, &lprops);
	}
	for (const bso::utilities::geometry::triangular_prism& i : triPrisms)
	{
		this->addPolyhedron(polygons, &i, &pprops, &lprops);
	}
	for (const bso::utilities::geometry::quadrilateral& i : quads)
	{		
		this->addPolygon(polygons, &i, &ppropsSurfaceType, &lprops,0.0);
	}
	for (const bso::utilities::geometry::triangle& i : triangles)
	{		
		this->addPolygon(polygons, &i, &ppropsSurfaceType, &lprops,0.0);
	}
	for (const bso::utilities::geometry::line_segment& i : lines)
	{
		this->addLineSegment(polygons, &i, &ppropsSurfaceType, &lprops_boundary);
	}

	pbsp = new random_bsp(polygons);
} //

GEOMETRY_Model::~GEOMETRY_Model()
{
	delete pbsp;

	for (std::list<polygon*>::iterator pit = polygons.begin();
			 pit != polygons.end(); pit++) delete *pit;

	for (std::list<label*>::iterator lbit = labels.begin();
			 lbit != labels.end(); lbit++) delete *lbit;
}

const std::string GEOMETRY_Model::get_description()
{
	return mTitle;
}

void GEOMETRY_Model::render(const camera &cam) const
{
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	pbsp->render_btf(cam);

	for (auto lbit = labels.begin(); lbit != labels.end(); lbit++) (*lbit)->render();

	glPopAttrib();
}

} // namespace visualization
} // namespace bso

#endif // VIS_GEOMETRY_HPP
