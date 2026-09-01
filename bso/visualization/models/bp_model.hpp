#ifndef VIS_BP_MODEL_HPP
#define VIS_BP_MODEL_HPP

#include <bso/building_physics/bp_model.hpp>
#include <bso/visualization/models/model_base.hpp>
#include <bso/visualization/bsp/bsp.hpp>
#include <bso/utilities/geometry.hpp>

#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <map>
#include <vector>
#include <sstream>
#include <iomanip>

namespace bso { namespace visualization
{

class BP_Model : public model_base
{
public:
	BP_Model(const bso::building_physics::bp_model& bp,
					 const std::string& type = "bp_model",
					 const std::string& title = "bp_model",
					 const bool& showValues = false);
	~BP_Model();
	void render(const camera &cam) const;
	const std::string get_description();
	bool key_pressed(int key);

protected:

private:
	std::string mTitle;
	std::string mType;
	bool mShowValues;
	std::list<polygon*> polygons;
	std::list<label*> labels;
	std::list<label*> mValueLabels; // R/C value labels, only rendered when mShowValues is true
	std::map<std::string, polygon_props*> pplist;

	line_props     lprops;
	line_props	   lprops_thick;
	label_props    lbprops;
	random_bsp     *pbsp;

	std::vector<std::pair<rgba,std::string>> mLegendRows; // only filled for the RC_model type

