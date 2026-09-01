#ifndef BSO_CF_BUILDING_MODEL_DEFAULT_WINDOW_GEOMETRY_GRAMMAR_CPP
#define BSO_CF_BUILDING_MODEL_DEFAULT_WINDOW_GEOMETRY_GRAMMAR_CPP

namespace bso { namespace spatial_design { namespace conformal {

	// Window grammar for models based on ms_building (orthogonal BSDs).
	// Reads WindowHeightRatio, WindowWidthRatio and StartLocationWindow from the settings
	// file, derives the window constraint vertices for each space and makes the conformal
	// model conformal to them, so it geometry is conformal to the window openings.
	template <>
	void cf_building_model::windowGrammar<DEFAULT_WINDOW_GEOMETRY_GRAMMAR>(const std::string& fileNameWindowSettings)
	{
        std::fstream input(fileNameWindowSettings);
		if (!input.is_open())
		{
			std::stringstream errorMessage;
			errorMessage << "\nCould not read window grammar settings, failed to open file:"
									<< fileNameWindowSettings << "\n(bso/spatial_design/conformal/window_grammars/default_window_grammar.cpp)" << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}

		double WindowHeightRatio;
		double WindowWidthRatio;
		double StartLocationWindow;

		std::string line;
		boost::char_separator<char> sep(","); // defines what separates tokens in a string
		typedef boost::tokenizer< boost::char_separator<char> > t_tokenizer; // settings for the boost::tokenizer
		char type_ID; // holds information about what type of information is described by the line currently read

		while(!input.eof())	//Get properties from file
		{
			getline(input, line); // get the next line from the file
			boost::algorithm::trim(line); // remove white space from start and end of line (to see if it is an empty line, remove any incidental white space)
			if (line == "") //skip empty lines (tokenizer does not like it)
			{
				continue; // continue to next line
			}
			t_tokenizer tok(line, sep); // tokenize the line
			t_tokenizer::iterator token = tok.begin(); // set iterator to first token

			try
			{
				type_ID = bso::utilities::trim_and_cast_char(*token);	++token; // interpret first token as type ID
			}
			catch (std::exception& e){continue;}

			if(type_ID == 'C') // the 'C' indicators, indicate the lines which describe the window properties
			{
				WindowHeightRatio = bso::utilities::trim_and_cast_double(*token); ++token;
				WindowWidthRatio = bso::utilities::trim_and_cast_double(*token); ++token;
				StartLocationWindow = bso::utilities::trim_and_cast_double(*token); ++token;
				break; // stop reading the file after retrieving the window properties line, since the other information in the file is not relevant and there is only one line defining the windows.
			}
			else{continue;} // if the line does not describe window properties, skip to next line
		}
		input.close();

		// Check retrieved window properties for validity, if not valid, throw error with explanation of the valid range and the retrieved values for easier debugging.
		if ((WindowHeightRatio+StartLocationWindow)>100 || WindowHeightRatio>100 || WindowWidthRatio>100)
		{
			std::stringstream errorMessage;
			errorMessage << "\nEither input variables: WindowHeightRatio[%], WindowWidthRatio[%], or StartLocationWindow[%] \n"
									<< "is out of valid range. The valid range for these variables is\n"
									<< " between 0 and 100, and the sum of WindowHeightRatio and StartLocationWindow should not exceed 100.\n"
									<< "The input values are: WindowHeightRatio = " << WindowHeightRatio << "%, WindowWidthRatio = " << WindowWidthRatio << "%, StartLocationWindow = " << StartLocationWindow << "%.\n"
									<< "the window settings file should be checked for the correct input of these variables, see: "
									<< fileNameWindowSettings << "\n(bso/spatial_design/conformal/window_grammars/default_window_grammar.cpp)" << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}

		// Generate the window constraint points to account for in the conformal model
		// for each space in the MS model based on the retrieved window properties and
		//store in member variable mConstrainsPerSpaceQH for later use in the generation of the conformal model.
		for (const auto& space : mMSModel)
		{
			auto dimensions = space->getDimensions();
			auto startVertex = space->getCoordinates();
			unsigned int spaceID = space->getID();

			double spaceX = dimensions[0];
			double spaceY = dimensions[1];
			double spaceZ = dimensions[2];

			double windowHeight = (WindowHeightRatio/100) * spaceZ;
			double windowWidthX = (WindowWidthRatio/100) *spaceX;
			double windowWidthY = (WindowWidthRatio/100) *spaceY;
			double windowStartPoint = ((100-StartLocationWindow)/100)*spaceZ; //upper left point window

			// Constrain vertex in y plane
			double window_x1 = (startVertex[0] + ((spaceX/2)-(windowWidthX/2)));
			double window_x2 = (startVertex[0] + ((spaceX/2)+(windowWidthX/2)));

			double window_y1 = (startVertex[1] + ((spaceY/2)-(windowWidthY/2)));
			double window_y2 = (startVertex[1] + ((spaceY/2)+(windowWidthY/2)));

			double window_z1 = (startVertex[2] + windowStartPoint - windowHeight); //lowest point
			double window_z2 = (startVertex[2] + windowStartPoint); //highest point

			// Constrains in the middle of the space as this adds lines on all the surfaces of the space.
			// See Quimara van den Hoeven master thesis for more information about this definition and a picture.
			std::vector<bso::utilities::geometry::vertex> addConstrainVertexQH;
			addConstrainVertexQH.push_back({window_x1, window_y1, window_z1});
			addConstrainVertexQH.push_back({window_x2, window_y2, window_z1});
			addConstrainVertexQH.push_back({window_x2, window_y1, window_z2});
			addConstrainVertexQH.push_back({window_x1, window_y2, window_z2});

			mConstrainVertexes.push_back({window_x1, window_y1, window_z1});
			mConstrainVertexes.push_back({window_x2, window_y2, window_z1});
			mConstrainVertexes.push_back({window_x2, window_y1, window_z2});
			mConstrainVertexes.push_back({window_x1, window_y2, window_z2});

			mWindowConstrainsPerSpaceQH[spaceID] = addConstrainVertexQH;

			// add all spaces to initial conformal model
			// then make the geometry conformal model conformal with the function
			// makeConformal, which will add the necessary geometry to make the geometry
			// conformal and remove the geometry that is not needed anymore.
			addSpace(*space);
		}

		this->makeConformal(mConstrainVertexes);
	}

