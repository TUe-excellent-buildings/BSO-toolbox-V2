#ifndef VIS_DL_MODEL_HPP
#define VIS_DL_MODEL_HPP

#ifdef DL_MODEL_HPP
#include <bso/daylight/dl_model.hpp>
#endif // DL_MODEL_HPP

#include <bso/visualization/models/model_base.hpp>
#include <bso/visualization/bsp/bsp.hpp>
#include <bso/utilities/geometry.hpp>

#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <map>
#include <regex>
#include <sstream>
#include <vector>

namespace bso { namespace visualization
{

class DL_Model : public model_base
{
public:
#ifdef DL_MODEL_HPP
	DL_Model(const bso::daylight::dl_model& dl, const std::string& type, const std::string title = "dl_model", bool writeToFile = false, std::string fileName = "");
	void addBoundaryLinesForSpace(bso::spatial_design::conformal::cf_space* cf_space);
#endif // DL_MODEL_HPP
	DL_Model(const std::string& fileName);
	~DL_Model();
	void render(const camera &cam) const;
	const std::string get_description();
	void writeToFile(const std::string& filename) const;

protected:

private:
    std::string mTitle;
	std::string mType; 
    std::list<polygon*> polygons;
	std::list<label*> labels;

	// Legend related members for DF visualization
	struct legend_bin
	{
		rgba color;
		double lower;
		double upper;
	};
	bool mDrawOverlayLegend = false;
	std::string mLegendTitle;
	std::vector<legend_bin> mLegendBins;
	std::vector<std::string> mLegendNotes;

    polygon_props pprops_window;
    polygon_props pprops_wall;
	polygon_props pprops_wall_translucent;
	polygon_props pprops_gridpoint;
	polygon_props pprops_Space;
	std::vector<polygon_props> pprops_color_gradient;

	label_props lbprops;

	line_props lprops_space_boundary;
	line_props lprops_rectangles;

	random_bsp *pbsp = nullptr;

}; // class DL_MODEL

namespace detail
{
	inline std::string trim_copy(const std::string& text)
	{
		const auto first = text.find_first_not_of(" \t\r\n");
		if (first == std::string::npos) return "";
		const auto last = text.find_last_not_of(" \t\r\n");
		return text.substr(first, last - first + 1);
	}

	inline std::vector<std::string> split_csv(const std::string& line)
	{
		std::vector<std::string> tokens;
		std::stringstream ss(line);
		std::string token;
		while (std::getline(ss, token, ','))
		{
			tokens.push_back(trim_copy(token));
		}
		return tokens;
	}

	inline std::vector<double> extract_numbers(const std::string& line)
	{
		std::vector<double> values;
		const std::regex numberRegex(R"([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?)");
		for (std::sregex_iterator it(line.begin(), line.end(), numberRegex), end; it != end; ++it)
		{
			values.push_back(std::stod(it->str()));
		}
		return values;
	}

	inline std::vector<bool> extract_bools(const std::string& line)
	{
		std::vector<bool> values;
		const std::regex boolRegex(R"((true|false))", std::regex_constants::icase);
		for (std::sregex_iterator it(line.begin(), line.end(), boolRegex), end; it != end; ++it)
		{
			std::string value = trim_copy(it->str());
			std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
			values.push_back(value == "true");
		}
		return values;
	}

	inline void assign_polygon_props(polygon_props& props, const std::string& line)
	{
		const auto values = extract_numbers(line);
		const auto bools = extract_bools(line);
		if (values.size() < 17)
		{
			throw std::invalid_argument("Invalid polygon properties line in DL_Model visualization file.");
		}

		props.ambient = rgba(values[0], values[1], values[2], values[3]);
		props.diffuse = rgba(values[4], values[5], values[6], values[7]);
		props.specular = rgba(values[8], values[9], values[10], values[11]);
		props.emission = rgba(values[12], values[13], values[14], values[15]);
		props.shininess = static_cast<float>(values[16]);

		// Support both textual booleans (true/false) and numeric booleans (0/1).
		if (bools.size() >= 2)
		{
			props.translucent = bools[0];
			props.twosided = bools[1];
		}
		else if (values.size() >= 19)
		{
			props.translucent = (values[17] != 0.0);
			props.twosided = (values[18] != 0.0);
		}
		else
		{
			throw std::invalid_argument("Invalid polygon properties line in DL_Model visualization file.");
		}
	}

	inline void assign_line_props(line_props& props, const std::string& line)
	{
		const auto values = extract_numbers(line);
		if (values.size() < 7)
		{
			throw std::invalid_argument("Invalid line properties line in DL_Model visualization file.");
		}

		props.color = rgba(values[0], values[1], values[2], values[3]);
		props.width = static_cast<float>(values[4]);
		props.pattern = static_cast<short>(values[5]);
		props.factor = static_cast<int>(values[6]);
	}

	inline void assign_label_props(label_props& props, const std::string& line)
	{
		const auto values = extract_numbers(line);
		if (values.size() < 4)
		{
			throw std::invalid_argument("Invalid label properties line in DL_Model visualization file.");
		}

		props.textcolor = rgba(values[0], values[1], values[2], values[3]);
		props.glutbmpfont = GLUT_BITMAP_8_BY_13;
	}

	inline std::string parse_label_text(const std::string& line)
	{
		const std::string marker = "LabelText,";
		const std::string positionMarker = ",Position,";
		const auto start = line.find(marker);
		const auto end = line.find(positionMarker);
		if (start == std::string::npos || end == std::string::npos || end <= start + marker.size())
		{
			throw std::invalid_argument("Invalid label line in DL_Model visualization file.");
		}
		return trim_copy(line.substr(start + marker.size(), end - (start + marker.size())));
	}

	inline vertex parse_label_position(const std::string& line)
	{
		const std::string positionMarker = ",Position,";
		const auto start = line.find(positionMarker);
		if (start == std::string::npos)
		{
			throw std::invalid_argument("Invalid label position line in DL_Model visualization file.");
		}
		const auto values = extract_numbers(line.substr(start + positionMarker.size()));
		if (values.size() < 3)
		{
			throw std::invalid_argument("Incomplete label position in DL_Model visualization file.");
		}
		return vertex(values[0], values[1], values[2]);
	}

