#ifndef SD_MODEL_HPP
#define SD_MODEL_HPP

#include <ostream>
#include <sstream>
#include <bso/structural_design/fea.hpp>
#include <bso/structural_design/component/point.hpp>
#include <bso/structural_design/component/line_segment.hpp>
#include <bso/structural_design/component/quadrilateral.hpp>
#include <bso/structural_design/component/triangle.hpp>
#include <bso/structural_design/component/quad_hexahedron.hpp>

namespace bso { namespace structural_design {
	
	struct sd_results;
	
	class sd_model
	{
	private:
		std::vector<component::point*> mPoints;
		std::vector<component::geometry*> mGeometries;
		std::vector<component::point*> mMeshedPoints;
		
		fea* mFEA;
		std::streambuf* mTopOptStreamBuffer;
		
		unsigned int mMeshSize = 1;
		bool mIsMeshed = false;
		void clearMesh();
	public:
		sd_model(); // Empty constructor, assignment of value's within class is done in the sd_grammar
		sd_model(const sd_model& rhs); // Copy constructor
		// sd_model& operator=(const sd_model& rhs); // Copy assignment operator
		~sd_model(); // Deconstructor
		
		component::point* addPoint(bso::utilities::geometry::vertex p);
		component::geometry* addGeometry(const bso::utilities::geometry::line_segment& g);
		component::geometry* addGeometry(const bso::utilities::geometry::quadrilateral& g);
		component::geometry* addGeometry(const bso::utilities::geometry::triangle& g);
		component::geometry* addGeometry(const bso::utilities::geometry::quad_hexahedron& g);
		
		void setMeshSize(const unsigned int& n);
		void mesh();
		void mesh(const unsigned int& n, bool meshLoadPanels = true);
		void analyze(std::string solver = "SimplicialLDLT");
		bool isStable();
		
		void rescaleStructuralVolume(const double& scaleFactor);
		void setElementDensities(const double& volumeFraction, const double& penalty);
		
		template <typename T, typename...ARGS>
		void topologyOptimization(const ARGS&...);
		void setTopOptOutputStream(std::ostream& out);
		
		sd_results getTotalResults();
		sd_results getPartialResults(bso::utilities::geometry::polygon* geom);
		sd_results getPartialResults(bso::utilities::geometry::polyhedron* geom);
		sd_results getPartialResults(bso::utilities::geometry::quad_hexahedron* geom);
		// NOT DONE YET: Batched partial results.
		// Batched partial results: compute sd_results for many spaces in one
		// pass over the FEA elements to reduce repeated work compared to
		// calling the single-space getPartialResults in a loop.
		// To indicate, for 50 spaces the getPartialResults() would require
		// 50 passes over the FEA elements, previously taking 5 minutes. 
		// Utilizing the getPartialResultsBatch() reduces this to 1 pass over the FEA elements, taking 
		// std::vector<sd_results> getPartialResultsBatch(
		// 	const std::vector<bso::utilities::geometry::polygon*>& geoms);
		// std::vector<sd_results> getPartialResultsBatch(
		// 	const std::vector<bso::utilities::geometry::polyhedron*>& geoms);
		double getTotalLoad();
		
		fea* getFEA() {return mFEA;}
		fea* getFEA() const {return mFEA;}
		const std::vector<component::point*>& getPoints() const {return mPoints;}
		const std::vector<component::geometry*> getGeometries() const {return mGeometries;}
	};
	
	struct sd_results
	{
		double mTotalStrainEnergy = 0.0;
		double mAxialStrainEnergy = 0.0;
		double mShearStrainEnergy = 0.0;
		double mBendStrainEnergy = 0.0;
		double mTotalStructuralVolume = 0.0;
		double mGhostStrainEnergy = 0.0;
		double mGhostStructuralVolume = 0.0;
		// std::map<element::load_case,double> strainEnergyPerLoadCase = {};
	};
	
} // namespace structural_design
} // namespace bso

#include <bso/structural_design/sd_model.cpp>

#endif // SD_MODEL_HPP