	// Window grammar for models based on ms_N_building (non-orthogonal BSDs, triangulation method).
	// Same settings file format as the orthogonal grammar above, but the window is positioned
	// relative to each space's actual vertical quadrilateral surfaces.
	template <>
	void cf_building_model::windowGrammarND<DEFAULT_WINDOW_GEOMETRY_GRAMMAR>(const std::string& fileNameWindowSettings)
	{
        std::fstream input(fileNameWindowSettings);
		if (!input.is_open())
		{
			std::stringstream errorMessage;
			errorMessage << "\nCould not read window grammar settings, failed to open file:"
									<< fileNameWindowSettings << "\n(bso/spatial_design/conformal/window_grammars/default_window_grammar.cpp)" << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}

		double WindowHeightRatio;
		double WindowWidthRatio;
		double StartLocationWindow;

		std::string line;
		boost::char_separator<char> sep(","); // defines what separates tokens in a string
		typedef boost::tokenizer< boost::char_separator<char> > t_tokenizer; // settings for the boost::tokenizer
		char type_ID; // holds information about what type of information is described by the line currently read

		while(!input.eof())	//Get properties from file
		{
			getline(input, line); // get the next line from the file
			boost::algorithm::trim(line); // remove white space from start and end of line (to see if it is an empty line, remove any incidental white space)
			if (line == "") //skip empty lines (tokenizer does not like it)
			{
				continue; // continue to next line
			}
			t_tokenizer tok(line, sep); // tokenize the line
			t_tokenizer::iterator token = tok.begin(); // set iterator to first token

			try
			{
				type_ID = bso::utilities::trim_and_cast_char(*token);	++token; // interpret first token as type ID
			}
			catch (std::exception& e){continue;}

			if(type_ID == 'C') // the 'C' indicators, indicate the lines which describe the window properties
			{
				WindowHeightRatio = bso::utilities::trim_and_cast_double(*token); ++token;
				WindowWidthRatio = bso::utilities::trim_and_cast_double(*token); ++token;
				StartLocationWindow = bso::utilities::trim_and_cast_double(*token); ++token;
				break; // stop reading the file after retrieving the window properties line, since the other information in the file is not relevant and there is only one line defining the windows.
			}
			else{continue;} // if the line does not describe window properties, skip to next line
		}
		input.close();

		// Check retrieved window properties for validity, if not valid, throw error with explanation of the valid range and the retrieved values for easier debugging.
		if ((WindowHeightRatio+StartLocationWindow)>100 || WindowHeightRatio>100 || WindowWidthRatio>100)
		{
			std::stringstream errorMessage;
			errorMessage << "\nEither input variables: WindowHeightRatio[%], WindowWidthRatio[%], or StartLocationWindow[%] \n"
									<< "is out of valid range. The valid range for these variables is\n"
									<< " between 0 and 100, and the sum of WindowHeightRatio and StartLocationWindow should not exceed 100.\n"
									<< "The input values are: WindowHeightRatio = " << WindowHeightRatio << "%, WindowWidthRatio = " << WindowWidthRatio << "%, StartLocationWindow = " << StartLocationWindow << "%.\n"
									<< "the window settings file should be checked for the correct input of these variables, see: "
									<< fileNameWindowSettings << "\n(bso/spatial_design/conformal/window_grammars/default_window_grammar.cpp)" << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}

		// Generate the window constraint points to account for in the conformal model
		// for each space in the MS model based on the retrieved window properties and
		// store in member variable mConstrainsPerSpaceQH for later use in the generation of the conformal model.
		for (const auto& space : mMSNModel)
		{
			unsigned int spaceID = space->getID();
			std::vector<bso::utilities::geometry::vertex> addConstrainVertexTP;
			const auto& spaceGeometry = space->getGeometry();

			// get vertical quadrilaterals of space to determine the window location and dimensions,
			// as these are defined in relation to the vertical surfaces of the space.
			for(const auto& quad : spaceGeometry.getQuadrilateral())
			{
				if(quad.getNormal().isHorizontal(mTol))
				{
					// This is a vertical surface; identify bottom and top edges
					// Find minZ and maxZ to identify which vertices are bottom vs top
					double minZ = quad[0].z();
					double maxZ = quad[0].z();

					for(const auto& vertex : quad)
					{
						minZ = std::min(minZ, vertex.z());
						maxZ = std::max(maxZ, vertex.z());
					}

					// Separate vertices into bottom and top pairs
					std::vector<bso::utilities::geometry::vertex> bottomVertices;
					std::vector<bso::utilities::geometry::vertex> topVertices;

					for(const auto& vertex : quad)
					{
						if(std::abs(vertex.z() - minZ) < mTol)
						{
							bottomVertices.push_back(vertex);
						}
						else if(std::abs(vertex.z() - maxZ) < mTol)
						{
							topVertices.push_back(vertex);
						}
					}

					// Calculate actual edge dimensions using the actual vertices
					// Horizontal width is the distance between the two bottom vertices
					double horizontalWidth = std::sqrt(
						std::pow(bottomVertices[1].x() - bottomVertices[0].x(), 2) +
						std::pow(bottomVertices[1].y() - bottomVertices[0].y(), 2)
					);

					double verticalHeight = maxZ - minZ;

					// Calculate the horizontal direction unit vector along the bottom edge
					bso::utilities::geometry::vertex horizontalDir = {
						(bottomVertices[1].x() - bottomVertices[0].x()) / horizontalWidth,
						(bottomVertices[1].y() - bottomVertices[0].y()) / horizontalWidth,
						0
					};

					// Calculate window dimensions based on actual surface dimensions
					double windowHeight = (WindowHeightRatio/100) * verticalHeight;
					double windowWidth = (WindowWidthRatio/100) * horizontalWidth;

					// Calculate window vertical position
					double windowBottomZ = minZ + ((100 - StartLocationWindow)/100) * verticalHeight - windowHeight;
					double windowTopZ = minZ + ((100 - StartLocationWindow)/100) * verticalHeight;

					// Calculate window horizontal position (centered on surface)
					// Start from the center of the bottom edge
					bso::utilities::geometry::vertex edgeCenter = {
						(bottomVertices[0].x() + bottomVertices[1].x()) / 2,
						(bottomVertices[0].y() + bottomVertices[1].y()) / 2,
						0
					};

					// Calculate the start and end points of the window along the horizontal direction
					bso::utilities::geometry::vertex windowHorizontalStart = {
						edgeCenter.x() - (windowWidth / 2) * horizontalDir.x(),
						edgeCenter.y() - (windowWidth / 2) * horizontalDir.y(),
						0
					};

					bso::utilities::geometry::vertex windowHorizontalEnd = {
						edgeCenter.x() + (windowWidth / 2) * horizontalDir.x(),
						edgeCenter.y() + (windowWidth / 2) * horizontalDir.y(),
						0
					};

					// Create constraint vertices at the four corners of the window
					addConstrainVertexTP.push_back({windowHorizontalStart.x(), windowHorizontalStart.y(), windowBottomZ});
					addConstrainVertexTP.push_back({windowHorizontalEnd.x(), windowHorizontalEnd.y(), windowBottomZ});
					addConstrainVertexTP.push_back({windowHorizontalEnd.x(), windowHorizontalEnd.y(), windowTopZ});
					addConstrainVertexTP.push_back({windowHorizontalStart.x(), windowHorizontalStart.y(), windowTopZ});

					mAddDelaunayConstrains.push_back({windowHorizontalStart.x(), windowHorizontalStart.y(), windowBottomZ});
					mAddDelaunayConstrains.push_back({windowHorizontalEnd.x(), windowHorizontalEnd.y(), windowBottomZ});
					mAddDelaunayConstrains.push_back({windowHorizontalEnd.x(), windowHorizontalEnd.y(), windowTopZ});
					mAddDelaunayConstrains.push_back({windowHorizontalStart.x(), windowHorizontalStart.y(), windowTopZ});
				}
			}

			mWindowConstrainsPerSpaceTP[spaceID] = addConstrainVertexTP;
		}

		makeConformalND(mMSNModel, mAddDelaunayConstrains);
	}

} // conformal
} // spatial_design
} // bso

#endif // BSO_CF_BUILDING_MODEL_DEFAULT_WINDOW_GEOMETRY_GRAMMAR_CPP
