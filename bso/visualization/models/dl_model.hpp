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

    polygon_props pprops_window;
    polygon_props pprops_wall;
	polygon_props pprops_wall_translucent;
	polygon_props pprops_gridpoint;
	polygon_props pprops_Space;

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

	//polygon properties for walls/floors/ceilings to show the grid points so walls are translucent
	pprops_wall_translucent.ambient = rgba(0.5f, 0.5f, 0.5f, 0.3f);
	pprops_wall_translucent.diffuse = rgba(0.6f, 0.6f, 0.6f, 0.3f);
	pprops_wall_translucent.specular = rgba(0.7f, 0.7f, 0.7f, 0.3f);
	pprops_wall_translucent.emission = rgba(0.0f, 0.0f, 0.0f, 0.3f);
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