	inline std::string get_glut_font_name(void* font)
	{
		if (font == GLUT_BITMAP_8_BY_13) return "GLUT_BITMAP_8_BY_13";
		if (font == GLUT_BITMAP_9_BY_15) return "GLUT_BITMAP_9_BY_15";
		if (font == GLUT_BITMAP_TIMES_ROMAN_10) return "GLUT_BITMAP_TIMES_ROMAN_10";
		if (font == GLUT_BITMAP_TIMES_ROMAN_24) return "GLUT_BITMAP_TIMES_ROMAN_24";
		if (font == GLUT_BITMAP_HELVETICA_10) return "GLUT_BITMAP_HELVETICA_10";
		if (font == GLUT_BITMAP_HELVETICA_12) return "GLUT_BITMAP_HELVETICA_12";
		if (font == GLUT_BITMAP_HELVETICA_18) return "GLUT_BITMAP_HELVETICA_18";
		return "GLUT_BITMAP_8_BY_13";
	}

}

#ifdef DL_MODEL_HPP
DL_Model::DL_Model(const bso::daylight::dl_model& dl, 
	const std::string& type /*= ""*/, const std::string title /*= "dl_model"*/, 
	bool writeToFile /*= false*/, std::string fileName /*= ""*/)
{
	namespace geom = bso::utilities::geometry;
	mTitle = title;
	mType = type;

	//polygon properties for windows
	pprops_window.ambient = rgba(0.3f, 0.5f, 0.7f, 0.15f);
	pprops_window.diffuse = rgba(0.2f, 0.6f, 0.8f, 0.15f);
	pprops_window.specular = rgba(0.4f, 0.7f, 0.8f, 0.15f);
	pprops_window.emission = rgba(0.0f, 0.0f, 0.0f, 0.15f);
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

	//polygon properties for walls/floors/ceilings to show the grid points so walls are translucent
	pprops_wall_translucent.ambient = rgba(0.5f, 0.5f, 0.5f, 0.15f);
	pprops_wall_translucent.diffuse = rgba(0.6f, 0.6f, 0.6f, 0.15f);
	pprops_wall_translucent.specular = rgba(0.7f, 0.7f, 0.7f, 0.15f);
	pprops_wall_translucent.emission = rgba(0.0f, 0.0f, 0.0f, 0.15f);
    pprops_wall_translucent.shininess = 60.0f;
    pprops_wall_translucent.translucent = true;
    pprops_wall_translucent.twosided = true;

	//boundary lines
	lprops_space_boundary.width = 5.0f;
	lprops_rectangles.width = 0.1f;

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
					this->addPolygon(polygons, &rectangleGeometry, &pprops_window, &lprops_rectangles, 0.0, "pprops_window", "lprops_rectangles");
				}
				else if (isWindow==false)
				{
					this->addPolygon(polygons, &rectangleGeometry, &pprops_wall, &lprops_rectangles, 0.0, "pprops_wall", "lprops_rectangles");
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

			this->addPolygon(polygons, &triangleGeometry, &pprops_wall, &lprops_rectangles, 0.0, "pprops_wall", "lprops_rectangles");
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
	else if(type == "grid")
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
					this->addPolygon(polygons, &rectangleGeometry, &pprops_window, &lprops_rectangles, 0.0, "pprops_window", "lprops_rectangles");
				}
				else if (isWindow==false)
				{
					this->addPolygon(polygons, &rectangleGeometry, &pprops_wall_translucent, &lprops_rectangles, 0.0, "pprops_wall_translucent", "lprops_rectangles");
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

			this->addPolygon(polygons, &triangleGeometry, &pprops_wall_translucent, &lprops_rectangles, 0.0, "pprops_wall_translucent", "lprops_rectangles");
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
		
		// add grid points as cubes
		pprops_gridpoint.ambient = rgba(1.0f, 0.0f, 0.0f, 1.0f);  // Red
		pprops_gridpoint.diffuse = rgba(1.0f, 0.2f, 0.2f, 1.0f);
		pprops_gridpoint.specular = rgba(1.0f, 0.4f, 0.4f, 1.0f);
		pprops_gridpoint.shininess = 60.0f;
		pprops_gridpoint.translucent = false;
		pprops_gridpoint.twosided = true;
		
		for (const auto& gridPoint : dl.getGridPoints())
		{
			this->addPoint(polygons, gridPoint, &pprops_gridpoint, &lprops_rectangles, 100.0, "pprops_gridpoint", "lprops_rectangles"); // 100mm cube size
		}
	}
	else if(type == "DF")
	{
		mDrawOverlayLegend = true;
		mLegendTitle = "Daylight factor legend [%]";
		mLegendBins.clear();
		mLegendNotes.clear();

		// Provide a translucent vieuw of the building for reference
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
					this->addPolygon(polygons, &rectangleGeometry, &pprops_window, &lprops_rectangles, 0.0, "pprops_window", "lprops_rectangles");
				}
				else if (isWindow==false)
				{
					this->addPolygon(polygons, &rectangleGeometry, &pprops_wall_translucent, &lprops_rectangles, 0.0, "pprops_wall_translucent", "lprops_rectangles");
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

			this->addPolygon(polygons, &triangleGeometry, &pprops_wall_translucent, &lprops_rectangles, 0.0, "pprops_wall_translucent", "lprops_rectangles");
		} //getTriangles()

		// Ad the DF results as colored cubes at the grid points
		const auto& gridPointResults = dl.getGridPointResults();
		pprops_color_gradient.clear();
		pprops_color_gradient.reserve(gridPointResults.size()); // reserve memory so addition to this vector will not invalidate the references to the polygon_props objects

