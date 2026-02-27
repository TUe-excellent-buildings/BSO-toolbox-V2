#ifndef GRAMMAR_DL_RECTANGLE_RULE_CPP
#define GRAMMAR_DL_RECTANGLE_RULE_CPP

namespace bso { namespace grammar { namespace rule_set { namespace dl_rule_set {

dl_rectangle_rule::dl_rectangle_rule(rectangle_property* rectangleProperty)
{
	mRectangleProperty = rectangleProperty;
} // ctor()

dl_rectangle_rule::~dl_rectangle_rule()
{
	
} // dtor()

/*
	rules are applied in the grammer
	rules are the functions that enable the dl model to be generated. 
	you apply it per structure type, in this case Rectangles!
*/

//----------------------------------------------------------------------------------------------------------------//
std::string createFolder(const std::string& fileName) 
{
    static int count = 0;
    
    std::string folderName = "Iteration_" + std::to_string(count);
    std::string command = "mkdir -p \"" + folderName + "\""; // Use mkdir command to create directory

    int result = std::system(command.c_str());

    if (result != 0) 
    {   
        std::cerr << "Error creating directory: " << folderName << std::endl;
        std::cerr << "Command executed: " << command << std::endl;
    }
    else    //if folder created, add dl_setting file also to it, since you can change it per run to change window properties. 
    {
        std::string copyCommand = "cp \"" + fileName + "\" \"" + folderName + "/\"";
        int copyResult = std::system(copyCommand.c_str());

        if (copyResult != 0) 
        {
            std::cerr << "Error copying file to directory: " << folderName << std::endl;
            std::cerr << "Command executed: " << copyCommand << std::endl;
        }
    }


    ++count;
    return folderName;    
}
//----------------------------------------------------------------------------------------------------------------//

float formatNumber(float num) 
{
    const float threshold = 1e-6; // Numbers below this threshold will be treated as zero
    return (std::fabs(num) < threshold) ? 0.0f : num;
}

double roundToTwoDecimals(double num) {
    return std::round(num * 100) / 100;
}

std::string formatDuration(std::chrono::microseconds duration) {
    auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
    duration -= hours;
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration);
    duration -= minutes;
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    duration -= seconds;
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration);

    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << hours.count() << ":"
        << std::setw(2) << std::setfill('0') << minutes.count() << ":"
        << std::setw(2) << std::setfill('0') << seconds.count();
    return oss.str();
}

void sortQuadVertex(std::vector<bso::utilities::geometry::vertex>& quadVertex) 
{

    /*
    dersired format:
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

    //if all y values are the same
    if (quadVertex[0][1] == quadVertex[1][1] && 
        quadVertex[1][1] == quadVertex[2][1] && 
        quadVertex[2][1] == quadVertex[3][1])
    {
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
    else if 
    //if all x values are the same
    (quadVertex[0][0] == quadVertex[1][0] && 
    quadVertex[1][0] == quadVertex[2][0] && 
    quadVertex[2][0] == quadVertex[3][0])
    {
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

    } else 
    //if it is a floor/celing. 
    { 
    quadVertex[0] = p0;
    quadVertex[1] = p1;
    quadVertex[2] = p2;
    quadVertex[3] = p3;
    }
}

//----------------------------------------------------------------------------------------------------------------//

void writeMaterials(std::ofstream& file, const std::vector<std::string> ReflectanceValues, const std::vector<std::string> WindowProperties)
{
	const std::string ReflectanceValueFloor = ReflectanceValues[0];
	const std::string ReflectanceValueCeiling = ReflectanceValues[1];
	const std::string ReflectanceValueWall = ReflectanceValues[2];
	const std::string Specularity = ReflectanceValues[3];
	const std::string Roughness = ReflectanceValues[4];
	const std::string transmissivity = WindowProperties[0];
	const std::string transmittance = WindowProperties[1];

    file << "void plastic Floor 0 0 5 "<< ReflectanceValueFloor <<" "<< Specularity <<" "<< Roughness << "\n";
    file << "void plastic Ceiling 0 0 5 "<< ReflectanceValueCeiling <<" "<< Specularity <<" "<< Roughness << "\n";
    file << "void plastic Wall 0 0 5 "<< ReflectanceValueWall <<" "<< Specularity <<" "<< Roughness << "\n\n";
    file << "void glass window_glass 0 0 3 "<<transmissivity<<"\n"; //material, only transmit light

	//Below window materials are if window needs to be of illum, aka a light source, to handle illuminance calculation more efficient. 
	//However than window is dependent on surface orientation. Therefore can only transmit light into one direction. 
    //file << "skyfunc brightfunc window_dist 2 winxmit winxmit.cal 0 0\n"; 						// include angluar transmittence
    //file << "window_dist illum window_illum \n\n1 window_glass\n0\n3 "<<transmittance<<"\n\n"; 	// illum makes it a light source
    file << std::endl;
}

void writeFloorRadiance(std::ofstream& file, std::vector<bso::utilities::geometry::vertex>& quadVertex)
{
    static int count = 0;
    ++count;

    file << "Floor polygon F"<<count<<"\n";
    file << "0\n0\n12\n";

    for (const auto& vertex : quadVertex) {
        file << std::setw(10) << std::right << vertex[0] << " "
             << std::setw(10) << std::right << vertex[1] << " "
             << std::setw(10) << std::right << vertex[2] << "\n";
    }
    file << std::endl;

}

void writeCeilingRadiance(std::ofstream& file, std::vector<bso::utilities::geometry::vertex>& quadVertex)
{
	static int count = 0;
    ++count;

    if (quadVertex[0][2] != 0)
    {
        file << "Ceiling polygon C"<<count<<"\n";
        file << "0\n0\n12\n";
        for (const auto& vertex : quadVertex) {
            file << std::setw(10) << std::right << vertex[0] << " "
                 << std::setw(10) << std::right << vertex[1] << " "
                 << std::setw(10) << std::right << (vertex[2]-0.1) << "\n"; //-1 to place ceiling below floor for correct reflectance value
        }
        file << std::endl;
    }
}

void writeWallRadiance(std::ofstream& file, std::vector<bso::utilities::geometry::vertex>& quadVertex)
{
    static int count = 0;
    ++count;

    sortQuadVertex(quadVertex);
    file << "Wall polygon W"<<count<<"\n";
    file << "0\n0\n12\n";
    for (const auto& vertex : quadVertex) {
        file << std::setw(10) << std::right << vertex[0] << " "
             << std::setw(10) << std::right << vertex[1] << " "
             << std::setw(10) << std::right << vertex[2] << "\n";
    }
    file << std::endl;
}

void writeWindowRadiance(std::ofstream& file, std::vector<bso::utilities::geometry::vertex>& quadVertex) 
{
    static int count = 0;
    ++count;

    sortQuadVertex(quadVertex);
    file << "window_glass polygon window"<<count<<"\n";
    file << "0\n0\n12\n"; // 12 coordinates: 4 points * 3 coordinates each
    
    file << std::setw(10) << formatNumber(quadVertex[0][0]) << " " 
         << std::setw(10) << formatNumber(quadVertex[0][1]) << " " 
         << std::setw(10) << formatNumber(quadVertex[0][2]) << "\n";
    
    file << std::setw(10) << formatNumber(quadVertex[1][0]) << " " 
         << std::setw(10) << formatNumber(quadVertex[1][1]) << " " 
         << std::setw(10) << formatNumber(quadVertex[1][2]) << "\n";
    
    file << std::setw(10) << formatNumber(quadVertex[2][0]) << " " 
         << std::setw(10) << formatNumber(quadVertex[2][1]) << " " 
         << std::setw(10) << formatNumber(quadVertex[2][2]) << "\n";
    
    file << std::setw(10) << formatNumber(quadVertex[3][0]) << " " 
         << std::setw(10) << formatNumber(quadVertex[3][1]) << " " 
         << std::setw(10) << formatNumber(quadVertex[3][2]) << "\n\n";
}

//----------------------------------------------------------------------------------------------------------------//

bool isQuadInWindowArea(std::vector<bso::utilities::geometry::vertex>& quadVertex, const std::map<unsigned int, std::vector<bso::utilities::geometry::vertex>>& ConstrainsPerSpace)
{
    /*
    this function checks if the current rectangle is within the bounds/area of the base window area of that space 
    it loops per space
    */

    sortQuadVertex(quadVertex);
    bool isWindow = false;

    for (const auto& [spaceID, BaseWindowPoints] : ConstrainsPerSpace) 
    {

        double XdomainStart = BaseWindowPoints[0][0];
        double XdomainEnd = BaseWindowPoints[1][0];

        double YdomainStart = BaseWindowPoints[0][1];
        double YdomainEnd = BaseWindowPoints[1][1];

        double ZdomainStart = BaseWindowPoints[0][2];
        double ZdomainEnd =BaseWindowPoints[2][2];


        if (quadVertex[0][1] == quadVertex[2][1]) //y the same
        {
            //if y the same check if points lay in z and x domain
            if (quadVertex[0][0] <= XdomainEnd && quadVertex[0][0] >= XdomainStart &&   //check quadVertex[0] X
                quadVertex[2][0] <= XdomainEnd && quadVertex[2][0] >= XdomainStart &&   //check quadVertex[2] X
                quadVertex[0][2] <= ZdomainEnd && quadVertex[0][2] >= ZdomainStart &&   //check quadVertex[0] Z
                quadVertex[2][2] <= ZdomainEnd && quadVertex[2][2] >= ZdomainStart      //check quadVertex[2] Z
                )
            {
                isWindow = true;
                break;
            }
        }
        else if (quadVertex[0][0] == quadVertex[2][0]) //x the same
        {
            //if x the same check if points lay in z and y domain
            if (quadVertex[0][1] <= YdomainEnd && quadVertex[0][1] >= YdomainStart &&   //check quadVertex[0] Y
                quadVertex[2][1] <= YdomainEnd && quadVertex[2][1] >= YdomainStart &&   //check quadVertex[2] Y
                quadVertex[0][2] <= ZdomainEnd && quadVertex[0][2] >= ZdomainStart &&   //check quadVertex[0] Z
                quadVertex[2][2] <= ZdomainEnd && quadVertex[2][2] >= ZdomainStart      //check quadVertex[2] Z
                )
            {
                isWindow = true;
                break;
            }
        }
    } //loop over space

  return isWindow;      
}

bool isOpenInternal(bso::spatial_design::conformal::cf_rectangle* cfRec)
{
    bool hasWindowProperties = false;
    std::vector<std::string> SpaceTypeSurf;
    
    for (const auto j: (cfRec->cfSurfaces())) //check to which surface this rectangle belongs (j=surface)
    {
        const auto SpaceType0 = ((j->cfSpaces())[0])->getSpaceType();
        SpaceTypeSurf.push_back(SpaceType0);
    } //loop surface

    if (
            SpaceTypeSurf[0] == "A" &&
            SpaceTypeSurf[1] == "A"
        )
        {
            return hasWindowProperties=true;
        }
    else if
        (
            SpaceTypeSurf[0] == "B" ||
            SpaceTypeSurf[1] == "B"                                
        )
        {
            return hasWindowProperties=false;
        }  
    else if //incase no space type was given as input -> default no windows
        (
            (SpaceTypeSurf[0] != "A" && SpaceTypeSurf[0] != "B") ||
            (SpaceTypeSurf[1] != "A" && SpaceTypeSurf[1] != "B")
        )
        {
            hasWindowProperties = false;
            // std::cerr   << "No Space type (open/closed) defined in input file"
            //             << "\n See isOpenInternal dl_rectangle_rule.cpp "
            //             << "\n Default is: closed" << std::endl;
        }
    return hasWindowProperties;
}

bool isOpenExternal(bso::spatial_design::conformal::cf_rectangle* cfRec)
{
    bool hasWindowProperties = false;

    for (const auto j: (cfRec->cfSurfaces())) //check to which surface this rectangle belongs (j=surface)
    {
        const auto SpaceType0 = ((j->cfSpaces())[0])->getSpaceType();
        if (SpaceType0 == "A") // A = Open
        {
            return hasWindowProperties=true;
        } 
        else if (SpaceType0 == "B") // B = Closed
        {
            return hasWindowProperties=false;
        } 
        else if //if no space type is applied.
            (SpaceType0 != "A" && SpaceType0 != "B")
        {
            hasWindowProperties = false;
            // std::cerr   << "No Space type (open/closed) defined in input file"
            //             << "\n See isOpenExternal dl_rectangle_rule.cpp "
            //             << "\n Default is: closed" << std::endl;
        }
    } //loop surface
    return hasWindowProperties;
}

//----------------------------------------------------------------------------------------------------------------//

std::vector<std::vector<const bso::spatial_design::conformal::cf_rectangle*>> setCardinalDirection(std::vector<const bso::spatial_design::conformal::cf_rectangle* /*rectangle*/>& Rectangles)
{
    //According to Thijs the Goede vertical layers for cardinal directions. See below for his instructins
    /*
    According to Thijs the Goede vertical layers for cardinal directions.

    https://research.tue.nl/en/studentTheses/co-evolutionary-building-design-simulation-for-structural-optimiz 
    Co-evolutionary building design simulation for structural optimization with facade openings.
    de Goede, T.Y.
    Award date: 2019

    {
        First, rectangles with equal center point’s 𝑥𝑥-coordinates are stored in a temporary North-South
        set, and rectangles with equal center point’s 𝑦𝑦-coordinates are stored in a temporary East-West set. Both temporary
        sets list vertical layers in ascending order over the width or depth of a BSD. 
        
        Second, East- or West-facing rectangles are removed from the temporary North-South set. For this purpose, the procedure compares the
        rectangle’s center point’s 𝑥𝑥-coordinate with its first-listed associated vertex 𝑥𝑥-coordinate, if they are equal then
        the rectangle is removed. The same principle applies to the temporary East-West set; here, 𝑦𝑦-coordinates are
        compared. 
        
        Third, the temporary North-South set is divided into a North set and South set and the temporary East-
        West set is divided into an East set and West set. To accomplish this reordering, each vertical layer in a temporary
        set is evaluated based on the rectangles center point’s 𝑧𝑧-coordinates. In case of the temporary North-South set,
        rectangles in a vertical layer that share a center point’s 𝑧𝑧-coordinate are listed in ascending order based on their
        center point’s 𝑦𝑦-coordinates. Subsequently, the first rectangle is stored in the corresponding vertical layer of the
        South set, the second rectangle is stored in the corresponding vertical layer of the North set, and vice versa. The
        same principle applies to the temporary East-West set; here, the ascending order is based on the center point’s 𝑥𝑥-
        coordinates.
    }

    */

    struct RectangleDataStructure 
    {
        const bso::spatial_design::conformal::cf_rectangle* rectangle;
        bso::utilities::geometry::vertex center; 
        std::vector<bso::utilities::geometry::vertex> cornerVertices;
    };

    std::vector<RectangleDataStructure> AllRectangles;
    RectangleDataStructure rectData;

    std::vector<std::vector<bso::utilities::geometry::vertex>> quadVertex; //vector rectangle vertexes
    std::vector<bso::utilities::geometry::vertex> RCCenter;  //vector center vertexes

    for(const auto j: (Rectangles))
    {
        const bso::utilities::geometry::quadrilateral* quad = j;
        std::vector<bso::utilities::geometry::vertex> verticesForQuad;
        for (const auto& vertex : *quad) {verticesForQuad.push_back(vertex);}

        rectData.rectangle = j;
        rectData.center = j->getCenter();
        rectData.cornerVertices = verticesForQuad;
        AllRectangles.push_back(rectData);
    }
    
    //-----------------------------------------------------//
    //First step: Group on x and y (NorthSouth and EastWest)
    //-----------------------------------------------------//

    std::vector<std::vector<RectangleDataStructure>> X_Groups_Rectangles; //North South
    std::vector<std::vector<RectangleDataStructure>> Y_Groups_Rectangles; //East West

    for (int q=0; q<2; ++q) //q=0=x, q=1=y
    {
        if (q==0) 
        {
            std::sort   (AllRectangles.begin(), AllRectangles.end(), 
                        [](const RectangleDataStructure& a, const RectangleDataStructure& b) 
                        { return a.center[0] < b.center[0]; }
                        );
        }
        else if (q==1)
        {
            std::sort   (AllRectangles.begin(), AllRectangles.end(), 
                        [](const RectangleDataStructure& a, const RectangleDataStructure& b) 
                        { return a.center[1] < b.center[1]; }
                        );
        }

        std::vector<std::vector<RectangleDataStructure>> Q_Groups_Rectangles;
        std::vector<RectangleDataStructure> currentGroup;
        double currentq = (AllRectangles.empty()) ? 0.0 : (q == 0 ? AllRectangles[0].center[0] : AllRectangles[0].center[1]);

        for (auto& rect : AllRectangles) 
        {
            double rectq = (q == 0) ? rect.center[0] : rect.center[1];
            if (rectq == currentq) 
            {
                currentGroup.push_back(rect);
            } 
            else 
            {
                // Push the current group to Q_Groups_Rectangles and start a new group
                Q_Groups_Rectangles.push_back(currentGroup);
                currentGroup.clear();
                currentGroup.push_back(rect);
                currentq = rectq;
            }
        }

        if (!currentGroup.empty()) {Q_Groups_Rectangles.push_back(currentGroup);}
    
        if (q == 0) {X_Groups_Rectangles = Q_Groups_Rectangles;}
        if (q == 1) {Y_Groups_Rectangles = Q_Groups_Rectangles;}
    }



    //-----------------------------------------------------//
    //Second step: Remove east-west from northside group and visa versa
    //-----------------------------------------------------//

    // Loop through Xgroups and Ygroups, marking the matching vertex indices
    std::vector<size_t> vertices_to_remove;

    // Loop through Xgroups, marking the matching vertex indices
    for (size_t i = 0; i < X_Groups_Rectangles.size(); ++i) 
    {
        if (!X_Groups_Rectangles[i].empty()) // first vertex of group, x - coordinate (assumed all to be the same)
        {
            const double vertex_x = X_Groups_Rectangles[i][0].center[0]; //i can acces first rectangle, because all rectangles within group have the same x coordinate
            const double corner_vertex_x = X_Groups_Rectangles[i][0].cornerVertices[0][0];
                if (vertex_x == corner_vertex_x ) //first vertex of rectangle, x-coordinate
                {
                    vertices_to_remove.push_back(i);
                }
        } 
    }

    // Remove the marked vertices from Xgroups
    for (int i = vertices_to_remove.size() - 1; i >= 0; --i) 
    {
        X_Groups_Rectangles.erase(X_Groups_Rectangles.begin() + vertices_to_remove[i]);
    }

    //do the same for Y
    vertices_to_remove.clear();
    for (size_t i = 0; i < Y_Groups_Rectangles.size(); ++i) 
    {
        if (!Y_Groups_Rectangles[i].empty()) // first vertex of group, x - coordinate (assumed all to be the same)
        {
            const double vertex_y = Y_Groups_Rectangles[i][0].center[1];    //i can acces first rectangle, because all rectangles within group have the same x coordinate
            const double corner_vertex_y = Y_Groups_Rectangles[i][0].cornerVertices[0][1];
                if (vertex_y == corner_vertex_y ) //first vertex of rectangle, x-coordinate
                {
                    vertices_to_remove.push_back(i);
                }
        } 
    }

    // Remove the marked vertices from Xgroups
    for (int i = vertices_to_remove.size() - 1; i >= 0; --i) 
    {
        Y_Groups_Rectangles.erase(Y_Groups_Rectangles.begin() + vertices_to_remove[i]);
    }
  

    //-----------------------------------------------------//
    // Third step: Split north and south set (X)
    //-----------------------------------------------------//
    std::vector<RectangleDataStructure> NorthRectangles;
    std::vector<RectangleDataStructure> SouthRectangles;

    for (size_t xgroup=0; xgroup < X_Groups_Rectangles.size(); ++xgroup)
    {
        // Group rectangles based on their z-coordinate
        std::vector<std::vector<RectangleDataStructure>> XZ_Groups_Rectangles;
        std::vector<RectangleDataStructure> currentGroup;

        // Sort X_Rectangles by the z-coordinate of their center
        std::sort   (X_Groups_Rectangles[xgroup].begin(), X_Groups_Rectangles[xgroup].end(), 
                    [](const RectangleDataStructure& a, const RectangleDataStructure& b) 
                    { return a.center[2] < b.center[2]; }  // Sort based on z-coordinate
                    );
        // Initialize the current group with the first rectangle's z-coordinate
        double currentz = X_Groups_Rectangles[xgroup].empty() ? 0.0 : X_Groups_Rectangles[xgroup][0].center[2];
        // Group rectangles based on their z-coordinate
        for (auto& rect : X_Groups_Rectangles[xgroup]) 
        {
            double rectz = rect.center[2];  // Get the z-coordinate of the current rectangle
            if (rectz == currentz) 
            {currentGroup.push_back(rect);} 
            else 
            {
                XZ_Groups_Rectangles.push_back(currentGroup);
                currentGroup.clear();
                currentGroup.push_back(rect);
                currentz = rectz;
            }
        }
        // Add the last group if it's not empty
        if (!currentGroup.empty()) {XZ_Groups_Rectangles.push_back(currentGroup);}

        // split group by alternately assigining to the south and north set
        for (auto& group : XZ_Groups_Rectangles) 
        {
            //sort based on y-center cooridnate
            std::sort   (group.begin(), group.end(), 
                        [](const RectangleDataStructure& a, const RectangleDataStructure& b) 
                        { return a.center[1] < b.center[1]; }
                        );

            // Alternate between North and South for each rectangle in the group
            bool toggle = true;
            for (const auto& rect : group) 
            {
                if (toggle) {SouthRectangles.push_back(rect);} 
                else {NorthRectangles.push_back(rect);}
                toggle = !toggle;  // Switch between North and South
            }
        } //assign north south

    } //xgroup


    //same for Y
    std::vector<RectangleDataStructure> EastRectangles;
    std::vector<RectangleDataStructure> WestRectangles;
    for (size_t ygroup=0; ygroup < Y_Groups_Rectangles.size(); ++ygroup)
    {
        // Group rectangles based on their z-coordinate
        std::vector<std::vector<RectangleDataStructure>> YZ_Groups_Rectangles;
        std::vector<RectangleDataStructure> currentGroup;

        // Sort X_Rectangles by the z-coordinate of their center
        std::sort   (Y_Groups_Rectangles[ygroup].begin(), Y_Groups_Rectangles[ygroup].end(), 
                    [](const RectangleDataStructure& a, const RectangleDataStructure& b) 
                    { return a.center[2] < b.center[2]; }  // Sort based on z-coordinate
                    );

        // Initialize the current group with the first rectangle's z-coordinate
        double currentz = Y_Groups_Rectangles[ygroup].empty() ? 0.0 : Y_Groups_Rectangles[ygroup][0].center[2];
        // Group rectangles based on their z-coordinate
        for (auto& rect : Y_Groups_Rectangles[ygroup]) 
        {
            double rectz = rect.center[2];  // Get the z-coordinate of the current rectangle
            if (rectz == currentz) 
            {currentGroup.push_back(rect);} 
            else 
            {
                YZ_Groups_Rectangles.push_back(currentGroup);
                currentGroup.clear();
                currentGroup.push_back(rect);
                currentz = rectz;
            }
        }
        // Add the last group if it's not empty
        if (!currentGroup.empty()) {YZ_Groups_Rectangles.push_back(currentGroup);}

        // split group by alternately assigining to the east and west set
        for (auto& group : YZ_Groups_Rectangles) 
        {
            //sort based on X-center cooridnate
            std::sort   (group.begin(), group.end(), 
                        [](const RectangleDataStructure& a, const RectangleDataStructure& b) 
                        { return a.center[0] < b.center[0]; }
                        );
        
            // Alternate between North and South for each rectangle in the group
            bool toggle = true;
            for (const auto& rect : group) 
            {
                if (toggle) {WestRectangles.push_back(rect);} 
                else {EastRectangles.push_back(rect);}
                toggle = !toggle;  // Switch between East and West
            }
        } //assign east and west
    } //ygroup


    //output (return of function)
    std::vector<std::vector<const bso::spatial_design::conformal::cf_rectangle*>> finalCardinalDirections;
    std::vector<const bso::spatial_design::conformal::cf_rectangle*> Temp;

    for (size_t i =0; i < NorthRectangles.size(); ++i) {Temp.push_back(NorthRectangles[i].rectangle);}
    finalCardinalDirections.push_back(Temp);
    Temp.clear();

    for (size_t i =0; i < EastRectangles.size(); ++i) {Temp.push_back(EastRectangles[i].rectangle);}
    finalCardinalDirections.push_back(Temp);
    Temp.clear();

    for (size_t i =0; i < SouthRectangles.size(); ++i) {Temp.push_back(SouthRectangles[i].rectangle);}
    finalCardinalDirections.push_back(Temp);
    Temp.clear();

    for (size_t i =0; i < WestRectangles.size(); ++i) {Temp.push_back(WestRectangles[i].rectangle);}
    finalCardinalDirections.push_back(Temp);
    Temp.clear();

    if ((NorthRectangles.size()+EastRectangles.size()+SouthRectangles.size()+WestRectangles.size())!= (Rectangles.size()))
    {
        std::stringstream errorMessage;
		errorMessage << "\nError, while determining cardinal directions per rectangle:\n"
								 << "\t"    << (NorthRectangles.size()+EastRectangles.size()+SouthRectangles.size()+WestRectangles.size()) << " != " << (Rectangles.size())
								 << "\t"    << "(bso/grammar/rule_set/dl_rule_set/dl_rectangle_rule.cpp)" << std::endl;
		throw std::invalid_argument(errorMessage.str());
    }

    return finalCardinalDirections;
}
//----------------------------------------------------------------------------------------------------------------//

void filterSurfacesForGrid(std::vector<bso::spatial_design::conformal::cf_surface>& vec, std::vector<unsigned int>& vec2) 
{
    //note that this function works assuming user input of spaces is correct. (NO identical spaceID's)

    for (size_t i = 0; i < vec2.size(); ++i) 
    {
        for (size_t j = i + 1; j < vec2.size(); ++j) 
        {
            if (vec2[i] == vec2[j]) // Check if they are in the same space
            {   
                // Determine which has the highest Z value
                int highestZ = (vec[i][0][2] > vec[j][0][2]) ? i : j;
                
                // Remove from both vectors
                vec2.erase(vec2.begin() + highestZ);
                vec.erase(vec.begin() + highestZ);

                // Since we've erased an element, adjust the outer loop index
                if (highestZ < j) 
                {
                    --j;
                }
                --i;
                break; // Exit the inner loop since we've modified the vectors
            }
        }
    }
}

std::vector<std::tuple<unsigned int, int>> generateGridNEN(	const std::vector<bso::spatial_design::conformal::cf_surface>& vec, std::ofstream& gridFile, 
															std::vector<unsigned int>& SpaceID, const std::vector<double> DF_Settings) {

	const double HorizontalOutdoorIlluminance = DF_Settings[0];
	const double ReferenceHeight = DF_Settings[1];
	const double space_margin = DF_Settings[2];
	const double minGridSize = DF_Settings[3];

    std::vector<int> amountGridPoints;
    std::vector<std::tuple<unsigned int, int>> tupleVec;

    for (int i = 0; i < vec.size(); i++) //loop per surface
    {   
        int numGridPoints = 0;

        // Determine the longer dimension of the surface
        double minX = vec[i][0][0];
        double minY = vec[i][0][1];
        double maxX = vec[i][2][0];
        double maxY = vec[i][2][1];
        double w = maxX-minX;
        double l = maxY-minY;
        double d = std::max(l, w);
        // Calculate the maximum grid cell size
        double p_meter = std::min(minGridSize, 0.5 * std::pow(5, std::log10(d/1000))); 
        double p = p_meter*1000; // mm
        // Calculate the number of points in the relevant dimension
        int num_points_dim = std::ceil(d/p);
        // Calculate the spacing between grid points
        double step = d / (num_points_dim - 1);
        // Calculate the actual dimensions of the grid area (excluding wall thickness)
        double actual_l = l - 2 * space_margin;
        double actual_w = w - 2 * space_margin;
        // Calculate the step sizes for x and y dimensions
        double step_x = actual_w / (num_points_dim - 1);
        double step_y = actual_l / (num_points_dim - 1);
        // make start points
        float start_x = minX;
        float start_y = minY;
        float start_z = vec[i][0][2]; //height per surface.

        // Generate grid points
        for (int k = 0; k < num_points_dim; ++k) {
            for (int j = 0; j < num_points_dim; ++j) {
                double x = start_x + j * step_x + space_margin;
                double y = start_y + k * step_y + space_margin;
                double z = start_z + ReferenceHeight;
                gridFile<< std::setw(10) << x
                        << std::setw(10) << y
                        << std::setw(10) << z
                        << std::setw(10) << 0
                        << std::setw(10) << 0
                        << std::setw(10) << 1 << std::endl;
                numGridPoints++; // Increment the number of grid points
            }
        }//generate grid points
        gridFile << "\n" << std::endl;
        amountGridPoints.push_back(numGridPoints);
        tupleVec.push_back(std::make_tuple(SpaceID[i], amountGridPoints[i]));
    }//loop per surface

    return tupleVec; 
} //function loop

//----------------------------------------------------------------------------------------------------------------//

void dl_rectangle_rule::apply(bso::daylight::dl_model& dl) const
{
	
} // apply()

} // namespace dl_rule_set
} // namespace rule_set
} // namespace grammar
} // namespace bso

#endif // GRAMMAR_DL_RECTANGLE_RULE_CPP