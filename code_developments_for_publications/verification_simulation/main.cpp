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

	rib1.push_back(verticesQH[0]); // first vertex of rib
	for(int j=1; j<= nrPartitions; j++)
	{
		bso::utilities::geometry::vertex tempv = verticesQH[0] + ((verticesQH[1]-verticesQH[0]) / (nrPartitions+1)) * j;
		rib1.push_back(tempv); // mid vertex rib
	}
	rib1.push_back(verticesQH[1]); // second vertex of rib

	rib2.push_back(verticesQH[3]); // first vertex of rib
	for(int j=1; j<= nrPartitions; j++)
	{
		bso::utilities::geometry::vertex tempv = verticesQH[3] + ((verticesQH[2]-verticesQH[3]) / (nrPartitions+1)) * j;
		rib2.push_back(tempv); // mid vertex rib
	}
	rib2.push_back(verticesQH[2]); // second vertex of rib

	rib3.push_back(verticesQH[4]); // first vertex of rib
	for(int j=1; j<= nrPartitions; j++)
	{
		bso::utilities::geometry::vertex tempv = verticesQH[4] + ((verticesQH[5]-verticesQH[4]) / (nrPartitions+1)) * j;
		rib3.push_back(tempv); // mid vertex rib
	}
	rib3.push_back(verticesQH[5]); // second vertex of rib

	rib4.push_back(verticesQH[7]); // first vertex of rib
	for(int j=1; j<= nrPartitions; j++)
	{
		bso::utilities::geometry::vertex tempv = verticesQH[7] + ((verticesQH[6]-verticesQH[7]) / (nrPartitions+1)) * j;
		rib4.push_back(tempv); // mid vertex rib
	}
	rib4.push_back(verticesQH[6]); // second vertex of rib

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

	rib1.push_back(verticesQH[0]); // first vertex of rib
	for(int j=1; j<= nrPartitions; j++)
	{
		bso::utilities::geometry::vertex tempv = verticesQH[0] + ((verticesQH[1]-verticesQH[0]) / (nrPartitions+1)) * j;
		rib1.push_back(tempv); // mid vertex rib
	}
	rib1.push_back(verticesQH[1]); // second vertex of rib

	rib2.push_back(verticesQH[3]); // first vertex of rib
	for(int j=1; j<= nrPartitions; j++)
	{
		bso::utilities::geometry::vertex tempv = verticesQH[3] + ((verticesQH[2]-verticesQH[3]) / (nrPartitions+1)) * j;
		rib2.push_back(tempv); // mid vertex rib
	}
	rib2.push_back(verticesQH[2]); // second vertex of rib

	rib3.push_back(verticesQH[4]); // first vertex of rib
	for(int j=1; j<= nrPartitions; j++)
	{
		bso::utilities::geometry::vertex tempv = verticesQH[4] + ((verticesQH[5]-verticesQH[4]) / (nrPartitions+1)) * j;
		rib3.push_back(tempv); // mid vertex rib
	}
	rib3.push_back(verticesQH[5]); // second vertex of rib

	rib4.push_back(verticesQH[7]); // first vertex of rib
	for(int j=1; j<= nrPartitions; j++)
	{
		bso::utilities::geometry::vertex tempv = verticesQH[7] + ((verticesQH[6]-verticesQH[7]) / (nrPartitions+1)) * j;
		rib4.push_back(tempv); // mid vertex rib
	}
	rib4.push_back(verticesQH[6]); // second vertex of rib

	// Add quad-hexahedrons to pQuadHexahedron
	for(int i=0; i<= nrPartitions; i++)
	{
		bso::utilities::geometry::quad_hexahedron temp1 = {{rib1[i]},{rib1[i+1]},{rib2[i]},{rib2[i+1]},{rib3[i]},{rib3[i+1]},{rib4[i]},{rib4[i+1]}};
		pQuadHexahedron.push_back(temp1);
	}

	return pQuadHexahedron;
}

