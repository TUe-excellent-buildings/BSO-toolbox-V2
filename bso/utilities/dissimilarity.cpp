#ifndef DISSIMILARITY_CPP
#define DISSIMILARITY_CPP

#include <vector>
#include <map>
#include <array>
#include <string>
#include <iostream>
#include <cmath>
#include <iomanip>
#include <fstream>
#include <stdexcept>

// Boost for graph structures
#include <boost/graph/adjacency_list.hpp>

// BSO Toolbox
#include <bso/spatial_design/ms_N_building.hpp>
#include <bso/utilities/graph_edit_distance.hpp>

// CGAL - for geometric operations
#include <CGAL/Polyhedron_3.h>
#include <CGAL/convex_hull_3.h>
#include <CGAL/Polygon_mesh_processing/corefinement.h>
#include <CGAL/Polygon_mesh_processing/measure.h>
#include <CGAL/Polygon_mesh_processing/intersection.h>

using namespace ged;

// CGAL typedefs
typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef Kernel::Point_3 Point_3;
typedef Kernel::Vector_3 Vector_3;
typedef CGAL::Polyhedron_3<Kernel> Polyhedron;
typedef Kernel::Point_2 Point_2;

constexpr double epsilon = 1e-9;

struct SpaceInfo {
	Polyhedron poly_space;
	Point_3 center_of_mass;
	std::vector<Point_3> points;
};

struct RotatedVariants {
	Polyhedron rot;		// Rotated
	Polyhedron mir_v;	// Vertical mirror
	Polyhedron mir_h;	// Horizontal mirror
	Polyhedron mir_vh;	// Vertical + Horizontal mirror
};

struct GEDResult {
	double cost;
    std::map<int, SpaceInfo> poly1;
    int num_nodes;
    int num_edges;
};

struct Stats {
    double min = std::numeric_limits<double>::infinity();
    double max = -std::numeric_limits<double>::infinity();
    double mean = 0.0;
};


// -------------------BEGIN CODE-------------------
double check(double value) {
	if (std::abs(value) < epsilon) {
		value = 0.0;
	}

	return value;
}


// Get number of spaces/rooms in a building design
int nspaces(const std::map<int, SpaceInfo>& space_polyhedra) {
	return space_polyhedra.size();
}


// Sort (3D) points counterclockwise (z ignored)
std::vector<Point_3> angularSort(const std::vector<Point_3>& points)
{
    if (points.empty()) return {};

    // Compute origin (average of x,y)
    double ox = 0, oy = 0;
    for (auto& v : points) {
        ox += v.x();
        oy += v.y();
    }
    ox /= points.size();
    oy /= points.size();

    const double refx = 0.0;
    const double refy = 1.0;

    auto angleKey = [&](const Point_3& v) {
        double vx = v.x() - ox;
        double vy = v.y() - oy;
        double det = (-refy) * vx + (refx) * vy;
        double dot = refx * vx + refy * vy;
        double angle = atan2(det, dot);
        if (angle < 0) angle += 2 * M_PI;
        return angle;
    };

    std::vector<Point_3> sortedPoints = points;
    std::sort(sortedPoints.begin(), sortedPoints.end(),
              [&](const Point_3& a, const Point_3& b){
                  return angleKey(a) < angleKey(b);
              });

    return sortedPoints;
}


