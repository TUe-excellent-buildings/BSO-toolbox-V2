#ifndef VIS_GEOMETRY_HPP
#define VIS_GEOMETRY_HPP

#include <bso/visualization/models/model_base.hpp>
#include <bso/visualization/bsp/bsp.hpp>

#include <boost/algorithm/string.hpp>
#include <utility>
#include <type_traits>

#ifdef SD_MODEL_HPP
#include <bso/structural_design/sd_model.hpp>
#endif // SD_MODEL_HPP

#ifdef BP_MODEL_HPP
#include <bso/building_physics/bp_model.hpp>
#endif // BP_MODEL_HPP

#include <bitset>

namespace bso { namespace visualization
{

// GEOMETRY_MixedLayer groups geometry primitives that should be rendered as one visual layer.
// The layer style is generic:
// - StyleT = rgba          -> simple color-driven styling
// - StyleT = polygon_props -> full material/shading styling
// Either style can be used with its appropriate alias: GEOMETRY_RgbaLayer or GEOMETRY_PolygonPropsLayer defined below.
//
// lineWidth controls rendering width for line primitives in this layer.
// If lineWidth <= 0, the model/default linewidth is used.
//
// Geometry is split by primitive type, so one layer can mix volumes, surfaces, and edges.
template <typename StyleT>
struct GEOMETRY_MixedLayer
{
	StyleT style;
	double lineWidth = -1.0;
	std::vector<bso::utilities::geometry::quad_hexahedron> QHs;
	std::vector<bso::utilities::geometry::triangular_prism> triPrisms;
	std::vector<bso::utilities::geometry::quadrilateral> quads;
	std::vector<bso::utilities::geometry::triangle> triangles;
	std::vector<bso::utilities::geometry::line_segment> lines;
};

// Convenience aliases for the two supported style modes.
using GEOMETRY_RgbaLayer = GEOMETRY_MixedLayer<rgba>;
using GEOMETRY_PolygonPropsLayer = GEOMETRY_MixedLayer<polygon_props>;

// A utility class to visualize any geometry objects when required,
// with any style (color, transparency, etc.) and line width. 
//The geometry objects are organized in layers, each layer can have its own style and line width. The layers are rendered in the order they are provided to the constructor.
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

	std::list<polygon_props> mLayerPolygonProps;
	std::list<line_props> mLayerLineProps;
	random_bsp     *pbsp;
public:
// Basic constructors for visualization of geometry without choice of color or line width, i.e., using default values.	
GEOMETRY_Model(const std::vector<bso::utilities::geometry::quad_hexahedron> QHs,
						const std::vector<bso::utilities::geometry::triangular_prism> triPrisms, 
						const std::vector<bso::utilities::geometry::quadrilateral> quads, 
						const std::vector<bso::utilities::geometry::triangle> triangles,
						const std::vector<bso::utilities::geometry::line_segment> lines,
						const std::string& title = "geometry_model",
						const double& linewidth = 1.0);

	GEOMETRY_Model(const std::vector<bso::utilities::geometry::triangular_prism> triPrisms, 
                        const std::string& title = "geometry_model",
                        const double& linewidth = 1.0);

	// Constructors utilizing the GEOMETRY_MixedLayer struct to provide a more flexible way to specify geometry layers with different styles and line widths for visualizations.
	template <typename GeometryT>
	GEOMETRY_Model(const std::vector<std::pair<rgba, std::vector<GeometryT>>>& layers,
						const std::string& title = "geometry_model",
						const double& linewidth = 1.0);

	template <typename StyleT>
	GEOMETRY_Model(const std::vector<GEOMETRY_MixedLayer<StyleT>>& layers,
						const std::string& title = "geometry_model",
						const double& linewidth = 1.0);

