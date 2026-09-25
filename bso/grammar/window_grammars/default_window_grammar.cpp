#ifndef BSO_GRAMMAR_WINDOW_GRAMMAR_DEFAULT_WINDOW_GRAMMAR_CPP
#define BSO_GRAMMAR_WINDOW_GRAMMAR_DEFAULT_WINDOW_GRAMMAR_CPP

/*
Window Grammar Specialization: DEFAULT_WINDOW_GRAMMAR

This grammar specifies windows by utilizing the settings WindowHeightRatio, WindowWidthRatio, and StartLocationWindow.
Which define the window height and width as a percentage of the vertical and horizontal dimensions 
of the space's vertical quadrilateral surfaces, respectively, and the vertical position of the window 
as a percentage of the vertical dimension of the space's vertical quadrilateral surfaces.

IMPORTANT: This grammar is designed to work in conjunction with the default window geometry grammar (DEFAULT_WINDOW_GEOMETRY_GRAMMAR) defined in
/BSO-toolbox-V2/bso/spatial_design/conformal/window_geometry_grammars/default_window_geometry_grammar.cpp. 
The window geometry grammar provides the necessary window geometry information (in the conformal model)
that this window grammar relies on to identify which existing geometry is a window or not. 
Therefore, it is first required" to initiate the conformal model with the constructor utilizing the prior mentioned window geometry grammar.
*/

namespace bso { namespace grammar {

	// Helper functions for window geometry detection (used only within this grammar)
	namespace window_grammar_helpers {
		void sortQuadVertex(std::vector<bso::utilities::geometry::vertex>& quadVertex) 
		{
			/*
            desired format:
            x1 y1 z1    || x1 y1 z1
            x1 y2 z1    || x2 y1 z1
            x1 y2 z2    || x2 y1 z2
            x1 y1 z2    || x1 y1 z2
            where: *1 = the smallest and *2=the largest
            */
            
            //First sort on z
            std::sort(quadVertex.begin(), quadVertex.end(), [](const bso::utilities::geometry::vertex& a, 
			const bso::utilities::geometry::vertex& b) 
			{
				return a[2] < b[2];
			});

			bso::utilities::geometry::vertex p0 = quadVertex[0];
			bso::utilities::geometry::vertex p1 = quadVertex[1];
			bso::utilities::geometry::vertex p2 = quadVertex[2];
			bso::utilities::geometry::vertex p3 = quadVertex[3];
			bso::utilities::geometry::vertex tempPoint;

            
			if (quadVertex[0][1] == quadVertex[1][1] && 
			    quadVertex[1][1] == quadVertex[2][1] && 
			    quadVertex[2][1] == quadVertex[3][1])
			{ //if all y values are the same
				if (p0[0]>p1[0]){
					tempPoint = p0;
					p0 = p1;
					p1 = tempPoint;
				}
				if (p3[0]>p2[0]){
					tempPoint = p3;
					p3 = p2;
					p2 = tempPoint;        
				}
				quadVertex[0] = p0;
				quadVertex[1] = p1;
				quadVertex[2] = p2;
				quadVertex[3] = p3;
			}   
			else if (quadVertex[0][0] == quadVertex[1][0] && 
			         quadVertex[1][0] == quadVertex[2][0] && 
			         quadVertex[2][0] == quadVertex[3][0])
			{ //if all x values are the same
				if (p0[1]>p1[1]){
					tempPoint = p0;
					p0 = p1;
					p1 = tempPoint;
				}
				if (p3[1]>p2[1]){
					tempPoint = p3;
					p3 = p2;
					p2 = tempPoint;        
				}
				quadVertex[0] = p0;
				quadVertex[1] = p1;
				quadVertex[2] = p2;
				quadVertex[3] = p3;
			}
            else 
            { //if it is a floor/ceiling. 
            quadVertex[0] = p0;
            quadVertex[1] = p1;
            quadVertex[2] = p2;
            quadVertex[3] = p3;
            }
		}

		bool isQuadInWindowArea(std::vector<bso::utilities::geometry::vertex> quadVertex, 
			const std::map<unsigned int, std::vector<bso::utilities::geometry::vertex>>& ConstrainsPerSpace, 
			const double& tol = 1e-3)
		{
			sortQuadVertex(quadVertex);
			bool isWindow = false;

			for (const auto& [spaceID, BaseWindowPoints] : ConstrainsPerSpace)
			{
				double XdomainStart = BaseWindowPoints[0][0];
				double XdomainEnd = BaseWindowPoints[1][0];
				double YdomainStart = BaseWindowPoints[0][1];
				double YdomainEnd = BaseWindowPoints[1][1];
				double ZdomainStart = BaseWindowPoints[0][2];
				double ZdomainEnd = BaseWindowPoints[2][2];

				if (quadVertex[0][1] == quadVertex[2][1])
				{
					if (quadVertex[0][0] <= XdomainEnd && quadVertex[0][0] >= XdomainStart &&
					    quadVertex[2][0] <= XdomainEnd && quadVertex[2][0] >= XdomainStart &&
					    quadVertex[0][2] <= ZdomainEnd && quadVertex[0][2] >= ZdomainStart &&
					    quadVertex[2][2] <= ZdomainEnd && quadVertex[2][2] >= ZdomainStart)
					{
						isWindow = true;
						break;
					}
				}
				else if (quadVertex[0][0] == quadVertex[2][0])
				{
					if (quadVertex[0][1] <= YdomainEnd && quadVertex[0][1] >= YdomainStart &&
					    quadVertex[2][1] <= YdomainEnd && quadVertex[2][1] >= YdomainStart &&
					    quadVertex[0][2] <= ZdomainEnd && quadVertex[0][2] >= ZdomainStart &&
					    quadVertex[2][2] <= ZdomainEnd && quadVertex[2][2] >= ZdomainStart)
					{
						isWindow = true;
						break;
					}
				}
			}
			return isWindow;      
		}

