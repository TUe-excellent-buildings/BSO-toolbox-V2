#ifndef GEOMETRY_FUNCTIONS_CPP
#define GEOMETRY_FUNCTIONS_CPP

#include <bitset>
#include <iomanip> // Allowes to use std::setprecision() to be able to enlarge the decimals printed to the console of a double		
#include <vector>
#include <map>

namespace bso { namespace utilities {
	
	std::vector<utilities::geometry::line_segment> getOuterBounds2D(std::vector<utilities::geometry::line_segment> lines, double mTol)
	{ // reference: https://stackoverflow.com/questions/31169125/outermost-polygon-from-a-set-of-edges
		// Split inserted lines at all their vertices
		if(lines.size() == 0)
		{
			std::stringstream errorMessage;
				errorMessage << "No lines were inserted in the getOuterBounds2D function. " << std::endl
										<< "(bso/utilities/delaunay.cpp)." << std::endl;
				throw std::invalid_argument(errorMessage.str());
		}
		// split the lines at the end vertices of the lines and remove duplicates
		std::vector<utilities::geometry::vertex> lineV;
		for(const auto& i: lines)
		{
			lineV.push_back(i[0]);
			lineV.push_back(i[1]);
		}
		splitLines(lineV, lines, mTol);		
		
		// Declarations
		std::vector<utilities::geometry::line_segment> outerBound;
		std::vector<utilities::geometry::vertex> connectivityVertex; //Note the id vertex of this variable form a pair with the same id from vector connectivityLines
		std::vector<std::vector<utilities::geometry::line_segment>> connectivityLines;
		utilities::geometry::vertex startV = (lines[0])[0];
		std::vector<utilities::geometry::line_segment> connectedLines;
		long unsigned int c=0;

		for(const auto& i: lines)
		{
			c=0;
			while(c < connectivityVertex.size())
			{
				if(connectivityVertex[c].isSameAs(i[0], mTol)) break;
				c++;
			}
			if(c != connectivityVertex.size())
			{
				// adjust vector of vertex
				connectivityLines[c].push_back(i);
			}
			else
			{
				// not found therefor add new vertex
				std::vector<utilities::geometry::line_segment> connectedLines;
				connectedLines.push_back(i);
				connectivityVertex.push_back(i[0]);
				connectivityLines.push_back(connectedLines);
			}

			c = 0;
			while(c < connectivityVertex.size())
			{
				if(connectivityVertex[c].isSameAs(i[1], mTol)) break;
				c++;
			}
			if(c != connectivityVertex.size())
			{
				// adjust vector of vertex
				connectivityLines[c].push_back({i[1],i[0]});
			}
			else
			{
				// not found therefor add new vertex
				std::vector<utilities::geometry::line_segment> connectedLines;
				connectedLines.push_back({i[1],i[0]}); //switch vertices so that the vector is pointing outwards from considered vertex
				connectivityVertex.push_back(i[1]);
				connectivityLines.push_back(connectedLines);
			}

			//find lowest x and y of vertexes
			if(i[0].x()<=startV.x())
			{
				if(i[0].y()<startV.y()) {startV = i[0];}
			}
			if(i[1].x()<=startV.x())
			{
				if(i[1].y()<startV.y()) {startV = i[1];}
			}
		}

		// find starting line segment
		utilities::geometry::line_segment consideredLine;
		bso::utilities::geometry::vertex consideredV;
		double azimuth;
		double smallestAzimuth;

		c = 0;
		while(c < connectivityVertex.size())
		{
			if(connectivityVertex[c].isSameAs(startV, mTol)) break;
			c++;
		}
		if(c != connectivityVertex.size())
		{
			smallestAzimuth = 360- ((connectivityLines[c])[0]).getVector().getHeading().first; // anti clockwise
			consideredLine = ((connectivityLines[c])[0]);
			for(int i=1; i<connectivityLines[c].size(); i++)
			{
				azimuth =  360 - ((connectivityLines[c])[i]).getVector().getHeading().first; // anti clockwise
				if(azimuth < smallestAzimuth) consideredLine = (connectivityLines[c])[i];
			}
			// Found start outerBound line
			outerBound.push_back(consideredLine);
			// Define the other vertex then the start vertex for searching for next outerBound line:
			if((consideredLine)[0].isSameAs(startV, mTol)) {consideredV = (consideredLine)[1];}
			else {consideredV = (consideredLine)[0];}
		}
		else
		{
			std::stringstream errorMessage;
				errorMessage << "Start vertex for determining the bounds of a set of lines has not been found. " << std::endl
										<< "The considered start vertex is: " << startV << std::endl
										<< "(bso/utilities/delaunay.cpp)." << std::endl;
				throw std::invalid_argument(errorMessage.str());
		}
		bso::utilities::geometry::vertex lastConsideredV;
		while(!consideredV.isSameAs(startV, mTol))
		{
			lastConsideredV = consideredV;
			bso::utilities::geometry::vector flipV = (outerBound.back())[0] - (outerBound.back())[1];
			double currentAzimuth = flipV.getHeading().first;
			
			c = 0;
			while(c < connectivityVertex.size())
			{
				if(connectivityVertex[c].isSameAs(consideredV, mTol)) break;
				c++;
			}
			if(c != connectivityVertex.size())
			{
				std::vector<utilities::geometry::line_segment> linesAtV = connectivityLines[c];
				// if(linesAtV.size() <= 1)
				// {
				// 	std::stringstream errorMessage;
				// 	errorMessage << "LinesAtV only consist of zero or one line. By definition, " << std::endl
				// 							<< "no continuation of the outerBound is possible. " << std::endl
				// 							<< "(bso/utilities/delaunay.cpp)." << std::endl;
				// 	throw std::invalid_argument(errorMessage.str());
				// }
				int count = 0;
				for(const auto& i: linesAtV)
				{
					if(outerBound.back().isSameAs(i, mTol)) continue; // find line other then already know outbound
					if(count == 0)
					{
						smallestAzimuth = currentAzimuth - i.getVector().getHeading().first; // anti clockwise
						if(smallestAzimuth < 0) smallestAzimuth = 360 + smallestAzimuth;
						consideredLine = i;
						count++;
						continue;
					}
					azimuth =  currentAzimuth - i.getVector().getHeading().first; // anti clockwise
					if(azimuth < 0) azimuth = 360 + azimuth;
					if(azimuth < smallestAzimuth) consideredLine = i;
				}
				// Found start outerBound line
				outerBound.push_back(consideredLine);
				
				// Define the other vertex then the considered vertex for searching for next outerBound line:
				consideredV = (consideredLine)[1];
				if(lastConsideredV.isSameAs(consideredV, mTol))
				{
					std::stringstream errorMessage;
					errorMessage << "The considered vertex is not changed, as would be expected. " << std::endl
											<< "The considered start vertex is: " << consideredV << std::endl
											<< "The lastConsideredV is: " << lastConsideredV << std::endl
											<< "The just added consideredLine is: " << consideredLine << std::endl
											<< "The content of outerBound is: " << std::endl;
											for(auto& add: outerBound) errorMessage << add << std::endl;
											errorMessage << "The content of linesAtV is: " << std::endl;
											for(auto& add: linesAtV) errorMessage << add << std::endl;
											errorMessage << "(bso/utilities/delaunay.cpp)." << std::endl;
					throw std::invalid_argument(errorMessage.str());
				}
			}
			else
			{
				std::stringstream errorMessage;
				errorMessage << "The considered vertex has not been found while determining the bounds of a set of lines. " << std::endl
										<< "The considered start vertex is: " << consideredV << std::endl
										<< "(bso/utilities/delaunay.cpp)." << std::endl;
				throw std::invalid_argument(errorMessage.str());
			}
		}

		return outerBound;
	}

