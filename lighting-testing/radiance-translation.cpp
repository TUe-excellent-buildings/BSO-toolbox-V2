#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <limits>
#include <cmath>

struct Point {
    float x, y, z;
};

void writeTriangle(std::ofstream& file, const Point& p1, const Point& p2, const Point& p3) {
    file << "polygon poly\n";
    file << "0\n0\n9\n";
    file << p1.x << " " << p1.y << " " << p1.z << " ";
    file << p2.x << " " << p2.y << " " << p2.z << " ";
    file << p3.x << " " << p3.y << " " << p3.z << "\n";
}

using Prism = std::vector<Point>;

std::vector<Prism> parseInput(std::ifstream& file) {
    std::vector<Prism> prisms;
    std::string line;

    while (std::getline(file, line)) {
        Prism prism;
        std::string::size_type start = 0;
        std::string::size_type end = 0;

        while ((start = line.find('{', end)) != std::string::npos) {
            end = line.find('}', start);
            if (end == std::string::npos) {
                std::cerr << "Malformed prism definition.\n";
                break;
            }

            std::string prismData = line.substr(start + 1, end - start - 1);
            std::istringstream iss(prismData);
            Point point;

            // Parse each point within the prism
            while (iss >> point.x) {
                if (iss.peek() == ',') iss.ignore();
                if (!(iss >> point.y)) break;
                if (iss.peek() == ',') iss.ignore();
                if (!(iss >> point.z)) break;

                prism.push_back(point);

                iss.ignore(std::numeric_limits<std::streamsize>::max(), ',');
            }

            if (!prism.empty()) {
                prisms.push_back(prism);
            }
            end++;
        }
    }

    return prisms;
}




float formatNumber(float num) {
    const float threshold = 1e-6; // Numbers below this threshold will be treated as zero
    return (std::fabs(num) < threshold) ? 0.0f : num;
}


void writeQuadrilateral(std::ofstream& file, const Point& p1, const Point& p2, const Point& p3, const Point& p4) {
    file << "generic_material polygon poly\n";
    file << "0\n0\n";
    file << "12 "; // 12 coordinates: 4 points * 3 coordinates each
    file << formatNumber(p1.x) << " " << formatNumber(p1.y) << " " << formatNumber(p1.z) << " ";
    file << formatNumber(p2.x) << " " << formatNumber(p2.y) << " " << formatNumber(p2.z) << " ";
    file << formatNumber(p3.x) << " " << formatNumber(p3.y) << " " << formatNumber(p3.z) << " ";
    file << formatNumber(p4.x) << " " << formatNumber(p4.y) << " " << formatNumber(p4.z) << "\n";
}

void convertToRadiance(const Prism& prism, std::ofstream& file) {
    // Top face
    writeQuadrilateral(file, prism[0], prism[1], prism[2], prism[3]);

    // Bottom face
    writeQuadrilateral(file, prism[4], prism[5], prism[6], prism[7]);

    // Side faces
    writeQuadrilateral(file, prism[0], prism[1], prism[5], prism[4]);
    writeQuadrilateral(file, prism[1], prism[2], prism[6], prism[5]);
    writeQuadrilateral(file, prism[2], prism[3], prism[7], prism[6]);
    writeQuadrilateral(file, prism[3], prism[0], prism[4], prism[7]);
}

void findBounds(const std::vector<Prism>& prisms, Point& min, Point& max) {
    min.x = min.y = min.z = std::numeric_limits<float>::max();
    max.x = max.y = max.z = -std::numeric_limits<float>::max();

    for (const auto& prism : prisms) {
        for (const auto& point : prism) {
            min.x = std::min(min.x, point.x);
            min.y = std::min(min.y, point.y);
            min.z = std::min(min.z, point.z);
            max.x = std::max(max.x, point.x);
            max.y = std::max(max.y, point.y);
            max.z = std::max(max.z, point.z);
        }
    }
}

bool isOnPositiveSideOfPlane(const Point& p, const Point& a, const Point& b, const Point& c) {
    Point normal;
    Point v1 = {b.x - a.x, b.y - a.y, b.z - a.z};
    Point v2 = {c.x - a.x, c.y - a.y, c.z - a.z};
    normal.x = v1.y * v2.z - v1.z * v2.y;
    normal.y = v1.z * v2.x - v1.x * v2.z;
    normal.z = v1.x * v2.y - v1.y * v2.x;

    float D = -(normal.x * a.x + normal.y * a.y + normal.z * a.z);
    float side = normal.x * p.x + normal.y * p.y + normal.z * p.z + D;

    return side >= 0;
}

bool isPointInsidePrism(const Point& point, const Prism& prism) {
    for (size_t i = 0; i < prism.size(); i += 4) {
        const Point& a = prism[i];
        const Point& b = prism[(i + 1) % prism.size()];
        const Point& c = prism[(i + 2) % prism.size()];

        if (!isOnPositiveSideOfPlane(point, a, b, c)) {
            return false;
        }
    }

    return true;
}

bool filterPointsInsideDesign(Point point, const std::vector<Prism>& prisms) {
    for (const auto& prism : prisms) {
        if (isPointInsidePrism(point, prism)) {
            return true;
            break; // No need to check other prisms if point is inside one already
        }
    }

    return false;
}

