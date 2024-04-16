#ifndef CONFORMAL_MODEL_HPP
#define CONFORMAL_MODEL_HPP

#include <bso/spatial_design/cf_building.hpp>
#include <bso/visualization/models/model_base.hpp>
#include <bso/visualization/bsp/bsp.hpp>

#include <cstdlib>

namespace bso { namespace visualization
{

class Conformal_Model : public model_base
{
public:
		Conformal_Model(const bso::spatial_design::cf_building& cf,
										const std::string& type, const std::string& title);
		~Conformal_Model();
		void render(const camera &cam) const;
		const std::string get_description();

		bool key_pressed(int key);

protected:

private:
		std::string mTitle;
		std::list<polygon*> polygons;
		std::list<label*>   labels;

		polygon_props  pprops_rectangle;
		polygon_props  pprops_cuboid;
		polygon_props  pprops_cuboid_overlap;
		polygon_props  pprops_poly_marked;

		line_props     lprops;
		line_props	   lprops_boundary;
		line_props	   lprops_delaunay;
		label_props    lbprops;
		random_bsp     *pbsp;
};

Conformal_Model::Conformal_Model(const bso::spatial_design::cf_building& cf,
																 const std::string& type, const std::string& title)
{
	mTitle = title;
	pprops_rectangle.ambient = rgba(0.5f, 0.5f, 0.05f, 0.3f);
	pprops_rectangle.diffuse = rgba(1.0f, 1.0f, 0.1f, 0.3f);
	pprops_rectangle.specular = rgba(1.0f, 1.0f, 0.2f, 0.3f);
	pprops_rectangle.emission = rgba(0.0f, 0.0f, 0.0f, 0.0f);
	pprops_rectangle.shininess = 60.0;
	pprops_rectangle.translucent = true;
	pprops_rectangle.twosided = true;
	lprops_boundary.color = rgba(1.0f, 0.0f, 0.0f, 1.0f);
	lprops_boundary.width = 2.0f;

	pprops_cuboid.ambient = rgba(0.1f, 0.5f, 0.1f, 0.3f);
	pprops_cuboid.diffuse = rgba(0.2f, 1.0f, 0.2f, 0.3f);
	pprops_cuboid.specular = rgba(0.2f, 1.0f, 0.2f, 0.3f);
	pprops_cuboid.emission = rgba(0.0f, 0.0f, 0.0f, 0.0f);
	pprops_cuboid.shininess = 60.0;
	pprops_cuboid.translucent = true;
	pprops_cuboid.twosided = true;

	pprops_cuboid_overlap.ambient = rgba(1.0f, 0.1f, 0.1f, 1.0f);
	pprops_cuboid_overlap.diffuse = rgba(1.0f, 0.2f, 0.2f, 1.0f);
	pprops_cuboid_overlap.specular = rgba(1.0f, 0.2f, 0.2f, 1.0f);
	pprops_cuboid_overlap.emission = rgba(0.0f, 0.0f, 0.0f, 0.0f);
	pprops_cuboid_overlap.shininess = 60.0;
	pprops_cuboid_overlap.translucent = true;
	pprops_cuboid_overlap.twosided = true;

	double scale = 1.0; // cell scale factor; 0.8 for orthogonal BSDs
	namespace geom = bso::utilities::geometry;

	if(type == "space_at_ground" || type == "delaunay" || type == "boundarylines")
	{
		/*do not add the space labels*/
	}
	else
	{
		for (const auto& i : cf.cfSpaces())
		{
			this->addLabel(labels,&lbprops,std::to_string(i->getSpaceID()),i->getCenter());
		}
	}

	if (type == "line_segment")
	{
		for (const auto& i : cf.cfLines())
		{
			std::vector<geom::vertex> lineVertices;
			for (const auto& j : *i)
			{
				lineVertices.push_back(i->getCenter() + scale*(j - i->getCenter()));
			}
			geom::line_segment lineGeometry(lineVertices);
			this->addLineSegment(polygons,&lineGeometry,&pprops_rectangle,&lprops);
		}
	}
	else if (type == "rectangle")
	{
		for (const auto& i : cf.cfRectangles())
		{
			std::vector<geom::vertex> rectangleVertices;
			for (const auto& j : *i)
			{
				rectangleVertices.push_back(i->getCenter() + scale*(j - i->getCenter()));
			}
			geom::quadrilateral rectangleGeometry(rectangleVertices);
			if(i->markInVisualization == false)
			{
				this->addPolygon(polygons,&rectangleGeometry,&pprops_rectangle,&lprops,0.0);
			}
			else
			{
				this->addPolygon(polygons,&rectangleGeometry,&pprops_poly_marked,&lprops,0.0);
			}
		}
	}
	else if (type == "trirec") //combines both the triangles and rectangles defined in the geometry conformal model
	{
		for (const auto& i : cf.cfRectangles())
		{
			std::vector<geom::vertex> rectangleVertices;
			for (const auto& j : *i)
			{
				rectangleVertices.push_back(i->getCenter() + scale*(j - i->getCenter()));
			}
			geom::quadrilateral rectangleGeometry(rectangleVertices);
			if(i->markInVisualization == false)
			{
				this->addPolygon(polygons,&rectangleGeometry,&pprops_rectangle,&lprops,0.0);
			}
			else
			{
				this->addPolygon(polygons,&rectangleGeometry,&pprops_poly_marked,&lprops,0.0);
			}
		}
		for (const auto& i : cf.cfTriangles())
		{
			std::vector<geom::vertex> triangleVertices;
			for (const auto& j : *i)
			{
				triangleVertices.push_back(i->getCenter() + scale*(j - i->getCenter()));
			}
			geom::triangle triangleGeometry(triangleVertices);
			if(i->markInVisualization == false)
			{
				this->addPolygon(polygons,&triangleGeometry,&pprops_rectangle,&lprops,0.0);
			}
			else
			{
				this->addPolygon(polygons,&triangleGeometry,&pprops_poly_marked,&lprops,0.0);
			}
		}
	}
	else if (type == "triangle")
	{
		for (const auto& i : cf.cfTriangles())
		{
			std::vector<geom::vertex> triangleVertices;
			for (const auto& j : *i)
			{
				triangleVertices.push_back(i->getCenter() + scale*(j - i->getCenter()));
			}
			geom::triangle triangleGeometry(triangleVertices);
			if(i->markInVisualization == false)
			{
				this->addPolygon(polygons,&triangleGeometry,&pprops_rectangle,&lprops,0.0);
			}
			else
			{
				this->addPolygon(polygons,&triangleGeometry,&pprops_poly_marked,&lprops,0.0);
			}
		}
	}
	else if (type == "cuboid")
	{
		for (const auto& i : cf.cfCuboids())
		{
			std::vector<geom::vertex> cuboidVertices;
			for (const auto& j : *i)
			{
				cuboidVertices.push_back(i->getCenter() + scale*(j - i->getCenter()));
			}
			geom::quad_hexahedron cuboidGeometry(cuboidVertices);
			if (i->cfSpaces().size() > 1)
			{
				this->addPolyhedron(polygons,&cuboidGeometry,&pprops_cuboid_overlap,&lprops);
			}
			else
			{
				this->addPolyhedron(polygons,&cuboidGeometry,&pprops_cuboid,&lprops);
			}
		}
	}
	else if (type == "cuboid_intersec")
	{
		for (const auto& i : cf.cfCuboids())
		{		
			std::vector<geom::vertex> cuboidVertices;
			for (const auto& j : *i)
			{
				cuboidVertices.push_back(i->getCenter() + scale*(j - i->getCenter()));
			}
			geom::quad_hexahedron cuboidGeometry(cuboidVertices);
			
			// Check for intersections
			bool intersects = false;
			for(const auto j : cf.cfVertices())
			{
				bool cornerVertex = false;
				for(const auto k : (i->cfVertices()))
				{
					if(j->isSameAs(*k, cf.tolerance()))
					{
						cornerVertex = true;
						break;
					}
				}
				
				if(i->isInsideOrOn(*j, cf.tolerance()) && cornerVertex == false)
				{
					intersects = true;
					break;
				}
			}
			
			if (i->cfSpaces().size() > 1) // Give visual error if cell belongs to multiple spaces
			{
				if(i->markInVisualization == false)
				{
					this->addPolyhedron(polygons,&cuboidGeometry,&pprops_cuboid,&lprops);
				}
				else
				{
					this->addPolyhedron(polygons,&cuboidGeometry,&pprops_poly_marked,&lprops);
				}
			}
			if(intersects == true) // Give visual error if a design is not conformal
			{
				if(i->markInVisualization == false)
				{
					this->addPolyhedron(polygons,&cuboidGeometry,&pprops_cuboid,&lprops);
				}
				else
				{
					this->addPolyhedron(polygons,&cuboidGeometry,&pprops_poly_marked,&lprops);
				}
			}
			else // Give no visual error since none of the above is the case
			{
				if(i->markInVisualization == false)
				{
					this->addPolyhedron(polygons,&cuboidGeometry,&pprops_cuboid,&lprops);
				}
				else
				{
					this->addPolyhedron(polygons,&cuboidGeometry,&pprops_poly_marked,&lprops);
				}
			}
		}
	}
	else if(type == "triangular_prism_no_errors")
	{
		for (const auto& i : cf.cfTriPrism())
		{
			std::vector<geom::vertex> triPrismVertices;
			for (const auto& j : *i)
			{
				triPrismVertices.push_back(i->getCenter() + scale*(j - i->getCenter()));
			}
			geom::triangular_prism triPrismGeometry(triPrismVertices);
			this->addPolyhedron(polygons,&triPrismGeometry,&pprops_cuboid,&lprops);
		}
	}
	else if(type == "triangular_prism")
	{
		for (const auto& i : cf.cfTriPrism())
		{
			std::vector<geom::vertex> triPrismVertices;
			for (const auto& j : *i)
			{
				triPrismVertices.push_back(i->getCenter() + scale*(j - i->getCenter()));
			}
			geom::triangular_prism triPrismGeometry(triPrismVertices);
			// Check for intersections
			bool intersects = false;
			for(const auto j : cf.cfVertices())
			{
				bool cornerVertex = false;
				for(const auto k : (i->cfVertices()))
				{
					if(j->isSameAs(*k, cf.tolerance()))
					{
						cornerVertex = true;
						break;
					}
				}
				
				if(i->isInsideOrOn(*j, cf.tolerance()) && cornerVertex == false)
				{
					intersects = true;
					break;
				}
			}
			if (i->cfSpaces().size() > 1) // Give visual error if cell belongs to multiple spaces
			{
				this->addPolyhedron(polygons,&triPrismGeometry,&pprops_cuboid_overlap,&lprops);
			}
			if(intersects == true) // Give visual error if a design is not conformal
			{
				this->addPolyhedron(polygons,&triPrismGeometry,&pprops_cuboid_overlap,&lprops);	
			}
			else
			{
				this->addPolyhedron(polygons,&triPrismGeometry,&pprops_cuboid,&lprops);
			}
		}
	}
	else if(type == "delaunay")
	{
		for (const auto& i : cf.getDelaunayTri())
		{
			for(const auto& j: i.getLines())
			{
				bool noBoundaryLine = true;
				for(const auto k: cf.getDelauneyLines())
				{					
					if(k.isSameAs(j,cf.tolerance()))
					{
						noBoundaryLine = false;
					}
				}
				if(noBoundaryLine)
				{
					this->addLineSegment(polygons,&j,&pprops_rectangle,&lprops_delaunay);
				}
			}
		}
		
		for (const auto& i : cf.getDelauneyLines())
		{
			this->addLineSegment(polygons,&i,&pprops_rectangle,&lprops_boundary);
		}
	}
	else if(type == "boundarylines")
	{
		for (const auto& i : cf.getDelauneyLines())
		{
			this->addLineSegment(polygons,&i,&pprops_rectangle,&lprops_boundary);
		}
	}
	else if(type == "space_at_ground")
	{		
		for (const auto& i : cf.getSpacesAtGround())
		{			
			this->addPolyhedron(polygons,&i,&pprops_cuboid,&lprops);
		}
	}
	
	pbsp = new random_bsp(polygons);
}

Conformal_Model::~Conformal_Model()
{
		delete pbsp;

		for (std::list<polygon*>::iterator pit = polygons.begin();
				 pit != polygons.end(); pit++)
				delete *pit;

		for (std::list<label*>::iterator lbit = labels.begin();
				 lbit != labels.end(); lbit++)
				delete *lbit;
}

const std::string Conformal_Model::get_description()
{
		return mTitle;
}

void Conformal_Model::render(const camera &cam) const
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

bool Conformal_Model::key_pressed(int key)
{
		switch (key)
		{
		case 't':
		case 'T':
			//toggle geometry translucency
			pprops_rectangle.translucent = !pprops_rectangle.translucent;
			pprops_cuboid.translucent = !pprops_cuboid.translucent;

			return true;
		};
	return false;
}

} // namespace Visualisation
} // namespace BSO


#endif // CONFORMAL_MODEL_HPP
