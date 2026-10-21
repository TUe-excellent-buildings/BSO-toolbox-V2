#include <iostream>
#include <string>

#include <bso/spatial_design/ms_building.hpp>
#include <bso/spatial_design/ms_N_building.hpp>
#include <bso/spatial_design/cf_building.hpp>
#include <bso/structural_design/sd_model.hpp>
#include <bso/building_physics/bp_model.hpp>
#include <bso/grammar/grammar.hpp>
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

// Function to subdivide a quad-hexahedron into multiple triangular prisms
std::vector<bso::utilities::geometry::triangular_prism> pTP(bso::utilities::geometry::quad_hexahedron QH, int nrPartitions)
{
	std::vector<bso::utilities::geometry::triangular_prism> pQuadHexahedron; 
	std::vector<bso::utilities::geometry::vertex> verticesQH = QH.getVertices();

	// Define all relevant vertices of future triangular prims along the four opposing ribs
	std::vector<bso::utilities::geometry::vertex> rib1; // lower two opposing ribs
	std::vector<bso::utilities::geometry::vertex> rib2; // lower two opposing ribs
	std::vector<bso::utilities::geometry::vertex> rib3; // top two opposing ribs
	std::vector<bso::utilities::geometry::vertex> rib4; // top two opposing ribs

	rib1.push_back(verticesQH[1]); 
	for(int j=1; j<= nrPartitions; j++)
	{
		bso::utilities::geometry::vertex tempv = verticesQH[1] + ((verticesQH[0]-verticesQH[1]) / (nrPartitions+1)) * j;
		rib1.push_back(tempv); 
	}
	rib1.push_back(verticesQH[0]); 

	rib2.push_back(verticesQH[2]); 
	for(int j=1; j<= nrPartitions; j++)
	{
		bso::utilities::geometry::vertex tempv = verticesQH[2] + ((verticesQH[3]-verticesQH[2]) / (nrPartitions+1)) * j;
		rib2.push_back(tempv); 
	}
	rib2.push_back(verticesQH[3]); 

	rib3.push_back(verticesQH[5]); 
	for(int j=1; j<= nrPartitions; j++)
	{
		bso::utilities::geometry::vertex tempv = verticesQH[5] + ((verticesQH[4]-verticesQH[5]) / (nrPartitions+1)) * j;
		rib3.push_back(tempv); 
	}
	rib3.push_back(verticesQH[4]); 

	rib4.push_back(verticesQH[6]); 
	for(int j=1; j<= nrPartitions; j++)
	{
		bso::utilities::geometry::vertex tempv = verticesQH[6] + ((verticesQH[7]-verticesQH[6]) / (nrPartitions+1)) * j;
		rib4.push_back(tempv); 
	}
	rib4.push_back(verticesQH[7]); 

	// Add triangular prisms to pQuadHexahedron
	for(int i=0; i<= nrPartitions; i++)
	{
		bso::utilities::geometry::triangular_prism temp1 = {{rib1[i]},{rib1[i+1]},{rib2[i]},{rib3[i]},{rib3[i+1]},{rib4[i]}};
		bso::utilities::geometry::triangular_prism temp2 = {{rib1[i+1]},{rib2[i]},{rib2[i+1]},{rib3[i+1]},{rib4[i]},{rib4[i+1]}};
		pQuadHexahedron.push_back(temp1);
		pQuadHexahedron.push_back(temp2);
	}

	return pQuadHexahedron;
}