void generateGrid(const Point& min, const Point& max, float resolution, std::ofstream& file, const std::vector<Prism>& prisms) {
    int iterations = std::fabs(max.x - min.x) / resolution;
    int i = 0;
    for (float x = min.x; x <= max.x; x += resolution) {
        std::cout << "Grid iteration: " << i << "/" << iterations << std::endl;
        for (float y = min.y; y <= max.y; y += resolution) {
            for (float z = min.z; z <= max.z; z += resolution) {
                Point p = {x, y, z};
                if(filterPointsInsideDesign(p, prisms)) file << x << " " << y << " " << z << "\n";
            }
        }
        i++;
    }
}

void generateBuilding(const std::vector<Prism>& prisms, std::ofstream& file) {
    file << "void plastic generic_material 0 0 5 0.5 0.5 0.5 0.1 0.05\n";

    for (const auto& prism : prisms) {
        convertToRadiance(prism, file);
    }
}

void generateRadianceScene(const Point& minBounds, const Point& maxBounds, const std::string& buildingFilePath) {
    // Determine the extension of the ground plane beyond the building footprint
    float extension = 100.0f; // Extend 100 units in all directions beyond the bounding box

    // Calculate ground plane corners
    Point groundMin = {minBounds.x - extension, minBounds.y - extension, minBounds.z};
    Point groundMax = {maxBounds.x + extension, maxBounds.y + extension, minBounds.z}; // Keep z at the minimum to place the ground

    // Open a file to write the scene description
    std::ofstream sceneFile("scene.rad");

    // Write the ground plane to the scene file
    sceneFile << "void plastic ground_material 0 0 5 0.5 0.5 0.5 0.1 0.05\n";
    sceneFile << "ground_material polygon ground_plane\n";
    sceneFile << "0 0 12\n";
    sceneFile << groundMin.x << " " << groundMin.y << " " << groundMin.z << "\n";
    sceneFile << groundMax.x << " " << groundMin.y << " " << groundMin.z << "\n";
    sceneFile << groundMax.x << " " << groundMax.y << " " << groundMin.z << "\n";
    sceneFile << groundMin.x << " " << groundMax.y << " " << groundMin.z << "\n";

    // Include the building design
    sceneFile << "!xform " << buildingFilePath << "\n";

    // Close the file
    sceneFile.close();
}

void runRadianceSetup(const std::string& gridFilename, const std::string& outputFilename, Point minBounds, Point maxBounds) {
    // Generating scene
    generateRadianceScene(minBounds, maxBounds, outputFilename);

    // Generate sky
    std::string command = "gensky 6 21 10:00 +s -a 37.7749 -o 122.4194 -m -120 > sky.rad";
    std::cout << "Running command: " << command << "\n";
    system(command.c_str());

    // Compile the scene
    command = "oconv scene.rad sky.rad > scene.oct";
    std::cout << "Running command: " << command << "\n";
    system(command.c_str());


    command = "rtrace -I -h -w -ab 2 -ad 512 -as 256 -ar 128 -aa 0.1 scene.oct < " + gridFilename + " > results.txt";
    std::cout << "Running command: " << command << "\n";
    system(command.c_str());

    command = "rpict -vp 0 0 10 -vd 0 1 0 -vu 0 0 1 -vh 60 -vv 60 -ab 2 scene.oct > scene.hdr";
    std::cout << "Running command: " << command << "\n";
    system(command.c_str());

    command = "pfilt -h scene.hdr > scene.hdr";
    std::cout << "Running command: " << command << "\n";
    system(command.c_str());

    command = "ra_tiff scene.hdr scene.tif";
    std::cout << "Running command: " << command << "\n";
    system(command.c_str());
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <input_filename> <grid_filename> <output_filename>\n";
        return 1;
    }

    std::ifstream inputFile(argv[1]);
    if (!inputFile.is_open()) {
        std::cerr << "Failed to open " << argv[1] << " for reading\n";
        return 1;
    }

    std::cout << "Parsing input file...\n";
    std::vector<Prism> prisms = parseInput(inputFile);
    inputFile.close();
    std::cout << "Parsed " << prisms.size() << " prisms.\n";

    if (prisms.empty()) {
        std::cerr << "No prisms parsed from the file.\n";
        return 1;
    }

    std::cout << "Boundingf box...\n";
    Point minBounds, maxBounds;
    findBounds(prisms, minBounds, maxBounds);
    std::cout << "Bounding box: (" << minBounds.x << ", " << minBounds.y << ", " << minBounds.z << ") to ("
              << maxBounds.x << ", " << maxBounds.y << ", " << maxBounds.z << ")\n";

    std::cout << "Opening Grid file...\n";
    std::ofstream gridFile(argv[2]);
    if (!gridFile.is_open()) {
        std::cerr << "Failed to open " << argv[2] << " for writing\n";
        return 1;
    }

    std::cout << "Writing Grid file...\n";
    float resolution = 50.0f; // Change this value as needed
    generateGrid(minBounds, maxBounds, resolution, gridFile, prisms);
    gridFile.close();
    std::cout<<"Grid file written\n";

    std::cout << "Opening output file...\n";
    std::ofstream outFile(argv[3]);
    if (!outFile.is_open()) {
        std::cerr << "Failed to open " << argv[2] << " for writing\n";
        return 1;
    }
    generateBuilding(prisms, outFile);
    outFile.close();
    std::cout << "Output file written\n";

    runRadianceSetup(argv[2], argv[3], minBounds, maxBounds);
    

    return 0;
}