// Calculate the area-weighted centroid "center-of-mass"
Point_3 getCenterOfMass(const std::vector<Point_3>& vertices) {
    // 1. Take first 4 vertices (bottom polygon)
	auto bottom = std::vector<Point_3>{vertices.begin(), vertices.begin() + 4};

	// 2. Sort counterclockwise/angular
	bottom = angularSort(bottom);

    //  3. Build 2D polygon points
    std::vector<Point_2> poly2d;
    for (auto& v : bottom)
        poly2d.emplace_back(v.x(), v.y());

    // 4. Compute area-weighted centroid
    double A = 0.0;
    double Cx = 0.0;
    double Cy = 0.0;
    int n = poly2d.size();
    for (int i = 0; i < n; ++i) {
        const auto& p = poly2d[i];
        const auto& q = poly2d[(i + 1) % n];
        double cross = p.x() * q.y() - q.x() * p.y();
        A += cross;
        Cx += (p.x() + q.x()) * cross;
        Cy += (p.y() + q.y()) * cross;
    }
    A *= 0.5;
    if (std::abs(A) < 1e-12)
        throw std::runtime_error("Polygon area too small or degenerate");
    Cx /= (6.0 * A);
    Cy /= (6.0 * A);

    // 5. Compute average Z over all vertices
    double z_avg = 0.0;
    for (auto& v : vertices)
        z_avg += v.z();
    z_avg /= vertices.size();

    return { Cx, Cy, z_avg };
}


// Return bottom polygon points of a polyhedron
std::vector<Point_3> get_bottom_face(const Polyhedron& poly, double tol = 1e-6) {
    std::vector<Point_3> bottom;
    if (poly.size_of_vertices() == 0) return bottom;

    // Find minimum z
    double min_z = std::numeric_limits<double>::infinity();
    for (auto v = poly.vertices_begin(); v != poly.vertices_end(); ++v)
        min_z = std::min(min_z, v->point().z());

    // Collect all points with z close to min_z
    for (auto v = poly.vertices_begin(); v != poly.vertices_end(); ++v) {
        if (std::abs(v->point().z() - min_z) < tol)
            bottom.push_back(v->point());
    }

    return bottom;
}


// Get the centroid by combining the above two functions to apply directly to a target polyhedron
Point_3 getCentroid(const Polyhedron& poly) {
    auto bottom = get_bottom_face(poly);

    // Get the footprint centroid
    Point_3 C2 = getCenterOfMass(bottom);
    double Cx = C2.x();
    double Cy = C2.y();

    // Get the height
    double minZ =  std::numeric_limits<double>::infinity();
    double maxZ = -std::numeric_limits<double>::infinity();
    for (auto v = poly.vertices_begin(); v != poly.vertices_end(); ++v) {
        minZ = std::min(minZ, v->point().z());
        maxZ = std::max(maxZ, v->point().z());
    }

    return Point_3(Cx, Cy, 0.5 * (minZ + maxZ));
}


// Get the bounding-box center of a polygon in the xy-plane
Point_3 bbox_center_xy(const Polyhedron& poly) {
    double minx =  std::numeric_limits<double>::infinity();
    double maxx = -std::numeric_limits<double>::infinity();
    double miny =  std::numeric_limits<double>::infinity();
    double maxy = -std::numeric_limits<double>::infinity();

    for (auto v = poly.vertices_begin(); v != poly.vertices_end(); ++v) {
        const auto& p = v->point();
        minx = std::min(minx, p.x());
        maxx = std::max(maxx, p.x());
        miny = std::min(miny, p.y());
        maxy = std::max(maxy, p.y());
    }

    return Point_3(0.5 * (minx + maxx), 0.5 * (miny + maxy), 0.0);
}