		// Determine the min and max DF values for normalization
		double minDF = 0.0;
		double maxDF = 0.0;
		if (!gridPointResults.empty())
		{
			minDF = gridPointResults.front().mDF;
			maxDF = gridPointResults.front().mDF;
			for (const auto& gridPointResult : gridPointResults)
			{
				minDF = std::min(minDF, gridPointResult.mDF);
				maxDF = std::max(maxDF, gridPointResult.mDF);
			}
		}

		// helper function to map normalized DF values to colors (blue -> green -> yellow)
		auto dfColor = [](double t) -> rgba
		{
			t = std::clamp(t, 0.0, 1.0);
			if (t <= (1.0 / 3.0))
			{
				double u = t / (1.0 / 3.0);
				return rgba(0.0f, static_cast<float>(u), 1.0f, 1.0f);
			}
			if (t <= (2.0 / 3.0))
			{
				double u = (t - (1.0 / 3.0)) / (1.0 / 3.0);
				return rgba(0.0f, 1.0f, static_cast<float>(1.0 - u), 1.0f);
			}
			double u = (t - (2.0 / 3.0)) / (1.0 / 3.0);
			return rgba(static_cast<float>(u), 1.0f, 0.0f, 1.0f);
		};

		for (const auto& gridPointResult : gridPointResults)
		{
			double normalizedDF = 0.0;
			if (maxDF > minDF)
			{
				normalizedDF = (gridPointResult.mDF - minDF) / (maxDF - minDF);
			}

			polygon_props dfProps = pprops_gridpoint;
			dfProps.ambient = dfColor(normalizedDF);
			dfProps.diffuse = dfProps.ambient;
			dfProps.specular = rgba(0.1f, 0.1f, 0.1f, 1.0f);
			dfProps.emission = rgba(0.0f, 0.0f, 0.0f, 1.0f);
			dfProps.shininess = 20.0f;
			dfProps.translucent = false;
			dfProps.twosided = false;
			pprops_color_gradient.push_back(dfProps);

			this->addPoint(polygons, gridPointResult.mGridPointLocation, &(pprops_color_gradient.back()), &lprops_rectangles, 100.0, "", "lprops_rectangles");
		}