	void sortLinesToChain(std::vector<utilities::geometry::line_segment>& lines,  const double mTol /*= 1e-3*/)
	{
		// Sort lines to form a chain of lines (i.e. vertex 1 of line 0 connect to vertex 0 of line 1, etc.) instead of random stored lines.
		// Note; for now this function only works when the lines form a continues line.
		if(lines.size() <= 1)
		{
			return; // since line are already sorted
		}
		bool reverse = false;
		bool done = false;
		long unsigned int i=0;
		int condition = lines.size();
		int vID = 1;
		while(!done)
		{	
			bool anyConnected = false;
			// find nex lineAtLevel which is connected
			if(!reverse)
			{
				if(i+1 == lines.size())
				{
					anyConnected = true;
					break;
				}
				for(int j=i+1; j < lines.size(); j++)
				{
					if(lines[i][vID].isSameAs(lines[j][1], mTol))
					{
						bso::utilities::geometry::line_segment temp;
						temp = lines[i+1];
						lines[i+1] = {lines[j][1],lines[j][0]};
						if(j != i+1)
						{
							lines[j] = temp;
						}
						anyConnected = true;
						break;
					} 
					else if (lines[i][vID].isSameAs(lines[j][0], mTol))
					{
						if(j != i+1)
						{
							bso::utilities::geometry::line_segment temp;
							temp = lines[i+1];
							lines[i+1] = lines[j];
							lines[j] = temp;
						}
						anyConnected = true;
						break;
					}
				}
				if(!anyConnected)
				{
					if(vID == 1)
					{
						// shift the content of the vector to the back of the vector and reverse iteration order
						int shift = condition-1 - i;
						std::vector<bso::utilities::geometry::line_segment> temp = lines;
						
						long unsigned int s = shift;
						for(long unsigned int count = 0; count < lines.size(); count++)
						{
							lines[s]= temp[count];
							s ++;
							if(s == lines.size()) {s = 0;}
						}
						
						vID = 0;
						i = shift+1; // +1 to compensate for the i-- at the end
						reverse = true;
					}
				}
			}
			else
			{
				bool anyConnected = false;
				// find next lineAtLevel which is connected
				if(i-1 < 0)
				{
					anyConnected = true;
					break;
				}
				for(int j=i-1; j >= 0 ; j--)
				{
					if(lines[i][vID].isSameAs(lines[j][1], mTol))
					{
						if(j != i-1)
						{
							bso::utilities::geometry::line_segment temp;
							temp = lines[i-1];
							lines[i-1] = lines[j];
							lines[j] = temp;
						}
						anyConnected = true;
						break;
					} 
					else if (lines[i][vID].isSameAs(lines[j][0], mTol))
					{
						bso::utilities::geometry::line_segment temp;
						temp = lines[i-1];
						lines[i-1] = {lines[j][1],lines[j][0]};
						if(j != i-1)
						{
							lines[j] = temp;
						}
						anyConnected = true;
						break;
					}
				}
				if(!anyConnected)
				{
					if(vID == 0)
					{
						// should start new line in future when surfaces are not continues
						done = true;
					}

				}
			}
			if(reverse){i--;}
			if(i >= lines.size()) {done = true;}
			else if(i < 0) {done = true;}
			if(!reverse){i++;}
		}
	}

