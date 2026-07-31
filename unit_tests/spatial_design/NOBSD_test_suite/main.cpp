#include <iostream>
#include <cmath>
#include <string>
#include <vector>

//#include <bso/spatial_design/ms_building.hpp>
#include <bso/spatial_design/ms_N_building.hpp>
//#include <bso/spatial_design/cf_building.hpp>
//#include <bso/structural_design/sd_model.hpp>
//#include <bso/building_physics/bp_model.hpp>
//#include <bso/grammar/grammar.hpp>
#include <bso/visualization/visualization.hpp>

int end, begin = 0;
template<class T>
void out(const T& t, const bool& e = false,
				 const bool& i = false, const bool& verbose = false){
	if (!verbose) return;
	std::cout << t;
	end = clock();
	if (i) std::cout << " (" << 1000*(end-begin)/CLOCKS_PER_SEC << " ms)";
	if (e) std::cout << std::endl;
	begin = end;
} // out()

namespace
{
void visualizeExtrudeDebugStages(
	const bso::spatial_design::extrude_debug_data& dbg,
	const std::string& titlePrefix,
	const bso::utilities::geometry::vertex& location,
	bso::utilities::geometry::vector direction,
	const double lineWidth = 1.0);

std::vector<bso::utilities::geometry::line_segment> makePlaneDirectionMarker(
	const bso::utilities::geometry::vertex& location,
	bso::utilities::geometry::vector direction,
	const double length = 12000.0,
	const bool withArrow = false)
{
	std::vector<bso::utilities::geometry::line_segment> lines;
	if (direction.isZero(1e-9)) return lines;
	direction.normalize();
	auto tip = location + direction * length;
	lines.push_back({location, tip});

	if (!withArrow) return lines;

	bso::utilities::geometry::vector reference = {0, 0, 1};
	if (std::abs(direction.dot(reference)) > 0.9) reference = {0, 1, 0};
	auto side = direction.cross(reference);
	if (side.isZero(1e-9)) side = direction.cross(bso::utilities::geometry::vector({1, 0, 0}));
	side.normalize();
	auto arrowLength = length * 0.12;
	auto arrowWidth = length * 0.05;
	lines.push_back({tip, tip - direction * arrowLength + side * arrowWidth});
	lines.push_back({tip, tip - direction * arrowLength - side * arrowWidth});
	return lines;
}

bso::utilities::geometry::quadrilateral makePlaneOverlayQuad(
	const bso::spatial_design::shift_fill_plane_debug_data& dbg,
	const bso::utilities::geometry::vertex& location,
	bso::utilities::geometry::vector direction,
	const double margin = 2000.0)
{
	direction.normalize();
	bso::utilities::geometry::vector reference = {0, 0, 1};
	if (std::abs(direction.dot(reference)) > 0.9) reference = {0, 1, 0};
	auto axisU = direction.cross(reference);
	if (axisU.isZero(1e-9)) axisU = direction.cross(bso::utilities::geometry::vector({1, 0, 0}));
	axisU.normalize();
	auto axisV = direction.cross(axisU);
	axisV.normalize();

	bool hasPoint = false;
	double minU = 0.0;
	double maxU = 0.0;
	double minV = 0.0;
	double maxV = 0.0;
	auto consider = [&](const auto& space)
	{
		for (const auto& point : space)
		{
			auto relative = point - location;
			double u = relative.dot(axisU);
			double v = relative.dot(axisV);
			if (!hasPoint)
			{
				minU = maxU = u;
				minV = maxV = v;
				hasPoint = true;
			}
			else
			{
				if (u < minU) minU = u;
				if (u > maxU) maxU = u;
				if (v < minV) minV = v;
				if (v > maxV) maxV = v;
			}
		}
	};

	for (const auto& space : dbg.selectedSpaces) consider(space);
	for (const auto& space : dbg.notSelectedSpaces) consider(space);

	if (!hasPoint) return {};

	minU -= margin;
	maxU += margin;
	minV -= margin;
	maxV += margin;

	return {
		location + axisU * minU + axisV * minV,
		location + axisU * maxU + axisV * minV,
		location + axisU * maxU + axisV * maxV,
		location + axisU * minU + axisV * maxV
	};
}

bso::utilities::geometry::quadrilateral makePlaneOverlayQuadFromSpaces(
	const std::vector<bso::utilities::geometry::quad_hexahedron>& selectedSpaces,
	const std::vector<bso::utilities::geometry::quad_hexahedron>& notSelectedSpaces,
	const bso::utilities::geometry::vertex& location,
	bso::utilities::geometry::vector direction,
	const double margin = 2000.0)
{
	direction.normalize();
	bso::utilities::geometry::vector reference = {0, 0, 1};
	if (std::abs(direction.dot(reference)) > 0.9) reference = {0, 1, 0};
	auto axisU = direction.cross(reference);
	if (axisU.isZero(1e-9)) axisU = direction.cross(bso::utilities::geometry::vector({1, 0, 0}));
	axisU.normalize();
	auto axisV = direction.cross(axisU);
	axisV.normalize();

	bool hasPoint = false;
	double minU = 0.0;
	double maxU = 0.0;
	double minV = 0.0;
	double maxV = 0.0;
	auto consider = [&](const auto& space)
	{
		for (const auto& point : space)
		{
			auto relative = point - location;
			double u = relative.dot(axisU);
			double v = relative.dot(axisV);
			if (!hasPoint)
			{
				minU = maxU = u;
				minV = maxV = v;
				hasPoint = true;
			}
			else
			{
				if (u < minU) minU = u;
				if (u > maxU) maxU = u;
				if (v < minV) minV = v;
				if (v > maxV) maxV = v;
			}
		}
	};

	for (const auto& space : selectedSpaces) consider(space);
	for (const auto& space : notSelectedSpaces) consider(space);

	if (!hasPoint) return {};

	minU -= margin;
	maxU += margin;
	minV -= margin;
	maxV += margin;

	return {
		location + axisU * minU + axisV * minV,
		location + axisU * maxU + axisV * minV,
		location + axisU * maxU + axisV * maxV,
		location + axisU * minU + axisV * maxV
	};
}

void visualizeShiftFillPlaneDebugStages(
	const bso::spatial_design::shift_fill_plane_debug_data& dbg,
	const std::string& titlePrefix,
	const bso::utilities::geometry::vertex& location,
	const bso::utilities::geometry::vector& direction,
	const double lineWidth = 1.0)
{
	auto makePropsFromColor = [](const bso::visualization::rgba& c)
	{
		bso::visualization::polygon_props p;
		p.ambient = c;
		p.diffuse = c;
		p.specular = c;
		p.emission = bso::visualization::rgba(0.0f, 0.0f, 0.0f, 0.0f);
		p.shininess = 60.0f;
		p.translucent = (c.a < 1.0f);
		p.twosided = true;
		return p;
	};

	auto makeNotSelectedProps = []()
	{
		bso::visualization::polygon_props p;
		p.ambient = bso::visualization::rgba(0.1f, 0.5f, 0.1f, 0.3f);
		p.diffuse = bso::visualization::rgba(0.2f, 1.0f, 0.2f, 0.3f);
		p.specular = bso::visualization::rgba(0.2f, 1.0f, 0.2f, 0.3f);
		p.emission = bso::visualization::rgba(0.0f, 0.0f, 0.0f, 0.0f);
		p.shininess = 60.0f;
		p.translucent = true;
		p.twosided = true;
		return p;
	};

	auto addPlaneMarkerLayer = [&](std::vector<bso::visualization::GEOMETRY_RgbaLayer>& layers, const bool includePlane)
	{
		if (includePlane)
		{
			auto plane = makePlaneOverlayQuad(dbg, location, direction);
			if (plane.begin() != plane.end())
			{
				bso::visualization::GEOMETRY_RgbaLayer planeLayer;
				planeLayer.style = bso::visualization::rgba(0.25f, 0.35f, 1.0f, 0.18f);
				planeLayer.quads = {plane};
				layers.push_back(std::move(planeLayer));
			}
		}

		bso::visualization::GEOMETRY_RgbaLayer planeMarkerLayer;
		planeMarkerLayer.style = bso::visualization::rgba(0.05f, 0.1f, 0.9f, 1.0f);
		planeMarkerLayer.lineWidth = 2.5;
		planeMarkerLayer.lines = makePlaneDirectionMarker(location, direction, 12000.0, includePlane);
		layers.push_back(std::move(planeMarkerLayer));
	};

	if (!dbg.selectedSpaces.empty() || !dbg.notSelectedSpaces.empty())
	{
		std::vector<bso::visualization::GEOMETRY_RgbaLayer> layers;
		addPlaneMarkerLayer(layers, true);

		// // to shifted spaces: orange/gold highlight with transparency, spaces with marker "highlightInVisualization" will be visualized with this color
		// ppropsHighlight.ambient = rgba(1.0f, 0.65f, 0.2f, 0.3f);
		// ppropsHighlight.diffuse = rgba(1.0f, 0.65f, 0.2f, 0.3f);
		// ppropsHighlight.specular = rgba(1.0f, 0.65f, 0.2f, 0.3f);
		// ppropsHighlight.emission = rgba(0.0f, 0.0f, 0.0f, 0.0f);
		// ppropsHighlight.shininess = 60.0f;
		// ppropsHighlight.translucent = true;
		// ppropsHighlight.twosided = true;

		// // to non-shifted spaces: clear green highlight with transparency, default color of spaces
		// pprops.ambient = rgba(0.1f, 0.5f, 0.1f, 0.3f);
		// pprops.diffuse = rgba(0.2f, 1.0f, 0.2f, 0.3f);
		// pprops.specular = rgba(0.2f, 1.0f, 0.2f, 0.3f);
		// pprops.emission = rgba(0.0f, 0.0f, 0.0f, 0.0f);
		// pprops.shininess = 60.0f;
		// pprops.translucent = true;
		// pprops.twosided = true;

		bso::visualization::GEOMETRY_RgbaLayer refLayer1;
		refLayer1.style = bso::visualization::rgba(1.0f, 0.65f, 0.2f, 0.3f);
		refLayer1.QHs = dbg.selectedSpaces;
		layers.push_back(std::move(refLayer1));

		bso::visualization::GEOMETRY_RgbaLayer refLayer2;
		refLayer2.style = bso::visualization::rgba(0.1f, 0.5f, 0.1f, 0.3f);
		refLayer2.QHs = dbg.notSelectedSpaces;
		layers.push_back(std::move(refLayer2));
		
		bso::visualization::visualize(layers, titlePrefix + "_stage2_postDedup_toughing_non_selected", lineWidth);
	}

	if (!dbg.toughingSQuadsPostDedup.empty() || !dbg.toughingNSQuadsPostDedup.empty())
	{
		std::vector<bso::visualization::GEOMETRY_RgbaLayer> layers;

		bso::visualization::GEOMETRY_RgbaLayer QuadLayerNotSelected;
		QuadLayerNotSelected.style = bso::visualization::rgba(0.05f, 0.05f, 1.00f, 1.00f);
		QuadLayerNotSelected.lineWidth = lineWidth;
		QuadLayerNotSelected.quads = dbg.toughingNSQuadsPostDedup;
		layers.push_back(std::move(QuadLayerNotSelected));


		bso::visualization::GEOMETRY_RgbaLayer refLayer1;
		refLayer1.style = bso::visualization::rgba(0.35f, 0.35f, 0.35f, 0.10f);
		refLayer1.QHs = dbg.selectedSpaces;
		layers.push_back(std::move(refLayer1));

		bso::visualization::GEOMETRY_RgbaLayer refLayer2;
		refLayer2.style = bso::visualization::rgba(0.35f, 0.35f, 0.35f, 0.10f);
		refLayer2.QHs = dbg.notSelectedSpaces;
		layers.push_back(std::move(refLayer2));
		
		bso::visualization::visualize(layers, titlePrefix + "_stage2_postDedup_toughing_non_selected", lineWidth);
	}

	if (!dbg.toughingSQuadsPostDedup.empty() || !dbg.toughingNSQuadsPostDedup.empty())
	{
		std::vector<bso::visualization::GEOMETRY_RgbaLayer> layers;

		bso::visualization::GEOMETRY_RgbaLayer QuadLayerNotSelected;
		QuadLayerNotSelected.style = bso::visualization::rgba(0.05f, 0.05f, 1.00f, 1.00f);
		QuadLayerNotSelected.lineWidth = lineWidth;
		QuadLayerNotSelected.quads = dbg.toughingNSQuadsPostDedup;
		layers.push_back(std::move(QuadLayerNotSelected));
		
		bso::visualization::visualize(layers, titlePrefix + "_stage2_postDedup_toughing_non_selected", lineWidth);
	}

	if (!dbg.toughingSQuadsPostDedup.empty() || !dbg.toughingNSQuadsPostDedup.empty())
	{
		std::vector<bso::visualization::GEOMETRY_RgbaLayer> layers;
		bso::visualization::GEOMETRY_RgbaLayer QuadLayer;
		QuadLayer.style = bso::visualization::rgba(1.00f, 0.05f, 0.05f, 1.00f);
		QuadLayer.lineWidth = lineWidth;
		QuadLayer.quads = dbg.toughingSQuadsPostDedup;
		layers.push_back(std::move(QuadLayer));

		bso::visualization::GEOMETRY_RgbaLayer refLayer1;
		refLayer1.style = bso::visualization::rgba(0.35f, 0.35f, 0.35f, 0.10f);
		refLayer1.QHs = dbg.selectedSpaces;
		layers.push_back(std::move(refLayer1));

		bso::visualization::GEOMETRY_RgbaLayer refLayer2;
		refLayer2.style = bso::visualization::rgba(0.35f, 0.35f, 0.35f, 0.10f);
		refLayer2.QHs = dbg.notSelectedSpaces;
		layers.push_back(std::move(refLayer2));
		
		bso::visualization::visualize(layers, titlePrefix + "_stage2_postDedup_toughing_selected", lineWidth);
	}

	if (!dbg.toughingSQuadsPostDedup.empty() || !dbg.toughingNSQuadsPostDedup.empty())
	{
		std::vector<bso::visualization::GEOMETRY_RgbaLayer> layers;
		bso::visualization::GEOMETRY_RgbaLayer QuadLayer;
		QuadLayer.style = bso::visualization::rgba(1.00f, 0.05f, 0.05f, 1.00f);
		QuadLayer.lineWidth = lineWidth;
		QuadLayer.quads = dbg.toughingSQuadsPostDedup;
		layers.push_back(std::move(QuadLayer));
		
		bso::visualization::visualize(layers, titlePrefix + "_stage2_postDedup_toughing_selected", lineWidth);
	}

	if (!dbg.extrudingQuadsPostSplit.empty() || !dbg.extrudingQuadsPostSplit.empty())
	{
		std::vector<bso::visualization::GEOMETRY_RgbaLayer> layers;

		bso::visualization::GEOMETRY_RgbaLayer QuadLayerExtrude;
		QuadLayerExtrude.style = bso::visualization::rgba(1.00f, 0.05f, 0.05f, 1.00f);
		QuadLayerExtrude.lineWidth = lineWidth;
		QuadLayerExtrude.quads = dbg.extrudingQuadsPostSplit;
		layers.push_back(std::move(QuadLayerExtrude));

		bso::visualization::GEOMETRY_RgbaLayer refLayer1;
		refLayer1.style = bso::visualization::rgba(0.35f, 0.35f, 0.35f, 0.10f);
		refLayer1.QHs = dbg.selectedSpaces;
		layers.push_back(std::move(refLayer1));

		bso::visualization::GEOMETRY_RgbaLayer refLayer2;
		refLayer2.style = bso::visualization::rgba(0.35f, 0.35f, 0.35f, 0.10f);
		refLayer2.QHs = dbg.notSelectedSpaces;
		layers.push_back(std::move(refLayer2));
		
		bso::visualization::visualize(layers, titlePrefix + "_stage2_postDedup_toughing", lineWidth);
	}

	if (!dbg.linesAtFloorFlattened.empty())
	{
		std::vector<bso::visualization::GEOMETRY_RgbaLayer> layers;
		bso::visualization::GEOMETRY_RgbaLayer lineLayer;
		lineLayer.style = bso::visualization::rgba(1.00f, 0.05f, 0.05f, 1.00f);
		lineLayer.lineWidth = lineWidth + 3.0;
		lineLayer.lines = dbg.linesAtFloorFlattened;
		layers.push_back(std::move(lineLayer));

		if (!dbg.linesAtFloorReferenceSpaces.empty())
		{
			bso::visualization::GEOMETRY_RgbaLayer refLayer;
			refLayer.style = bso::visualization::rgba(0.35f, 0.35f, 0.35f, 0.10f);
			refLayer.QHs = dbg.linesAtFloorReferenceSpaces;
			layers.push_back(std::move(refLayer));
		}

		bso::visualization::visualize(layers, titlePrefix + "_stage3_linesAtFloor_with_reference", lineWidth);
	}
}

void shiftFillPlaneWithDebugStages(
	bso::spatial_design::ms_N_building& ms,
	const bso::utilities::geometry::vertex& location,
	const bso::utilities::geometry::vector& direction,
	const double shift,
	const std::string& titlePrefix,
	const double tol = 1e-3)
{
	bso::spatial_design::shift_fill_plane_debug_data dbg;
	try
	{
		ms.shiftFillPlane(location, direction, shift, true, 20.0, tol, true, &dbg);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception during shiftFillPlane: " << e.what() << std::endl;
		visualizeShiftFillPlaneDebugStages(dbg, titlePrefix + "_partial_error", location, direction);
		return;
	}
	visualizeShiftFillPlaneDebugStages(dbg, titlePrefix, location, direction);
}

void shiftFillPlaneCentreWithDebugStages(
	bso::spatial_design::ms_N_building& ms,
	const double shift,
	const std::string& titlePrefix,
	const bool shiftInMostHeadingDirection = false,
	const int amountOfAzimuthCategories = 6,
	const double tol = 1e-3)
{
	bso::spatial_design::shift_fill_plane_centre_debug_data dbg;
	auto visualizeCentre = [&]()
	{
		if (!dbg.selectedSpaces.empty() || !dbg.notSelectedSpaces.empty() || !dbg.partialSpaces.empty())
		{
			std::vector<bso::visualization::GEOMETRY_RgbaLayer> layers;

			auto plane = makePlaneOverlayQuadFromSpaces(dbg.selectedSpaces, dbg.notSelectedSpaces, dbg.location, dbg.appliedDirection);
			if (plane.begin() != plane.end())
			{
				bso::visualization::GEOMETRY_RgbaLayer planeLayer;
				planeLayer.style = bso::visualization::rgba(0.25f, 0.35f, 1.0f, 0.18f);
				planeLayer.quads = {plane};
				layers.push_back(std::move(planeLayer));
			}

			bso::visualization::GEOMETRY_RgbaLayer markerLayer;
			markerLayer.style = bso::visualization::rgba(0.05f, 0.1f, 0.9f, 1.0f);
			markerLayer.lineWidth = 2.5;
			markerLayer.lines = makePlaneDirectionMarker(dbg.location, dbg.appliedDirection, 12000.0, true);
			layers.push_back(std::move(markerLayer));

			if (!dbg.selectedSpaces.empty())
			{
				bso::visualization::GEOMETRY_RgbaLayer selectedLayer;
				selectedLayer.style = bso::visualization::rgba(1.0f, 0.65f, 0.2f, 0.30f);
				selectedLayer.QHs = dbg.selectedSpaces;
				layers.push_back(std::move(selectedLayer));
			}
			if (!dbg.partialSpaces.empty())
			{
				bso::visualization::GEOMETRY_RgbaLayer partialLayer;
				partialLayer.style = bso::visualization::rgba(1.0f, 0.92f, 0.20f, 0.30f);
				partialLayer.QHs = dbg.partialSpaces;
				layers.push_back(std::move(partialLayer));
			}
			if (!dbg.notSelectedSpaces.empty())
			{
				bso::visualization::GEOMETRY_RgbaLayer notSelectedLayer;
				notSelectedLayer.style = bso::visualization::rgba(0.1f, 0.5f, 0.1f, 0.30f);
				notSelectedLayer.QHs = dbg.notSelectedSpaces;
				layers.push_back(std::move(notSelectedLayer));
			}

			bso::visualization::visualize(layers, titlePrefix + "_centre_stage0_selection", 1.0);
		}

		if (dbg.usedShiftSelectedSide || !dbg.selectedSideShiftDebug.selectedSpaces.empty() || !dbg.selectedSideShiftDebug.notSelectedSpaces.empty())
		{
			visualizeShiftFillPlaneDebugStages(dbg.selectedSideShiftDebug, titlePrefix + "_centre_selectedSide", dbg.location, dbg.appliedDirection);
		}
		if (dbg.usedShiftNonSelectedSide || !dbg.nonSelectedSideShiftDebug.selectedSpaces.empty() || !dbg.nonSelectedSideShiftDebug.notSelectedSpaces.empty())
		{
			visualizeShiftFillPlaneDebugStages(dbg.nonSelectedSideShiftDebug, titlePrefix + "_centre_nonSelectedSide", dbg.location, dbg.appliedDirection);
		}
		if (dbg.usedExtrude || !dbg.extrudeDebug.selectedSpaces.empty() || !dbg.extrudeDebug.notSelectedSpaces.empty() || !dbg.extrudeDebug.testSpaces.empty())
		{
			std::string suffix = dbg.flippedExtrudeDirection ? "_centre_extrude_flipped" : "_centre_extrude";
			visualizeExtrudeDebugStages(dbg.extrudeDebug, titlePrefix + suffix, dbg.location, dbg.appliedDirection);
		}
	};

	try
	{
		ms.shiftFillPlaneCentre(shift, shiftInMostHeadingDirection, amountOfAzimuthCategories, true, tol, &dbg);
	}
	catch (const std::exception& e)
	{
		std::cerr << "shiftFillPlaneCentre failed: " << e.what() << std::endl;
	}

	if (!dbg.selectedSideShiftError.empty())
	{
		std::cerr << "Selected-side centre shift path error: " << dbg.selectedSideShiftError << std::endl;
	}
	if (!dbg.nonSelectedSideShiftError.empty())
	{
		std::cerr << "Non-selected-side centre shift path error: " << dbg.nonSelectedSideShiftError << std::endl;
	}

	visualizeCentre();
}

void visualizeExtrudeDebugStages(
	const bso::spatial_design::extrude_debug_data& dbg,
	const std::string& titlePrefix,
	const bso::utilities::geometry::vertex& location,
	bso::utilities::geometry::vector direction,
	const double lineWidth)
{
	auto addPlaneMarkerLayer = [&](std::vector<bso::visualization::GEOMETRY_RgbaLayer>& layers, const bool includePlane)
	{
		if (includePlane)
		{
			auto plane = makePlaneOverlayQuadFromSpaces(dbg.selectedSpaces, dbg.notSelectedSpaces, location, direction);
			if (plane.begin() != plane.end())
			{
				bso::visualization::GEOMETRY_RgbaLayer planeLayer;
				planeLayer.style = bso::visualization::rgba(0.25f, 0.35f, 1.0f, 0.18f);
				planeLayer.quads = {plane};
				layers.push_back(std::move(planeLayer));
			}
		}

		bso::visualization::GEOMETRY_RgbaLayer planeMarkerLayer;
		planeMarkerLayer.style = bso::visualization::rgba(0.05f, 0.1f, 0.9f, 1.0f);
		planeMarkerLayer.lineWidth = 2.5;
		planeMarkerLayer.lines = makePlaneDirectionMarker(location, direction, 12000.0, includePlane);
		layers.push_back(std::move(planeMarkerLayer));
	};

	auto addContextLayers = [&](std::vector<bso::visualization::GEOMETRY_RgbaLayer>& layers)
	{
		if (!dbg.selectedSpaces.empty())
		{
			bso::visualization::GEOMETRY_RgbaLayer refLayer;
			refLayer.style = bso::visualization::rgba(0.35f, 0.35f, 0.35f, 0.10f);
			refLayer.QHs = dbg.selectedSpaces;
			layers.push_back(std::move(refLayer));
		}

		// if (!dbg.partialSpaces.empty())
		// {
		// 	bso::visualization::GEOMETRY_RgbaLayer refLayer;
		// 	refLayer.style = bso::visualization::rgba(0.55f, 0.45f, 0.1f, 0.18f);
		// 	refLayer.QHs = dbg.partialSpaces;
		// 	layers.push_back(std::move(refLayer));
		// }

		if (!dbg.notSelectedSpaces.empty())
		{
			bso::visualization::GEOMETRY_RgbaLayer refLayer;
			refLayer.style = bso::visualization::rgba(0.35f, 0.35f, 0.35f, 0.10f);
			refLayer.QHs = dbg.notSelectedSpaces;
			layers.push_back(std::move(refLayer));
		}
	};

	if (!dbg.selectedSpaces.empty() || !dbg.notSelectedSpaces.empty() || !dbg.partialSpaces.empty() || !dbg.fullyOnExtrusionSideQuads.empty() || !dbg.partiallyOnExtrusionSideQuads.empty() || !dbg.notOnExtrusionSideQuads.empty())
	{
		std::vector<bso::visualization::GEOMETRY_RgbaLayer> layers;
		addPlaneMarkerLayer(layers, true);

		if (!dbg.selectedSpaces.empty())
		{
			bso::visualization::GEOMETRY_RgbaLayer refLayer;
			refLayer.style = bso::visualization::rgba(0.35f, 0.35f, 0.35f, 0.10f);
			refLayer.QHs = dbg.selectedSpaces;
			layers.push_back(std::move(refLayer));
		}

		// if (!dbg.partialSpaces.empty())
		// {
		// 	bso::visualization::GEOMETRY_RgbaLayer refLayer;
		// 	refLayer.style = bso::visualization::rgba(0.55f, 0.45f, 0.1f, 0.18f);
		// 	refLayer.QHs = dbg.partialSpaces;
		// 	layers.push_back(std::move(refLayer));
		// }

		if (!dbg.notSelectedSpaces.empty())
		{
			bso::visualization::GEOMETRY_RgbaLayer refLayer;
			refLayer.style = bso::visualization::rgba(0.35f, 0.35f, 0.35f, 0.10f);
			refLayer.QHs = dbg.notSelectedSpaces;
			layers.push_back(std::move(refLayer));
		}

		bso::visualization::visualize(layers, titlePrefix + "_stage1_quads", lineWidth);
	}

	if (!dbg.selectedSpaces.empty() || !dbg.notSelectedSpaces.empty() || !dbg.partialSpaces.empty() || !dbg.fullyOnExtrusionSideQuads.empty() || !dbg.partiallyOnExtrusionSideQuads.empty() || !dbg.notOnExtrusionSideQuads.empty())
	{
		std::vector<bso::visualization::GEOMETRY_RgbaLayer> layers;
		addPlaneMarkerLayer(layers, true);
		addContextLayers(layers);

		if (!dbg.fullyOnExtrusionSideQuads.empty())
		{
			bso::visualization::GEOMETRY_RgbaLayer layer;
			layer.style = bso::visualization::rgba(0.10f, 0.35f, 1.00f, 0.90f); // blue color
			layer.lineWidth = lineWidth;
			layer.quads = dbg.fullyOnExtrusionSideQuads;
			layers.push_back(std::move(layer));
		}

		if (!dbg.partiallyOnExtrusionSideQuads.empty())
		{
			bso::visualization::GEOMETRY_RgbaLayer layer;
			layer.style = bso::visualization::rgba(1.0f, 1.0f, 0.2f, 0.9f); // yellow color
			layer.lineWidth = lineWidth;
			layer.quads = dbg.partiallyOnExtrusionSideQuads;
			layers.push_back(std::move(layer));
		}

		if (!dbg.notOnExtrusionSideQuads.empty())
		{
			bso::visualization::GEOMETRY_RgbaLayer layer;
			layer.style = bso::visualization::rgba(1.0f, 0.2f, 0.2f, 0.9f); // red color
			layer.lineWidth = lineWidth;
			layer.quads = dbg.notOnExtrusionSideQuads;
			layers.push_back(std::move(layer));
		}

		bso::visualization::visualize(layers, titlePrefix + "_stage1_quads", lineWidth);
	}

	if (!dbg.testSpaces.empty())
	{
		std::vector<bso::visualization::GEOMETRY_RgbaLayer> layers;
		addPlaneMarkerLayer(layers, true);
		addContextLayers(layers);

		if (!dbg.testSpaces.empty())
		{
			bso::visualization::GEOMETRY_RgbaLayer layer;
			layer.style = bso::visualization::rgba(1.0f, 1.0f, 0.2f, 0.9f); // yellow color
			layer.QHs = dbg.testSpaces;
			layers.push_back(std::move(layer));
		}

		bso::visualization::visualize(layers, titlePrefix + "_stage2_validity", lineWidth);
	}

	if (!dbg.testSpaces.empty() || !dbg.validTestSpaces.empty() || !dbg.rejectedTestSpaces.empty())
	{
		std::vector<bso::visualization::GEOMETRY_RgbaLayer> layers;
		addPlaneMarkerLayer(layers, true);
		addContextLayers(layers);

		// if (!dbg.testSpaces.empty())
		// {
		// 	bso::visualization::GEOMETRY_RgbaLayer layer;
		// 	layer.style = bso::visualization::rgba(0.65f, 0.65f, 0.65f, 0.12f);
		// 	layer.QHs = dbg.testSpaces;
		// 	layers.push_back(std::move(layer));
		// }

		if (!dbg.validTestSpaces.empty())
		{
			bso::visualization::GEOMETRY_RgbaLayer layer;
			layer.style = bso::visualization::rgba(0.10f, 0.35f, 1.00f, 0.90f); // blue color
			layer.QHs = dbg.validTestSpaces;
			layers.push_back(std::move(layer));
		}

		if (!dbg.rejectedTestSpaces.empty())
		{
			bso::visualization::GEOMETRY_RgbaLayer layer;
			layer.style = bso::visualization::rgba(1.0f, 0.2f, 0.2f, 0.9f); // red color
			layer.QHs = dbg.rejectedTestSpaces;
			layers.push_back(std::move(layer));
		}

		bso::visualization::visualize(layers, titlePrefix + "_stage2_validity", lineWidth);
	}
}

void extrudeWithDebugStages(
	bso::spatial_design::ms_N_building& ms,
	const bso::utilities::geometry::vertex& location,
	const bso::utilities::geometry::vector& direction,
	const double amount,
	const std::string& titlePrefix,
	const double tol = 1e-3)
{
	bso::spatial_design::extrude_debug_data dbg;
	try
	{
		ms.extrude(location, direction, amount, tol, &dbg);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception during extrude: " << e.what() << std::endl;
		visualizeExtrudeDebugStages(dbg, titlePrefix + "_partial_error", location, direction);
		return;
	}
	visualizeExtrudeDebugStages(dbg, titlePrefix, location, direction);
}
}