		// Build overlay legend with 12 categories using DF value bins.
		const unsigned int nLegendBins = 12;
		if (!gridPointResults.empty())
		{
			double span = maxDF - minDF;
			for (unsigned int i = 0; i < nLegendBins; ++i)
			{
				double lower = minDF;
				double upper = maxDF;
				if (span > 0.0)
				{
					lower = minDF + (static_cast<double>(i) / static_cast<double>(nLegendBins)) * span;
					upper = minDF + (static_cast<double>(i + 1) / static_cast<double>(nLegendBins)) * span;
				}

				double tMid = (static_cast<double>(i) + 0.5) / static_cast<double>(nLegendBins);
				legend_bin bin;
				bin.color = dfColor(tMid);
				bin.lower = lower;
				bin.upper = upper;
				mLegendBins.push_back(bin);
			}
		}
	}
	else if(type == "Horizontal_Illuminance")
	{
		mDrawOverlayLegend = true;
		mLegendTitle = "Horizontal illuminance legend [lx]";
		mLegendBins.clear();
		mLegendNotes.clear();

		// Provide a translucent view of the building for reference
		for (const auto& map : dl.getWindowProperties())
		{
			for (const auto& [cfRec,isWindow] : map)
			{
				std::vector<geom::vertex> rectangleVertices;
				for (const auto& vertex : *cfRec)
				{
					rectangleVertices.push_back(cfRec->getCenter() + vertex - cfRec->getCenter());
				}

				geom::quadrilateral rectangleGeometry(rectangleVertices);

				if (isWindow==true)
				{
					this->addPolygon(polygons, &rectangleGeometry, &pprops_window, &lprops_rectangles, 0.0, "pprops_window", "lprops_rectangles");
				}
				else if (isWindow==false)
				{
					this->addPolygon(polygons, &rectangleGeometry, &pprops_wall_translucent, &lprops_rectangles, 0.0, "pprops_wall_translucent", "lprops_rectangles");
				}
			}
		}
		for (const auto& i : dl.getTriangles())
		{
			std::vector<geom::vertex> triangleVertices;
			for (const auto& vertex : *i)
			{
				triangleVertices.push_back(i->getCenter() + vertex - i->getCenter());
			}

			geom::triangle triangleGeometry(triangleVertices);
			this->addPolygon(polygons, &triangleGeometry, &pprops_wall_translucent, &lprops_rectangles, 0.0, "pprops_wall_translucent", "lprops_rectangles");
		}

		// Add the horizontal illuminance results as colored cubes at the grid points
		const auto& gridPointResults = dl.getGridPointResults();
		pprops_color_gradient.clear();
		pprops_color_gradient.reserve(gridPointResults.size());

		double minEh = 0.0;
		double maxEh = 0.0;
		if (!gridPointResults.empty())
		{
			minEh = gridPointResults.front().horizontal_illuminance;
			maxEh = gridPointResults.front().horizontal_illuminance;
			for (const auto& gridPointResult : gridPointResults)
			{
				minEh = std::min(minEh, gridPointResult.horizontal_illuminance);
				maxEh = std::max(maxEh, gridPointResult.horizontal_illuminance);
			}
		}

		// helper function to map normalized values to colors (blue -> green -> yellow)
		auto illuminanceColor = [](double t) -> rgba
		{
			t = std::clamp(t, 0.0, 1.0);
			if (t <= (1.0 / 3.0))
			{
				double u = t / (1.0 / 3.0);
				return rgba(0.0f, static_cast<float>(u), 1.0f, 1.0f);
			}
			if (t <= (2.0 / 3.0))
			{
				double u = (t - (1.0 / 3.0)) / (1.0 / 3.0);
				return rgba(0.0f, 1.0f, static_cast<float>(1.0 - u), 1.0f);
			}
			double u = (t - (2.0 / 3.0)) / (1.0 / 3.0);
			return rgba(static_cast<float>(u), 1.0f, 0.0f, 1.0f);
		};

		for (const auto& gridPointResult : gridPointResults)
		{
			double normalizedEh = 0.0;
			if (maxEh > minEh)
			{
				normalizedEh = (gridPointResult.horizontal_illuminance - minEh) / (maxEh - minEh);
			}

			polygon_props illumProps = pprops_gridpoint;
			illumProps.ambient = illuminanceColor(normalizedEh);
			illumProps.diffuse = illumProps.ambient;
			illumProps.specular = rgba(0.1f, 0.1f, 0.1f, 1.0f);
			illumProps.emission = rgba(0.0f, 0.0f, 0.0f, 1.0f);
			illumProps.shininess = 20.0f;
			illumProps.translucent = false;
			illumProps.twosided = false;
			pprops_color_gradient.push_back(illumProps);

			this->addPoint(polygons, gridPointResult.mGridPointLocation, &(pprops_color_gradient.back()), &lprops_rectangles, 100.0, "", "lprops_rectangles");
		}

		// Build overlay legend with 12 categories using horizontal illuminance bins.
		const unsigned int nLegendBins = 12;
		if (!gridPointResults.empty())
		{
			double span = maxEh - minEh;
			for (unsigned int i = 0; i < nLegendBins; ++i)
			{
				double lower = minEh;
				double upper = maxEh;
				if (span > 0.0)
				{
					lower = minEh + (static_cast<double>(i) / static_cast<double>(nLegendBins)) * span;
					upper = minEh + (static_cast<double>(i + 1) / static_cast<double>(nLegendBins)) * span;
				}

				double tMid = (static_cast<double>(i) + 0.5) / static_cast<double>(nLegendBins);
				legend_bin bin;
				bin.color = illuminanceColor(tMid);
				bin.lower = lower;
				bin.upper = upper;
				mLegendBins.push_back(bin);
			}
		}
	}
	else if(type == "NEN")
	{
		mDrawOverlayLegend = true;
		mLegendTitle = "NEN score legend [-]";
		mLegendBins.clear();
		mLegendNotes.clear();
		mLegendNotes.push_back("score = 0: both Ptarget and Pmintarget are met");
		mLegendNotes.push_back("score = 0-5: only minTarget is met");
		mLegendNotes.push_back("score = 5-10: neither targets are met");
		pprops_color_gradient.clear();
		pprops_color_gradient.reserve(dl.getSpacePropertiesQH().size());

		// Provide a translucent view of the building for reference
		for (const auto& map : dl.getWindowProperties())
		{
			for (const auto& [cfRec,isWindow] : map)
			{
				std::vector<geom::vertex> rectangleVertices;
				for (const auto& vertex : *cfRec)
				{
					rectangleVertices.push_back(cfRec->getCenter() + vertex - cfRec->getCenter());
				}

				geom::quadrilateral rectangleGeometry(rectangleVertices);

				if (isWindow==true)
				{
					this->addPolygon(polygons, &rectangleGeometry, &pprops_window, &lprops_rectangles, 0.0, "pprops_window", "lprops_rectangles");
				}
				else if (isWindow==false)
				{
					this->addPolygon(polygons, &rectangleGeometry, &pprops_wall_translucent, &lprops_rectangles, 0.0, "pprops_wall_translucent", "lprops_rectangles");
				}
			}
		}
		for (const auto& i : dl.getTriangles())
		{
			std::vector<geom::vertex> triangleVertices;
			for (const auto& vertex : *i)
			{
				triangleVertices.push_back(i->getCenter() + vertex - i->getCenter());
			}

			geom::triangle triangleGeometry(triangleVertices);
			this->addPolygon(polygons, &triangleGeometry, &pprops_wall_translucent, &lprops_rectangles, 0.0, "pprops_wall_translucent", "lprops_rectangles");
		}

		const auto& spaceResults = dl.getSpaceResults();
		std::map<unsigned int, double> scoreBySpaceID;
		if (!spaceResults.empty())
		{
			for (const auto& spaceResult : spaceResults)
			{
				scoreBySpaceID[spaceResult.mSpaceID] = spaceResult.mScore;
			}
		}

		const auto& dfSettings = dl.getDF_Settings();
		double referenceHeight = 0.0;
		if (dfSettings.size() > 1)
		{
			referenceHeight = dfSettings[1];
		}

		// helper function to map normalized scores to colors (green pass -> red fail)
		auto scoreColor = [](double t) -> rgba
		{
			t = std::clamp(t, 0.0, 1.0);
			return rgba(static_cast<float>(t), static_cast<float>(1.0 - t), 0.0f, 1.0f);
		};

		// Draw one colored quadrilateral per space at the specified reference height.
		for (const auto& spacePtr : dl.getSpacePropertiesQH())
		{
			auto scoreIter = scoreBySpaceID.find(spacePtr->getSpaceID());
			if (scoreIter == scoreBySpaceID.end()) continue;

			double normalizedScore = std::clamp(scoreIter->second / 10.0, 0.0, 1.0);

			polygon_props planeProps = pprops_gridpoint;
			planeProps.ambient = scoreColor(normalizedScore);
			planeProps.diffuse = planeProps.ambient;
			planeProps.specular = rgba(0.1f, 0.1f, 0.1f, 1.0f);
			planeProps.emission = rgba(0.0f, 0.0f, 0.0f, 1.0f);
			planeProps.shininess = 20.0f;
			planeProps.translucent = true;
			planeProps.twosided = true;
			pprops_color_gradient.push_back(planeProps);

			bool floorFound = false;
			std::vector<geom::vertex> floorVertices;
			double minFloorZ = 0.0;
			for (const auto* poly : spacePtr->getPolygons())
			{
				auto normal = poly->getNormal();
				normal.normalize();
				if (std::fabs(normal[2]) < 0.999) continue;

				double currentZ = poly->getCenter()[2];
				if (!floorFound || currentZ < minFloorZ)
				{
					floorFound = true;
					minFloorZ = currentZ;
					floorVertices.clear();
					for (const auto& v : *poly)
					{
						floorVertices.push_back(v);
					}
				}
			}
			if (!floorFound) continue;

			std::vector<geom::vertex> refPlaneVertices;
			for (const auto& v : floorVertices)
			{
				refPlaneVertices.push_back(v + geom::vertex{0.0, 0.0, referenceHeight});
			}

			geom::quadrilateral refPlane(refPlaneVertices);
			this->addPolygon(polygons, &refPlane, &(pprops_color_gradient.back()), &lprops_rectangles, 20.0, "", "lprops_rectangles");
		}

		// Build overlay legend with 10 fixed categories for NEN score scale [0,10].
		const unsigned int nLegendBins = 10;
		if (!spaceResults.empty())
		{
			for (unsigned int i = 0; i < nLegendBins; ++i)
			{
				double lower = static_cast<double>(i);
				double upper = static_cast<double>(i + 1);

				double tMid = (static_cast<double>(i) + 0.5) / static_cast<double>(nLegendBins);
				legend_bin bin;
				bin.color = scoreColor(tMid);
				bin.lower = lower;
				bin.upper = upper;
				mLegendBins.push_back(bin);
			}
		}
	}
	else if(type == "ADF")
	{
		mDrawOverlayLegend = true;
		mLegendTitle = "Average daylight factor legend [%]";
		mLegendBins.clear();
		mLegendNotes.clear();
		pprops_color_gradient.clear();
		pprops_color_gradient.reserve(dl.getSpacePropertiesQH().size());

		// Provide a translucent view of the building for reference
		for (const auto& map : dl.getWindowProperties())
		{
			for (const auto& [cfRec,isWindow] : map)
			{
				std::vector<geom::vertex> rectangleVertices;
				for (const auto& vertex : *cfRec)
				{
					rectangleVertices.push_back(cfRec->getCenter() + vertex - cfRec->getCenter());
				}

				geom::quadrilateral rectangleGeometry(rectangleVertices);

				if (isWindow==true)
				{
					this->addPolygon(polygons, &rectangleGeometry, &pprops_window, &lprops_rectangles, 0.0, "pprops_window", "lprops_rectangles");
				}
				else if (isWindow==false)
				{
					this->addPolygon(polygons, &rectangleGeometry, &pprops_wall_translucent, &lprops_rectangles, 0.0, "pprops_wall_translucent", "lprops_rectangles");
				}
			}
		}
		for (const auto& i : dl.getTriangles())
		{
			std::vector<geom::vertex> triangleVertices;
			for (const auto& vertex : *i)
			{
				triangleVertices.push_back(i->getCenter() + vertex - i->getCenter());
			}

			geom::triangle triangleGeometry(triangleVertices);
			this->addPolygon(polygons, &triangleGeometry, &pprops_wall_translucent, &lprops_rectangles, 0.0, "pprops_wall_translucent", "lprops_rectangles");
		}

		const auto& spaceResults = dl.getSpaceResults();
		std::map<unsigned int, double> adfBySpaceID;
		double minADF = 0.0;
		double maxADF = 0.0;
		if (!spaceResults.empty())
		{
			minADF = spaceResults.front().mADF;
			maxADF = spaceResults.front().mADF;
			for (const auto& spaceResult : spaceResults)
			{
				adfBySpaceID[spaceResult.mSpaceID] = spaceResult.mADF;
				minADF = std::min(minADF, spaceResult.mADF);
				maxADF = std::max(maxADF, spaceResult.mADF);
			}
		}

		const auto& dfSettings = dl.getDF_Settings();
		double referenceHeight = 0.0;
		if (dfSettings.size() > 1)
		{
			referenceHeight = dfSettings[1];
		}

		// helper function to map normalized values to colors (blue -> green -> yellow)
		auto adfColor = [](double t) -> rgba
		{
			t = std::clamp(t, 0.0, 1.0);
			if (t <= (1.0 / 3.0))
			{
				double u = t / (1.0 / 3.0);
				return rgba(0.0f, static_cast<float>(u), 1.0f, 1.0f);
			}
			if (t <= (2.0 / 3.0))
			{
				double u = (t - (1.0 / 3.0)) / (1.0 / 3.0);
				return rgba(0.0f, 1.0f, static_cast<float>(1.0 - u), 1.0f);
			}
			double u = (t - (2.0 / 3.0)) / (1.0 / 3.0);
			return rgba(static_cast<float>(u), 1.0f, 0.0f, 1.0f);
		};

		// Draw one colored quadrilateral per space at the specified reference height.
		for (const auto& spacePtr : dl.getSpacePropertiesQH())
		{
			auto adfIter = adfBySpaceID.find(spacePtr->getSpaceID());
			if (adfIter == adfBySpaceID.end()) continue;

			double normalizedADF = 0.0;
			if (maxADF > minADF)
			{
				normalizedADF = (adfIter->second - minADF) / (maxADF - minADF);
			}

			polygon_props planeProps = pprops_gridpoint;
			planeProps.ambient = adfColor(normalizedADF);
			planeProps.diffuse = planeProps.ambient;
			planeProps.specular = rgba(0.1f, 0.1f, 0.1f, 1.0f);
			planeProps.emission = rgba(0.0f, 0.0f, 0.0f, 1.0f);
			planeProps.shininess = 20.0f;
			planeProps.translucent = true;
			planeProps.twosided = true;
			pprops_color_gradient.push_back(planeProps);

			bool floorFound = false;
			std::vector<geom::vertex> floorVertices;
			double minFloorZ = 0.0;
			for (const auto* poly : spacePtr->getPolygons())
			{
				auto normal = poly->getNormal();
				normal.normalize();
				if (std::fabs(normal[2]) < 0.999) continue;

				double currentZ = poly->getCenter()[2];
				if (!floorFound || currentZ < minFloorZ)
				{
					floorFound = true;
					minFloorZ = currentZ;
					floorVertices.clear();
					for (const auto& v : *poly)
					{
						floorVertices.push_back(v);
					}
				}
			}
			if (!floorFound) continue;

			std::vector<geom::vertex> refPlaneVertices;
			for (const auto& v : floorVertices)
			{
				refPlaneVertices.push_back(v + geom::vertex{0.0, 0.0, referenceHeight});
			}

			geom::quadrilateral refPlane(refPlaneVertices);
			this->addPolygon(polygons, &refPlane, &(pprops_color_gradient.back()), &lprops_rectangles, 20.0, "", "lprops_rectangles");
		}

		// Build overlay legend with 12 categories using ADF value bins.
		const unsigned int nLegendBins = 12;
		if (!spaceResults.empty())
		{
			double span = maxADF - minADF;
			for (unsigned int i = 0; i < nLegendBins; ++i)
			{
				double lower = minADF;
				double upper = maxADF;
				if (span > 0.0)
				{
					lower = minADF + (static_cast<double>(i) / static_cast<double>(nLegendBins)) * span;
					upper = minADF + (static_cast<double>(i + 1) / static_cast<double>(nLegendBins)) * span;
				}

				double tMid = (static_cast<double>(i) + 0.5) / static_cast<double>(nLegendBins);
				legend_bin bin;
				bin.color = adfColor(tMid);
				bin.lower = lower;
				bin.upper = upper;
				mLegendBins.push_back(bin);
			}
		}
	}
	else if(type == "ADF_inverse")
	{
		mDrawOverlayLegend = true;
		mLegendTitle = "Average daylight factor inverse legend [1/%]";
		mLegendBins.clear();
		mLegendNotes.clear();
		pprops_color_gradient.clear();
		pprops_color_gradient.reserve(dl.getSpacePropertiesQH().size());

		// Provide a translucent view of the building for reference
		for (const auto& map : dl.getWindowProperties())
		{
			for (const auto& [cfRec,isWindow] : map)
			{
				std::vector<geom::vertex> rectangleVertices;
				for (const auto& vertex : *cfRec)
				{
					rectangleVertices.push_back(cfRec->getCenter() + vertex - cfRec->getCenter());
				}

				geom::quadrilateral rectangleGeometry(rectangleVertices);

				if (isWindow==true)
				{
					this->addPolygon(polygons, &rectangleGeometry, &pprops_window, &lprops_rectangles, 0.0, "pprops_window", "lprops_rectangles");
				}
				else if (isWindow==false)
				{
					this->addPolygon(polygons, &rectangleGeometry, &pprops_wall_translucent, &lprops_rectangles, 0.0, "pprops_wall_translucent", "lprops_rectangles");
				}
			}
		}
		for (const auto& i : dl.getTriangles())
		{
			std::vector<geom::vertex> triangleVertices;
			for (const auto& vertex : *i)
			{
				triangleVertices.push_back(i->getCenter() + vertex - i->getCenter());
			}

			geom::triangle triangleGeometry(triangleVertices);
			this->addPolygon(polygons, &triangleGeometry, &pprops_wall_translucent, &lprops_rectangles, 0.0, "pprops_wall_translucent", "lprops_rectangles");
		}

		const auto& spaceResults = dl.getSpaceResults();
		std::map<unsigned int, double> adfInverseBySpaceID;
		double minADFInverse = 0.0;
		double maxADFInverse = 0.0;
		if (!spaceResults.empty())
		{
			minADFInverse = spaceResults.front().mADFInverse;
			maxADFInverse = spaceResults.front().mADFInverse;
			for (const auto& spaceResult : spaceResults)
			{
				adfInverseBySpaceID[spaceResult.mSpaceID] = spaceResult.mADFInverse;
				minADFInverse = std::min(minADFInverse, spaceResult.mADFInverse);
				maxADFInverse = std::max(maxADFInverse, spaceResult.mADFInverse);
			}
		}

		const auto& dfSettings = dl.getDF_Settings();
		double referenceHeight = 0.0;
		if (dfSettings.size() > 1)
		{
			referenceHeight = dfSettings[1];
		}

		// helper function to map normalized values to colors (blue -> green -> yellow)
		auto adfInverseColor = [](double t) -> rgba
		{
			t = std::clamp(t, 0.0, 1.0);
			if (t <= (1.0 / 3.0))
			{
				double u = t / (1.0 / 3.0);
				return rgba(0.0f, static_cast<float>(u), 1.0f, 1.0f);
			}
			if (t <= (2.0 / 3.0))
			{
				double u = (t - (1.0 / 3.0)) / (1.0 / 3.0);
				return rgba(0.0f, 1.0f, static_cast<float>(1.0 - u), 1.0f);
			}
			double u = (t - (2.0 / 3.0)) / (1.0 / 3.0);
			return rgba(static_cast<float>(u), 1.0f, 0.0f, 1.0f);
		};

		// Draw one colored quadrilateral per space at the specified reference height.
		for (const auto& spacePtr : dl.getSpacePropertiesQH())
		{
			auto adfInverseIter = adfInverseBySpaceID.find(spacePtr->getSpaceID());
			if (adfInverseIter == adfInverseBySpaceID.end()) continue;

			double normalizedADFInverse = 0.0;
			if (maxADFInverse > minADFInverse)
			{
				normalizedADFInverse = (adfInverseIter->second - minADFInverse) / (maxADFInverse - minADFInverse);
			}

			polygon_props planeProps = pprops_gridpoint;
			planeProps.ambient = adfInverseColor(normalizedADFInverse);
			planeProps.diffuse = planeProps.ambient;
			planeProps.specular = rgba(0.1f, 0.1f, 0.1f, 1.0f);
			planeProps.emission = rgba(0.0f, 0.0f, 0.0f, 1.0f);
			planeProps.shininess = 20.0f;
			planeProps.translucent = true;
			planeProps.twosided = true;
			pprops_color_gradient.push_back(planeProps);

			bool floorFound = false;
			std::vector<geom::vertex> floorVertices;
			double minFloorZ = 0.0;
			for (const auto* poly : spacePtr->getPolygons())
			{
				auto normal = poly->getNormal();
				normal.normalize();
				if (std::fabs(normal[2]) < 0.999) continue;
				double currentZ = poly->getCenter()[2];
				if (!floorFound || currentZ < minFloorZ)
				{
					floorFound = true;
					minFloorZ = currentZ;
					floorVertices.clear();
					for (const auto& v : *poly)
					{
						floorVertices.push_back(v);
					}
				}
			}
			if (!floorFound) continue;

			std::vector<geom::vertex> refPlaneVertices;
			for (const auto& v : floorVertices)
			{
				refPlaneVertices.push_back(v + geom::vertex{0.0, 0.0, referenceHeight});
			}

			geom::quadrilateral refPlane(refPlaneVertices);
			this->addPolygon(polygons, &refPlane, &(pprops_color_gradient.back()), &lprops_rectangles, 20.0, "", "lprops_rectangles");
		}

		// Build overlay legend with 12 categories using ADF inverse value bins.
		const unsigned int nLegendBins = 12;
		if (!spaceResults.empty())
		{
			double span = maxADFInverse - minADFInverse;
			for (unsigned int i = 0; i < nLegendBins; ++i)
			{
				double lower = minADFInverse;
				double upper = maxADFInverse;
				if (span > 0.0)
				{
					lower = minADFInverse + (static_cast<double>(i) / static_cast<double>(nLegendBins)) * span;
					upper = minADFInverse + (static_cast<double>(i + 1) / static_cast<double>(nLegendBins)) * span;
				}

				double tMid = (static_cast<double>(i) + 0.5) / static_cast<double>(nLegendBins);
				legend_bin bin;
				bin.color = adfInverseColor(tMid);
				bin.lower = lower;
				bin.upper = upper;
				mLegendBins.push_back(bin);
			}
		}
	}
	
