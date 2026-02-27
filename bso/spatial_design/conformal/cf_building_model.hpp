#ifndef CF_BUILDING_MODEL_HPP
#define CF_BUILDING_MODEL_HPP

#include <bso/spatial_design/ms_building.hpp>
#include <bso/spatial_design/ms_N_building.hpp>

namespace bso { namespace spatial_design { namespace conformal {
	
	class cf_building_model : public cf_geometry_model
	{
	private:
		std::vector<cf_point*> mCFPoints;
		std::vector<cf_edge*> mCFEdges;
		std::vector<cf_surface*> mCFSurfaces;
		std::vector<cf_space*> mCFSpaces;

		// Iterative partition method functions for orthogonal cuboid BSDs
		void addSpace(const ms_space& msSpace); // defines an initial geometry conformal model to partition and a finished building conformal model
		void addSpace(const ms_space& msSpace, std::vector<utilities::geometry::quad_hexahedron> quadHexa); // Add space with known conformal cuboids, no need for making it conformal!
		void makeConformal(); // orthogonal BSD partitioning method
		void makeConformal(std::vector<utilities::geometry::vertex> addConstrainVertexes); // orthogonal BSD partitioning method, considering additional constrains, for example for the windows.
		
		// Triangulation partitioning method functions for non-orthogonal BSDs (see public addSpaceD function for last function used for the method)
		void addSpaceD(const ms_space& msSpace, std::vector<utilities::geometry::triangular_prism> triPrismPtr); // Generates the final building- and geometry-conformal model after partitioning.
		void makeConformalND(const ms_N_building& msModel, const std::vector<utilities::geometry::vertex>& addConstrainVertexes = std::vector<utilities::geometry::vertex>()); // Initiates the triangulation partitioning method, optionally considering additional constrains (e.g., for windows).

		// safe it, in case copy constructor is called
		ms_building mMSModel; 
		ms_N_building mMSNModel; 
		std::vector<utilities::geometry::triangular_prism> mTriPrismInput;
		std::vector<utilities::geometry::quad_hexahedron> mQuadHexaInput;
		std::vector<bso::utilities::geometry::vertex> mConstrainVertexes;
		std::vector<bso::utilities::geometry::vertex> mAddDelaunayConstrains; // vertices which should be added as constrain points for the delaunay triangulation, for example to make sure that the partitioning is conformal to the window openings.
		std::string mfileNameDLSettings = ""; 
		std::map<unsigned int, std::vector<bso::utilities::geometry::vertex>> mConstrainsPerSpaceQH;
		std::map<unsigned int, std::vector<bso::utilities::geometry::vertex>> mConstrainsPerSpaceTP;

		double mTol;
		
		// safe it, in case of an iterm visualization the Triangulation partitioning method is required
		std::vector<utilities::geometry::quad_hexahedron> mSpacesAtGround; // spaces representation used to find intersection and constrain lines on the z=0 plane for 2D constrained delaunay aplication
		std::vector<utilities::geometry::line_segment> delauneyLines; //Lines where the constrained delaney triangulation should constrain to
		std::vector<utilities::geometry::triangle> delaunayTri; //resulting delaunay triangles at z=0 for triPrism definition in the geometry conformal model
		
		cf_building_model& operator = (cf_building_model& rhs) = default;
		friend class cf_geometry_entity;
	public:
		cf_building_model(const cf_building_model& rhs); //copy constructor
		cf_building_model(const ms_building& msModel, const double& tol = 1e-3);
		cf_building_model(const ms_building& msModel, const std::vector<utilities::geometry::quad_hexahedron>& quadHexa, const double& tol = 1e-3); // Insert an already valid conformal geometry represented in quad_hexehedrons
		cf_building_model(const ms_N_building& msNModel, const double& tol = 1e-3);
		cf_building_model(const ms_N_building& msNModel, const std::vector<utilities::geometry::triangular_prism>& triPrismPtr, const double& tol = 1e-3); // Insert an already valid conformal geometry represented in triangular prims
		
		// Below, conformal making functions which include the constrains of windows are defined
		// In case of daylight analysis, the window definition should be included in the conformal model,
		// as properties can only be assigned the the whole of a rectangle/triangle, and not to a part of it.
		// This way, the other disciplines can also use this base geometry.  
		cf_building_model(const ms_building& msModel, const std::string& fileNameDLSettings, const double& tol = 1e-3); 
		cf_building_model(const ms_N_building& msNModel, const std::string& fileNameDLSettings, const double& tol = 1e-3);
		
		~cf_building_model();		

		const std::vector<cf_point*			>& cfPoints() 		const { return mCFPoints;}
		const std::vector<cf_edge*			>& cfEdges() 			const { return mCFEdges;}
		const std::vector<cf_surface*		>& cfSurfaces() 	const { return mCFSurfaces;}
		const std::vector<cf_space*			>& cfSpaces() 		const { return mCFSpaces;}
		
		void writeToFile(std::string fileName, int writeOption = 1) const;

		// Functions used to allow for visualization of the interim stages of the Delaunay method
		const std::vector<utilities::geometry::quad_hexahedron> getSpacesAtGround() const {return mSpacesAtGround;} 
		const std::vector<utilities::geometry::line_segment> getDelauneyLines() const {return delauneyLines;} 
		const std::vector<utilities::geometry::triangle> getDelaunayTri() const {return delaunayTri;} 

		//function to give access to 
		const std::map<unsigned int, std::vector<bso::utilities::geometry::vertex>> getConstrainsPerSpaceQH() {return mConstrainsPerSpaceQH;}
		const std::map<unsigned int, std::vector<bso::utilities::geometry::vertex>> getConstrainsPerSpaceTP() {return mConstrainsPerSpaceTP;}
		
		friend std::ostream& operator << (std::ostream& stream, const cf_building_model& building);
	};
	
} // conformal
} // spatial_design
} // bso

#endif // CF_BUILDING_MODEL_HPP