	template <typename GeometryT>
	GEOMETRY_Model(const std::vector<std::pair<polygon_props, std::vector<GeometryT>>>& layers,
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

GEOMETRY_Model::GEOMETRY_Model(const std::vector<bso::utilities::geometry::triangular_prism> triPrisms, 
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

    for (const bso::utilities::geometry::triangular_prism& i : triPrisms)
    {
        this->addPolyhedron(polygons, &i, &pprops, &lprops);
    }

    pbsp = new random_bsp(polygons);
} //

template <typename GeometryT>
GEOMETRY_Model::GEOMETRY_Model(const std::vector<std::pair<rgba, std::vector<GeometryT>>>& layers,
						const std::string& title /*= "geometry_model"*/,
						const double& linewidth /*= 1.0*/)
	: GEOMETRY_Model([&layers]()
	{
		std::vector<GEOMETRY_RgbaLayer> mixedLayers;
		mixedLayers.reserve(layers.size());
		for (const auto& layer : layers)
		{
			GEOMETRY_RgbaLayer mixedLayer;
			mixedLayer.style = layer.first;
			if constexpr (std::is_same<GeometryT, bso::utilities::geometry::quad_hexahedron>::value)
			{
				mixedLayer.QHs = layer.second;
			}
			else if constexpr (std::is_same<GeometryT, bso::utilities::geometry::triangular_prism>::value)
			{
				mixedLayer.triPrisms = layer.second;
			}
			else if constexpr (std::is_same<GeometryT, bso::utilities::geometry::quadrilateral>::value)
			{
				mixedLayer.quads = layer.second;
			}
			else if constexpr (std::is_same<GeometryT, bso::utilities::geometry::triangle>::value)
			{
				mixedLayer.triangles = layer.second;
			}
			else if constexpr (std::is_same<GeometryT, bso::utilities::geometry::line_segment>::value)
			{
				mixedLayer.lines = layer.second;
			}
			else
			{
				static_assert(std::is_same<GeometryT, void>::value,
					"GEOMETRY_Model template constructor received unsupported geometry type.");
			}
			mixedLayers.push_back(std::move(mixedLayer));
		}
		return mixedLayers;
	}(), title, linewidth)
{
}

template <typename StyleT>
GEOMETRY_Model::GEOMETRY_Model(const std::vector<GEOMETRY_MixedLayer<StyleT>>& layers,
						const std::string& title /*= "geometry_model"*/,
						const double& linewidth /*= 1.0*/)
{
	lprops.width = linewidth;
	lprops_boundary.color = rgba(1.0f, 0.0f, 0.0f, 1.0f);
	lprops_boundary.width = 2.0f;
	mTitle = title;

	for (const auto& layer : layers)
	{
		if constexpr (std::is_same<StyleT, rgba>::value)
		{
			polygon_props p;
			p.ambient = layer.style;
			p.diffuse = layer.style;
			p.specular = layer.style;
			p.emission = rgba(0.0f, 0.0f, 0.0f, 0.0f);
			p.shininess = 60.0f;
			p.translucent = (layer.style.a < 1.0f);
			p.twosided = true;
			mLayerPolygonProps.push_back(p);
		}
		else if constexpr (std::is_same<StyleT, polygon_props>::value)
		{
			mLayerPolygonProps.push_back(layer.style);
		}
		else
		{
			static_assert(std::is_same<StyleT, void>::value,
				"GEOMETRY_Model mixed-layer constructor supports only rgba or polygon_props styles.");
		}

		polygon_props* p = &mLayerPolygonProps.back();

		for (const auto& g : layer.QHs) this->addPolyhedron(polygons, &g, p, &lprops);
		for (const auto& g : layer.triPrisms) this->addPolyhedron(polygons, &g, p, &lprops);
		for (const auto& g : layer.quads) this->addPolygon(polygons, &g, p, &lprops, 0.0);
		for (const auto& g : layer.triangles) this->addPolygon(polygons, &g, p, &lprops, 0.0);

		if (!layer.lines.empty())
		{
			mLayerLineProps.push_back(lprops_boundary);
			line_props* lp = &mLayerLineProps.back();
			if constexpr (std::is_same<StyleT, rgba>::value)
			{
				lp->color = layer.style;
			}
			else
			{
				lp->color = p->diffuse;
			}
			lp->width = (layer.lineWidth > 0.0 ? layer.lineWidth : linewidth);
			for (const auto& g : layer.lines) this->addLineSegment(polygons, &g, p, lp);
		}
	}

	pbsp = new random_bsp(polygons);
}

template <typename GeometryT>
GEOMETRY_Model::GEOMETRY_Model(const std::vector<std::pair<polygon_props, std::vector<GeometryT>>>& layers,
						const std::string& title /*= "geometry_model"*/,
						const double& linewidth /*= 1.0*/)
{
	lprops.width = linewidth;
	lprops_boundary.color = rgba(1.0f, 0.0f, 0.0f, 1.0f);
	lprops_boundary.width = 2.0f;
	mTitle = title;

	for (const auto& layer : layers)
	{
		mLayerPolygonProps.push_back(layer.first);
		polygon_props* p = &mLayerPolygonProps.back();
		if constexpr (std::is_same<GeometryT, bso::utilities::geometry::quad_hexahedron>::value ||
						  std::is_same<GeometryT, bso::utilities::geometry::triangular_prism>::value)
		{
			for (const auto& g : layer.second) this->addPolyhedron(polygons, &g, p, &lprops);
		}
		else if constexpr (std::is_same<GeometryT, bso::utilities::geometry::quadrilateral>::value ||
							   std::is_same<GeometryT, bso::utilities::geometry::triangle>::value)
		{
			for (const auto& g : layer.second) this->addPolygon(polygons, &g, p, &lprops, 0.0);
		}
		else if constexpr (std::is_same<GeometryT, bso::utilities::geometry::line_segment>::value)
		{
			mLayerLineProps.push_back(lprops_boundary);
			line_props* lp = &mLayerLineProps.back();
			lp->color = p->diffuse;
			lp->width = linewidth;
			for (const auto& g : layer.second) this->addLineSegment(polygons, &g, p, lp);
		}
		else
		{
			static_assert(std::is_same<GeometryT, void>::value,
				"GEOMETRY_Model template constructor received unsupported geometry type.");
		}
	}

	pbsp = new random_bsp(polygons);
}

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