// Function to subdivide a quad-hexahedron into multiple quad-hexahedrons
std::vector<bso::utilities::geometry::quad_hexahedron> pQH(bso::utilities::geometry::quad_hexahedron QH, int nrPartitions)
{
	std::vector<bso::utilities::geometry::quad_hexahedron> pQuadHexahedron; 
	std::vector<bso::utilities::geometry::vertex> verticesQH = QH.getVertices();

	// Define all relevant vertices of future quad-hexahedrons along the four opposing ribs
	std::vector<bso::utilities::geometry::vertex> rib1; // lower two opposing ribs
	std::vector<bso::utilities::geometry::vertex> rib2; // lower two opposing ribs
	std::vector<bso::utilities::geometry::vertex> rib3; // top two opposing ribs
	std::vector<bso::utilities::geometry::vertex> rib4; // top two opposing ribs

	rib1.push_back(verticesQH[0]); 
	for(int j=1; j<= nrPartitions; j++)
	{
		bso::utilities::geometry::vertex tempv = verticesQH[0] + ((verticesQH[1]-verticesQH[0]) / (nrPartitions+1)) * j;
		rib1.push_back(tempv); 
	}
	rib1.push_back(verticesQH[1]); 

	rib2.push_back(verticesQH[3]); 
	for(int j=1; j<= nrPartitions; j++)
	{
		bso::utilities::geometry::vertex tempv = verticesQH[3] + ((verticesQH[2]-verticesQH[3]) / (nrPartitions+1)) * j;
		rib2.push_back(tempv); 
	}
	rib2.push_back(verticesQH[2]); 

	rib3.push_back(verticesQH[4]); 
	for(int j=1; j<= nrPartitions; j++)
	{
		bso::utilities::geometry::vertex tempv = verticesQH[4] + ((verticesQH[5]-verticesQH[4]) / (nrPartitions+1)) * j;
		rib3.push_back(tempv); 
	}
	rib3.push_back(verticesQH[5]); 

	rib4.push_back(verticesQH[7]); 
	for(int j=1; j<= nrPartitions; j++)
	{
		bso::utilities::geometry::vertex tempv = verticesQH[7] + ((verticesQH[6]-verticesQH[7]) / (nrPartitions+1)) * j;
		rib4.push_back(tempv); 
	}
	rib4.push_back(verticesQH[6]); 

	// Add quad-hexahedrons to pQuadHexahedron
	for(int i=0; i<= nrPartitions; i++)
	{
		bso::utilities::geometry::quad_hexahedron temp1 = {{rib1[i]},{rib1[i+1]},{rib2[i]},{rib2[i+1]},{rib3[i]},{rib3[i+1]},{rib4[i]},{rib4[i+1]}};
		pQuadHexahedron.push_back(temp1);
	}

	return pQuadHexahedron;
}

// Function to subdivide a quad-hexahedron into multiple quad-hexahedrons
std::vector<bso::utilities::geometry::quad_hexahedron> pQHbase(bso::utilities::geometry::quad_hexahedron QH, int nrPartitions1, int nrPartitions2)
{	
	std::vector<bso::utilities::geometry::quad_hexahedron> pQuadHexahedron; 
	std::vector<bso::utilities::geometry::vertex> verticesQH = QH.getVertices();

	// Add quad-hexahedrons to pQuadHexahedron
	for(int i=0; i<= nrPartitions1; i++)
	{
		for(int j=0; j<= nrPartitions2; j++)
		{
			bso::utilities::geometry::vertex temp1 = verticesQH[0] + ((verticesQH[1]-verticesQH[0]) / (nrPartitions1+1)) * i + ((verticesQH[3]-verticesQH[0]) / (nrPartitions2+1)) * j;
			bso::utilities::geometry::vertex temp2 = verticesQH[0] + ((verticesQH[1]-verticesQH[0]) / (nrPartitions1+1)) * (i+1) + ((verticesQH[3]-verticesQH[0]) / (nrPartitions2+1)) * (j);
			bso::utilities::geometry::vertex temp3 = verticesQH[0] + ((verticesQH[1]-verticesQH[0]) / (nrPartitions1+1)) * (i+1) + ((verticesQH[3]-verticesQH[0]) / (nrPartitions2+1)) * (j+1);
			bso::utilities::geometry::vertex temp4 = verticesQH[0] + ((verticesQH[1]-verticesQH[0]) / (nrPartitions1+1)) * (i) + ((verticesQH[3]-verticesQH[0]) / (nrPartitions2+1)) * (j+1);
			bso::utilities::geometry::vertex temp5 = verticesQH[4] + ((verticesQH[5]-verticesQH[4]) / (nrPartitions1+1)) * i + ((verticesQH[7]-verticesQH[4]) / (nrPartitions2+1)) * j;
			bso::utilities::geometry::vertex temp6 = verticesQH[4] + ((verticesQH[5]-verticesQH[4]) / (nrPartitions1+1)) * (i+1) + ((verticesQH[7]-verticesQH[4]) / (nrPartitions2+1)) * (j);
			bso::utilities::geometry::vertex temp7 = verticesQH[4] + ((verticesQH[5]-verticesQH[4]) / (nrPartitions1+1)) * (i+1) + ((verticesQH[7]-verticesQH[4]) / (nrPartitions2+1)) * (j+1);
			bso::utilities::geometry::vertex temp8 = verticesQH[4] + ((verticesQH[5]-verticesQH[4]) / (nrPartitions1+1)) * (i) + ((verticesQH[7]-verticesQH[4]) / (nrPartitions2+1)) * (j+1);
			bso::utilities::geometry::quad_hexahedron tempQuadHexa = {temp1,temp2,temp3,temp4,temp5,temp6,temp7,temp8};
			pQuadHexahedron.push_back(tempQuadHexa);
		}
	}

	return pQuadHexahedron;
}