/*
// Function to subdivide a quad-hexahedron into multiple quad-hexahedrons
std::vector<bso::utilities::geometry::quad_hexahedron> pQHbase(bso::utilities::geometry::quad_hexahedron QH, int nrPartitions)
{	
	std::vector<bso::utilities::geometry::quad_hexahedron> pQuadHexahedron; 
	std::vector<bso::utilities::geometry::vertex> verticesQH = QH.getVertices();

	// Define all relevant vertices of future quad-hexahedrons along the four opposing ribs
	std::vector<bso::utilities::geometry::vertex> rib1; // lower two opposing ribs
	std::vector<bso::utilities::geometry::vertex> rib2; // lower two opposing ribs
	std::vector<bso::utilities::geometry::vertex> rib3; // top two opposing ribs
	std::vector<bso::utilities::geometry::vertex> rib4; // top two opposing ribs
	std::vector<bso::utilities::geometry::vertex> rib5; // lower two opposing ribs
	std::vector<bso::utilities::geometry::vertex> rib6; // lower two opposing ribs
	std::vector<bso::utilities::geometry::vertex> rib7; // top two opposing ribs
	std::vector<bso::utilities::geometry::vertex> rib8; // top two opposing ribs

	rib1.push_back(verticesQH[0]); // first vertex of rib
	for(int j=1; j<= nrPartitions; j++)
	{
		bso::utilities::geometry::vertex tempv = verticesQH[0] + ((verticesQH[1]-verticesQH[0]) / (nrPartitions+1)) * j;
		rib1.push_back(tempv); // mid vertex rib
	}
	rib1.push_back(verticesQH[1]); // second vertex of rib

	rib2.push_back(verticesQH[3]); // first vertex of rib
	for(int j=1; j<= nrPartitions; j++)
	{
		bso::utilities::geometry::vertex tempv = verticesQH[3] + ((verticesQH[2]-verticesQH[3]) / (nrPartitions+1)) * j;
		rib2.push_back(tempv); // mid vertex rib
	}
	rib2.push_back(verticesQH[2]); // second vertex of rib

	rib3.push_back(verticesQH[4]); // first vertex of rib
	for(int j=1; j<= nrPartitions; j++)
	{
		bso::utilities::geometry::vertex tempv = verticesQH[4] + ((verticesQH[5]-verticesQH[4]) / (nrPartitions+1)) * j;
		rib3.push_back(tempv); // mid vertex rib
	}
	rib3.push_back(verticesQH[5]); // second vertex of rib

	rib4.push_back(verticesQH[7]); // first vertex of rib
	for(int j=1; j<= nrPartitions; j++)
	{
		bso::utilities::geometry::vertex tempv = verticesQH[7] + ((verticesQH[6]-verticesQH[7]) / (nrPartitions+1)) * j;
		rib4.push_back(tempv); // mid vertex rib
	}
	rib4.push_back(verticesQH[6]); // second vertex of rib

	// Add quad-hexahedrons to pQuadHexahedron
	for(int i=0; i<= nrPartitions; i++)
	{
		bso::utilities::geometry::quad_hexahedron temp1 = {{rib1[i]},{rib1[i+1]},{rib2[i]},{rib2[i+1]},{rib3[i]},{rib3[i+1]},{rib4[i]},{rib4[i+1]}};
		pQuadHexahedron.push_back(temp1);
	}

	return pQuadHexahedron;
}
*/