// Very important function!
Polyhedron transform_polyhedron(const Polyhedron& poly, double angle_deg, bool mirror_x, bool mirror_y) {
	// 1. Get bounding-box center for affine rotation,
	// 2. Perform rotation
	// 3. Get area-weighted centroid
	// 4. Mirror w.r.t. the origin over given axis
	// 5. Translate mirrored/rotated result back using area-weighted centroid

	// 1. Bounding-box center (Shapely origin="center")
	Point_3 C = bbox_center_xy(poly);
    const double x0 = C.x();
    const double y0 = C.y();

    // Rotation parameters
    const double a = angle_deg * M_PI / 180.0;
    double c = std::cos(a);
    double s = std::sin(a);

    // Match Shapely's thresholding
    if (std::abs(c) < 2.5e-16) c = 0.0;
    if (std::abs(s) < 2.5e-16) s = 0.0;

    // Define affine offset
    const double xoff = x0 - x0 * c + y0 * s;
    const double yoff = y0 - x0 * s - y0 * c;

    Polyhedron result = poly;

    // 2. Apply 2D rotation
    for (auto v = result.vertices_begin(); v != result.vertices_end(); ++v) {
        const auto& p = v->point();

        const double x = p.x();
        const double y = p.y();

        const double xr = c * x - s * y + xoff;
        const double yr = s * x + c * y + yoff;

        v->point() = Point_3(xr, yr, p.z());
    }

	// 3. Get area-weighted centroid after rotation
    Point_3 C_rot = getCentroid(result);
	const double cx = C_rot.x();
	const double cy = C_rot.y();

    // 4. Mirroring (about the origin)
    if (mirror_x || mirror_y) {
        for (auto v = result.vertices_begin(); v != result.vertices_end(); ++v) {
            auto p = v->point();
            double x = p.x();
            double y = p.y();

			if (mirror_x) x = -x;   // horizontal mirror
			if (mirror_y) y = -y;   // vertical mirror

			v->point() = Point_3(x, y, p.z());
        }

        // Compute the centroid AFTER mirroring
        Point_3 C_mir = getCentroid(result);

        // Translate back to the match the rotated centroid
        const double dx = C_mir.x() - cx;
        const double dy = C_mir.y() - cy;

        for (auto v = result.vertices_begin(); v != result.vertices_end(); ++v) {
			auto p = v->point();
			v->point() = Point_3(p.x() - dx, p.y() - dy, p.z());
        }
    }

    return result;
}


// Calculate volumetric intersection
double intersec(const Polyhedron& poly1, const Polyhedron& poly2, int n_rot) {
	// Get area-weighted centroid and displacement vector
    const Point_3 com1 = getCentroid(poly1);
    const Point_3 com2 = getCentroid(poly2);
    const Vector_3 va = com1 - com2;

    std::vector<double> overlaps;
    overlaps.reserve(4 * n_rot);

    for (int i = 0; i < n_rot; ++i) {
        double angle_deg = 360.0 * i / n_rot;

		// Get rotation variants
        Polyhedron variants[4] = {
            transform_polyhedron(poly2, angle_deg, false, false),
            transform_polyhedron(poly2, angle_deg, true,  false),
            transform_polyhedron(poly2, angle_deg, false, true),
            transform_polyhedron(poly2, angle_deg, true,  true)
        };

        for (int v = 0; v < 4; ++v) {
            Polyhedron moved = variants[v];

            if (!CGAL::Polygon_mesh_processing::is_outward_oriented(moved)) {
				CGAL::Polygon_mesh_processing::reverse_face_orientations(moved);
            }

            for (auto vert = moved.vertices_begin(); vert != moved.vertices_end(); ++vert) {
                const Point_3& p = vert->point();
                vert->point() = Point_3(
                    p.x() + va.x(),
                    p.y() + va.y(),
                    p.z() + va.z()
                );
            }

			// Define placeholder copies
            Polyhedron poly1_copy = poly1;
            Polyhedron moved_copy = moved;
            Polyhedron inter;

            // Compute volumetric intersection
            CGAL::Polygon_mesh_processing::corefine_and_compute_intersection(poly1_copy, moved_copy, inter);

            double volume = 0.0;
            if (!inter.empty()) {
                volume = std::abs(CGAL::Polygon_mesh_processing::volume(inter));
            }
            overlaps.push_back(volume);
        }
    }

    const double maximum = *std::max_element(overlaps.begin(), overlaps.end());
    const double volume_sum = CGAL::Polygon_mesh_processing::volume(poly1) +
							  CGAL::Polygon_mesh_processing::volume(poly2);

    return 1.0 - maximum / (volume_sum - maximum);
}