		bool isTPQuadInWindowArea(std::vector<bso::utilities::geometry::vertex> quadVertex, 
			const std::map<unsigned int, std::vector<bso::utilities::geometry::vertex>>& ConstrainsPerSpace,
			const double& tol = 1e-3)
		{
			bool isWindow = false;
			for (const auto& [spaceID, BaseWindowPoints] : ConstrainsPerSpace) 
			{
				for (size_t constraintIdx = 0; constraintIdx + 3 < BaseWindowPoints.size(); constraintIdx += 4)
				{
					std::vector<bso::utilities::geometry::vertex> constraintQuad;
					for (size_t i = 0; i < 4; ++i)
						constraintQuad.push_back(BaseWindowPoints[constraintIdx + i]);
					
					bso::utilities::geometry::quadrilateral windowQuad(constraintQuad, tol);
					bool allVerticesInWindow = true;
					for (const auto& v : quadVertex)
					{
						if (!windowQuad.isInsideOrOn(v, tol))
						{
							allVerticesInWindow = false;
							break;
						}
					}
					if (allVerticesInWindow)
					{
						isWindow = true;
						break;
					}
				}
				if (isWindow) break;
			}
			return isWindow;      
		}
	} // namespace window_grammar_helpers

	// Window grammar for DEFAULT_WINDOW_GRAMMAR: identifies rectangles that should be marked as windows.
	// This runs after properties are initialized but before any dependent grammars (sd, bp, dl).
	template <>
	void grammar::window_grammar<DEFAULT_WINDOW_GRAMMAR>()
	{
		for (const auto& [cfRec, rectProp] : mRectangleProperties)
		{
			// Skip non-space-separating rectangles (windows must separate spaces)
			if (!rectProp->isSpaceSeparating())
				continue;

			// Skip floors/roofs/overhangs - windows must be walls
			if (!rectProp->isWall())
				continue;

			// Get rectangle vertices
			std::vector<bso::utilities::geometry::vertex> quadVertex;
			for (const auto& v : *cfRec)
				quadVertex.push_back(v);

			// Determine if this rectangle is in a window area based on conformal model constraints
			bool isInWindowArea = false;
			if (mTriangleProperties.size() > 0) // Triangles only occur in the TP conformal model, so it is a good indicator that we are utilizing a TP conformal model
				isInWindowArea = window_grammar_helpers::isTPQuadInWindowArea(quadVertex, mWindowsPerSpace, 1e-3);
			else
				isInWindowArea = window_grammar_helpers::isQuadInWindowArea(quadVertex, mWindowsPerSpace, 1e-3);

			if (!isInWindowArea)
				continue;

			// Check if rectangle is between/adjacent 'open' spaces (space type "A") or if it is adjacent to a closed space (space type "B")
            // if space type is not defined, it is automatically defined as 'open' for external walls, and closed for internal walls.
			bool isWindow = false;
			const auto& surfaces = cfRec->cfSurfaces();
			
			if (surfaces.size() == 2)  // Internal wall between two spaces
			{
				const auto spaceType0 = (surfaces[0]->cfSpaces())[0]->getSpaceType();
				const auto spaceType1 = (surfaces[1]->cfSpaces())[0]->getSpaceType();
                if(spaceType0 == "A" && spaceType1 == "A") isWindow = true;
                else if (spaceType0 == "B" && spaceType1 == "A") isWindow = false;
                else if (spaceType0 == "A" && spaceType1 == "B") isWindow = false;
                else if (spaceType0 != "A" && spaceType0 != "B" && spaceType1 != "A" && spaceType1 != "B") isWindow = true;
			}
			else if (surfaces.size() == 1)  // External wall
			{
				const auto spaceType0 = (surfaces[0]->cfSpaces())[0]->getSpaceType();
                if(spaceType0 == "A") isWindow = true;
                else if (spaceType0 == "B") isWindow = false;
                else if (spaceType0 != "A" && spaceType0 != "B") isWindow = true;               
			}

			// Mark as window if all conditions met
			if (isWindow) rectProp->setIsWindow(true);
		}
	}

} // namespace grammar
} // namespace bso

#endif // BSO_GRAMMAR_WINDOW_GRAMMAR_DEFAULT_WINDOW_GRAMMAR_CPP