	#if defined(BSO_USE_FADE) // At the moment is function is only available for the fade delaunay triangulation, since it uses delaunayZoned which is only available for FADE-2D.
	// However, at the moment, this function is not used in the code.
	bool hasCourtyard(std::vector<bso::utilities::geometry::quadrilateral> exteriorWalls, std::vector<std::vector<bso::utilities::geometry::line_segment>>& outerboundsExport, std::vector<bso::utilities::geometry::triangle>& triangleExport, /*std::vector<std::pair<double,std::vector<bso::utilities::geometry::polygon_non_convex>>>& courtyardsEnclosedPerElevation, std::vector<std::pair<double,std::vector<bso::utilities::geometry::polygon_non_convex>>>& courtyardsConnectedToAirPerElevation,*/ const double mTol /*= 1e-3*/)
	{
		bool hasCourtyard = false;
		
		// Find all elevations in the building
		std::vector<std::pair<double,std::vector<bso::utilities::geometry::line_segment>>> linesAtElevation;
		for(const auto& wall: exteriorWalls)
		{
			for(const auto& point: wall)
			{
				bool isAdded = false;
				for(auto k: linesAtElevation)
				{
					if(point.z() == k.first) 
					{
						isAdded = true;
						break;
					}
				}
				if(!isAdded){linesAtElevation.emplace_back(point.z(), std::vector<bso::utilities::geometry::line_segment>{});}
			}
		}

		// Sort linesAtElevation by the double value in the std::pair and remove the pair with the highest double value (i.e. roof of the building)
		std::sort(linesAtElevation.begin(), linesAtElevation.end(), comparisonFirstOfPair); //comparisonFirstOfPair function defined in delaunay.hpp
		if (!linesAtElevation.empty()) {linesAtElevation.pop_back();}
		
		// Get the horizontal lines at each elevation
		for(const auto& wall: exteriorWalls)
		{
			bool firstLine = false;
			bso::utilities::geometry::line_segment line1;
			for(const auto& line: wall.getLines())
			{
				if(line.getVector().isHorizontal())
				{
					// Get both horizontal lines, by storing and continuing the for loop. 
					if(firstLine == false)
					{
						firstLine = true;
						line1 = line;
						continue;
					}
		
					// Add lines if at elevation and not already added
					bool line1IsBottomLine = line1[0].z() < line[0].z();
					for(auto& elevation: linesAtElevation)
					{
						bool add = false;
						if(line1IsBottomLine){add = (elevation.first >= line1[0].z()- mTol && elevation.first < line[0].z()- mTol);}
						else{add = (elevation.first >= line[0].z()- mTol && elevation.first < line1[0].z() - mTol);}
						if(add) 
						{ 
							bso::utilities::geometry::line_segment toAddLine = {{line[0].x(),line[0].y(),elevation.first},{line[1].x(),line[1].y(),elevation.first}};
							bool toAddLineAlreadyAdded = false;
							for(const auto& existingLine: elevation.second)
							{
								if(toAddLine.isSameAs(existingLine,mTol))
								{
									toAddLineAlreadyAdded = true;
									break;
								}
							}
							if(!toAddLineAlreadyAdded){elevation.second.push_back(toAddLine);}
						}
					}
				}
			}
		}

		// Identify each outbound in the exterior walls
		std::vector<std::pair<double,std::vector<std::vector<bso::utilities::geometry::line_segment>>>> outerBoundPerElevation;
		for(const auto& elevation: linesAtElevation)
		{
			std::vector<bso::utilities::geometry::line_segment> remainingLines = elevation.second;
			std::vector<std::vector<bso::utilities::geometry::line_segment>> outerBound;
			while(remainingLines.size() > 0)
			{
				if(remainingLines.size() <= 3)
				{
					std::stringstream errorMessage;
					errorMessage << "The are less then three lines in container to get the outer bounds of these lines might form. " << std::endl
											<< "As such the program is terminated, since otherwise a while loop will not end. "<< std::endl
											<< "(bso/utilities/delaunay.cpp)." << std::endl;
					throw std::invalid_argument(errorMessage.str());
				}
				std::vector<bso::utilities::geometry::line_segment> outerBoundLines = getOuterBounds2D(remainingLines, mTol);
				outerBound.push_back(outerBoundLines);
				remainingLines.clear();

				for(const auto& j: elevation.second)
				{
					bool isFoundInAnyOuterBound = false;
					for(const auto& k: outerBound)
					{
						for(const auto& l: k)
						{
							if(l.isSameAs(j,mTol))
							{
								isFoundInAnyOuterBound = true;
								break;
							}
						}
						if(isFoundInAnyOuterBound){break;}
					}
					if(!isFoundInAnyOuterBound){remainingLines.push_back(j);}
				}
			}

			auto pTemp = std::make_pair(elevation.first,outerBound);
			outerBoundPerElevation.push_back(pTemp);
		}

		for(auto& o: outerBoundPerElevation)
		{
			for(auto l: o.second){outerboundsExport.push_back(l);}
		}

		// Identify from the outerBoundPerElevation if any are courtyards
		std::vector<std::pair<double,std::vector<bso::utilities::geometry::polygon_non_convex>>> courtyardsPerElevation;
		std::vector<std::pair<double,std::vector<bso::utilities::geometry::polygon_non_convex>>> polyOutboundsPerElevation;
		for(const auto& elevation: outerBoundPerElevation)
		{
			courtyardsPerElevation.emplace_back(elevation.first, std::vector<bso::utilities::geometry::polygon_non_convex>{});
			polyOutboundsPerElevation.emplace_back(elevation.first, std::vector<bso::utilities::geometry::polygon_non_convex>{});
			if(elevation.second.size() > 1)
			{
				// Redefine the outerbound to a polygon_non_convex, so we can use its geometry checks.
				std::vector<bso::utilities::geometry::polygon_non_convex> polyOutbounds;
				for(const auto& outboundLines: elevation.second)
				{
					std::vector<bso::utilities::geometry::vertex> outerboundVertexes;
					for(const auto& line: outboundLines) {outerboundVertexes.push_back(line[0]);}
					if(outerboundVertexes.size()<3)
					{
						std::stringstream errorMessage;
						errorMessage << "The are less then three vertexes in container, " << std::endl
												<< "therefor a polygon_non_convex would not be possible to be defined. " << std::endl
												<< "(bso/utilities/delaunay.cpp)." << std::endl;
						throw std::invalid_argument(errorMessage.str());
					}
					polyOutboundsPerElevation.back().second.push_back(bso::utilities::geometry::polygon_non_convex(outerboundVertexes));
					polyOutbounds.push_back(bso::utilities::geometry::polygon_non_convex(outerboundVertexes));
					for(auto& t: polyOutbounds.back().getTriangles())
					{
						triangleExport.push_back(t);
					}
				}
				for(long unsigned int i=0; i<polyOutbounds.size(); i++) // iterate trough each outerBound at elevation
				{
					for(long unsigned int j=i+1; j<polyOutbounds.size(); j++) // iterate trough each outerBound at elevation
					{
						// Check if any vertex of outbound j is inside outbound i, if so, then outbound j is a courtyard in outbound i.
						for(const auto& vertexj: polyOutbounds[j])
						{
							if(polyOutbounds[i].isInside(vertexj, mTol))
							{
								courtyardsPerElevation.back().second.push_back(polyOutbounds[j]);
								break;
							}
						}
						// Alternatively, check if any vertex of outbound i is inside outbound j, if so, then outbound i is a courtyard in outbound j.
						for(const auto& vertexi: polyOutbounds[i])
						{
							if(polyOutbounds[j].isInside(vertexi, mTol))
							{
								courtyardsPerElevation.back().second.push_back(polyOutbounds[i]);
								break;
							}
						}
					}
				}
			}
		}

		// This code is not yet fully developed, but the idea is to check for each courtyard if it is connected to the air or if it is fully enclosed by other courtyards and/or outerbounds.

		// Check if the found courtyards are connected to the air or if they are fully enclosed courtyards
		// for(long unsigned int i=0; i<courtyardsPerElevation.size(); i++)
		// {
		// 	courtyardsEnclosedPerElevation.emplace_back(courtyardsPerElevation[i].first, std::vector<bso::utilities::geometry::polygon_non_convex>{});
		// 	courtyardsConnectedToAirPerElevation.emplace_back(courtyardsPerElevation[i].first, std::vector<bso::utilities::geometry::polygon_non_convex>{});
		// 	if(courtyardsPerElevation[i].second.size() > 0)
		// 	{
		// 		hasCourtyard = true;
		// 	}
		// 	else {continue;}
			
		// 	for(auto lowerCourtyard: courtyardsPerElevation[i].second)
		// 	{
		// 		long unsigned int upperCourtyardIndex=i+1;
		// 		bool seriesOfCourtyardsConnectedToAir = true;
		// 		std::vector<std::pair<double,std::vector<bso::utilities::geometry::polygon_non_convex>>> courtyardsConnectedToAir;
		// 		for(int c=0; courtyardsPerElevation.size(); c++)
		// 		{
		// 			courtyardsConnectedToAir.emplace_back(courtyardsPerElevation[c].first, std::vector<bso::utilities::geometry::polygon_non_convex>{});
		// 			if(courtyardsPerElevation[c].first == courtyardsConnectedToAir[i].first){courtyardsConnectedToAir[i].second.push_back(lowerCourtyard);}
		// 		}
		// 		// Check if there are any upperCourtyards which make the connection to the air
		// 		while(upperCourtyardIndex+1 < courtyardsPerElevation.size())
		// 		{
		// 			bool connectedUpperCourtyardFound = false;
		// 			for(const auto& upperCourtyard: courtyardsPerElevation[upperCourtyardIndex].second)
		// 			{
		// 				if(lowerCourtyard.isOverlapping(upperCourtyard, mTol))
		// 				{
		// 					// Courtyard not yet fully enclosed
		// 					courtyardsConnectedToAir[i].second.push_back(upperCourtyard);
		// 					lowerCourtyard = upperCourtyard;
		// 					connectedUpperCourtyardFound = true;
		// 					break;
		// 				}
		// 			}
		// 			if(!connectedUpperCourtyardFound)
		// 			{
		// 				// Courtyard is fully enclosed
		// 				seriesOfCourtyardsConnectedToAir = false;
		// 				courtyardsEnclosedPerElevation.back().second.push_back(lowerCourtyard);
		// 				break;
		// 			}

		// 			upperCourtyardIndex++;
		// 		}
		// 		// Check if the courtyards is outside the outerbound of the above elevation
		// 		bool outSideOutbounds = true;
		// 		for(const auto& outerbound: polyOutboundsPerElevation[i+1].second)
		// 		{
		// 			if(lowerCourtyard.isOverlapping(outerbound, mTol))
		// 			{
		// 				// Check if the overlap is full or not
		// 				bool allInside = true;
		// 				for(const auto& vertex: lowerCourtyard)
		// 				{
		// 					if(outerbound.isInside(vertex, mTol))
		// 					{
		// 						allInside = false;
		// 						break;
		// 					}
		// 				}
		// 				if(allInside)
		// 				{
		// 					// fully enclosed courtyard
		// 					outSideOutbounds = false;
		// 					courtyardsEnclosedPerElevation.back().second.push_back(lowerCourtyard);
		// 					break;
		// 				}
		// 			}
		// 		}
		// 		if(outSideOutbounds)
		// 		{
		// 			courtyardsConnectedToAirPerElevation[i].second.push_back(lowerCourtyard);
		// 		}

		// 		if(seriesOfCourtyardsConnectedToAir)
		// 		{
		// 			for(long unsigned int l=0; l<courtyardsConnectedToAirPerElevation.size(); l++)
		// 			{
		// 				for(auto& k: courtyardsConnectedToAir[l].second)
		// 				{
		// 					courtyardsConnectedToAirPerElevation[l].second.push_back(k);
		// 				}
		// 			}
		// 		}
		// 	}
		// }

		return hasCourtyard;
	}
	#endif // defined(BSO_USE_FADE)

} // namespace utilities
} // namespace bso


#endif // GEOMETRY_FUNCTIONS_CPP