// Function to check whether two spaces are coplanar, i.e. do not share volume but do touch
bool polyhedra_touch(const Polyhedron& A, const Polyhedron& B) {
    for (auto fA = A.facets_begin(); fA != A.facets_end(); ++fA) {
        auto hA = fA->halfedge();
        std::vector<Point_3> vertsA;

        do {
            vertsA.push_back(hA->vertex()->point());
            hA = hA->next();
        } while(hA != fA->halfedge());

        for (auto fB = B.facets_begin(); fB != B.facets_end(); ++fB) {
            auto hB = fB->halfedge();
            std::vector<Point_3> vertsB;

            do {
                vertsB.push_back(hB->vertex()->point());
                hB = hB->next();
            } while(hB != fB->halfedge());

            // Check intersection of these two triangles/polygons
            if (CGAL::Polygon_mesh_processing::do_intersect(vertsA, vertsB))
                return true;
        }
    }
    return false;
}


Graph graph_con_spaces_boost(const std::map<int, SpaceInfo>& building) {
    const int n = nspaces(building);
    // Create graph with n vertices
    Graph g(n);
    for (int i = 1; i <= n; ++i) {
        for (int j = i + 1; j <= n; ++j) {
            const Polyhedron& A = building.at(i).poly_space;
            const Polyhedron& B = building.at(j).poly_space;
            if (CGAL::Polygon_mesh_processing::do_intersect(A, B) || polyhedra_touch(A, B)) {
                // Convert 1-based IDs to 0-based vertex indices
                boost::add_edge(i - 1, j - 1, g);
            }
        }
    }
    return g;
}


std::map<int, SpaceInfo> relabel_building(const std::map<int, SpaceInfo>& A, const std::map<int, int>& nm) {
	std::map<int, SpaceInfo> relabeled;

	for (const auto& [i, j] : nm) {
		// Get current position of SpaceInfo
		auto it = A.find(i);
        if (it == A.end()) {
			throw std::runtime_error("Invalid node mapping: space ID " + std::to_string(i) + " not found in building");
		}

		// Copy SpaceInfo and reassign under new ID
		relabeled[j] = it->second;
	}

	return relabeled;
}


// Compute GED and remapped poly
GEDResult optimized_GED_poly(const std::map<int, SpaceInfo>& A, const std::map<int, SpaceInfo>& B) {
	// Create boost graphs
	Graph G1 = graph_con_spaces_boost(A);
	Graph G2 = graph_con_spaces_boost(B);

	GEDConfig config;
	// Set costs explicitly - set high del/ins cost to avoid those operations unless strictly necessary
	config.node_subst_cost = [](Vertex u, Vertex v, const Graph& G1, const Graph& G2) {
		return 0.0;  // Substitution is free
	};
	config.edge_subst_cost = [](Edge e1, Edge e2, const Graph& G1, const Graph& G2) {
		return 0.0;  // Edge substitution is free
	};
	config.edge_del_cost = [](Edge e, const Graph& G1) {
		return 1.0;  // Edge deletion costs 1
	};
	config.edge_ins_cost = [](Edge e, const Graph& G2) {
		return 1.0;  // Edge insertion costs 1
	};

    if (nspaces(A) == nspaces(B)) {
        config.node_del_cost = [](Vertex u, const Graph& G1) {
		return 100.0;  // Deletion costs 100
	};
	config.node_ins_cost = [](Vertex v, const Graph& G2) {
		return 100.0;  // Insertion costs 100
	};
    } else {
        config.node_del_cost = [](Vertex u, const Graph& G1) {
		return 1.0;  // Deletion costs 1
	};
	config.node_ins_cost = [](Vertex v, const Graph& G2) {
		return 1.0;  // Insertion costs 1
	};
    }

	GraphEditDistance ged(G1, G2, config);

	EditPath optimal = ged.get_optimal_edit_path();

	// Build space ID vectors
	std::vector<int> space_ids_1, space_ids_2;
	for (const auto& [id, info] : A) {
		space_ids_1.push_back(id);
	}
	for (const auto& [id, info] : B) {
		space_ids_2.push_back(id);
	}

	// Build the space mapping
	std::map<int, int> space_mapping;

	for (const auto& edit : optimal.node_edits) {
		if (edit.u >= 0 && edit.v >= 0) {
			// Substitution
			space_mapping[space_ids_1[edit.u]] = space_ids_2[edit.v];
		}
	}

    // // Print space mapping u -> v
    // std::cout << "Space mapping (A -> B):" << std::endl;
    // for (const auto& [u, v] : space_mapping) {
    //     std::cout << "  " << u << " -> " << v << std::endl;
    // }

	int num_vertices = boost::num_vertices(G1);
	int num_edges = boost::num_edges(G1);
	std::map<int, SpaceInfo> relabeled_building = relabel_building(A, space_mapping);

	// Prepare result
    GEDResult result;
    result.cost = optimal.cost;
    result.poly1 = relabeled_building;
    result.num_nodes = num_vertices;
    result.num_edges = num_edges;

    return result;
}