	polygon_props* getPProps(const std::pair<std::string,Eigen::Vector4d>& visProps, bool translucent = false);
};

polygon_props* BP_Model::getPProps(const std::pair<std::string,Eigen::Vector4d>& visProps, bool translucent /*= false*/)
{
	auto ppropSearch = pplist.find(visProps.first);
	if (ppropSearch == pplist.end())
	{
		auto newPPropPtr = new polygon_props();
		float alpha = translucent ? 0.05f : (float)(visProps.second)(3);

		newPPropPtr->ambient     = rgba((visProps.second)(0),(visProps.second)(1),(visProps.second)(2),alpha);
		newPPropPtr->diffuse     = rgba((visProps.second)(0),(visProps.second)(1),(visProps.second)(2),alpha);
		newPPropPtr->specular    = rgba(0.1,0.1,0.1,1);
		newPPropPtr->shininess   = 60;
		newPPropPtr->translucent = ((visProps.second)(3) < 1);
		newPPropPtr->twosided    = true;
		pplist.emplace(visProps.first, newPPropPtr);
		return newPPropPtr;
	}
	else return ppropSearch->second;
} // getPProps

BP_Model::BP_Model(const bso::building_physics::bp_model& bp,
									 const std::string& type /*= "bp_model"*/,
									 const std::string& title /*= "bp_model"*/,
									 const bool& showValues /*= false*/)
: mTitle(title), mType(type), mShowValues(showValues)
{
	namespace geom = bso::utilities::geometry;
	namespace bp_state = bso::building_physics::state;

	lprops_thick.width = 2.0f;

	bool addModelTranslucent = false;
	if(mType == "rc_model"){addModelTranslucent = true;}
	for (const auto& i : bp.getWalls())
	{
		auto ppropsPtr = getPProps(i->getVisualizationProperties(), addModelTranslucent);
		double thickness = addModelTranslucent ? 0.0 : i->getThickness();
		this->addPolygon(polygons,i->getGeometry(),ppropsPtr,&lprops, thickness);
	}
	for (const auto& i : bp.getFloors())
	{
		auto ppropsPtr = getPProps(i->getVisualizationProperties(), addModelTranslucent);
		double thickness = addModelTranslucent ? 0.0 : i->getThickness();
		this->addPolygon(polygons,i->getGeometry(),ppropsPtr,&lprops, thickness);
	}
	for (const auto& i : bp.getWindows())
	{
		auto ppropsPtr = getPProps(i->getVisualizationProperties());
		this->addPolygon(polygons,i->getGeometry(),ppropsPtr,&lprops);
	}
	if (mType == "bp_model"){/* Model already added in previous section, so do nothing.*/}
	else if (mType == "rc_model")
	{
		// a state's description, falling back to a generic "type_index" name if none was set
		auto stateLabel = [](bp_state::state* s) -> std::string
		{
			if (!s->getDescription().empty()) return s->getDescription();
			std::string typeName = "state";
			if      (s->isSpace())  typeName = "space";
			else if (s->isWall())   typeName = "wall";
			else if (s->isFloor())  typeName = "floor";
			else if (s->isWindow()) typeName = "window";
			return typeName + "_" + std::to_string(s->getIndex());
		};
		auto formatValue = [](const double& v) -> std::string
		{
			std::ostringstream ss;
			double av = std::abs(v);
			if (av >= 1000.0 || (av > 0.0 && av < 1e-3)) ss << std::scientific << std::setprecision(2) << v;
			else ss << std::fixed << std::setprecision(3) << v;
			return ss.str();
		};

		// first pass: gather node positions (geometric center) of every state that has geometry,
		// and the overall bounding box, needed to size icons and orient the outdoor/ground stubs
		std::map<bp_state::state*, geom::vertex> nodePos;
		geom::vertex bboxMin, bboxMax;
		bool bboxInit = false;
		auto updateBBox = [&](const geom::vertex& v)
		{
			if (!bboxInit) { bboxMin = v; bboxMax = v; bboxInit = true; return; }
			for (unsigned int j = 0; j < 3; ++j)
			{
				if (v(j) < bboxMin(j)) bboxMin(j) = v(j);
				if (v(j) > bboxMax(j)) bboxMax(j) = v(j);
			}
		};
		for (const auto& i : bp.getSpaces())  { auto c = i->getGeometry()->getCenter(); nodePos[i] = c; updateBBox(c); }
		for (const auto& i : bp.getWalls())   { auto c = i->getGeometry()->getCenter(); nodePos[i] = c; updateBBox(c); }
		for (const auto& i : bp.getFloors())  { auto c = i->getGeometry()->getCenter(); nodePos[i] = c; updateBBox(c); }
		for (const auto& i : bp.getWindows()) { auto c = i->getGeometry()->getCenter(); nodePos[i] = c; updateBBox(c); }

		geom::vertex overallCentroid = bboxInit ? geom::vertex((bboxMin + bboxMax) * 0.5) : geom::vertex{0,0,0};
		double bboxDiagonal = bboxInit ? (bboxMax - bboxMin).norm() : 0.0;
		if (bboxDiagonal < 1e-6) bboxDiagonal = 1000.0; // fallback scale for a degenerate/tiny model

		double stubOffset    = std::max(0.20  * bboxDiagonal, 300.0);
		double resistorSize  = std::max(0.01  * bboxDiagonal, 80.0);
		double capStubLength = std::max(0.050 * bboxDiagonal, 120.0);
		double nodeSizeSmall = std::max(0.012 * bboxDiagonal, 100.0);
		double nodeSizeLarge = std::max(0.025 * bboxDiagonal, 200.0);

		// fixed color scheme, one entry per icon type, also registered in pplist so the
		// destructor cleans them up and render() picks up their translucency automatically
		auto makeProps = [&](const std::string& name, const rgba& color, bool translucent) -> polygon_props*
		{
			auto p = new polygon_props();
			p->ambient = color;
			p->diffuse = color;
			p->specular = rgba(0.1f,0.1f,0.1f,1.0f);
			p->shininess = 60;
			p->translucent = translucent;
			p->twosided = true;
			pplist[name] = p;
			return p;
		};
		auto pSpace     = makeProps("rc_space",     rgba(1.00f,0.55f,0.15f,1.00f), true);
		auto pWall      = makeProps("rc_wall",      rgba(0.55f,0.55f,0.55f,1.00f), true);
		auto pFloor     = makeProps("rc_floor",     rgba(0.55f,0.35f,0.15f,1.00f), true);
		auto pWindow    = makeProps("rc_window",    rgba(0.40f,0.80f,1.00f,1.00f), true);
		auto pOutdoor   = makeProps("rc_outdoor",   rgba(0.30f,0.70f,1.00f,1.00f), true);
		auto pGround    = makeProps("rc_ground",    rgba(0.25f,0.50f,0.25f,1.00f), true);
		auto pResistor  = makeProps("rc_resistor",  rgba(0.20f,0.20f,0.20f,1.00f), false);
		auto pCapacitor = makeProps("rc_capacitor", rgba(0.00f,0.00f,0.00f,1.00f), false);
		auto pWire      = makeProps("rc_wire",      rgba(0.15f,0.15f,0.15f,1.00f), false);

		mLegendRows = {
			{rgba(1.00f,0.55f,0.15f,1.0f), "Space"},
			{rgba(0.55f,0.55f,0.55f,1.0f), "Wall"},
			{rgba(0.55f,0.35f,0.15f,1.0f), "Floor"},
			{rgba(0.40f,0.80f,1.00f,1.0f), "Window"},
			{rgba(0.30f,0.70f,1.00f,1.0f), "Outdoor (weather profile)"},
			{rgba(0.25f,0.50f,0.25f,1.0f), "Ground"},
			{rgba(0.20f,0.20f,0.20f,1.0f), "Resistor (cuboid)"},
			{rgba(0.00f,0.00f,0.00f,1.0f), "Capacitor (45 degree T-stub)"}
		};

		// draws a resistor icon + wire between elemPos and the node of "side" (or a local
		// outdoor/ground stub if "side" is an independent state without its own geometry)
		auto connectSide = [&](const geom::vertex& elemPos, const geom::vector& outwardNormal,
														bp_state::state* side, const double& R, const std::string& prefix)
		{
			geom::vertex targetPos;
			if (side->isWeatherProfile())
			{
				targetPos = elemPos + outwardNormal*stubOffset;
				this->addCube(polygons, targetPos, pOutdoor, &lprops, nodeSizeSmall, "rc_outdoor");
			}
			else if (side->isGroundProfile())
			{
				targetPos = elemPos + outwardNormal*stubOffset;
				this->addCube(polygons, targetPos, pGround, &lprops, nodeSizeSmall, "rc_ground");
			}
			else targetPos = nodePos.at(side);

			std::vector<geom::vertex> wireVerts = {elemPos, targetPos};
			geom::line_segment wireGeom(wireVerts);
			this->addLineSegment(polygons, &wireGeom, pWire, &lprops);

			geom::vertex mid = (elemPos + targetPos) * 0.5;
			this->addSquarePrism(polygons, mid, pResistor, &lprops, 4*resistorSize, resistorSize, resistorSize, /*geom::vector({0.0,0.0,1.0})*/wireGeom.getVector(), "rc_resistor");

			if (mShowValues)
			{
				this->addLabel(mValueLabels, &lbprops, prefix + formatValue(R) + " m2K/W", mid);
			}
		};

		// This definition does not hold, so it needs to be altered.
		// returns the polygon's normal, flipped so it points away from the building's centroid
		auto outwardNormal = [&](bso::utilities::geometry::polygon* geomPtr, const geom::vertex& elemPos) -> geom::vector
		{
			geom::vector n = geomPtr->getNormal();
			n.normalize();
			if (n.dot(elemPos - overallCentroid) < 0) n = geom::vector(-1.0*n);
			return n;
		};

		// draws a 45 degree T-stub capacitor symbol at a dependent state's node, returns its tip
		auto addCapacitorIcon = [&](const geom::vertex& pos) -> geom::vertex
		{
			geom::vector capDir({1.0,1.0,1.0});
			capDir.normalize();
			geom::vector plateDir = capDir.cross(geom::vector({0.0,0.0,1.0}));
			if (plateDir.norm() < 1e-6) plateDir = geom::vector({1.0,0.0,0.0});
			else plateDir.normalize();

			geom::vertex stubEnd = pos + capDir*capStubLength;
			std::vector<geom::vertex> stubVerts = {pos, stubEnd};
			geom::line_segment stubGeom(stubVerts);
			this->addLineSegment(polygons, &stubGeom, pCapacitor, &lprops_thick);

			double plateHalf = capStubLength*0.35;
			std::vector<geom::vertex> plateVerts = {stubEnd - plateDir*plateHalf, stubEnd + plateDir*plateHalf};
			geom::line_segment plateGeom(plateVerts);
			this->addLineSegment(polygons, &plateGeom, pCapacitor, &lprops_thick);

			return stubEnd;
		};

		// second pass: add icons, wires and labels for every state
		for (const auto& i : bp.getSpaces())
		{
			geom::vertex pos = nodePos.at(i);
			this->addPoint(polygons, pos, pSpace, &lprops, nodeSizeLarge, "rc_space");
			//this->addLabel(labels, &lbprops, stateLabel(i), pos);
			auto stubEnd = addCapacitorIcon(pos);
			if (mShowValues)
				this->addLabel(mValueLabels, &lbprops, "C=" + formatValue(i->getCapacitance()) + " J/K", stubEnd);
		}
		for (const auto& i : bp.getWalls())
		{
			geom::vertex elemPos = nodePos.at(i);
			this->addPoint(polygons, elemPos, pWall, &lprops, nodeSizeSmall, "rc_wall");
			//this->addLabel(labels, &lbprops, stateLabel(i), elemPos);
			auto n = outwardNormal(i->getGeometry(), elemPos);
			connectSide(elemPos, n, i->getSide1(), i->getConstruction().getResistanceSide1(), "R1=");
			connectSide(elemPos, n, i->getSide2(), i->getConstruction().getResistanceSide2(), "R2=");
			auto stubEnd = addCapacitorIcon(elemPos);
			if (mShowValues)
				this->addLabel(mValueLabels, &lbprops, "C=" + formatValue(i->getCapacitance()) + " J/K", stubEnd);
		}
		for (const auto& i : bp.getFloors())
		{
			geom::vertex elemPos = nodePos.at(i);
			this->addPoint(polygons, elemPos, pFloor, &lprops, nodeSizeSmall, "rc_floor");
			//this->addLabel(labels, &lbprops, stateLabel(i), elemPos);
			auto n = outwardNormal(i->getGeometry(), elemPos);
			connectSide(elemPos, n, i->getSide1(), i->getConstruction().getResistanceSide1(), "R1=");
			connectSide(elemPos, n, i->getSide2(), i->getConstruction().getResistanceSide2(), "R2=");
			auto stubEnd = addCapacitorIcon(elemPos);
			if (mShowValues)
				this->addLabel(mValueLabels, &lbprops, "C=" + formatValue(i->getCapacitance()) + " J/K", stubEnd);
		}
		for (const auto& i : bp.getWindows())
		{
			geom::vertex elemPos = nodePos.at(i);
			this->addPoint(polygons, elemPos, pWindow, &lprops, nodeSizeSmall, "rc_window");
			//this->addLabel(labels, &lbprops, stateLabel(i), elemPos);
			auto n = outwardNormal(i->getGeometry(), elemPos);
			double Rwindow = 1.0/(2.0*i->getGlazing().getU());
			connectSide(elemPos, n, i->getSide1(), Rwindow, "R1=");
			connectSide(elemPos, n, i->getSide2(), Rwindow, "R2=");
			auto stubEnd = addCapacitorIcon(elemPos);
			if (mShowValues)
				this->addLabel(mValueLabels, &lbprops, "C=" + formatValue(i->getCapacitance()) + " J/K", stubEnd);
		}
	}
	else
	{
		std::stringstream errorMessage;
		errorMessage << "\nError, in bp_model visualization, the type of visualization is not recognized: " << mType << "\n"
								 << "(bso/visualization/models/bp_model.hpp)" << std::endl;
		throw std::runtime_error(errorMessage.str());
	}

	pbsp = new random_bsp(polygons);
}

BP_Model::~BP_Model()
{
	delete pbsp;
	for (auto& i : polygons) delete i;
	for (auto& i : labels) delete i;
	for (auto& i : mValueLabels) delete i;
	for (auto& i : pplist) delete i.second;
	polygons.clear();
	pplist.clear();
}

const std::string BP_Model::get_description()
{
	return mTitle;
}

bool BP_Model::key_pressed(int key)
{
	if (mType != "RC_model") return false;
	if (key == 'r' || key == 'R')
	{
		mShowValues = !mShowValues;
		return true;
	}
	return false;
}

void BP_Model::render(const camera &cam) const
{
	glPushAttrib(GL_ENABLE_BIT);

	glDisable(GL_DEPTH_TEST);
	if (std::any_of(pplist.begin(), pplist.end(), [](const auto& property)
		{ return property.second->translucent; }))
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	pbsp->render_btf(cam);

	for (auto& i : labels) i->render();
	if (mShowValues) for (auto& i : mValueLabels) i->render();

	if (mType == "RC_model" && !mLegendRows.empty())
	{
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
		const int panelW = 300;
		const int panelH = titleHeight + 14 + (int)mLegendRows.size() * (boxH + rowGap) + pad;
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
		std::string legendTitle = "RC network legend";
		for (char c : legendTitle) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);

		int rowIndex = 0;
		for (auto it = mLegendRows.rbegin(); it != mLegendRows.rend(); ++it, ++rowIndex)
		{
			int rowY = panelY + panelH - titleHeight - 12 - (rowIndex + 1) * (boxH + rowGap);

			glColor4f(it->first[0], it->first[1], it->first[2], 1.0f);
			glBegin(GL_QUADS);
			glVertex3i(panelX + pad, rowY, 0);
			glVertex3i(panelX + pad + boxW, rowY, 0);
			glVertex3i(panelX + pad + boxW, rowY + boxH, 0);
			glVertex3i(panelX + pad, rowY + boxH, 0);
			glEnd();

			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			glRasterPos3i(panelX + pad + textXOffset, rowY + 2, 0);
			for (char c : it->second) glutBitmapCharacter(GLUT_BITMAP_8_BY_13, c);
		}

		glPopAttrib();

		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
	}
	glPopAttrib();
}

} // namespace Visualisation
} // namespace BSO


#endif // VIS_BP_MODEL_HPP
