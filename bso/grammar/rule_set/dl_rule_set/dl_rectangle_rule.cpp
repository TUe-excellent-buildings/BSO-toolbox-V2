#ifndef GRAMMAR_DL_RECTANGLE_RULE_CPP
#define GRAMMAR_DL_RECTANGLE_RULE_CPP

namespace bso { namespace grammar { namespace rule_set { namespace dl_rule_set {

// Global polygon counters - reset per scene file
static int gWallCount = 0;
static int gFloorCount = 0;
static int gCeilingCount = 0;
static int gWindowCount = 0;

void resetPolygonCounters()
{
	gWallCount = 0;
	gFloorCount = 0;
	gCeilingCount = 0;
	gWindowCount = 0;
} // resetPolygonCounters()

dl_rectangle_rule::dl_rectangle_rule(rectangle_property* rectangleProperty)
{
	mRectangleProperty = rectangleProperty;
} // ctor()

dl_rectangle_rule::~dl_rectangle_rule()
{
	
} // dtor()

float formatNumber(float num) 
{
    const float threshold = 1e-6; // Numbers below this threshold will be treated as zero
    return (std::fabs(num) < threshold) ? 0.0f : num;
} // formatNumber()

std::string formatDuration(std::chrono::microseconds duration) 
{
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
} // formatDuration()

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
} // sortQuadVertex()

void sortPolygonVertexGeneric(std::vector<bso::utilities::geometry::vertex>& vertices)
{
    /*
    Type-agnostic polygon vertex sorting using convex hull approach.
    Works for any convex polygon with N >= 3 vertices.
    
    Algorithm:
    1. Compute polygon normal via cross product of first two edges
    2. Find centroid (center of mass)
    3. Sort vertices by polar angle around centroid in polygon's plane
    4. Result: vertices ordered counter-clockwise when viewed from normal direction
    */
    
    if (vertices.size() < 3) 
    {
        return; // Cannot sort polygon with less than 3 vertices
    }

    // Step 1: Compute polygon normal using Newell's method (robust for any polygon)
    double nx = 0.0, ny = 0.0, nz = 0.0;
    for (size_t i = 0; i < vertices.size(); ++i) 
    {
        const auto& v0 = vertices[i];
        const auto& v1 = vertices[(i + 1) % vertices.size()];
        
        nx += (v0[1] - v1[1]) * (v0[2] + v1[2]);
        ny += (v0[2] - v1[2]) * (v0[0] + v1[0]);
        nz += (v0[0] - v1[0]) * (v0[1] + v1[1]);
    }
    
    // Normalize the normal vector
    double normalLength = std::sqrt(nx*nx + ny*ny + nz*nz);
    if (normalLength < 1e-10)
    {
        return; // Degenerate polygon (all points collinear or coincident)
    }
    nx /= normalLength;
    ny /= normalLength;
    nz /= normalLength;

    // Step 2: Compute centroid
    double cx = 0.0, cy = 0.0, cz = 0.0;
    for (const auto& vertex : vertices)
    {
        cx += vertex[0];
        cy += vertex[1];
        cz += vertex[2];
    }
    cx /= vertices.size();
    cy /= vertices.size();
    cz /= vertices.size();

    // Step 3: Create orthonormal basis in the polygon's plane
    // First basis vector: direction along first edge from centroid
    double ux = vertices[0][0] - cx;
    double uy = vertices[0][1] - cy;
    double uz = vertices[0][2] - cz;
    
    double uLength = std::sqrt(ux*ux + uy*uy + uz*uz);
    if (uLength < 1e-10)
    {
        return; // All vertices at centroid (degenerate)
    }
    ux /= uLength;
    uy /= uLength;
    uz /= uLength;

    // Second basis vector: perpendicular to both normal and first basis vector
    // v = normal × u
    double vx = ny*uz - nz*uy;
    double vy = nz*ux - nx*uz;
    double vz = nx*uy - ny*ux;

    // Step 4: Compute polar angles and sort
    std::vector<std::pair<double, size_t>> angleIndices;
    
    for (size_t i = 0; i < vertices.size(); ++i)
    {
        // Vector from centroid to vertex
        double dx = vertices[i][0] - cx;
        double dy = vertices[i][1] - cy;
        double dz = vertices[i][2] - cz;
        
        // Project onto basis vectors to get 2D coordinates in polygon plane
        double px = dx*ux + dy*uy + dz*uz;  // Component along u
        double py = dx*vx + dy*vy + dz*vz;  // Component along v
        
        // Compute angle (atan2 handles all quadrants correctly)
        double angle = std::atan2(py, px);
        angleIndices.push_back({angle, i});
    }

    // Sort by angle
    std::sort(angleIndices.begin(), angleIndices.end());

    // Step 5: Reorder vertices based on sorted angles
    std::vector<bso::utilities::geometry::vertex> sortedVertices;
    for (const auto& [angle, idx] : angleIndices)
    {
        sortedVertices.push_back(vertices[idx]);
    }
    
    vertices = sortedVertices;
} // sortPolygonVertexGeneric()

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