double geometrical_dis(const std::map<int, SpaceInfo>& A, const std::map<int, SpaceInfo>& B, int n_rot) {
	int n = std::min(A.size(), B.size());
	if (n == 0) return 0.0;

	double sum = 0.0;

	for (int i = 1; i <= n; ++i) {
		const Polyhedron& poly1 = A.at(i).poly_space;
		const Polyhedron& poly2 = B.at(i).poly_space;

		sum += intersec(poly1, poly2, n_rot);
	}

	return sum / static_cast<double>(n);
}


double building_diagonal(const std::map<int, SpaceInfo>& building) {
    double x_min =  std::numeric_limits<double>::infinity();
    double x_max = -std::numeric_limits<double>::infinity();
    double y_min =  std::numeric_limits<double>::infinity();
    double y_max = -std::numeric_limits<double>::infinity();
    double z_min =  std::numeric_limits<double>::infinity();
    double z_max = -std::numeric_limits<double>::infinity();

    for (const auto& [id, space] : building) {
        for (const auto& p : space.points) {
            x_min = std::min(x_min, p.x());
            x_max = std::max(x_max, p.x());
            y_min = std::min(y_min, p.y());
            y_max = std::max(y_max, p.y());
            z_min = std::min(z_min, p.z());
            z_max = std::max(z_max, p.z());
        }
    }

    double dx = x_max - x_min;
    double dy = y_max - y_min;
    double dz = z_max - z_min;

    return std::sqrt(dx*dx + dy*dy + dz*dz);
}


double transposition_dis(const std::map<int, SpaceInfo>& A_remap, const std::map<int, SpaceInfo>& buildingA, const std::map<int, SpaceInfo>& buildingB) {
	const int n = std::min(buildingA.size(), buildingB.size());
	if (n == 0) return 0.0;

	double t = 0.0;

	// Compare matching spaces only
	for (int i = 1; i <= n; ++i) {
		const Point_3& a = getCentroid(A_remap.at(i).poly_space);
		const Point_3& b = getCentroid(buildingB.at(i).poly_space);
		const Vector_3 dis = a - b;

		t += std::sqrt(dis.x() * dis.x() +
					   dis.y() * dis.y() +
					   dis.z() * dis.z());
	}

	// Normalize by the building diagonal
	double diag1 = building_diagonal(buildingA);
	double diag2 = building_diagonal(buildingB);
	double norm = std::max(diag1, diag2);
	if (norm == 0.0) return 0.0;

	return (t / static_cast<double>(n)) / norm;
}