using namespace bso;
int main(int argc, char* argv[])
{
	std::cout.precision(17); //this precision only counts with cout not with the error message handling used in the toolbox
	
	int nrPartitions = 10; // Partitions subdivided to opposing sides with the generation of triangular prisms in between

	// Generate triangular prism conformal models
	std::vector<bso::spatial_design::cf_building*> CFTP;
	bso::spatial_design::ms_N_building ms1("design_1");
	out("Created an MS model",true,true,true);
	for(int i=0; i< nrPartitions; i++)
	{	
		// get triangular prism within space
		bso::spatial_design::ms_space* space = (ms1.getSpacePtrs())[0];
		bso::utilities::geometry::quad_hexahedron quadToPartition = space->getGeometry();
		std::vector<bso::utilities::geometry::triangular_prism> tempTP = pTP(quadToPartition, i);
		out("Get triangular prisms within space",true,true,true);

		// Generate conformal geometry
		int tol = 0.001;		
		CFTP.push_back(new bso::spatial_design::cf_building(ms1, tol, false));
		out("Created an CF model",true,true,true);
		CFTP.back()->addSpaceD((*(ms1.getSpacePtrs())[0]), tempTP);
		out("Added all spaces with triangular prisms to CF model",true,true,true);
	}

	// Generate quad-hexahedron conformal models
	bso::spatial_design::ms_building ms2("design_2");
	std::vector<bso::spatial_design::cf_building*> CFQH;
	out("Created an MS model",true,true,true);
	for(int i=0; i< nrPartitions; i++)
	{	
		// get quad-hexahedrons within space
		bso::spatial_design::ms_space* space = (ms1.getSpacePtrs())[0];
		bso::utilities::geometry::quad_hexahedron quadToPartition = space->getGeometry();
		std::vector<bso::utilities::geometry::quad_hexahedron> tempQH = pQH(quadToPartition, i);
		out("get quad-hexahedrons within space",true,true,true);

		// Generate conformal geometry		
		CFQH.push_back(new bso::spatial_design::cf_building(ms2));
		out("Created an CF model",true,true,true);
		for(const auto j: tempQH)
		{
			CFQH.back()->addCuboid(j);
		}
		out("Added all Cuboids to CF model",true,true,true);
	}

	// // Generate quad-hexahedron conformal models for baseline SD approach
	// bso::spatial_design::ms_building ms2("design_2");
	// std::vector<bso::spatial_design::cf_building*> CFQHbase;
	// for(int i=0; i< nrPartitions; i++)
	// {	
	// 	// get quad-hexahedrons within space
	// 	bso::spatial_design::ms_space* space = (ms1.getSpacePtrs())[0];
	// 	bso::utilities::geometry::quad_hexahedron quadToPartition = space->getGeometry();
	// 	std::vector<bso::utilities::geometry::quad_hexahedron> tempQH = pQH(quadToPartition, i);

	// 	// Generate conformal geometry		
	// 	CFQHbase.push_back(new bso::spatial_design::cf_building(ms2));
	// 	for(const auto j: tempQH)
	// 	{
	// 		CFQHbase.back()->addCuboid(j);
	// 	}
	// }

	// Setup the data files:
	std::ofstream allFile;
	allFile.open ("TP_SD", std::fstream::in | std::fstream::out | std::fstream::app);
		allFile << "ConformalGeom nrPartitions SDResult" << std::endl;
		allFile.close();

	allFile.open ("QH_SD", std::fstream::in | std::fstream::out | std::fstream::app);
		allFile << "ConformalGeom nrPartitions SDResult" << std::endl;
		allFile.close();

	
	// Write data to files:
	std::cout << "TP conformal geometry results: " << std::endl;
	for(int i=0; i<CFTP.size(); i++)
	{
		*(CFTP[i]);
		bso::grammar::grammar gram1(*(CFTP[i]));
		out("Created an GRAM model",true,true,true);
		bso::structural_design::sd_model SD1 = gram1.sd_grammar <bso::grammar::DEFAULT_SD_GRAMMAR >(std::string("settings/sd_settings.txt"));
		out("Created an SD model",true,true,true);
		SD1.analyze();
		out("Analyzed an SD model",true,true,true);
		std::cout << "Structural compliance: " << SD1.getTotalResults().mTotalStrainEnergy << std::endl;

		allFile.open ("TP_SD", std::fstream::in | std::fstream::out | std::fstream::app);
		allFile << "TP " << nrPartitions << " " << SD1.getTotalResults().mTotalStrainEnergy << std::endl;
		allFile.close();
	}
	
	std::cout << "QH conformal geometry results: " << std::endl;
	for(int i=0; i<CFQH.size(); i++)
	{
		bso::grammar::grammar gram1(*(CFQH[i]));
		out("Created an GRAM model",true,true,true);
		bso::structural_design::sd_model SD1 = gram1.sd_grammar <bso::grammar::DEFAULT_SD_GRAMMAR >(std::string("settings/sd_settings.txt"));
		out("Created an SD model",true,true,true);
		SD1.analyze();
		out("Analyzed an SD model",true,true,true);
		std::cout << "Structural compliance: " << SD1.getTotalResults().mTotalStrainEnergy << std::endl;

		allFile.open ("QH_SD", std::fstream::in | std::fstream::out | std::fstream::app);
		allFile << "QH " << nrPartitions << " " << SD1.getTotalResults().mTotalStrainEnergy << std::endl;
		allFile.close();
	}


	// visualize the different models
	bso::visualization::initVisualization(argc, argv);
	out("Started visualizations",true,true,true);
	bso::visualization::visualize(ms1);
	bso::visualization::visualize(ms2);
	for(int i=0; i<CFTP.size(); i++) bso::visualization::visualize(*(CFTP[i]),"triangular_prism");
	for(int i=0; i<CFQH.size(); i++) bso::visualization::visualize(*(CFQH[i]),"cuboid");


	//bso::visualization::visualize(SD,"component");
	//bso::visualization::visualize(SD,"element");
	//bso::visualization::visualize(BP);
	bso::visualization::endVisualization();
	out("End visualizations",true,true,true);

	return 0;
}