using namespace bso;
int main(int argc, char* argv[])
{
	std::cout.precision(17); //this precision only counts with cout not with the error message handling used in the toolbox

	// Toggle which results are requested:
	bool baseLineResults = false; // The benchmark results executed with the single orthogonal space representation (design_1 and design_2 in this directory)
	bool meshConvergencePerPartitionSizeTP = false; // A mesh convergence study where the size of all partitions are gradually increased.
	bool meshConvergencePerPartitionSizeQH = false; // A mesh convergence study where the size of all partitions are gradually increased.
	bool partitionShift = false; // A mesh convergence study where the size ratio between partitions is increased.
	bool fullBuilding = false; // A mesh convergence study for two orthogonal buildings
	bool NOBSDbuilding = false; // A mesh convergence study for all 20 NOBSD in the test suite.
	
	// Set variables used for studies
	int nrPartitions = 5; // Partitions subdivided to opposing sides with the generation of triangular prisms in between
	int nrMeshSteps = 15; // Amount of steps taken in mesh studies
	int nrMeshStepsFullBuilding = 8; // Amount of steps taken in mesh studies
	double tol = 0.001;	
	std::vector<int> nrPartitionsSteps = {{0},{3},{5},{10},{20},{30}};
	std::vector<int> nrPartitionsStepsRec = {{0},{1},{2},{3},{4}};
	std::vector<int> ratioSteps = {{1},{3},{5},{10},{20},{30}};

	// Initialize visualization for future different models
	bso::visualization::initVisualization(argc, argv);
	out("Started visualizations",true,true,true);

	// Generate base ms model utilizing the NOBSD representation
	bso::spatial_design::ms_N_building ms1("design_1");
	bso::visualization::visualize(ms1);
	out("Created an MS model",true,true,true);

	// Generate base ms model for utilizing the bench-mark orthogonal representation
	bso::spatial_design::ms_building ms2("design_2");
	bso::visualization::visualize(ms2);
	out("Created an MS model",true,true,true);

	// Setup the data stream:
	std::ofstream allFile;

	if(baseLineResults)
	{	
		bso::spatial_design::cf_building cfTpBase(ms1,tol); //Generate the basic conformal geometry
		out("Generated base CF model",true,true,true);
		bso::visualization::visualize(cfTpBase,"triangular_prism");
		out("Visualized base CF model",true,true,true);
		
		// Setup data file
		allFile.open ("TP_SD_mesh_dif", std::fstream::in | std::fstream::out | std::fstream::app);
		allFile << "ConformalGeom meshSize SDResult" << std::endl;
		allFile.close();

		// Write data to files:
		std::cout << "TP mesh size difference results: " << std::endl;
		for(int i=0; i<nrMeshSteps; i++)
		{
			bso::grammar::grammar gram1(cfTpBase);
			out("Created an TP mesh dif GRAM model",true,true,true);

			std::string base = "settings/sd_settings_";
			std::string integer = std::to_string(i);
			std::string end = ".txt";
			std::string settingFile = base + integer +end;	

			bso::structural_design::sd_model SDTPmesh = gram1.sd_grammar <bso::grammar::DEFAULT_SD_GRAMMAR >(std::string(settingFile));
			out("Created an TP mesh dif SD model",true,true,true);
			SDTPmesh.analyze();
			out("Analyzed an TP mesh dif SD model",true,true,true);
			std::cout << "Structural compliance: " << SDTPmesh.getTotalResults().mTotalStrainEnergy << "iteration i: " << i  << std::endl;

			allFile.open ("TP_SD_mesh_dif", std::fstream::in | std::fstream::out | std::fstream::app);
			allFile << "TP " << 2+i*2 << " " << std::setprecision(16) << SDTPmesh.getTotalResults().mTotalStrainEnergy << std::endl;
			allFile.close();


			if(i == 0) 
			{
				bso::visualization::visualize(SDTPmesh,"component");
				bso::visualization::visualize(SDTPmesh,"element");
			}
			if(i == nrMeshSteps-1) 
			{
				bso::visualization::visualize(SDTPmesh,"component");
				bso::visualization::visualize(SDTPmesh,"element");	
			}	
		}
	}
	
	if(meshConvergencePerPartitionSizeTP)
	{
		for(const int i: nrPartitionsSteps)
		{
			// Generate conformal geometry
			bso::spatial_design::ms_space* space = (ms1.getSpacePtrs())[0];
			bso::utilities::geometry::quad_hexahedron quadToPartition = space->getGeometry();
			std::vector<bso::utilities::geometry::triangular_prism> tempTP = pTP(quadToPartition, i);
			out("Get triangular prisms within space",true,true,true);

			// Generate conformal geometry
			bso::spatial_design::cf_building CFTP(ms1, tempTP, tol);
			out("Created an CF model",true,true,true);
			bso::visualization::visualize(CFTP,"triangular_prism");
			out("Visualized CF model",true,true,true);
			
			// Generate result file name
			std::string file = "TP_SD_mesh_dif_partition";
			std::string integer = std::to_string(i);
			std::string fileName = file + integer;	

			// Setup result file
			allFile.open (fileName, std::fstream::in | std::fstream::out | std::fstream::app);
			allFile << "ConformalGeom meshSize SDResult" << std::endl;
			allFile.close();
			
			// Generate convergence study results
			for(int i=0; i<nrMeshSteps; i++)
			{
				bso::grammar::grammar gram1(CFTP);
				out("Created an TP GRAM model",true,true,true);

				std::string base = "settings/sd_settings_";
				std::string integer = std::to_string(i);
				std::string end = ".txt";
				std::string settingFile = base + integer +end;	

				bso::structural_design::sd_model SDTPmesh = gram1.sd_grammar <bso::grammar::DEFAULT_SD_GRAMMAR >(std::string(settingFile));
				out("Created an TP SD model",true,true,true);		
				
				SDTPmesh.analyze();
				out("Analyzed an TP SD model",true,true,true);
				std::cout << "Structural compliance: " << SDTPmesh.getTotalResults().mTotalStrainEnergy << "iteration i: " << i << std::endl;

				allFile.open (fileName, std::fstream::in | std::fstream::out | std::fstream::app);
				allFile << "TP " << 2+i*2 << " " << std::setprecision(16) << SDTPmesh.getTotalResults().mTotalStrainEnergy << std::endl;
				allFile.close();

				if(true) //always visualize
				{
					bso::visualization::visualize(SDTPmesh,"component");
					bso::visualization::visualize(SDTPmesh,"element");
					continue;
				}
				if(i == 0) 
				{
					bso::visualization::visualize(SDTPmesh,"component");
					bso::visualization::visualize(SDTPmesh,"element");
				}
				if(i == nrMeshSteps-1) 
				{
					bso::visualization::visualize(SDTPmesh,"component");
					bso::visualization::visualize(SDTPmesh,"element");	
				}
			}
		}
	}

	if(partitionShift)
	{
		// Generate conformal geometry
		bso::spatial_design::ms_space* space = (ms1.getSpacePtrs())[0];
		bso::utilities::geometry::quad_hexahedron quadToPartition = space->getGeometry();
		std::vector<bso::utilities::geometry::vertex> verticesQH = quadToPartition.getVertices();
		
		for(const int i: ratioSteps)
		{
			int nrPartitions1 = i;
			int nrPartitions2 = 0;
			int j = 0;
			bso::utilities::geometry::vertex temp1 = verticesQH[0] + ((verticesQH[1]-verticesQH[0]) / (nrPartitions1+1)) * i + ((verticesQH[3]-verticesQH[0]) / (nrPartitions2+1)) * j;
			bso::utilities::geometry::vertex temp4 = verticesQH[0] + ((verticesQH[1]-verticesQH[0]) / (nrPartitions1+1)) * (i) + ((verticesQH[3]-verticesQH[0]) / (nrPartitions2+1)) * (j+1);
			bso::utilities::geometry::vertex temp5 = verticesQH[4] + ((verticesQH[5]-verticesQH[4]) / (nrPartitions1+1)) * i + ((verticesQH[7]-verticesQH[4]) / (nrPartitions2+1)) * j;
			bso::utilities::geometry::vertex temp8 = verticesQH[4] + ((verticesQH[5]-verticesQH[4]) / (nrPartitions1+1)) * (i) + ((verticesQH[7]-verticesQH[4]) / (nrPartitions2+1)) * (j+1);
			bso::utilities::geometry::quad_hexahedron tempQuadHexa1 = {temp1,verticesQH[0],verticesQH[3],temp4,temp5,verticesQH[4],verticesQH[7],temp8};
			bso::utilities::geometry::quad_hexahedron tempQuadHexa2 = {temp1,verticesQH[1],verticesQH[2],temp4,temp5,verticesQH[5],verticesQH[6],temp8};
			
			std::vector<bso::utilities::geometry::triangular_prism> tempTP1 = pTP(tempQuadHexa1, j);
			std::vector<bso::utilities::geometry::triangular_prism> tempTP2 = pTP(tempQuadHexa2, j);
			std::vector<bso::utilities::geometry::triangular_prism> tempTP;
			for(const auto k: tempTP1) tempTP.push_back(k);
			for(const auto k: tempTP2) tempTP.push_back(k);
			out("Get triangular prisms within space",true,true,true);

			// Generate conformal geometry
			bso::spatial_design::cf_building CFTP(ms1, tempTP, tol);
			out("Created an CF model",true,true,true);
			//bso::visualization::visualize(CFTP,"triangular_prism");
			//out("Visualized CF model",true,true,true);
			
			// Generate result file name
			std::string file = "TP_SD_mesh_dif_ratioStep";
			std::string integer = std::to_string(i);
			std::string fileName = file + integer;	

			// Setup result file
			allFile.open (fileName, std::fstream::in | std::fstream::out | std::fstream::app);
			allFile << "ConformalGeom meshSize SDResult" << std::endl;
			allFile.close();
			
			// Generate convergence study results
			for(int i=0; i<nrMeshSteps; i++)
			{
				bso::grammar::grammar gram1(CFTP);
				out("Created an TP GRAM model",true,true,true);

				std::string base = "settings/sd_settings_";
				std::string integer = std::to_string(i);
				std::string end = ".txt";
				std::string settingFile = base + integer +end;	

				bso::structural_design::sd_model SDTPmesh = gram1.sd_grammar <bso::grammar::DEFAULT_SD_GRAMMAR >(std::string(settingFile));
				out("Created an TP SD model",true,true,true);		
				
				SDTPmesh.analyze();
				out("Analyzed an TP SD model",true,true,true);
				std::cout << "Structural compliance: " << SDTPmesh.getTotalResults().mTotalStrainEnergy << "iteration i: " << i << std::endl;

				allFile.open (fileName, std::fstream::in | std::fstream::out | std::fstream::app);
				allFile << "TP " << 2+i*2 << " " << std::setprecision(16) << SDTPmesh.getTotalResults().mTotalStrainEnergy << std::endl;
				allFile.close();

				if(false) //always visualize
				{
					bso::visualization::visualize(SDTPmesh,"component");
					bso::visualization::visualize(SDTPmesh,"element");
					continue;
				}
				if(i == 0)
				{
					bso::visualization::visualize(SDTPmesh,"component");
					bso::visualization::visualize(SDTPmesh,"element");
					continue;
				}
				if(i == nrMeshSteps-1) 
				{
					bso::visualization::visualize(SDTPmesh,"component");
					bso::visualization::visualize(SDTPmesh,"element");	
				}
			}
		}
	}

	if(baseLineResults)
	{
		bso::spatial_design::cf_building cfQHBase(ms2,tol); //Generate the basic conformal geometry
		out("Generated base CF model",true,true,true);
		bso::visualization::visualize(cfQHBase,"cuboid");
		out("Visualized base CF model",true,true,true);
		
		// Setup the data files:
		allFile.open ("QH_SD_mesh_dif", std::fstream::in | std::fstream::out | std::fstream::app);
			allFile << "ConformalGeom meshSize SDResult" << std::endl;
			allFile.close();

		// Write data to files:
		std::cout << "QH mesh size difference results: " << std::endl;
		for(int i=0; i<nrMeshSteps; i++)
		{
			bso::grammar::grammar gram1(cfQHBase);
			out("Created an QH mesh dif GRAM model",true,true,true);

			std::string base = "settings/sd_settings_";
			std::string integer = std::to_string(i);
			std::string end = ".txt";
			std::string settingFile = base + integer +end;	

			bso::structural_design::sd_model SDQHmesh = gram1.sd_grammar <bso::grammar::DEFAULT_SD_GRAMMAR >(std::string(settingFile));
			out("Created an QH mesh dif SD model",true,true,true);
			SDQHmesh.analyze();
			out("Analyzed an QH mesh dif SD model",true,true,true);
			std::cout << "Structural compliance: " << SDQHmesh.getTotalResults().mTotalStrainEnergy << "iteration i: " << i << std::endl;

			allFile.open ("QH_SD_mesh_dif", std::fstream::in | std::fstream::out | std::fstream::app);
			allFile << "QH " << 2+i*2 << " " << std::setprecision(16) << SDQHmesh.getTotalResults().mTotalStrainEnergy << std::endl;
			allFile.close();

			if(i == 0) 
			{
				bso::visualization::visualize(SDQHmesh,"component");
				bso::visualization::visualize(SDQHmesh,"element");
			}
			if(i == nrMeshSteps-1) 
			{
				bso::visualization::visualize(SDQHmesh,"component");
				bso::visualization::visualize(SDQHmesh,"element");	
			}
		}
	}

	if(meshConvergencePerPartitionSizeQH)
	{
		for(const int i: nrPartitionsSteps)
		{
			// Generate conformal geometry
			bso::spatial_design::ms_space* space = (ms2.getSpacePtrs())[0];
			bso::utilities::geometry::quad_hexahedron quadToPartition = space->getGeometry();
			std::vector<bso::utilities::geometry::quad_hexahedron> tempQH = pQH(quadToPartition, i);
			out("Get triangular prisms within space",true,true,true);

			// Generate conformal geometry
			bso::spatial_design::cf_building CFQH(ms2, tempQH, tol);
			out("Created an CF model",true,true,true);
			//bso::visualization::visualize(CFQH,"cuboids");
			//out("Visualized CF model",true,true,true);
			
			// Generate result file name
			std::string file = "QH_SD_mesh_dif_partition";
			std::string integer = std::to_string(i);
			std::string fileName = file + integer;	

			// Setup result file
			allFile.open (fileName, std::fstream::in | std::fstream::out | std::fstream::app);
			allFile << "ConformalGeom meshSize SDResult" << std::endl;
			allFile.close();
			
			// Generate convergence study results
			for(int i=-1; i<nrMeshSteps; i++)
			{
				bso::grammar::grammar gram1(CFQH);
				out("Created an QH GRAM model",true,true,true);

				std::string base = "settings/sd_settings_";
				std::string integer = std::to_string(i);
				std::string end = ".txt";
				std::string settingFile = base + integer +end;	

				bso::structural_design::sd_model SDQH = gram1.sd_grammar <bso::grammar::DEFAULT_SD_GRAMMAR >(std::string(settingFile));
				out("Created an QH SD model",true,true,true);		
				
				SDQH.analyze();
				out("Analyzed an QH SD model",true,true,true);
				std::cout << "Structural compliance: " << SDQH.getTotalResults().mTotalStrainEnergy << "iteration i: " << i << std::endl;

				allFile.open (fileName, std::fstream::in | std::fstream::out | std::fstream::app);
				if(i==-1) allFile << "QH " << 1 << " " << std::setprecision(16) << SDQH.getTotalResults().mTotalStrainEnergy << std::endl;
				else allFile << "QH " << 2+i*2 << " " << std::setprecision(16) << SDQH.getTotalResults().mTotalStrainEnergy << std::endl;
				allFile.close();

				if(false) //always visualize
				{
					bso::visualization::visualize(SDQH,"component");
					bso::visualization::visualize(SDQH,"element");
					continue;
				}
				if(i == -1) 
				{
					bso::visualization::visualize(SDQH,"component");
					bso::visualization::visualize(SDQH,"element");
				}
				if(i == nrMeshSteps-1) 
				{
					bso::visualization::visualize(SDQH,"component");
					bso::visualization::visualize(SDQH,"element");	
				}
			}
		}
	}
	
	// Generate convergence study results of QH with different partition size ratios
	if(partitionShift)
	{
		// Generate conformal geometry
		bso::spatial_design::ms_space* space = (ms2.getSpacePtrs())[0];
		bso::utilities::geometry::quad_hexahedron quadToPartition = space->getGeometry();
		std::vector<bso::utilities::geometry::vertex> verticesQH = quadToPartition.getVertices();

		
		for(const int i: ratioSteps)
		{
			int nrPartitions1 = i;
			int nrPartitions2 = 0;
			int j = 0;
			bso::utilities::geometry::vertex temp1 = verticesQH[0] + ((verticesQH[1]-verticesQH[0]) / (nrPartitions1+1)) * i + ((verticesQH[3]-verticesQH[0]) / (nrPartitions2+1)) * j;
			bso::utilities::geometry::vertex temp4 = verticesQH[0] + ((verticesQH[1]-verticesQH[0]) / (nrPartitions1+1)) * (i) + ((verticesQH[3]-verticesQH[0]) / (nrPartitions2+1)) * (j+1);
			bso::utilities::geometry::vertex temp5 = verticesQH[4] + ((verticesQH[5]-verticesQH[4]) / (nrPartitions1+1)) * i + ((verticesQH[7]-verticesQH[4]) / (nrPartitions2+1)) * j;
			bso::utilities::geometry::vertex temp8 = verticesQH[4] + ((verticesQH[5]-verticesQH[4]) / (nrPartitions1+1)) * (i) + ((verticesQH[7]-verticesQH[4]) / (nrPartitions2+1)) * (j+1);
			bso::utilities::geometry::quad_hexahedron tempQuadHexa1 = {temp1,verticesQH[0],verticesQH[3],temp4,temp5,verticesQH[4],verticesQH[7],temp8};
			bso::utilities::geometry::quad_hexahedron tempQuadHexa2 = {temp1,verticesQH[1],verticesQH[2],temp4,temp5,verticesQH[5],verticesQH[6],temp8};
			
			std::vector<bso::utilities::geometry::quad_hexahedron> tempQH;
			tempQH.push_back(tempQuadHexa1);
			tempQH.push_back(tempQuadHexa2);
			out("Get triangular prisms within space",true,true,true);

			// Generate conformal geometry
			bso::spatial_design::cf_building CFQH(ms2, tempQH, tol);
			out("Created an CF model",true,true,true);
			//bso::visualization::visualize(CFQH,"triangular_prism");
			//out("Visualized CF model",true,true,true);
			
			// Generate result file name
			std::string file = "QH_QH_mesh_dif_ratioStep";
			std::string integer = std::to_string(i);
			std::string fileName = file + integer;	

			// Setup result file
			allFile.open (fileName, std::fstream::in | std::fstream::out | std::fstream::app);
			allFile << "ConformalGeom meshSize SDResult" << std::endl;
			allFile.close();
			
			
			// Generate convergence study results
			for(int i=-1; i<nrMeshSteps; i++)
			{
				bso::grammar::grammar gram1(CFQH);
				out("Created an QH GRAM model",true,true,true);

				std::string base = "settings/sd_settings_";
				std::string integer = std::to_string(i);
				std::string end = ".txt";
				std::string settingFile = base + integer +end;	

				bso::structural_design::sd_model SDQHmesh = gram1.sd_grammar <bso::grammar::DEFAULT_SD_GRAMMAR >(std::string(settingFile));
				out("Created an QH SD model",true,true,true);		
				
				SDQHmesh.analyze();
				out("Analyzed an QH SD model",true,true,true);
				std::cout << "Structural compliance: " << SDQHmesh.getTotalResults().mTotalStrainEnergy << "iteration i: " << i << std::endl;

				allFile.open (fileName, std::fstream::in | std::fstream::out | std::fstream::app);
				if(i==-1) allFile << "QH " << 1 << " " << std::setprecision(16) << SDQHmesh.getTotalResults().mTotalStrainEnergy << std::endl;
				else allFile << "QH " << 2+i*2 << " " << std::setprecision(16) << SDQHmesh.getTotalResults().mTotalStrainEnergy << std::endl;
				allFile.close();

				if(false) //always visualize
				{
					bso::visualization::visualize(SDQHmesh,"component");
					bso::visualization::visualize(SDQHmesh,"element");
					continue;
				}
				if(i == 0)
				{
					bso::visualization::visualize(SDQHmesh,"component");
					bso::visualization::visualize(SDQHmesh,"element");
					continue;
				}
				if(i == nrMeshSteps-1) 
				{
					bso::visualization::visualize(SDQHmesh,"component");
					bso::visualization::visualize(SDQHmesh,"element");	
				}
			}
		}
	}


	// Mesh convergence study of two orthogonal full buildings
	if(fullBuilding)
	{
		// Generate conformal models
		// TP method
		bso::spatial_design::ms_N_building ms3("design_3");
		bso::visualization::visualize(ms3);
		out("Created an MS model",true,true,true);
		bso::spatial_design::cf_building CFTP(ms3, tol);
		out("Created an CF model",true,true,true);

		// QH method
		bso::spatial_design::ms_building ms4("design_4");
		bso::visualization::visualize(ms4);
		out("Created an MS model",true,true,true);
		bso::spatial_design::cf_building CFQH(ms4, tol);
		out("Created an CF model",true,true,true);

		// Generate result file name
		std::string fileName1 = "TP_mesh_dif_full_building";

		// Setup result file
		allFile.open (fileName1, std::fstream::in | std::fstream::out | std::fstream::app);
		allFile << "ConformalGeom meshSize SDResult" << std::endl;
		allFile.close();
		
		// Generate convergence study results
		for(int i=0; i<nrMeshStepsFullBuilding; i++)
		{
			bso::grammar::grammar gram1(CFTP);
			out("Created an TP GRAM model",true,true,true);

			std::string base = "settings/sd_settings_";
			std::string integer = std::to_string(i);
			std::string end = ".txt";
			std::string settingFile = base + integer + end;	
			
			bso::structural_design::sd_model SDTPmesh = gram1.sd_grammar <bso::grammar::DEFAULT_SD_GRAMMAR >(std::string(settingFile));
			out("Created an TP SD model",true,true,true);		
			
			SDTPmesh.analyze();
			out("Analyzed an TP SD model",true,true,true);
			std::cout << "Structural compliance: " << SDTPmesh.getTotalResults().mTotalStrainEnergy << "iteration i: " << i << std::endl;

			allFile.open (fileName1, std::fstream::in | std::fstream::out | std::fstream::app);
			allFile << "TP " << 2+i*2 << " " << std::setprecision(16) << SDTPmesh.getTotalResults().mTotalStrainEnergy << std::endl;
			allFile.close();

			if(false) //always visualize
			{
				bso::visualization::visualize(SDTPmesh,"component");
				bso::visualization::visualize(SDTPmesh,"element");
				continue;
			}
			if(i == 0) 
			{
				bso::visualization::visualize(SDTPmesh,"component");
				bso::visualization::visualize(SDTPmesh,"element");
			}
			if(i == nrMeshStepsFullBuilding-1) 
			{
				bso::visualization::visualize(SDTPmesh,"component");
				bso::visualization::visualize(SDTPmesh,"element");	
			}
		}

		// Generate result file name
		std::string fileName2 = "QH_mesh_dif_full_building";

		// Setup result file
		allFile.open (fileName2, std::fstream::in | std::fstream::out | std::fstream::app);
		allFile << "ConformalGeom meshSize SDResult" << std::endl;
		allFile.close();
		
		// Generate convergence study results
		for(int i=0; i<nrMeshStepsFullBuilding; i++)
		{
			bso::grammar::grammar gram1(CFQH);
			out("Created an QH GRAM model",true,true,true);

			std::string base = "settings/sd_settings_";
			std::string integer = std::to_string(i);
			std::string end = ".txt";
			std::string settingFile = base + integer + end;	

			bso::structural_design::sd_model SDQHmesh = gram1.sd_grammar <bso::grammar::DEFAULT_SD_GRAMMAR >(std::string(settingFile));
			out("Created an QH SD model",true,true,true);		
			
			SDQHmesh.analyze();
			out("Analyzed an QH SD model",true,true,true);
			std::cout << "Structural compliance: " << SDQHmesh.getTotalResults().mTotalStrainEnergy << "iteration i: " << i << std::endl;

			allFile.open (fileName2, std::fstream::in | std::fstream::out | std::fstream::app);
			allFile << "QH " << 2+i*2 << " " << std::setprecision(16) << SDQHmesh.getTotalResults().mTotalStrainEnergy << std::endl;
			allFile.close();

			if(false) //always visualize
			{
				bso::visualization::visualize(SDQHmesh,"component");
				bso::visualization::visualize(SDQHmesh,"element");
				continue;
			}
			if(i == 0) 
			{
				bso::visualization::visualize(SDQHmesh,"component");
				bso::visualization::visualize(SDQHmesh,"element");
			}
			if(i == nrMeshStepsFullBuilding-1) 
			{
				bso::visualization::visualize(SDQHmesh,"component");
				bso::visualization::visualize(SDQHmesh,"element");	
			}
		}
	}


	// Mesh convergence study of NOBSD utilizing the triangular partition method
	if(NOBSDbuilding)
	{
		for(int i=1; i<=20; i++)
		{
			std::string name = "../Non_orthogonal_designs/design_";
			std::string integerDesign = std::to_string(i);
			std::string designName = name + integerDesign;	
			
			// Check if design exist
			std::ifstream input;
			if (!designName.empty()) input.open(designName.c_str());
			if (designName.empty() || !input.is_open()){continue;}
			
			try
			{
				// Generate Movable and Sizable model
				bso::spatial_design::ms_N_building ms(designName);
				bso::visualization::visualize(ms);
				out("Created an MS model",true,true,true);
				
				// Generate conformal model
				bso::spatial_design::cf_building CF(ms, tol);
				out("Created an CF model",true,true,true);

				// Generate result file name
				std::string file = "mesh_dif_design_";
				std::string integer = std::to_string(i);
				std::string fileName = file + integer;	

				// Setup result file
				allFile.open (fileName, std::fstream::in | std::fstream::out | std::fstream::app);
				allFile << "ConformalGeom meshSize SDResult" << std::endl;
				allFile.close();
				
				// Generate convergence study results
				for(int i=0; i<nrMeshStepsFullBuilding; i++)
				{
					bso::grammar::grammar gram1(CF);
					out("Created an TP GRAM model",true,true,true);

					std::string base = "settings/sd_settings_";
					std::string integer = std::to_string(i);
					std::string end = ".txt";
					std::string settingFile = base + integer +end;	

					bso::structural_design::sd_model SDTP = gram1.sd_grammar <bso::grammar::DEFAULT_SD_GRAMMAR >(std::string(settingFile));
					out("Created an TP SD model",true,true,true);		
					
					SDTP.analyze();
					out("Analyzed an TP SD model",true,true,true);
					std::cout << "Structural compliance: " << SDTP.getTotalResults().mTotalStrainEnergy << "iteration i: " << i << std::endl;

					allFile.open (fileName, std::fstream::in | std::fstream::out | std::fstream::app);
					allFile << "TP " << 2+i*2 << " " << std::setprecision(16) << SDTP.getTotalResults().mTotalStrainEnergy << std::endl;
					allFile.close();

					if(i == -1) 
					{
						bso::visualization::visualize(SDTP,"component");
						bso::visualization::visualize(SDTP,"element");
					}
					if(i == nrMeshStepsFullBuilding-1) 
					{
						bso::visualization::visualize(SDTP,"component");
						bso::visualization::visualize(SDTP,"element");	
					}
				}
			}
			catch(const std::exception& e)
			{
				std::cerr << "Error: " << e.what() << std::endl;
				continue; // Skip to the next design
			}
		}
	}


	//bso::visualization::visualize(BP);
	bso::visualization::endVisualization();
	out("End visualizations",true,true,true);

	return 0;
}