	pbsp = new random_bsp(polygons); 

	if (writeToFile) 
	{
		this->writeToFile(fileName);
	}
} //ctor DL_MODEL()
#endif // DL_MODEL_HPP

DL_Model::DL_Model(const std::string& fileName)
{
	std::ifstream input(fileName.c_str());
	if (!input.is_open())
	{
		std::stringstream errorMessage;
		errorMessage << "Could not initialize DL_Model visualization from file: " << fileName << "\n"
						 << "(bso/visualization/models/dl_model.hpp)";
		throw std::invalid_argument(errorMessage.str());
	}

	std::map<std::string, polygon_props*> polygonPropsMap = {
		{"pprops_window", &pprops_window},
		{"pprops_wall", &pprops_wall},
		{"pprops_wall_translucent", &pprops_wall_translucent},
		{"pprops_gridpoint", &pprops_gridpoint},
		{"pprops_Space", &pprops_Space}
	};
	std::map<std::string, line_props*> linePropsMap = {
		{"lprops_space_boundary", &lprops_space_boundary},
		{"lprops_rectangles", &lprops_rectangles}
	};

	std::string line;
	while (std::getline(input, line))
	{
		line = detail::trim_copy(line);
		if (line.empty() || line[0] == '#') continue;

		if (line.rfind("ModelDescription,", 0) == 0)
		{
			auto tokens = detail::split_csv(line);
			if (tokens.size() >= 4)
			{
				mTitle = tokens[2];
				mType = tokens[3];
			}
			else if (tokens.size() >= 3)
			{
				mTitle = tokens[1];
				mType = tokens[2];
			}
			continue;
		}

		if (line.rfind("pprops_", 0) == 0)
		{
			auto keyEnd = line.find(',');
			const auto key = line.substr(0, keyEnd);
			detail::assign_polygon_props(*polygonPropsMap.at(key), line);
			continue;
		}

		if (line.rfind("lbprops,", 0) == 0)
		{
			detail::assign_label_props(lbprops, line);
			continue;
		}

		if (line.rfind("lprops_", 0) == 0)
		{
			auto keyEnd = line.find(',');
			const auto key = line.substr(0, keyEnd);
			detail::assign_line_props(*linePropsMap.at(key), line);
			continue;
		}

		if (line.rfind("PolygonPropsName,", 0) == 0)
		{
			std::cout << "Reading line: " << line << std::endl; // Debug output to check the line content
			auto tokens = detail::split_csv(line);
			if (tokens.size() < 4)
			{
				throw std::invalid_argument("Invalid polygon line in DL_Model visualization file.");
			}
			polygon* poly = new polygon(line, polygonPropsMap.at(tokens[1]), linePropsMap.at(tokens[3]));
			polygons.push_back(poly);
			const edge* boundary = poly->get_boundary();
			if (boundary)
			{
				const edge* current = boundary;
				do {
					bbox.add(*current->pvert);
					current = current->pnext;
				} while (current != boundary);
			}
			continue;
		}

		if (line.rfind("LabelPropsName,", 0) == 0)
		{
			const std::string labelText = detail::parse_label_text(line);
			const vertex labelPosition = detail::parse_label_position(line);
			bbox.add(labelPosition);
			labels.push_back(new label(&lbprops, labelText, store.get_vertex(labelPosition)));
			continue;
		}
	}

	pbsp = new random_bsp(polygons);
} // ctor DL_Model(file)

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
	if (pbsp) pbsp->render_btf(cam);
	std::list<label*>::const_iterator lbit;
	for (lbit = labels.begin(); lbit != labels.end(); lbit++)
	{
		(*lbit)->render();
	}

	if (mDrawOverlayLegend && !mLegendBins.empty())
	{
		auto formatValue = [](double value) -> std::string
		{
			std::ostringstream ss;
			double absValue = std::abs(value);
			if (absValue >= 1000.0 || (absValue > 0.0 && absValue < 1e-3))
			{
				ss << std::scientific << std::setprecision(2) << value;
			}
			else
			{
				ss << std::fixed << std::setprecision(3) << value;
			}
			return ss.str();
		};

		int vp[4];
		glGetIntegerv(GL_VIEWPORT, vp);
		int viewportWidth = vp[2];
		int viewportHeight = vp[3];

		const int pad = 10;
		const int boxW = 18;
		const int boxH = 12;
		const int rowGap = 5;
		const int textXOffset = boxW + 10;
		const int titleHeight = 16;
		const int panelW = 340;
		const int noteLineH = 14;
		const int noteSectionGap = mLegendNotes.empty() ? 0 : 10;
		const int notesH = (int)mLegendNotes.size() * noteLineH;
		const int panelH = titleHeight + 14 + (int)mLegendBins.size() * (boxH + rowGap) + noteSectionGap + notesH + pad;
		const int panelX = viewportWidth - panelW - pad;
		const int panelY = viewportHeight - panelH - pad;

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
		glRasterPos3i(panelX + pad, panelY + panelH - titleHeight, 0);
		for (char c : mLegendTitle) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);

		int rowIndex = 0;
		for (auto it = mLegendBins.rbegin(); it != mLegendBins.rend(); ++it, ++rowIndex)
		{
			int rowY = panelY + panelH - titleHeight - 12 - (rowIndex + 1) * (boxH + rowGap);

			glColor4f(it->color[0], it->color[1], it->color[2], 1.0f);
			glBegin(GL_QUADS);
			glVertex3i(panelX + pad, rowY, 0);
			glVertex3i(panelX + pad + boxW, rowY, 0);
			glVertex3i(panelX + pad + boxW, rowY + boxH, 0);
			glVertex3i(panelX + pad, rowY + boxH, 0);
			glEnd();

			std::string rangeText = "[" + formatValue(it->lower) + ", " + formatValue(it->upper) + "]";

			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			glRasterPos3i(panelX + pad + textXOffset, rowY + 2, 0);
			for (char c : rangeText) glutBitmapCharacter(GLUT_BITMAP_8_BY_13, c);
		}

		if (!mLegendNotes.empty())
		{
			int notesStartY = panelY + pad + (int)mLegendNotes.size() * noteLineH;
			for (const auto& note : mLegendNotes)
			{
				glColor4f(1.0f, 1.0f, 1.0f, 0.95f);
				glRasterPos3i(panelX + pad, notesStartY, 0);
				for (char c : note) glutBitmapCharacter(GLUT_BITMAP_8_BY_13, c);
				notesStartY -= noteLineH;
			}
		}

		glPopAttrib();

		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
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