	// Below window materials are if window needs to be of illum, aka a light source, to handle illuminance calculation more efficient. 
	// However than window is dependent on surface orientation. Therefore can only transmit light into one direction. 
    // file << "skyfunc brightfunc window_dist 2 winxmit winxmit.cal 0 0\n"; 						// include angluar transmittence
    // file << "window_dist illum window_illum \n\n1 window_glass\n0\n3 "<<transmittance<<"\n\n"; 	// illum makes it a light source
    file << std::endl;
} // writeMaterials()

void writePolygonFloorRadiance(std::ofstream& file, std::vector<bso::utilities::geometry::vertex>& PolyVertex)
{
    gFloorCount++;

    file << "Floor polygon F"<<gFloorCount<<"\n";
    file << "0\n0\n" << PolyVertex.size()*3 << "\n";

    for (const auto& vertex : PolyVertex) {
        file << std::setw(10) << std::right << vertex[0] << " "
             << std::setw(10) << std::right << vertex[1] << " "
             << std::setw(10) << std::right << vertex[2] << "\n";
    }
    file << std::endl;
} // writePolygonFloorRadiance()

void writePolygonCeilingRadiance(std::ofstream& file, std::vector<bso::utilities::geometry::vertex>& PolyVertex)
{
    if (PolyVertex[0][2] != 0)
    {
        gCeilingCount++;
        file << "Ceiling polygon C"<<gCeilingCount<<"\n";
        file << "0\n0\n" << PolyVertex.size()*3 << "\n";
        for (const auto& vertex : PolyVertex) {
            file << std::setw(10) << std::right << vertex[0] << " "
                 << std::setw(10) << std::right << vertex[1] << " "
                 << std::setw(10) << std::right << (vertex[2]-0.1) << "\n"; //-1 to place ceiling below floor for correct reflectance value
        }
        file << std::endl;
    }
} // writePolygonCeilingRadiance()

void writePolygonWallRadiance(std::ofstream& file, std::vector<bso::utilities::geometry::vertex>& PolyVertex)
{
    gWallCount++;

    sortPolygonVertexGeneric(PolyVertex);
    file << "Wall polygon W"<<gWallCount<<"\n";
    file << "0\n0\n" << PolyVertex.size()*3 << "\n";
    for (const auto& vertex : PolyVertex) {
        file << std::setw(10) << std::right << vertex[0] << " "
             << std::setw(10) << std::right << vertex[1] << " "
             << std::setw(10) << std::right << vertex[2] << "\n";
    }
    file << std::endl;
} // writePolygonWallRadiance()

void writeWindowRadiance(std::ofstream& file, std::vector<bso::utilities::geometry::vertex>& quadVertex, rule_set::rectangle_property* rectangleProp)
{
    gWindowCount++;

    sortPolygonVertexGeneric(quadVertex);
    file << "window_glass polygon window"<<gWindowCount<<"\n";
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
} // writeWindowRadiance()

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
} // filterSurfacesForGrid()

std::vector<std::tuple<unsigned int, int>> generateGridNEN(	const std::vector<bso::spatial_design::conformal::cf_surface>& vec, std::ofstream& gridFile, 
															std::vector<unsigned int>& SpaceID, const std::vector<double> DF_Settings, std::vector<bso::utilities::geometry::vertex>& gridPoints) 
{
	const double ReferenceHeight = DF_Settings[1]; // [mm]
	const double space_margin = DF_Settings[2]; // [mm]
	const double maxGridSize = DF_Settings[3]; // [m]

    std::vector<int> amountGridPoints;
    std::vector<std::tuple<unsigned int, int>> tupleVec;

    for (int i = 0; i < vec.size(); i++) //loop per surface
    {
        int numGridPoints = 0;

        // Collect and sort vertices counter-clockwise
        std::vector<bso::utilities::geometry::vertex> quadVertices;
        for (const auto& v : vec[i]) 
        {
            quadVertices.push_back(v);
        }

        if (quadVertices.size() < 4)
        {
            amountGridPoints.push_back(0);
            tupleVec.push_back(std::make_tuple(SpaceID[i], amountGridPoints[i]));
            continue;
        }

        sortPolygonVertexGeneric(quadVertices);

        // Calculate edge lengths directly from sorted vertices to ensure correct pairing
        // After sorting counter-clockwise, edges are: 0->1, 1->2, 2->3, 3->0
        double edge01 = quadVertices[0].getDistance(quadVertices[1]);
        double edge12 = quadVertices[1].getDistance(quadVertices[2]);
        double edge23 = quadVertices[2].getDistance(quadVertices[3]);
        double edge30 = quadVertices[3].getDistance(quadVertices[0]);

        // Characteristic dimensions (shortest edge in each direction)
        // For a quadrilateral: opposite edges are 0-1 & 2-3, and 1-2 & 3-0
        double dimA = std::min(edge01, edge23); // opposite edges 0->1 and 2->3
        double dimB = std::min(edge12, edge30); // opposite edges 1->2 and 3->0
        double opposingA = std::max(edge01, edge23); // opposite edge length used to determine nr. of grid points in that direction
        double opposingB = std::max(edge12, edge30); // opposite edge length used to determine nr. of grid points in that direction

        if (dimA <= 1e-9 || dimB <= 1e-9)
        {
            std::stringstream errorMessage;
            errorMessage << "Error in generateGridNEN: Surface " << i << " (SpaceID: " << SpaceID[i] 
                      << ")\n has dimensions (dimA=" << dimA << ", dimB=" << dimB 
                      << ").\n These are infeasable and should not occur. \n"
                      << "(bso/grammar/rule_set/dl_rule_set/dl_rectangle_rule.cpp). " << std::endl;
            throw std::invalid_argument(errorMessage.str());            
        }

        // Determine d according to NEN B.2
        // If aspect ratio >= 2, use shorter dimension; otherwise use longer dimension
        bool aIsLonger = (dimA > dimB);
        double longerDim          = aIsLonger ? dimA      : dimB;
        double shorterDim         = aIsLonger ? dimB      : dimA;
        double opposingLongerDim  = aIsLonger ? opposingA : opposingB;
        double opposingShorterDim = aIsLonger ? opposingB : opposingA;

        double aspectRatio;
        if (shorterDim > 1e-9) aspectRatio = longerDim / shorterDim;
        else aspectRatio = std::numeric_limits<double>::infinity();

        bool useShorterDimFord = (aspectRatio >= 2.0);
        double d = useShorterDimFord ? shorterDim : longerDim;
        double d_opposing = useShorterDimFord ? opposingShorterDim : opposingLongerDim;

        bool dIsLongerDim = (d == longerDim);
        double d_other = dIsLongerDim ? shorterDim : longerDim; // the other dimension that is not used for d calculation, needed for grid point calculation in that direction
        double d_other_opposing = dIsLongerDim ? opposingShorterDim : opposingLongerDim; // the opposing edge length of the other dimension, needed for grid point calculation in that direction
        
        if (d == longerDim) d_other = shorterDim;
        else d_other = longerDim;

        bool d_is_dimA = (std::abs(d - dimA) < 1e-9);

        // Calculate maximum grid cell size per NEN B.2: p = 0.5 × 5^log10(d)
        double p_meter = 0.5 * std::pow(5, std::log10(d / 1000.0));
        p_meter = std::min(p_meter, maxGridSize); // user cap

        // Convert to mm for consistency with dimensions in mm
        double p = p_meter * 1000.0;

        // Number of points in relevant dimension: ceil(d/p)
        int num_points_relevant = static_cast<int>(std::ceil(d_opposing / p)); // Calculated with the longer opposing edge length to ensure grid cells do not exceed max size in that direction
        if (num_points_relevant < 2) num_points_relevant = 2;

        // Spacing in relevant dimension
        double step = d / (num_points_relevant - 1);

        // Number of points in the other dimension based on same spacing.
        // Add 1 to include both boundary endpoints (same convention as relevant dimension).
        int num_points_other = static_cast<int>(std::ceil(d_other_opposing / step)) + 1; // Calculated with the opposing edge length of the other dimension to maintain consistent spacing, ensuring grid cells do not exceed max size in that direction
        if (num_points_other < 2) num_points_other = 2;

        int num_u = d_is_dimA ? num_points_relevant : num_points_other;
        int num_v = d_is_dimA ? num_points_other : num_points_relevant;

        // Check if space is too narrow for margin to be applied uniformly
        double minDim = std::min(dimA, dimB);
        bool validOffset = true;
        if (minDim < 2.0 * space_margin)
        {
            // Space is too narrow for uniform margin on all sides; margin cannot be applied
            std::cerr << "Warning: Surface " << i << " (SpaceID: " << SpaceID[i] 
                      << ") is too narrow (min dimension=" << minDim 
                      << " mm) for space margin (" << space_margin 
                      << " mm). Using original polygon without margin offset." << std::endl;
            validOffset = false;
        }

        // Build inward-offset quadrilateral (exclude wall band = space_margin)
        std::vector<bso::utilities::geometry::vertex> poly(4);
        for (int vi = 0; vi < 4; ++vi) {poly[vi] = {quadVertices[vi][0], quadVertices[vi][1], 0.0};} // flatten the polygon to z=0

        // Utilize the shoelace formula to determine polygon orientation (clockwise vs counter-clockwise)
        double signedArea2 = 0.0;
        for (int vi = 0; vi < 4; ++vi)
        {
            const auto& a = poly[vi];
            const auto& b = poly[(vi + 1) % 4];
            signedArea2 += (a[0] * b[1] - b[0] * a[1]);
        }
        // orientationSign = 1.0 for counter-clockwise, -1.0 for clockwise, 
        // to be used for inward offset direction determination.
        double orientationSign = (signedArea2 >= 0.0) ? 1.0 : -1.0; 

        std::vector<bso::utilities::geometry::line_segment> offsetLines(4);

        /*
            Image for understanding the offset line construction per edge:
        
            original edge (for this ei):
            a ---------------------------> b
            \            n=(nx,ny)
             \             (unit inward normal)
              \  space_margin
               \
                p ----------------------> p + d
                    shifted edge = offsetLines[ei]
                    where d = (ex, ey)
        */

        for (int ei = 0; ei < 4; ++ei)
        {
            const auto& a = poly[ei]; // defines current vertex
            const auto& b = poly[(ei + 1) % 4]; // defines next vertex with wrap-around for last edge of the polygon
            double ex = b[0] - a[0]; // edge vector x component
            double ey = b[1] - a[1]; // edge vector y component
            double elen = std::sqrt(ex * ex + ey * ey); // length of the edge
            if (elen <= 1e-9) // only checks if division by zero would be performed
            {
                validOffset = false;
                break;
            }

            double nx = orientationSign * (-ey / elen); // inward pointing unit normal vector to the current edge (x component)
            double ny = orientationSign * ( ex / elen); // inward pointing unit normal vector to the current edge (y component)

            bso::utilities::geometry::vertex p = {a[0] + nx * space_margin, a[1] + ny * space_margin, 0.0};
            bso::utilities::geometry::vertex d = {ex, ey, 0.0};
            offsetLines[ei] = bso::utilities::geometry::line_segment(p, p + d);
        }

        std::vector<bso::utilities::geometry::vertex> inset(4);
        if (validOffset)
        {
            for (int vi = 0; vi < 4; ++vi)
            { // define the inset vertices and check if adjacent offset lines intersect properly to form a valid inset quadrilateral
                const auto& prev = offsetLines[(vi + 3) % 4];
                const auto& curr = offsetLines[vi];
                if (!prev.intersectsWith(curr, inset[vi], 1e-9))
                {
                    validOffset = false;
                    break;
                }
            }
        }

        if (!validOffset)
        {
            // Fall back to 25% offset if margin offset is not geometrically feasible
            double minDim = std::min(dimA, dimB);
            double fallbackMargin = 0.25 * minDim;
            
            std::vector<bso::utilities::geometry::line_segment> fallbackOffsetLines(4);
            bool fallbackValid = true;
            
            // Recompute offset lines with reduced margin
            for (int ei = 0; ei < 4 && fallbackValid; ++ei)
            {
                const auto& a = poly[ei];
                const auto& b = poly[(ei + 1) % 4];
                double ex = b[0] - a[0];
                double ey = b[1] - a[1];
                double elen = std::sqrt(ex * ex + ey * ey);
                if (elen <= 1e-9)
                {
                    fallbackValid = false;
                    break;
                }
                
                double nx = orientationSign * (-ey / elen);
                double ny = orientationSign * ( ex / elen);
                
                bso::utilities::geometry::vertex p = {a[0] + nx * fallbackMargin, a[1] + ny * fallbackMargin, 0.0};
                bso::utilities::geometry::vertex d = {ex, ey, 0.0};
                fallbackOffsetLines[ei] = bso::utilities::geometry::line_segment(p, p + d);
            }
            
            // Try to compute intersections for fallback offset
            if (fallbackValid)
            {
                for (int vi = 0; vi < 4 && fallbackValid; ++vi)
                {
                    const auto& prev = fallbackOffsetLines[(vi + 3) % 4];
                    const auto& curr = fallbackOffsetLines[vi];
                    if (!prev.intersectsWith(curr, inset[vi], 1e-9))
                    {
                        fallbackValid = false;
                        break;
                    }
                }
            }
            
            // If fallback offset fails, use original polygon
            if (!fallbackValid)
            {
                std::stringstream errorMessage;
                errorMessage << "Error in generateGridNEN: Surface " << i << " (SpaceID: " << SpaceID[i] 
                        << ")\n has dimensions (dimA=" << dimA << ", dimB=" << dimB 
                        << ").\n The fallbackOffset where invalid and should not occur be posible to occur. \n"
                        << "(bso/grammar/rule_set/dl_rule_set/dl_rectangle_rule.cpp). " << std::endl;
                throw std::invalid_argument(errorMessage.str());  
            }
        }

        // Use average z of the surface vertices (for flat z=0 this stays 0)
        double baseZ = 0.0;
        for (const auto& v : quadVertices)
        {
            baseZ += v[2];
        }
        baseZ /= static_cast<double>(quadVertices.size());

        // Track worst cell aspect ratio for NEN B.2 compliance check
        double worstAspectRatio = 1.0;
        int worstCellJu = 0, worstCellKv = 0;
        double worstCellWidth = 0.0, worstCellHeight = 0.0;

        // Store generated points in 2D structure for cell aspect ratio checking
        std::vector<std::array<double, 2>> gridPointsXY(num_u * num_v);

        // Generate grid points over inset quad using bilinear interpolation
        for (int kv = 0; kv < num_v; ++kv)
        {
            double v = (num_v == 1) ? 0.0 : static_cast<double>(kv) / static_cast<double>(num_v - 1);
            for (int ju = 0; ju < num_u; ++ju)
            {
                double u = (num_u == 1) ? 0.0 : static_cast<double>(ju) / static_cast<double>(num_u - 1);

                double x = (1.0 - u) * (1.0 - v) * inset[0][0]
                         + (u)       * (1.0 - v) * inset[1][0]
                         + (u)       * (v)       * inset[2][0]
                         + (1.0 - u) * (v)       * inset[3][0];

                double y = (1.0 - u) * (1.0 - v) * inset[0][1]
                         + (u)       * (1.0 - v) * inset[1][1]
                         + (u)       * (v)       * inset[2][1]
                         + (1.0 - u) * (v)       * inset[3][1];

                double z = baseZ + ReferenceHeight;

                gridPoints.push_back({x, y, z});
                gridFile << std::setprecision(12)
                         << x << ' '
                         << y << ' '
                         << z << ' '
                         << 0 << ' '
                         << 0 << ' '
                         << 1 << std::endl;
                numGridPoints++;
                
                // Store XY for cell checking
                int idx = kv * num_u + ju;
                gridPointsXY[idx] = {x, y};
                
                // Check cell aspect ratio (if not on last row or column)
                if (ju < num_u - 1 && kv < num_v - 1)
                {
                    int idx_10 = kv * num_u + (ju + 1);
                    int idx_11 = (kv + 1) * num_u + (ju + 1);
                    int idx_01 = (kv + 1) * num_u + ju;
                    
                    // Get the 4 corners of this cell
                    auto& p00 = gridPointsXY[idx];
                    auto& p10 = gridPointsXY[idx_10];
                    auto& p11 = gridPointsXY[idx_11];
                    auto& p01 = gridPointsXY[idx_01];
                    
                    // Calculate cell dimensions (average of opposite sides)
                    double width1 = std::sqrt(std::pow(p10[0] - p00[0], 2) + std::pow(p10[1] - p00[1], 2));
                    double width2 = std::sqrt(std::pow(p11[0] - p01[0], 2) + std::pow(p11[1] - p01[1], 2));
                    double height1 = std::sqrt(std::pow(p01[0] - p00[0], 2) + std::pow(p01[1] - p00[1], 2));
                    double height2 = std::sqrt(std::pow(p11[0] - p10[0], 2) + std::pow(p11[1] - p10[1], 2));
                    
                    double avgWidth = (width1 + width2) / 2.0;
                    double avgHeight = (height1 + height2) / 2.0;
                    
                    if (avgWidth > 1e-9 && avgHeight > 1e-9)
                    {
                        double aspectRatio = avgHeight / avgWidth;
                        
                        // Track worst aspect ratio (furthest from ideal 1.0)
                        double deviationFromIdeal = std::abs(std::log(aspectRatio));
                        double worstDeviation = std::abs(std::log(worstAspectRatio));
                        
                        if (deviationFromIdeal > worstDeviation)
                        {
                            worstAspectRatio = aspectRatio;
                            worstCellJu = ju;
                            worstCellKv = kv;
                            worstCellWidth = avgWidth;
                            worstCellHeight = avgHeight;
                        }
                    }
                }
            }
        }
        
        // Report NEN B.2 compliance warning if worst cell is out of range
        if (worstAspectRatio < 0.5 || worstAspectRatio > 2.0)
        {
            std::cerr << "Warning: Surface " << i << " (SpaceID: " << SpaceID[i] 
                      << ") has worst-case grid cell aspect ratio = " << worstAspectRatio 
                      << ", which is outside NEN B.2 recommended range [0.5, 2.0]. "
                      << "(Grid: " << num_u << " x " << num_v 
                      << " points, worst cell at [" << worstCellJu << "," << worstCellKv << "]"
                      << ", size: " << (worstCellWidth/1000.0) << " x " << (worstCellHeight/1000.0) << " m)" 
                      << std::endl;
        }
        
        gridFile << "\n" << std::endl;
        amountGridPoints.push_back(numGridPoints);
        tupleVec.push_back(std::make_tuple(SpaceID[i], amountGridPoints[i]));
    }

    return tupleVec;
} // generateGridNEN()

void dl_rectangle_rule::apply(bso::daylight::dl_model& dl) const
{
	
} // apply()

} // namespace dl_rule_set
} // namespace rule_set
} // namespace grammar
} // namespace bso

#endif // GRAMMAR_DL_RECTANGLE_RULE_CPP