int main(int argc, char* argv[])
{   
    bso::visualization::initVisualization(argc, argv);
	out("Started visualizations",true,true,true);
    
	// Shift-fill-plane debugging visualizations:
	bool design_327 = false;
	bool design_x1 	= false;
	bool design_x1a = false;
	bool design_x2 	= false;
	bool design_x3 	= false;
	bool design_x4 	= false;
	bool design_x5 	= false;
	bool design_x6 	= false;
	bool design_x7 	= false;
	bool design_x9 	= false;
	bool design_x10 = false;
	bool design_x11 = false;
	bool design_x12 = false;

	// Extrude space debugging visualizations:
	bool design_x13 = false;
	bool design_x14 = false;
	bool design_x15 = false;

	// Shift-fill-plane-centre debugging visualizations:
	bool design_x16 = true;
	bool design_x17 = true;
	bool design_x18 = true;
	bool design_x8 	= true;
	

	// Shift-fill-plane debugging visualizations:
	if(design_327)
	{
		bso::spatial_design::ms_N_building ms("design_327");
		bso::visualization::visualize(ms);

		bso::utilities::geometry::vertex location = {7000,0,0};
		bso::utilities::geometry::vector direction = {1,0,0};
		double shift = 3000;
		shiftFillPlaneWithDebugStages(ms, location, direction, shift, "design_327_shiftFillPlane");
		ms.writeToFile("design_327_Linear_shift");
		bso::visualization::visualize(ms);
	}
	if(design_x1)
	{
		bso::spatial_design::ms_N_building ms("design_325_one_elevation");
		bso::visualization::visualize(ms);

		bso::utilities::geometry::vertex location = {3000,0,0};
		bso::utilities::geometry::vector direction = {1,-1,0}; //{1,0,0};
		double shift = sqrt(2) * 3000;
		shiftFillPlaneWithDebugStages(ms, location, direction, shift, "design_325_cornerDiagonalShift");
		ms.writeToFile("design_325_cornerDiagonalShift");
		std::cout << "shiftFillPlane() is done" << std::endl;
		bso::visualization::visualize(ms);
	}
	if(design_x1a)
	{
		bso::spatial_design::ms_N_building ms("design_325");
		bso::visualization::visualize(ms);

		bso::utilities::geometry::vertex location = {3000,0,0};
		bso::utilities::geometry::vector direction = {1,-1,0}; //{1,0,0};
		double shift = sqrt(2) * 3000;
		shiftFillPlaneWithDebugStages(ms, location, direction, shift, "design_325_cornerDiagonalShift");
		ms.writeToFile("design_325_cornerDiagonalShift");
		std::cout << "shiftFillPlane() is done" << std::endl;
		bso::visualization::visualize(ms);
	}
	if(design_x2)
	{
		bso::spatial_design::ms_N_building ms("design_325");
		bso::visualization::visualize(ms);

		bso::utilities::geometry::vertex location = {3000,0,0};
		bso::utilities::geometry::vector direction = {1,0,0};
		double shift = sqrt(2) * 3000;
		shiftFillPlaneWithDebugStages(ms, location, direction, shift, "design_325_cornerFullShift");
		ms.writeToFile("design_325_cornerFullShift");
		std::cout << "shiftFillPlane() is done" << std::endl;
		bso::visualization::visualize(ms);
	}
	if(design_x3)
	{
		bso::spatial_design::ms_N_building ms("design_325");
		bso::visualization::visualize(ms);

		bso::utilities::geometry::vertex location = {3000,0,0};
		bso::utilities::geometry::vector direction = {1,0,0};
		double shift = 3000;
		shiftFillPlaneWithDebugStages(ms, location, direction, shift, "design_325_cornerMatchShift");
		ms.writeToFile("design_325_cornerMatchShift");
		std::cout << "shiftFillPlane() is done" << std::endl;
		bso::visualization::visualize(ms);
	}
	if(design_x4)
	{
		bso::spatial_design::ms_N_building ms("design_325");
		bso::visualization::visualize(ms);

		bso::utilities::geometry::vertex location = {3000,0,0};
		bso::utilities::geometry::vector direction = {1,0,0};
		double shift = 1500;
		shiftFillPlaneWithDebugStages(ms, location, direction, shift, "design_325_cornerOverlapShift");
		ms.writeToFile("design_325_cornerOverlapShift");
		std::cout << "shiftFillPlane() is done" << std::endl;
		bso::visualization::visualize(ms);
	}
	if(design_x5)
	{
		bso::spatial_design::ms_N_building ms("design_325");
		bso::visualization::visualize(ms);

		bso::utilities::geometry::vertex location = {0,6000,0};
		bso::utilities::geometry::vector direction = {0,1,0};
		double shift = 3000;
		shiftFillPlaneWithDebugStages(ms, location, direction, shift, "design_325_shiftSlantedSide");
		ms.writeToFile("design_325_shiftSlantedSide");
		std::cout << "shiftFillPlane() is done" << std::endl;
		bso::visualization::visualize(ms);
	}
	if(design_x6)
	{
		bso::spatial_design::ms_N_building ms("design_308");
		bso::visualization::visualize(ms);

		bso::utilities::geometry::vertex location = {0,15000,0};
		bso::utilities::geometry::vector direction = {0,1,0};
		double shift = 6000;
		shiftFillPlaneWithDebugStages(ms, location, direction, shift, "design_308_splitLevelAndHorizontalOverlapShift");
		ms.writeToFile("design_308_splitLevelAndHorizontalOverlapShift");
		std::cout << "shiftFillPlane() is done" << std::endl;
		bso::visualization::visualize(ms);
	}
	if(design_x7)
	{
		bso::spatial_design::ms_N_building ms("design_308");
		bso::visualization::visualize(ms);

		bso::utilities::geometry::vertex location = {0,15000,0};
		bso::utilities::geometry::vector direction = {0,1,0};
		double shift = 6000;
		ms.duplicateElevation(3000);
		bso::visualization::visualize(ms);
		ms.duplicateElevation();
		bso::visualization::visualize(ms);
		ms.writeToFile("design_308_dupElevation");
		shiftFillPlaneWithDebugStages(ms, location, direction, shift, "design_308_dupElevation_shiftFillPlane");
		std::cout << "shiftFillPlane() is done" << std::endl;
		bso::visualization::visualize(ms);
	}
	if(design_x9)
	{
		bso::spatial_design::ms_N_building ms("design_308_higher_nr_5");
		bso::visualization::visualize(ms);

		bso::utilities::geometry::vertex location = {0,15000,0};
		bso::utilities::geometry::vector direction = {0,1,0};
		double shift = 6000;
		shiftFillPlaneWithDebugStages(ms, location, direction, shift, "design_308_higher_nr_5");
		std::cout << "shiftFillPlane() is done" << std::endl;
		bso::visualization::visualize(ms);
	}
	if(design_x10)
	{
		bso::spatial_design::ms_N_building ms("design_308_higher_nr_5_extended_nr_4");
		bso::visualization::visualize(ms);

		bso::utilities::geometry::vertex location = {0,15000,0};
		bso::utilities::geometry::vector direction = {0,1,0};
		double shift = 6000;
		shiftFillPlaneWithDebugStages(ms, location, direction, shift, "design_308_higher_nr_5_extended_nr_4");
		std::cout << "shiftFillPlane() is done" << std::endl;
		bso::visualization::visualize(ms);
	}
	if(design_x11)
	{
		bso::spatial_design::ms_N_building ms("design_split_level");
		bso::visualization::visualize(ms);

		bso::utilities::geometry::vertex location = {0,4000,0};
		bso::utilities::geometry::vector direction = {0,1,0};
		double shift = 6000;
		shiftFillPlaneWithDebugStages(ms, location, direction, shift, "design_split_level");
		std::cout << "shiftFillPlane() is done" << std::endl;
		bso::visualization::visualize(ms);
	}
	if(design_x12) // overview
	{
		bso::spatial_design::ms_N_building ms("overvieuw_design");
		bso::visualization::visualize(ms);

		bso::utilities::geometry::vertex location = {0,4000,0};
		bso::utilities::geometry::vector direction = {0,1,0};
		double shift = 6000;
		shiftFillPlaneWithDebugStages(ms, location, direction, shift, "overview_design");
		std::cout << "shiftFillPlane() is done" << std::endl;
		bso::visualization::visualize(ms);
	}

	// Extrude space debugging visualizations:
	if(design_x13)
	{
		bso::spatial_design::ms_N_building ms("design_325_one_elevation");
		bso::visualization::visualize(ms);

		bso::utilities::geometry::vertex location = {3000,0,0};
		bso::utilities::geometry::vector direction = {1,0,0};
		double amount = 3000;
		extrudeWithDebugStages(ms, location, direction, amount, "design_325_one_elevation_extrude");
		ms.writeToFile("design_325_one_elevation_extrude");
		std::cout << "extrude() is done" << std::endl;
		bso::visualization::visualize(ms);
	}
	if(design_x13)
	{
		bso::spatial_design::ms_N_building ms("design_325_one_elevation");
		bso::visualization::visualize(ms);

		bso::utilities::geometry::vertex location = {5000,0,0};
		bso::utilities::geometry::vector direction = {1,0,0};
		double amount = 3000;
		extrudeWithDebugStages(ms, location, direction, amount, "design_325_one_elevation_extrude");
		ms.writeToFile("design_325_one_elevation_extrude");
		std::cout << "extrude() is done" << std::endl;
		bso::visualization::visualize(ms);
	}
	if(design_x14)
	{
		bso::spatial_design::ms_N_building ms("design_308");
		bso::visualization::visualize(ms);

		bso::utilities::geometry::vertex location = {0,15000,0};
		bso::utilities::geometry::vector direction = {0,1,0};
		double amount = 6000;
		extrudeWithDebugStages(ms, location, direction, amount, "design_308_extrude");
		ms.writeToFile("design_308_extrude");
		std::cout << "extrude() is done" << std::endl;
		bso::visualization::visualize(ms);
	}
	if(design_x14)
	{
		bso::spatial_design::ms_N_building ms("design_308");
		bso::visualization::visualize(ms);

		bso::utilities::geometry::vertex location = {7000,30000,0};
		bso::utilities::geometry::vector direction = {2,1,0};
		double amount = 6000;
		extrudeWithDebugStages(ms, location, direction, amount, "design_308_extrude");
		ms.writeToFile("design_308_extrude");
		std::cout << "extrude() is done" << std::endl;
		bso::visualization::visualize(ms);
	}
	if(design_x14)
	{
		bso::spatial_design::ms_N_building ms("design_308");
		bso::visualization::visualize(ms);

		bso::utilities::geometry::vertex location = {0,30000,0};
		bso::utilities::geometry::vector direction = {1,1,0};
		double amount = 6000;
		extrudeWithDebugStages(ms, location, direction, amount, "design_308_extrude");
		ms.writeToFile("design_308_extrude");
		std::cout << "extrude() is done" << std::endl;
		bso::visualization::visualize(ms);
	}
	if(design_x14)
	{
		bso::spatial_design::ms_N_building ms("design_308");
		bso::visualization::visualize(ms);

		bso::utilities::geometry::vertex location = {7000,0,0};
		bso::utilities::geometry::vector direction = {1,0,0};
		double amount = 6000;
		extrudeWithDebugStages(ms, location, direction, amount, "design_308_extrude");
		ms.writeToFile("design_308_extrude");
		std::cout << "extrude() is done" << std::endl;
		bso::visualization::visualize(ms);
	}
	if(design_x14)
	{
		bso::spatial_design::ms_N_building ms("design_308");
		bso::visualization::visualize(ms);

		bso::utilities::geometry::vertex location = {15000,0,0};
		bso::utilities::geometry::vector direction = {1,0,0};
		double amount = 6000;
		extrudeWithDebugStages(ms, location, direction, amount, "design_308_extrude");
		ms.writeToFile("design_308_extrude");
		std::cout << "extrude() is done" << std::endl;
		bso::visualization::visualize(ms);
	}
	if(design_x15)
	{
		bso::spatial_design::ms_N_building ms("design_split_level");
		bso::visualization::visualize(ms);

		bso::utilities::geometry::vertex location = {0,4000,0};
		bso::utilities::geometry::vector direction = {0,1,0};
		double amount = 6000;
		extrudeWithDebugStages(ms, location, direction, amount, "design_split_level_extrude");
		ms.writeToFile("design_split_level_extrude");
		std::cout << "extrude() is done" << std::endl;
		bso::visualization::visualize(ms);
	}

	// Shift-fill-plane-centre debugging visualizations:
	if(design_x16)
	{
		bso::spatial_design::ms_N_building ms("design_308");
		bso::visualization::visualize(ms);

		shiftFillPlaneCentreWithDebugStages(ms, 5000, "design_308_centreShiftFillPlane_selectedHeading", false, 6);
		ms.writeToFile("design_308_centreShiftFillPlane_selectedHeading");
		std::cout << "shiftFillPlaneCentre() is done" << std::endl;
		bso::visualization::visualize(ms);
	}
	if(design_x17)
	{
		bso::spatial_design::ms_N_building ms("design_325_one_elevation");
		bso::visualization::visualize(ms);

		shiftFillPlaneCentreWithDebugStages(ms, 3000, "design_325_one_elevation_centreShiftFillPlane_headingDirection", true, 8);
		ms.writeToFile("design_325_one_elevation_centreShiftFillPlane_headingDirection");
		std::cout << "shiftFillPlaneCentre() is done" << std::endl;
		bso::visualization::visualize(ms);
	}
	if(design_x18)
	{
		bso::spatial_design::ms_N_building ms("design_split_level");
		bso::visualization::visualize(ms);

		shiftFillPlaneCentreWithDebugStages(ms, 6000, "design_split_level_centreShiftFillPlane", false, 6);
		ms.writeToFile("design_split_level_centreShiftFillPlane");
		std::cout << "shiftFillPlaneCentre() is done" << std::endl;
		bso::visualization::visualize(ms);
	}
	if(design_x8)
	{
		bso::spatial_design::ms_N_building ms("design_308");
		bso::visualization::visualize(ms);
		shiftFillPlaneCentreWithDebugStages(ms, 5000, "design_308_centreShiftFillPlane");
		ms.writeToFile("design_308_centreShiftFillPlane");
		std::cout << "shiftFillPlaneCentre() is done" << std::endl;
		bso::visualization::visualize(ms);

	}




    bso::visualization::endVisualization();
	out("End visualizations",true,true,true);

 return 0;
}