#ifdef DL_MODEL_HPP
void DL_Model::addBoundaryLinesForSpace(bso::spatial_design::conformal::cf_space* cf_space)
{
	std::vector<bso::utilities::geometry::line_segment> cf_line_segments;
	cf_line_segments  = cf_space->getLines();

    for (const auto& line_segment : cf_line_segments)
    {
		this->addLineSegment(polygons, &line_segment, &pprops_Space, &lprops_space_boundary, 0.0, {0,0,0}, 0, "pprops_Space", "lprops_space_boundary");
    }
}
#endif // DL_MODEL_HPP

void DL_Model::writeToFile(const std::string& filename) const
{
	std::ofstream outFile(filename);
	if (!outFile.is_open()) 
	{
		std::cerr << "Warning:Error opening file for writing dl_model visualization: " << filename << std::endl;
		return;
	}

	// Write model description
	outFile << "ModelDescription, DL_Model," << mTitle << "," << mType << "\n";

	// write all pprops_, lbprops and lprops_ values to the file for later use in visualization with their indentifiers (e.g., pprops_window, lbprops, lprops_space_boundary, etc.)
	outFile << "pprops_window," << pprops_window.ambient << "," << pprops_window.diffuse << "," << pprops_window.specular << "," << pprops_window.emission << "," << pprops_window.shininess << "," << pprops_window.translucent << "," << pprops_window.twosided << "\n";
	outFile << "pprops_wall," << pprops_wall.ambient << "," << pprops_wall.diffuse << "," << pprops_wall.specular << "," << pprops_wall.emission << "," << pprops_wall.shininess <<	"," << pprops_wall.translucent << "," << pprops_wall.twosided << "\n";
	outFile << "pprops_wall_translucent," << pprops_wall_translucent.ambient << "," << pprops_wall_translucent.diffuse << "," << pprops_wall_translucent.specular << "," << pprops_wall_translucent.emission << "," << pprops_wall_translucent.shininess <<	"," << pprops_wall_translucent.translucent << "," << pprops_wall_translucent.twosided << "\n";
	outFile << "pprops_gridpoint," << pprops_gridpoint.ambient << "," << pprops_gridpoint.diffuse << "," << pprops_gridpoint.specular << "," << pprops_gridpoint.emission << "," << pprops_gridpoint.shininess <<	"," << pprops_gridpoint.translucent << "," << pprops_gridpoint.twosided << "\n";
	outFile << "pprops_Space," << pprops_Space.ambient << "," << pprops_Space.diffuse << "," << pprops_Space.specular << "," << pprops_Space.emission << "," << pprops_Space.shininess <<	"," << pprops_Space.translucent << "," << pprops_Space.twosided << "\n";
	
	outFile << "lbprops," << lbprops.textcolor << "," << detail::get_glut_font_name(lbprops.glutbmpfont) << "\n";

	outFile << "lprops_space_boundary," << lprops_space_boundary.color << "," << lprops_space_boundary.width << "," << lprops_space_boundary.pattern << "," << lprops_space_boundary.factor << "\n";
	outFile << "lprops_rectangles," << lprops_rectangles.color << "," << lprops_rectangles.width << "," << lprops_rectangles.pattern << "," << lprops_rectangles.factor << "\n";

	// Write polygons
	for (const auto& poly : polygons) 
	{
		poly->write_to_file(outFile);
	}

	// Write labels
	for (const auto& currentLabel : labels) 
	{
		currentLabel->write_to_file(outFile);
	}

	outFile.close();
} // writeToFile()

} // namespace Visualisation
} // namespace BSO


#endif // VIS_DL_MODEL_HPP