std::array<double, 3> opt_vec_dis(const std::map<int, SpaceInfo>& buildingA, const std::map<int, SpaceInfo>& buildingB, int r = 3) {
	// Part 1 - Topological dissimilarity
	GEDResult x = optimized_GED_poly(buildingA, buildingB);
	double part1 = x.cost / (x.num_nodes + x.num_edges);
	std::map<int, SpaceInfo> A_remap = x.poly1;  // Take remap of buildingA for the next steps

	// Part 2 - Geometrical dissimilarity part 1
	double part2 = geometrical_dis(A_remap, buildingB, r);

	// Part 3 - Geometrical dissimilarity part 2 (Transposition dissimilarity)
	double part3 = transposition_dis(A_remap, buildingA, buildingB);

	// Check and correct values
	part1 = check(part1);
	part2 = check(part2);
	part3 = check(part3);

	std::array<double, 3> dissimilarity = {part1, part2, part3};

//	std::cout<< "Topological dissimilarity:   " << part1 << std::endl;
//	std::cout<< "Geometrical dissimilarity:   " << part2 << std::endl;
//	std::cout<< "Transposition dissimilarity: " << part3 << std::endl;
	return dissimilarity;
}


double scale_dissimilarity(const std::map<int, SpaceInfo>& buildingA, const std::map<int, SpaceInfo>& buildingB,
					       int r = 3, double w1 = 1, double w2 = 1, double w3 = 1) {
	double weightSum = w1 + w2 + w3;
	std::array<double, 3> dissimilarity = opt_vec_dis(buildingA, buildingB, r);

	double diss_value = (w1 * dissimilarity[0] + w2 * dissimilarity[1] + w3 * dissimilarity[2]) / weightSum;
//	std::cout << "Dissimilarity measure: " << diss_value << "\n" << std::endl;

	return diss_value;
}


std::map<int, SpaceInfo> getBuildingData(const bso::spatial_design::ms_N_building MS) {
	// Create polyhedra for all spaces
    std::map<int, SpaceInfo> space_polyhedra;

	for (const auto p : MS) {
		auto space = *p;
		int spaceID = (int)space.getID();

		// Collect points for this space
		std::vector<Point_3> points;
        for (const auto& v : space.getVertices()) {
            points.push_back(Point_3(v[0], v[1], v[2]));
		}

		// Create convex hull --> polyhedron
		Polyhedron poly;
		CGAL::convex_hull_3(points.begin(), points.end(), poly);

		// Store results
		SpaceInfo space_info;
		space_info.poly_space = poly;
		space_info.points = points;
		space_polyhedra[spaceID] = space_info;
	}

	return space_polyhedra;
}


// -------------------------------------------------- //
// EXPOSED FUNCTIONS
// -------------------------------------------------- //
double calculate_dissimilarity(const bso::spatial_design::ms_N_building MS1, const bso::spatial_design::ms_N_building MS2) {
	// Load MS model A
	std::map<int, SpaceInfo> buildingA = getBuildingData(MS1);
	// Load MS model B
	std::map<int, SpaceInfo> buildingB = getBuildingData(MS2);
	double result = scale_dissimilarity(buildingA, buildingB);
	return result;
}

double calculate_dissimilarity_weighted(const bso::spatial_design::ms_N_building MS1, const bso::spatial_design::ms_N_building MS2, 
                                        int rot = 3, double w1 = 1, double w2 = 1, double w3 = 1) {
    // Load MS model A
    std::map<int, SpaceInfo> buildingA = getBuildingData(MS1);
    // Load MS model B
    std::map<int, SpaceInfo> buildingB = getBuildingData(MS2);
    double result = scale_dissimilarity(buildingA, buildingB, rot, w1, w2, w3);
    return result;
}


std::array<double, 3> calculate_dissimilarity_components(const bso::spatial_design::ms_N_building MS1, const bso::spatial_design::ms_N_building MS2) {
    // Load MS model A
    std::map<int, SpaceInfo> buildingA = getBuildingData(MS1);
    // Load MS model B
    std::map<int, SpaceInfo> buildingB = getBuildingData(MS2);
    std::array<double, 3> result = opt_vec_dis(buildingA, buildingB);
    return result;
}


#endif // DISSIMILARITY_CPP
