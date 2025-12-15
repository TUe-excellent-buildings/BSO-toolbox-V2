#include <GL/freeglut.h>
#include <cmath>
#include <vector>
#include <iostream>
// #include <Windows.h>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <variant>
#include <tuple>
#include <mutex>

#include <bso/spatial_design/ms_N_building.hpp>
#include <bso/spatial_design/cf_building.hpp>
#include <bso/structural_design/sd_model.hpp>
#include <bso/building_physics/bp_model.hpp>
#include <bso/grammar/grammar.hpp>
#include <bso/visualization/visualization.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "freeglut/include_GL/stb_image.h"

using namespace std;

bool visualizationActive = true;
bso::visualization::viewportmanager vpmanager_local;
bso::visualization::orbitalcamera   cam_local;
int prevx, prevy;

std::vector<int> tableClicked;
bool tableInitialized = false;

typedef void (*ButtonCallback)(int);

bso::spatial_design::ms_N_building MS("Villa");
bso::spatial_design::cf_building CF(MS, 1e-3);
bso::grammar::grammar grm(CF);
std::vector<bso::spatial_design::ms_N_building> msModels;
std::vector<bso::structural_design::sd_model> sdModels;

bso::structural_design::component::structure trussStructure("truss",{{"A",2250},{"E",3e4}});
bso::structural_design::component::structure beamStructure("beam",{{"width",150},{"height",150},{"poisson",0.3},{"E",3e4}});
bso::structural_design::component::structure flatShellStructure("flat_shell",{{"thickness",150},{"poisson",0.3},{"E",3e4}});
bso::structural_design::component::structure substituteStructure("flat_shell",{{"thickness",150},{"poisson",0.3},{"E",3e-2}});

double etaBend = 0.4;
double etaAx = 0.4;
double etaShear = 0.4;
double etaNoise = 0.025;
int etaConverge = 1;
std::string checkingOrder = "321";

int nRectangles = 50;

//bso::structural_design::sd_model SD_model;
//bso::structural_design::sd_model SD_model_2;

auto cfVertices = CF.cfVertices();
auto cfLines = CF.cfLines();
auto cfRectangles = CF.cfRectangles();

/*
for rec in cfRectangles{
    rec->getVertices().getcords();
}
*/

// CREATE SD MODEL FOR INTIIAL VISUALIZATION
std::vector<bso::structural_design::component::structure> structureAssignments(nRectangles);
std::vector<bso::structural_design::component::structure> structureAssignments_it2(nRectangles);

//bso::structural_design::sd_model SD_model_initial = grm.sd_grammar<bso::grammar::DEFAULT_SD_GRAMMAR >(std::string("settings/sd_settings.txt"));

//bso::structural_design::sd_model SD_model_sub = grm.sd_grammar<bso::grammar::DEFAULT_SD_GRAMMAR >(std::string("settings/sd_settings.txt"));
//auto subRectangle = SD_model_sub.getSubRectangles();

//std::vector<bso::structural_design::component::structure> structureAssignments(subRectangle.size());

//size_t splitIndex = subRectangle.size() / 2;  // This finds the midpoint for splitting

struct Button {
    float x, y, width, height;
    ButtonCallback callback;
    const char* text;
    int variable;
};

struct TextField {
    float x, y, width, height;
    std::string text;
    int cursorPosition;
    bool isActive;

    // Constructor
    TextField() : cursorPosition(0), isActive(false), x(0), y(0), width(0), height(0) {}

    // Add a character where the cursor is
    void addChar(char c) {
        if (cursorPosition < text.length()) {
            text.insert(text.begin() + cursorPosition, c);
        } else {
            text.push_back(c);
        }
        cursorPosition++;
    }

    // Remove a character at the cursor
    void removeChar() {
        if (!text.empty() && cursorPosition > 0) {
            text.erase(text.begin() + cursorPosition - 1);
            cursorPosition--;
        }
    }

    // Handle the Enter key
    void submit() {
        std::cout << text << std::endl;
        // Clear text if needed
        text.clear();
        cursorPosition = 0;
    }
};

// Define the variant to handle different types of values
using LogValue = std::variant<std::string, double, int>;

struct LogEntry {
    std::string time;
    int iteration;
    std::string description;
    LogValue value;  // Now using LogValue

    // Constructor for different types of LogValue
    LogEntry(int iter, const std::string& desc, const LogValue& val)
        : iteration(iter), description(desc), value(val) {
        // Format the current time upon entry creation
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        std::ostringstream oss;
        oss << std::put_time(std::localtime(&now_c), "%H:%M:%S");
        time = oss.str();
    }
};

std::vector<LogEntry> logEntries;
std::mutex logMutex;  // For thread-safe logging

void initializeModels() {
    bso::spatial_design::ms_N_building MS("Villa");  // Create an MS model
    msModels.push_back(MS);  // Add the MS model to the vector
}

void logAction(int iteration, const std::string& description, const LogValue& value) {
    std::lock_guard<std::mutex> lock(logMutex); // Ensure thread safety
    logEntries.emplace_back(iteration, description, value);
}

// Helper to process variant values
struct VariantVisitor {
    std::ostream& os;

    void operator()(const std::string& val) const {
        os << "\"" << val << "\"";  // Output strings with quotes
    }

    void operator()(double val) const {
        os << val;  // Output doubles directly
    }
};

void writeToProcessFile(const std::string& fileName) {
    std::lock_guard<std::mutex> lock(logMutex); // Ensure thread safety
    std::ofstream processFile(fileName, std::ios::app);
    if (!processFile.is_open()) {
        std::cerr << "Failed to open log file" << std::endl;
        return; // Optionally handle the error more robustly here
    }

    static bool headerPrinted = false;
    if (!headerPrinted) {
        processFile << "Time;Iteration;Description;Value\n";
        headerPrinted = true;
    }

    for (const auto& entry : logEntries) {
        processFile << entry.time << ";" << entry.iteration << ";"
                    << entry.description << ";";
        std::visit(VariantVisitor{processFile}, entry.value);
        processFile << "\n";
    }
    processFile.close();
    logEntries.clear();
}


std::vector<Button> buttons;

TextField removeSpace;
TextField splitSpace;
TextField explanation;
TextField survey1;
TextField survey2;
TextField survey3;
TextField survey4;
TextField survey5;
TextField survey6;
TextField email;

// Global variables for current screen and screen dimensions
int currentScreen = 0;
int marginText = 10;
int startText;
int textWidth;
int screenHeight;
int screenWidth;

// Text margin as a percentage of the window width
const float MARGIN_PERCENT = 1.0f; // Margin as a percentage of the window width

// Counter for iterations
int iteration_counter = 1;
const int max_iterations = 2;

// Vector of rectangles
std::vector<bso::utilities::geometry::polygon*> rectanglesGeometry;
std::vector<int> rectanglesSpaces;
std::vector<int> rectanglesInternalIDs;

// Function prototypes
void display();
void keyboard(unsigned char key, int x, int y);
void reshape(int width, int height);
void updateTextureCoordinates(int width, int height);
void introScreen();
void buildingSpatialScreen();
void structuralModelScreen();
void structuralModel1Screen();
void structuralModel2Screen();
void removeSpaceScreen();
void splitSpaceScreen();
void iteration1CompleteScreen();
void iteration2CompleteScreen();
void surveyScreen1();
void surveyScreen2();
void surveyScreen3();
void surveyScreen4();
void surveyScreen5();
void surveyScreen6();
void outroScreen();
void structureAssignment();
void createSDmodel();
void displayConfirmationRequest(int spaceID, const std::string& message, void (*action)(int));
void drawText(const char* text, float startX, float centerY, float textWidth, float r, float g, float b, bool bold = false);
void drawButton(const char *text, float x, float y, float width, float height, ButtonCallback callback, int variable);
void drawArrow(float x, float y, bool leftArrow);
void drawUndoRedoButtons();
void drawTextField(int x, int y, int width, int height, TextField& textfield);
void onMouseClick(int button, int state, int x, int y);
void drawBuilding();
void drawFourColumnTable(int x, int y, int width, int cellHeight, const std::vector<std::string>& column1);
// void setWindowIcon(const char* iconPath);
void setup2D();
void setup3D();
void introScreen();
bool checkIfRemovePossible();
bool checkIfSplitPossible();
void splitSpaceScreen();
std::string clean_str(const std::string& input);

void visualise(const bso::spatial_design::ms_N_building& ms, const std::string& type = "spaces", const std::string& title = "", const double& lineWidth = 1.0)
{
    vpmanager_local.changeviewport(new bso::visualization::viewport(new bso::visualization::MS_Model(ms, type, title,lineWidth)));
}

void visualise(const bso::spatial_design::cf_building& cf, std::string type, std::string title = "sc_building")
{
	vpmanager_local.changeviewport(new bso::visualization::viewport(new bso::visualization::Conformal_Model(cf, type, title)));
}

void visualise_sd(const bso::structural_design::sd_model& sd, const std::string& type ="component",
							 const std::string& title ="", const bool& ghostly = true,
							 const std::vector<std::pair<bso::utilities::geometry::vertex,
							 bso::utilities::geometry::vector>>& cuttingPlanes = {})
{
	vpmanager_local.changeviewport(new bso::visualization::viewport(new bso::visualization::SD_Model(sd, type, title, ghostly, cuttingPlanes)));
}

void visualise_sd_2(const bso::structural_design::sd_model& sd, const std::string& type ="component",
							 const std::string& title ="", const bool& ghostly = true,
							 const std::vector<std::pair<bso::utilities::geometry::vertex,
							 bso::utilities::geometry::vector>>& cuttingPlanes = {})
{
	vpmanager_local.changeviewport(new bso::visualization::viewport(new bso::visualization::SD_Model(sd, type, title, ghostly, cuttingPlanes)));
}

void visualise_sd_add(const bso::structural_design::sd_model& sd, const std::string& type ="component",
							 const std::string& title ="", const bool& ghostly = false,
							 const std::vector<std::pair<bso::utilities::geometry::vertex,
							 bso::utilities::geometry::vector>>& cuttingPlanes = {})
{
	vpmanager_local.addviewport(new bso::visualization::viewport(new bso::visualization::SD_Model(sd, type, title, ghostly, cuttingPlanes)));
}

void visualise_ms_0(const bso::spatial_design::ms_N_building& ms, const std::string& type = "spaces", const std::string& title = "", const double& lineWidth = 1.0)
{
    vpmanager_local.changeviewport(new bso::visualization::viewport(new bso::visualization::MS_Model(ms, type, title,lineWidth)));
}

void visualise_ms_1(const bso::spatial_design::ms_N_building& ms, const std::string& type = "spaces", const std::string& title = "", const double& lineWidth = 1.0)
{
    vpmanager_local.addviewport(new bso::visualization::viewport(new bso::visualization::MS_Model(ms, type, title,lineWidth)));
}

void visualise_ms_2(const bso::spatial_design::ms_N_building& ms, const std::string& type = "spaces", const std::string& title = "", const double& lineWidth = 1.0)
{
    vpmanager_local.addviewport(new bso::visualization::viewport(new bso::visualization::MS_Model(ms, type, title,lineWidth)));
}

void checkGLError(const char* action) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cout << "OpenGL error after " << action << ": " << gluErrorString(err) << std::endl;
    }
}

//Declare a global variable to store the selected button label
std::string selectedButtonLabel = "";

// Function to get the selected button label
std::string getSelectedButtonLabel() {
    return selectedButtonLabel;
}


void buttonClicked(int variable) {
    std::cout << "Button clicked: " << variable << std::endl;

    // Set the selected button label based on the variable
    switch (variable) {
    case 1:
        selectedButtonLabel = "1";
        break;
    case 2:
        selectedButtonLabel = "2";
        break;
    case 3:
        selectedButtonLabel = "3";
        break;
    case 4:
        selectedButtonLabel = "4";
        break;
    case 5:
        selectedButtonLabel = "5";
        break;
    }
}

std::string clean_str(const std::string& input) {
    std::string result;
    for (char ch : input) {
        if (isdigit(ch)) {
            result += ch;
        }
    }
    return result;
}

std::set<int> removedSpaceIDs;
std::set<int> splitSpaceIDs;

unsigned int initialSpaceCount = MS.getSpacePtrs().size();
double floorArea = MS.getFloorArea();

bool scalingCompleted = false;
bool splittingConfirmed = false;
bool deletionConfirmed = false;
bool inputFieldDisabled = false;
bool spaceInputError = false;
bool confirmSDmodel = false;
std::string spaceInputErrorMessage;

bool awaitingConfirmation = false;
int pendingOperationSpaceID = -1;
std::string confirmationMessage;
void (*confirmationAction)(int);

GLuint loadImageAsTexture(const char* filename) {
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << filename << std::endl;
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum format = GL_RGB; // Default to GL_RGB
    if (nrChannels == 1)
        format = GL_RED;
    else if (nrChannels == 3)
        format = GL_RGB; // Explicitly set, even though it's the default
    else if (nrChannels == 4)
        format = GL_RGBA;

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    return textureID;
}

void displayTexture(GLuint texture, float x, float y, float width, float height) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(x, y);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(x + width, y);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(x + width, y + height);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(x, y + height);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

GLuint imgVilla;
GLuint imgElements;

void initializeTextures() {
    imgVilla = loadImageAsTexture("Villa.png");
    imgElements = loadImageAsTexture("Elements.png");
    // Load more textures as needed
}

std::string getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::map<int, std::chrono::steady_clock::time_point> screenStartTimes;
int lastScreen = -1;

void createSDmodel() {
    if (currentScreen == 3) {
        bso::structural_design::sd_model SD_model = grm.sd_grammar<bso::grammar::DEFAULT_SD_GRAMMAR >(std::string("settings/sd_settings.txt"));
        visualise_sd(SD_model);
    }
    else {
        bso::structural_design::sd_model SD_model_2 = grm.sd_grammar<bso::grammar::DEFAULT_SD_GRAMMAR >(std::string("settings/sd_settings.txt"));
        visualise_sd(SD_model_2);
    }
}

void analyzeSDmodel() {
    bso::structural_design::sd_model SD_model = grm.sd_grammar<bso::grammar::DEFAULT_SD_GRAMMAR >(std::string("settings/sd_settings.txt"));
    SD_model.analyze();
	double strainEnergy = SD_model.getTotalResults().mTotalStrainEnergy;
	double structuralVolume =SD_model.getTotalResults().mTotalStructuralVolume;

	std::cout << "Strain Energy" << strainEnergy << std::endl;
	std::cout << "Structural Volume" << structuralVolume << std::endl;

    logAction(iteration_counter, "Total strain energy", strainEnergy);
    logAction(iteration_counter, "Total structural volume", structuralVolume);

    // Get performances per space
    std::vector<std::pair<double, bso::spatial_design::ms_space*>> spacePerformances;
    for (const auto& space : MS.getSpacePtrs()) {
        auto spaceGeom = space->getGeometry();
        double spacePerformance = SD_model.getPartialResults(&spaceGeom).mTotalStrainEnergy;
        //spacePerformance /= space->getFloorArea();
        spacePerformances.push_back({spacePerformance, space});
    }

    //drawText(iterationText.c_str()

    // Sort spaces from worst to best (low strain energy to high strain energy)

    std::sort(spacePerformances.rbegin(), spacePerformances.rend());

    std::cout << "Ranked Performances:\n";
    int rank = 1;
    for (const auto& performance : spacePerformances) {
        auto id = performance.second->getID();
        std::cout << rank << ", Space ID: " << id << ", Performance: " << performance.first << std::endl;
        std::ostringstream oss;
        oss << rank << ", Space ID: " << id ;

        std::string ranking = oss.str(); // Converts the ostringstream to a string
        logAction(iteration_counter, ranking, performance.first);
        rank++;
    }
}

void changeScreen(int screen) {
    if (lastScreen != -1) {
        auto endTime = std::chrono::steady_clock::now();
        double duration = std::chrono::duration_cast<std::chrono::seconds>(endTime - screenStartTimes[lastScreen]).count();
        std::cout << "Time spent on screen " << lastScreen << ": " << duration << " seconds." << std::endl;

        // Log the time spent on the last screen
        logAction(iteration_counter, "Time Spent", duration);  // Assuming duration is acceptable as LogValue
    }

    // Record the start time for the new screen
    screenStartTimes[screen] = std::chrono::steady_clock::now();
    lastScreen = screen;  // Update lastScreen to the current one
    selectedButtonLabel = "";
    // Log entering the new screen
    logAction(iteration_counter, "Entered Screen", screen);

    inputFieldDisabled = false;
    currentScreen = screen;
    std::cout << "Screen changed to: Screen " << screen << std::endl;

    if(screen  <= 2) {
        vpmanager_local.clearviewports();
        visualise(MS);
        visualizationActive = true;
    } else if(screen == 3) {
        vpmanager_local.clearviewports();
        //bso::structural_design::sd_model SD_model = grm.sd_grammar<bso::grammar::DEFAULT_SD_GRAMMAR >(std::string("settings/sd_settings.txt"));
        std::vector<bso::structural_design::component::structure> structureAssignments(nRectangles);
        bso::structural_design::sd_model SD_model = grm.sd_grammar<bso::grammar::DEFAULT_SD_GRAMMAR >(std::string("settings/sd_settings.txt"));
        visualise_sd(SD_model);
    } else if(screen == 4) {
        vpmanager_local.clearviewports();
        //bso::structural_design::sd_model SD_model = grm.sd_grammar<bso::grammar::DEFAULT_SD_GRAMMAR >(std::string("settings/sd_settings.txt"));
        std::vector<bso::structural_design::component::structure> structureAssignments_it2(nRectangles);
        bso::structural_design::sd_model SD_model_2 = grm.sd_grammar<bso::grammar::DEFAULT_SD_GRAMMAR >(std::string("settings/sd_settings.txt"));
        visualise_sd_2(SD_model_2);
    } else if(screen > 4 && screen <= 6) {
        vpmanager_local.clearviewports();
        visualise(MS);
        if (iteration_counter == 1) {
            bso::structural_design::sd_model SD_model = grm.sd_grammar<bso::grammar::DEFAULT_SD_GRAMMAR >(std::string("settings/sd_settings.txt"));
            visualise_sd_add(SD_model);
            sdModels.push_back(SD_model);
        } else {
            bso::structural_design::sd_model SD_model_2 = grm.sd_grammar<bso::grammar::DEFAULT_SD_GRAMMAR >(std::string("settings/sd_settings.txt"));
            visualise_sd_add(SD_model_2);
        }
        visualizationActive = true;
    } else if(screen == 7) {
        vpmanager_local.clearviewports();
        visualise_ms_0(msModels[0]);
        visualise_sd_add(sdModels[0]);
        visualise_ms_1(msModels[1]);
        visualise_ms_2(msModels[2]);
        visualizationActive = true;
    } else if(screen == 8) {
        vpmanager_local.clearviewports();
        visualise_ms_0(msModels[0]);
        visualise_sd_add(sdModels[0]);
        //visualise_ms_1(msModels[1]);
        visualise_ms_2(msModels[2]);
        // iteration 2
        visualise_ms_1(msModels[2]);
        visualise_sd_add(sdModels[1]);
        //visualise_ms_1(msModels[3]);
        visualise_ms_2(msModels[4]);
        visualizationActive = true;
    } else {
        vpmanager_local.clearviewports();
        visualizationActive = false;
    }
    glutPostRedisplay();
    buttons.clear();
}

void reshape(int width, int height) {
    screenHeight = height;
    screenWidth = width;

    marginText = 20;
    startText = screenWidth / 1.28f + marginText;
    textWidth = screenWidth - startText - marginText;

    glViewport(0, 0, width, height);
    glutPostRedisplay();
    // For 2D rendering
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, screenWidth, 0.0, screenHeight);

    //For 3D rendering (commented out; use if needed)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    gluPerspective(45.0, aspectRatio, 0.1, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


void displayConfirmationRequest(int spaceID, const std::string& message, void (*action)(int)) {
    pendingOperationSpaceID = spaceID;
    confirmationMessage = message;
    confirmationAction = action;
    awaitingConfirmation = true;

    spaceInputError = true;
    spaceInputErrorMessage = confirmationMessage;
    glutPostRedisplay();
}

bool validateSpaceRemoveID(const std::string& input, int& outNumber, std::string& errorMessage) {
    try {
        outNumber = std::stoi(input);
    } catch (...) {
        errorMessage = "Input is not a valid number.";
        return false;
    }

    if (outNumber <= 0 || outNumber > MS.getLastSpaceID()) {
        errorMessage = "ID is out of range.";
        return false;
    }

    if (removedSpaceIDs.find(outNumber) != removedSpaceIDs.end()) {
        errorMessage = "Space has been removed and is not eligible.";
        return false;
    }

    if (splitSpaceIDs.find(outNumber) != splitSpaceIDs.end()) {
        errorMessage = "Space has been split and is not eligible.";
        return false;
    }

    return true;
}


void removeSpaceConfirmed(int spaceID) {
    MS.deleteSpace(MS.getSpacePtr(MS.getSpaceWithID(spaceID)));
    std::cout << "Space removed: " << spaceID << std::endl;
    removedSpaceIDs.insert(spaceID); // Track the removed space ID
    std::cout << "Adding to removed: " << spaceID << std::endl;
    glutPostRedisplay();

    logAction(iteration_counter, "Space Removed", spaceID);

    // Reset state as necessary
    awaitingConfirmation = false;
    spaceInputError = false;
    spaceInputErrorMessage = "";
    deletionConfirmed = true;
    inputFieldDisabled = true;

    // Refresh the visualization with the updated MS
    /*
    CF = bso::spatial_design::conformal::cf_building_model(MS);
    grm.~grammar(); // Call the destructor manually
    new (&grm) bso::grammar::grammar(CF);
    */
    visualise(MS);

    if (iteration_counter == 1) {
            bso::structural_design::sd_model SD_model = grm.sd_grammar<bso::grammar::DEFAULT_SD_GRAMMAR >(std::string("settings/sd_settings.txt"));
            visualise_sd_add(SD_model);
        } else {
            bso::structural_design::sd_model SD_model_2 = grm.sd_grammar<bso::grammar::DEFAULT_SD_GRAMMAR >(std::string("settings/sd_settings.txt"));
            visualise_sd_add(SD_model_2);
    }

    visualizationActive = true;

    msModels.push_back(MS);
}

bool validateSpaceSplitID(const std::string& input, int& outNumber, int initialSpaceCount, std::string& errorMessage) {
    try {
        outNumber = std::stoi(input);
    } catch (...) {
        errorMessage = "Input is not a valid number.";
        return false;
    }

    if (outNumber <= 0) {
        errorMessage = "ID is out of range.";
        return false;
    }

    if (removedSpaceIDs.find(outNumber) != removedSpaceIDs.end()) {
        errorMessage = "Space has been removed and is not eligible.";
        return false;
    }

    if (outNumber > MS.getLastSpaceID()) {
        errorMessage = "ID is out of range.";
        return false;
    }

    if (outNumber > initialSpaceCount) {
        errorMessage = "Space is created by split and is not eligible.";
        return false;
    }

    if (splitSpaceIDs.find(outNumber) != splitSpaceIDs.end()) {
        errorMessage = "Space has been split and is not eligible.";
        return false;
    }

    return true;
}

void scaleModel() {
    // SCALE TO RECOVER INITIAL FLOOR AREA
    std::cout << "Start scaling" << std::endl;
    double scaleFactor = sqrt(floorArea / MS.getFloorArea());
    MS.scale({{0,scaleFactor},{1,scaleFactor}});
    // Update the visualization to reflect the changes

    logAction(iteration_counter, "Scale Factor", scaleFactor);
    std::cout << scaleFactor << std::endl;
    std::cout << "Finished scaling" << std::endl;
}

void splitSpaceConfirmed(int spaceID) {
    auto* spaceToSplit = MS.getSpacePtr(MS.getSpaceWithID(spaceID));

    if (spaceToSplit) {
        // Find the largest dimension of the space
        double largestDimension = -1.0; // Initialize with a small value
        unsigned int largestDimensionIndex = 0;
        for (unsigned int k = 0; k < 3; ++k) {
            double dimension = spaceToSplit->getDimensions()(k);
            if (dimension > largestDimension) {
                largestDimension = dimension;
                largestDimensionIndex = k;
            }
        }

        // Split the space along its largest dimension
        MS.splitSpace(spaceToSplit, {{largestDimensionIndex, 2}});

        std::cout << "Space " << spaceID << " split along its largest dimension (" << largestDimensionIndex << ")." << std::endl;
        splitSpaceIDs.insert(spaceID); // Track the split space ID
        // Update the visualization to reflect the changes

        visualise(MS);
        if (iteration_counter == 1) {
            bso::structural_design::sd_model SD_model = grm.sd_grammar<bso::grammar::DEFAULT_SD_GRAMMAR >(std::string("settings/sd_settings.txt"));
            visualise_sd_add(SD_model);
        } else {
            bso::structural_design::sd_model SD_model_2 = grm.sd_grammar<bso::grammar::DEFAULT_SD_GRAMMAR >(std::string("settings/sd_settings.txt"));
            visualise_sd_add(SD_model_2);
        }
        splittingConfirmed = true;

    } else {
        std::cout << "Failed to find space for ID: " << spaceID << std::endl;
        spaceInputError = true;
        spaceInputErrorMessage = "Failed to find space for splitting.";
    }

    logAction(iteration_counter, "Space Split", spaceID);

    // Reset the UI and state flags regardless of outcome
    awaitingConfirmation = false;
    spaceInputError = false;
    spaceInputErrorMessage = "";
    inputFieldDisabled = true;

    scaleModel();

    msModels.push_back(MS);

    glutPostRedisplay();
}


void motion(int x, int y)
{
    // Calculate the boundary of the 3D view area dynamically
    float viewWidth = screenWidth / 1.28f;

    // Only perform rotation if the mouse is within the 3D view area
    if (x <= viewWidth) {
        double dx = prevx - x;
        double dy = prevy - y;

        cam_local.setrotation(cam_local.getrotation() + (dx * 0.5));
        cam_local.setelevation(cam_local.getelevation() + (dy * 0.5));

        prevx = x;
        prevy = y;
    }

    // Always handle other motion events
    vpmanager_local.mousemove_event(x, y);

    glutPostRedisplay();
}


void passive_motion(int x, int y)
{
    vpmanager_local.mousemove_event(x, y);
}


void display() {
    // Clear the window with white background
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up 2D projection matrix
    setup2D();
    // visualise(MS);

    // Set up modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (visualizationActive) {
        // Set viewport for the left half of the screen
        setup3D();

        // Render the visualization
        vpmanager_local.render(cam_local);
        checkGLError("render");

        // Reset the viewport to full window size for the rest of your GUI, if necessary
        setup2D();
        checkGLError("setup2D");
    }

    // Render the current screen
    switch (currentScreen) {
        case 0: introScreen(); break;
        case 1: buildingSpatialScreen(); break;
        case 2: structuralModelScreen(); break;
        case 3: structuralModel1Screen(); break;
        case 4: structuralModel2Screen(); break;
        case 5: removeSpaceScreen(); break;
        case 6: splitSpaceScreen(); break;
        case 7: iteration1CompleteScreen(); break;
        case 8: iteration2CompleteScreen(); break;
        case 9: surveyScreen1(); break;
        case 10: surveyScreen2(); break;
        case 11: surveyScreen3(); break;
        case 12: surveyScreen4(); break;
        case 13: surveyScreen5(); break;
        case 14: surveyScreen6(); break;
        case 15: outroScreen(); break;
        default: break;
    }

    // Swap buffers
    glutSwapBuffers();

    // Check for any OpenGL errors
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << gluErrorString(err) << std::endl;
    }

    glutPostRedisplay();
}

void setup2D() {
    glViewport(0, 0, screenWidth, screenHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, screenWidth, 0.0, screenHeight);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);

    // Disable lighting for 2D
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
}

void setup3D() {
    GLint viewportWidth = screenWidth / 1.28;
    GLint viewportHeight = 1.1 * screenHeight;

    vpmanager_local.resize(viewportWidth, viewportHeight);

    // Set the viewport to cover the left part of the screen
    glViewport(0, 0, viewportWidth, viewportHeight);

    // Setup the projection matrix for 3D rendering
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Adjust the perspective projection to match the new aspect ratio
    GLfloat aspectRatio = (GLfloat)viewportWidth / (GLfloat)viewportHeight;
    gluPerspective(45.0, aspectRatio, 0.1f, 1000.0f);

    // Switch back to modelview matrix mode
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Enable depth testing, required for 3D rendering
    glEnable(GL_DEPTH_TEST);

    // Enable lighting if your visualization uses it
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
}

bool allRowsClicked(const std::vector<int>& tableClicked) {
    for (int clicked : tableClicked) {
        if (clicked == 0) {  // Assuming 0 means no selection
            return false;  // Found a row without a selection
        }
    }
    return true;  // All rows have a selection
}

void keyboard(unsigned char key, int x, int y) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << gluErrorString(err) << std::endl;
    }

    if (currentScreen == 5) {
        if (awaitingConfirmation) {
            if (key == 'y' || key == 'Y') {
                confirmationAction(pendingOperationSpaceID);
                awaitingConfirmation = false;
                spaceInputError = false;
                spaceInputErrorMessage = "";
            } else if (key == 'n' || key == 'N') {
                awaitingConfirmation = false;
                spaceInputError = false;
                spaceInputErrorMessage = "";
                removeSpace.text = "";
                glutPostRedisplay();
            }
            return;
        }
        else if (key == 13) { // Enter key pressed
            if (inputFieldDisabled) {
            spaceInputError = false;
            spaceInputErrorMessage = "";
            glutPostRedisplay();
            return;
            }
            int spaceID;
            std::string errorMessage; // Variable to hold the validation error message
            if (validateSpaceRemoveID(removeSpace.text, spaceID, errorMessage)) { // Validate input
                // If validation succeeds, set up for confirmation
                awaitingConfirmation = true;
                // Call displayConfirmationRequest
                displayConfirmationRequest(spaceID, "Press Y to submit, N to cancel.", removeSpaceConfirmed);
            } else {
                // Handle invalid input based on the specific error message returned by validation
                removeSpace.text = "";
                spaceInputError = true;
                spaceInputErrorMessage = errorMessage;
                glutPostRedisplay();
            }
            return;
        }
        else if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            removeSpace.text += key; // Append the character to the input string
        } else if (key == 8 && removeSpace.text != "") { // Backspace key
            removeSpace.text.pop_back(); // Remove the last character from input string
        }
    }

    if (currentScreen == 6) {
        if (awaitingConfirmation) {
            if (key == 'y' || key == 'Y') {
                confirmationAction(pendingOperationSpaceID);
                awaitingConfirmation = false;
                spaceInputError = false;
                spaceInputErrorMessage = "";
            } else if (key == 'n' || key == 'N') {
                awaitingConfirmation = false;
                spaceInputError = false;
                spaceInputErrorMessage = "";
                splitSpace.text = "";
                glutPostRedisplay();
            }
            return;
        }
        else if (key == 13) { // Enter key pressed
            if (inputFieldDisabled) {
            spaceInputError = false;
            spaceInputErrorMessage = "";
            glutPostRedisplay();
            return;
            }
            int spaceID;
            std::string errorMessage; // Variable to hold the validation error message
            if (validateSpaceSplitID(splitSpace.text, spaceID, initialSpaceCount, errorMessage)) {
                // If validation succeeds, set up for confirmation
                awaitingConfirmation = true;
                // Call displayConfirmationRequest
                displayConfirmationRequest(spaceID, "Press Y to submit, N to cancel.", splitSpaceConfirmed);
            } else {
                // Handle invalid input based on the specific error message returned by validation
                splitSpace.text = "";
                spaceInputError = true;
                spaceInputErrorMessage = errorMessage;
                glutPostRedisplay();
            }
            return;
        }
        else if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            splitSpace.text += key; // Append the character to the input string
        } else if (key == 8 && removeSpace.text != "") { // Backspace key
            splitSpace.text.pop_back(); // Remove the last character from input string
        }
    }

    if (currentScreen == 9){
        if (key >= 32 && key <= 126) {
            survey1.text += key;
        } else if (key == 8 && survey1.text != "") {
            survey1.text.pop_back();
        }
    }
    if (currentScreen == 10){
        if (key >= 32 && key <= 126) {
            survey2.text += key;
        } else if (key == 8 && survey2.text != "") {
            survey2.text.pop_back();
        }
    }
    if (currentScreen == 11){
        if (key >= 32 && key <= 126) {
            survey3.text += key;
        } else if (key == 8 && survey3.text != "") {
            survey3.text.pop_back();
        }
    }
    if (currentScreen == 12){
        if (key >= 32 && key <= 126) {
            survey4.text += key;
        } else if (key == 8 && survey4.text != "") {
            survey4.text.pop_back();
        }
    }
    if (currentScreen == 13){
        if (key >= 32 && key <= 126) {
            survey5.text += key;
        } else if (key == 8 && survey5.text != "") {
            survey5.text.pop_back();
        }
    }
    if (currentScreen == 14){
        if (key >= 32 && key <= 126) {
            survey6.text += key;
        } else if (key == 8 && survey6.text != "") {
            survey6.text.pop_back();
        }
    }
    if (currentScreen == 15){
        if (key >= 32 && key <= 126) {
            email.text += key;
        } else if (key == 8 && email.text != "") {
            email.text.pop_back();
        }
    }
    glutPostRedisplay();
}

void handleSpecialKeypress(int key, int x, int y) {
    if (key == GLUT_KEY_F11) {  // Check if the key is F11
        static bool isFullScreen = false;
        isFullScreen = !isFullScreen;
        if (isFullScreen) {
            glutFullScreen(); // Go full screen
        } else {
            glutReshapeWindow(screenWidth, screenHeight); // Return to window mode with initial dimensions
            glutPositionWindow(100, 100); // Optionally reposition window
        }
    }
}

void drawText(const char* text, float startX, float centerY, float textWidth, float r, float g, float b, bool bold) {
    float lineHeight = 18; // Approximate line height
    float effectiveTextWidth = textWidth; // Assuming margins are already accounted for

    // Simplified adjustments for bold text rendering
    const float offsets[][2] = {{0.0f, 0.0f}, {0.5f, 0.0f}, {-0.5f, 0.0f}};
    int numOffsets = bold ? 3 : 1;

    glColor3f(r, g, b);

    for (int i = 0; i < numOffsets; ++i) {
        float offsetX = offsets[i][0];
        float currentX = startX;
        float currentY = centerY;

        const char* word = text;
        while (*word != '\0') { // Iterate through each character in text
            const char* nextSpace = strchr(word, ' ');
            int wordLength;
            if (nextSpace != NULL) {
                wordLength = nextSpace - word;
            } else {
                wordLength = strlen(word); // Last word
            }

            // Calculate width of the next word
            float wordWidth = 0;
            for (int j = 0; j < wordLength; j++) {
                wordWidth += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, word[j]);
            }

            // Check if the word fits in the remaining line, wrap if it doesn't
            if (currentX - startX + wordWidth > effectiveTextWidth) {
                currentY -= lineHeight;
                currentX = startX;
            }

            // Draw the word
            for (int j = 0; j < wordLength; j++) {
                glRasterPos2f(currentX + offsetX, currentY);
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, word[j]);
                currentX += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, word[j]);
            }

            // Skip the space
            if (nextSpace != NULL) {
                currentX += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, ' '); // Add space width if not the last word
                word = nextSpace + 1; // Move to the start of the next word
            } else {
                break; // End of text
            }
        }
    }
}

int calculateTextWidth(const char* text) {
    int textWidth = 0;
    while (*text) {
        textWidth += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, *text);
        ++text;
    }
    return textWidth;
}

void drawButton(const char *text, float x, float y, float width, float height, ButtonCallback callback, int variable) {
    float borderWidth = 2.0;

    // Draw button border
    glColor3f(0.0, 0.0, 0.0); // Black
    glBegin(GL_QUADS);
    glVertex2f(x - borderWidth, y - borderWidth);
    glVertex2f(x + width + borderWidth, y - borderWidth);
    glVertex2f(x + width + borderWidth, y + height + borderWidth);
    glVertex2f(x - borderWidth, y + height + borderWidth);
    glEnd();


    if (getSelectedButtonLabel() == text) {
        glColor4f(0.784, 0.098, 0.098, 0.8); // Light grey color for button background
    }
    else {
        glColor3f(1.0, 1.0, 1.0); // White color for button background
    }

    // Draw button background
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();

    // Accurately calculate text width
    int textWidth = calculateTextWidth(text);
    // Center the text horizontally and vertically
    float textX = x + (width - textWidth) / 2.0f;
    // Approximate vertical centering (adjust as needed for different fonts)
    float textY = y + (height - 18) / 2.0f; // Using 18 as an approximate height for GLUT_BITMAP_HELVETICA_18

    // Set text color and draw text
    glColor3f(0.0f, 0.0f, 0.0f); // Black
    glRasterPos2f(textX, textY);
    for (const char* p = text; *p; p++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p);
    }

    Button button = {x, y, width, height, callback, text, variable};
    buttons.push_back(button);
}

void checkTextFieldClick(TextField& textField, float mouseX, float mouseY) {
    if (mouseX >= textField.x && mouseX <= textField.x + textField.width &&
        mouseY >= textField.y && mouseY <= textField.y + textField.height) {
        textField.isActive = true;
    }
    else {
        textField.isActive = false;
    }
}

void handleCellClick(int clickedRow, int clickedColumn) {
    if (currentScreen == 3) {
        if (clickedColumn == 1) {
            structureAssignments[clickedRow] = beamStructure;
            std::cout << "Truss structure added to rectangle " << clickedRow << std::endl;
        }
        else if (clickedColumn == 2) {
            structureAssignments[clickedRow] = trussStructure;
            std::cout << "Beam structure added to rectangle " << clickedRow << std::endl;
        }
        else if (clickedColumn == 3) {
            structureAssignments[clickedRow] = flatShellStructure;
            std::cout << "Flat shell structure added to rectangle " << clickedRow << std::endl;
        }
        if(clickedColumn != 0) {
            tableClicked[clickedRow] = clickedColumn;
        }
    }

    if (currentScreen == 4) {
        if (clickedColumn == 1) {
            structureAssignments_it2[clickedRow] = beamStructure;
            std::cout << "Truss structure added to rectangle " << clickedRow << std::endl;
        }
        else if (clickedColumn == 2) {
            structureAssignments_it2[clickedRow] = trussStructure;
            std::cout << "Beam structure added to rectangle " << clickedRow << std::endl;
        }
        else if (clickedColumn == 3) {
            structureAssignments_it2[clickedRow] = flatShellStructure;
            std::cout << "Flat shell structure added to rectangle " << clickedRow << std::endl;
        }
        if(clickedColumn != 0) {
            tableClicked[clickedRow] = clickedColumn;
        }
    }


    createSDmodel();
}


void onMouseClick(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        float mouseY = screenHeight - static_cast<float>(y);
        float mouseX = static_cast<float>(x);

        // Check each text field individually
        checkTextFieldClick(removeSpace, mouseX, mouseY);
        checkTextFieldClick(splitSpace, mouseX, mouseY);
        checkTextFieldClick(explanation, mouseX, mouseY);
        checkTextFieldClick(survey1, mouseX, mouseY);
        checkTextFieldClick(survey2, mouseX, mouseY);
        checkTextFieldClick(survey3, mouseX, mouseY);
        checkTextFieldClick(survey4, mouseX, mouseY);
        checkTextFieldClick(survey5, mouseX, mouseY);
        checkTextFieldClick(survey6, mouseX, mouseY);
        checkTextFieldClick(email, mouseX, mouseY);

        // Check for button clicks
        for (const auto& btn : buttons) {
            if (mouseX >= btn.x && mouseX <= btn.x + btn.width &&
                mouseY >= btn.y && mouseY <= btn.y + btn.height) {
                if (btn.callback) {
                    btn.callback(btn.variable);
                }
                break;
            }
        }

        if(currentScreen == 3 || currentScreen == 4) {
            int x = 1550;
            int y = 130;
            int width = 350;
            int cellHeight = 18;
            int numRows = nRectangles;
            int columnWidth = width / 4;

            int clickedRow = (y + numRows * cellHeight - mouseY) / cellHeight;
            int clickedColumn = (mouseX - x) / columnWidth;

            if (clickedRow >= 0 && clickedRow < numRows && clickedColumn >= 0 && clickedColumn < 4) {
                handleCellClick(clickedRow, clickedColumn);
            }

        }
    }
}

void drawTextField(int x, int y, int width, int height, TextField& textfield) {
    textfield.x = x;
    textfield.y = y;
    textfield.width = width;
    textfield.height = height;
    float borderWidth = 2.0;

    // Calculate the adjusted width and height considering padding
    int adjustedWidth = width - 2 * borderWidth;
    int adjustedHeight = height - 2 * borderWidth;

    glColor3f(0.0, 0.0, 0.0); // Black color for border
    glBegin(GL_QUADS);
    glVertex2f(x - borderWidth, y - borderWidth);
    glVertex2f(x + width + borderWidth, y - borderWidth);
    glVertex2f(x + width + borderWidth, y + height + borderWidth);
    glVertex2f(x - borderWidth, y + height + borderWidth);
    glEnd();

    // Draw text field background
    glColor3f(1.0, 1.0, 1.0); // white background for text field
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();

    // Set the color for the text
    glColor3f(0.0, 0.0, 0.0); // black text

    // Implement text wrapping within the width of the text field
    // This is a simplistic approach and might need adjustment for different font widths
    int maxWidth = adjustedWidth; // maximum width for text before wrapping
    int currentWidth = 0;
    std::string line;
    std::vector<std::string> lines;

    for (char c : textfield.text) {
        if (c == '\n' || currentWidth > maxWidth) {
            lines.push_back(line);
            line.clear();
            currentWidth = 0;
        }
        if (c != '\n') {
            line += c;
            // Estimate the width of the character, adjust for the new font size (18)
            currentWidth += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, c);
        }
    }
    // Add the last line if it's not empty
    if (!line.empty()) {
        lines.push_back(line);
    }

    // Now draw the text line by line from the top
    int lineHeight = 20; // Adjusted for the new font size (18) and additional padding
    int textHeight = lines.size() * lineHeight; // Total height of the text
    int startY = y + height - lineHeight; // Calculate the starting Y coordinate from the top with padding

    for (size_t i = 0; i < lines.size(); ++i) {
        int currentY = startY - i * lineHeight; // Calculate the Y coordinate for this line
        glRasterPos2f(x + borderWidth, currentY); // Adjust for the left padding
        for (char c : lines[i]) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c); // Use GLUT_BITMAP_HELVETICA_18 for the new font size
        }
    }

    // Draw the cursor if the text field is active
    if (textfield.isActive) {
        int cursorX = x + borderWidth + glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)textfield.text.c_str()); // Adjust for left padding
        int cursorY = startY; // Use the same starting Y coordinate as the text
        glColor3f(0.0, 0.0, 0.0); // black cursor
        glBegin(GL_LINES);
        glVertex2f(cursorX + 2, cursorY + 18);
        glVertex2f(cursorX + 2, cursorY - 3);  // Adjust the Y coordinate to draw the cursor above the text
        glEnd();
    }
}

void iterationDone() {
    // Update the grammar with new MS
    CF = bso::spatial_design::conformal::cf_building_model(MS);
    grm.~grammar(); // Call the destructor manually
    new (&grm) bso::grammar::grammar(CF); // Reconstruct 'grm' in the existing memory

    bso::structural_design::sd_model SD_model = grm.sd_grammar<bso::grammar::DEFAULT_SD_GRAMMAR >(std::string("settings/sd_settings.txt"));
    nRectangles = SD_model.getSubRectangles().size();
    tableClicked = std::vector<int>(nRectangles, 0);
}

void drawFourColumnTable(int x, int y, int width, int cellHeight, const std::vector<std::string>& column1, std::vector<int> clickedOption) {
    size_t numRows = column1.size();

    int tableHeight = numRows * cellHeight;

    glColor3f(0.0, 0.0, 0.0);

    int currentY = y + tableHeight;
    int columnWidth = width / 4;

    for (size_t i = 0; i < numRows; ++i) {
        glBegin(GL_LINES);
        glVertex2f(x, currentY);
        glVertex2f(x + width, currentY);
        glEnd();

        for (int col = 0; col < 4; ++col) {
            int columnX = x + col * columnWidth;

            if (col > 0) {
                glBegin(GL_LINES);
                glVertex2f(columnX, currentY);
                glVertex2f(columnX, y);
                glEnd();
            }

            if (i < clickedOption.size() && col == clickedOption[i] && clickedOption[i] > 0 && clickedOption[i] < 4) {
                glColor4f(0.784, 0.098, 0.098, 0.8);
                glBegin(GL_QUADS);
                glVertex2f(columnX, currentY);
                glVertex2f(columnX + columnWidth, currentY);
                glVertex2f(columnX + columnWidth, currentY - cellHeight);
                glVertex2f(columnX, currentY - cellHeight);
                glEnd();
                glColor3f(0.0, 0.0, 0.0); // Reset color to black for text and lines
            }

            // Set text and positioning
            std::string text = (col == 0 && i < column1.size()) ? column1[i] : (col == 1 ? "beam" : (col == 2 ? "truss" : "flat_shell"));
            int textWidth = glutBitmapLength(GLUT_BITMAP_8_BY_13, (const unsigned char*)text.c_str());
            int textX = columnX + (columnWidth - textWidth) / 2; // Center text horizontally
            int textY = currentY - 11; // Set vertical position

            glRasterPos2f(textX, textY);
            for (char c : text) {
                glutBitmapCharacter(GLUT_BITMAP_8_BY_13, c);
            }
        }

        currentY -= cellHeight;
    }
}



// void setWindowIcon(const char* iconPath) {
//     // Load the icon from the given file path
//     HICON hIcon = (HICON)LoadImage(NULL, iconPath, IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);

//     // Set the window icon
//     SendMessage(GetActiveWindow(), WM_SETICON, ICON_BIG, (LPARAM)hIcon);
// }

void countSelectionsFromTable(const std::vector<int>& tableClicked) {
    int countBeams = 0, countTrusses = 0, countFlatShells = 0;

    for (int clickedColumn : tableClicked) {
        switch (clickedColumn) {
            case 1:
                countBeams++;
                break;
            case 2:
                countTrusses++;
                break;
            case 3:
                countFlatShells++;
                break;
            default:
                // handle unclicked or improperly clicked rows, if necessary
                break;
        }
    }
    logAction(iteration_counter, "Number of beams", countBeams);
    logAction(iteration_counter, "Number of trusses", countTrusses);
    logAction(iteration_counter, "Number of flat shells", countFlatShells);
    std::cout << "Beams: " << countBeams << ", Trusses: " << countTrusses << ", Flat Shells: " << countFlatShells << std::endl;
}


void introScreen() {
    glEnable(GL_LIGHTING); // Enable to show image becomes black
    glEnable(GL_LIGHT0); // Enable to prevent image becomes black

    float picWidth = screenWidth / 1.28f; // Width of the picture as specified.
    float picHeight = screenHeight;
    displayTexture(imgVilla, 0, 0, picWidth, picHeight);

    glDisable(GL_LIGHTING); //Disbale for other GUI elements
    glDisable(GL_LIGHT0); //Disbale for other GUI elements

    // Draw the title
    drawText("Simulation of Co-evolutionary Design Processes", startText, 1000, textWidth, 0.0f, 0.0f, 0.0f, true);
    drawText("You are about to start an assignment involving a simulation of co-evolutionary design processes. This is a method to iteratively optimize a building spatial design based on the structural performance of the design. Therefore, you will create a structural model and consequently change the building spatial design. You will in total complete 2 iterations of SCDP. Your assignment starts with a building spatial design from the villa depicted on the left.", startText, 900, textWidth, 0.0f, 0.0f, 0.0f);

    drawText("As part of this assignment, we ask you to 'think aloud.' This means we want you to say what you are thinking about at each step of the process, as if you were explaining your thoughts to someone else. This helps us understand your decision-making process better. Please remember to keep verbalizing your thoughts as you work through each screen.", startText, 550, textWidth, 0.0f, 0.0f, 0.0f);

    drawText("Please read the instructions carefully on each screen. If you have any questions, please raise your hand.", startText, 200, textWidth, 0.0f, 0.0f, 0.0f);

    drawButton("Start assignment", startText, 80, textWidth, 50, changeScreen, 1);
}

void buildingSpatialScreen() {
    drawText("Building spatial design", startText, 1000, textWidth, 0.0f, 0.0f, 0.0f, true);
    drawText("This screen displays your current building spatial design. Use the mouse to rotate the model and familiarize yourself with each uniquely numbered space. The counter at the top left indicates your current iteration. When you are ready, click 'Continue' to proceed.", startText, 850, textWidth, 0.0f, 0.0f, 0.0f);
    drawButton("Continue", startText, 80, textWidth, 50, changeScreen, 2);

    std::string iterationText = "Iteration " + std::to_string(iteration_counter);
    drawText(iterationText.c_str(), 80, 1000, 250, 0.0f, 0.0f, 0.0f, true);
}

void structuralModelScreen() {
    drawText("Structural design", startText, 1000, textWidth, 0.0f, 0.0f, 0.0f, true);
    drawText("The first step in this assignment is the creation of a structural model. You will assign one of three possible structural types to each rectangle of the spatial model. The available types are:", startText, 950, textWidth, 0.0f, 0.0f, 0.0f);
    drawText("- Beam", startText, 840, textWidth, 0.0f, 0.0f, 0.0f, false);
    drawText("- Truss", startText, 820, textWidth, 0.0f, 0.0f, 0.0f, false);
    drawText("- Flat shell", startText, 800, textWidth, 0.0f, 0.0f, 0.0f, false);


    GLfloat emissionColor[4] = {1.0f, 1.0f, 1.0f, 1.0f}; // Emit the texture's color
    glMaterialfv(GL_FRONT, GL_EMISSION, emissionColor); // Apply to front face

    float picWidth = textWidth;
    float picHeight = 136;
    displayTexture(imgElements, startText, 600, picWidth, picHeight);

    GLfloat defaultEmission[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    glMaterialfv(GL_FRONT, GL_EMISSION, defaultEmission);

    drawText("In the next screen you have to assign a structural type to each rectangle by clicking the corresponding option in the table. Choose the type you believe is best suited for each rectangle. All horizontal rectangles (floors & roof) are assigned a flat shell by default and can not be changed.", startText, 450, textWidth, 0.0f, 0.0f, 0.0f);
    drawText("Continue to think aloud as you assign structural types to each rectangle; describe your choices and the reasoning behind them.", startText, 250, textWidth, 0.0f, 0.0f, 0.0f);
    if (iteration_counter == 1) {
        drawButton("Create structural design", startText, 80, textWidth, 50, changeScreen, 3);
    }
    else {
        drawButton("Create structural design", startText, 80, textWidth, 50, changeScreen, 4);
    }

    std::string iterationText = "Iteration " + std::to_string(iteration_counter);
    drawText(iterationText.c_str(), 80, 1000, 250, 0.0f, 0.0f, 0.0f, true);
}

void structuralModel1Screen() {
    std::vector<std::string> nr_rectangles;
    // Populate with numbers corresponding to each rectangle
    for (size_t j = 0; j < nRectangles; ++j) {
        nr_rectangles.push_back(std::to_string(j + 1));  // numbering from 1
    }

    // Initialize or reset the tableClicked vector for the entire list
    if(!tableInitialized){
        tableClicked = std::vector<int>(nRectangles, 0);
        tableInitialized = true;
    }

    // Draw table and other UI elements as before, adjust positioning if needed
    drawFourColumnTable(1550, 130, 350, 18, nr_rectangles, tableClicked);

    // Display a text prompt below the table
    //drawText("Once you are finished, please continue below.", startText, 200, textWidth, 0.0f, 0.0f, 0.0f);
    if (allRowsClicked(tableClicked)) {
        drawText("Are you sure this is your final design? Press the button below to continue.", startText, 100, textWidth, 0.784f, 0.098f, 0.098f);
        drawButton("Confirm structural model", startText, 20, textWidth, 40, [](int){analyzeSDmodel(); countSelectionsFromTable(tableClicked); changeScreen(5);}, 0);
    } else {
        drawText("Please think aloud why you choose certain types. You can only continue if you have selected a type for every rectangle.", startText, 100, textWidth, 0.784f, 0.098f, 0.098f);
    }
    // Provide a button to proceed to the next view in the UI
    //drawButton("Part two structural design", startText, 20, textWidth, 50, [](int){analyzeSDmodel(); changeScreen(5);}, 0);

    // Display the current iteration number at the bottom
    std::string iterationText = "Iteration " + std::to_string(iteration_counter);
    drawText(iterationText.c_str(), 80, 1000, 250, 0.0f, 0.0f, 0.0f, true);
}

void structuralModel2Screen() {
    std::vector<std::string> nr_rectangles;
    // Populate with numbers corresponding to each rectangle
    for (size_t j = 0; j < nRectangles; ++j) {
        nr_rectangles.push_back(std::to_string(j + 1));  // numbering from 1
    }

    // Initialize or reset the tableClicked vector for the entire list
    if(!tableInitialized){
        tableClicked = std::vector<int>(nRectangles, 0);
        tableInitialized = true;
    }

    // Draw table and other UI elements as before, adjust positioning if needed
    drawFourColumnTable(1550, 130, 350, 18, nr_rectangles, tableClicked);

    // Display a text prompt below the table
    //drawText("Once you are finished, please continue below.", startText, 200, textWidth, 0.0f, 0.0f, 0.0f);

    if (allRowsClicked(tableClicked)) {
        drawText("Are you sure this is your final design? Press the button below to continue.", startText, 100, textWidth, 0.784f, 0.098f, 0.098f);
        drawButton("Confirm structural model", startText, 20, textWidth, 40, [](int){analyzeSDmodel(); countSelectionsFromTable(tableClicked); changeScreen(5);}, 0);
    } else {
        drawText("Please think aloud why you choose certain types. You can only continue if you have selected a type for every rectangle.", startText, 100, textWidth, 0.784f, 0.098f, 0.098f);
    }
    // Provide a button to proceed to the next view in the UI
    //drawButton("Part two structural design", startText, 20, textWidth, 50, [](int){analyzeSDmodel(); changeScreen(5);}, 0);

    // Display the current iteration number at the bottom
    std::string iterationText = "Iteration " + std::to_string(iteration_counter);
    drawText(iterationText.c_str(), 80, 1000, 250, 0.0f, 0.0f, 0.0f, true);
}

void removeSpaceScreen() {
    drawText("Space removal", startText, 1000, textWidth, 0.0f, 0.0f, 0.0f, true);
    drawText("You are asked to remove a maximum of 1 space. Think aloud about your reasons for choosing a particular space to remove. Enter the space ID below and press 'Enter' to confirm. Please think aloud as you decide which space to remove. Explain your reasoning for the choice you're making.", startText, 850, textWidth, 0.0f, 0.0f, 0.0f);

    if (deletionConfirmed) {
        // Draw the message indicating the space has been deleted
        drawText("Space deleted successfully. Click 'Continue' to proceed.", startText, 400, textWidth, 0.0f, 0.0f, 0.0f);

        // Draw the "Continue" button
        drawButton("Continue", startText, 80, textWidth, 50, changeScreen, 6);
    } else if (!inputFieldDisabled) {
        // Render the input field only if it's not disabled
        drawText("Please enter the space ID below to remove.", startText, 600, textWidth, 0.0f, 0.0f, 0.0f);
        drawTextField(startText, 500, textWidth, 25, removeSpace);
    }

    // Check and display error message below the text field if there's an error
    if (spaceInputError) {
        bool isConfirmation = spaceInputErrorMessage == "Press Y to submit, N to cancel.";

        float r = isConfirmation ? 0.0f : 1.0f; // Blue for confirmation, red for error
        float g = 0.0f;
        float b = isConfirmation ? 1.0f : 0.0f;

        drawText(spaceInputErrorMessage.c_str(), startText, 400, textWidth, r, g, b);
    }

    drawText("Please note that the SD model is not updated after spatial modifications", 800, 50, 800, 1.0f, 0.0f, 0.0f, true);
    std::string iterationText = "Iteration " + std::to_string(iteration_counter);
    drawText(iterationText.c_str(), 80, 1000, 250, 0.0f, 0.0f, 0.0f, true);
}


void splitSpaceScreen() {
    drawText("Space splitting", startText, 1000, textWidth, 0.0f, 0.0f, 0.0f, true);
    drawText("You are asked to split a maximum of 1 space. As you decide which space to split, please explain your reasoning aloud. Enter the space ID for the space you wish to split and press 'Enter' to confirm.", startText, 850, textWidth, 0.0f, 0.0f, 0.0f);

    if (splittingConfirmed) {
        // Draw the message indicating the space has been deleted
        drawText("Space split successfully. Click 'Continue' to proceed.", startText, 400, textWidth, 0.0f, 0.0f, 0.0f);
        if (iteration_counter < max_iterations) {
            drawButton("Continue", startText, 80, textWidth, 50, changeScreen, 7);
        }
        else {
            drawButton("Continue", startText, 80, textWidth, 50, changeScreen, 8);
        }
    }
    else if (!inputFieldDisabled) {
        // Render the input field only if it's not disabled
        drawText("Please enter the space ID below to split.", startText, 600, textWidth, 0.0f, 0.0f, 0.0f);
        drawTextField(startText, 500, textWidth, 25, splitSpace);
    }

    // Check and display error message below the text field if there's an error
    if (spaceInputError) {
        bool isConfirmation = spaceInputErrorMessage == "Press Y to submit, N to cancel.";

        float r = isConfirmation ? 0.0f : 1.0f; // Blue for confirmation, red for error
        float g = 0.0f;
        float b = isConfirmation ? 1.0f : 0.0f;

        drawText(spaceInputErrorMessage.c_str(), startText, 400, textWidth, r, g, b);
    }
    scalingCompleted = false;
    drawText("Please note that the SD model is not updated after spatial modifications", 800, 50, 800, 1.0f, 0.0f, 0.0f, true);
    std::string iterationText = "Iteration " + std::to_string(iteration_counter);
    drawText(iterationText.c_str(), 80, 1000, 250, 0.0f, 0.0f, 0.0f, true);
}


void iteration1CompleteScreen() {
    drawText("Iteration 1 complete", startText, 1000, textWidth, 0.0f, 0.0f, 0.0f, true);
    std::string iterationText = "Iteration " + std::to_string(iteration_counter);
    drawText(iterationText.c_str(), 80, 1000, 250, 0.0f, 0.0f, 0.0f, true);
    drawText("First iteration complete! The building spatial model has been adjusted to recover the initial floor area. Review all previous steps on the screen to your left. The updated design will now serve as the starting point for the second iteration.", startText, 850, textWidth, 0.0f, 0.0f, 0.0f);

    std::string nextIterationText = "Start iteration " + std::to_string(iteration_counter + 1);
    drawButton(nextIterationText.c_str(), startText, 80, textWidth, 50, [](int){iteration_counter++; iterationDone(); changeScreen(1);}, 0);

    deletionConfirmed = false;
    splittingConfirmed = false;
    removeSpace.text = "";
    splitSpace.text = "";
}

void iteration2CompleteScreen() {
    drawText("Assignment complete", startText, 1000, textWidth, 0.0f, 0.0f, 0.0f, true);
    std::string iterationText = "Iteration " + std::to_string(iteration_counter);
    drawText(iterationText.c_str(), 80, 1000, 250, 0.0f, 0.0f, 0.0f, true);
    drawText("Congratulations! You have successfully completed the second iteration and the main portion of this assignment. You may now stop verbalizing your thoughts. Please remain in the Teams meeting. On the left, you can review your entire assignment progression, from the initial building spatial design to the final building spatial design. Please proceed to fill out the survey.", startText, 850, textWidth, 0.0f, 0.0f, 0.0f);

    drawButton("Start survey", startText, 80, textWidth, 50, changeScreen, 9);

    deletionConfirmed = false;
    splittingConfirmed = false;
    removeSpace.text = "";
    splitSpace.text = "";
}

float surveyStart = 100;
float surveyWidth = 800;
float buttonWidth = 50;
float totalSpacingWidth = surveyWidth - (5 * buttonWidth);
float spacing = totalSpacingWidth / 4;

void surveyScreen1() {
    drawText("Survey", surveyStart, 1000, surveyWidth, 0.0f, 0.0f, 0.0f, true);
    drawText("Question 1/6", surveyStart + surveyWidth - 100, 1000, surveyWidth, 0.0f, 0.0f, 0.0f);

    drawText("How satisfied were you with the overall design process you experienced?", surveyStart, 900, surveyWidth, 0.0f, 0.0f, 0.0f);

    drawButton("1", surveyStart, 800, buttonWidth, 30, buttonClicked, 1);
    drawButton("2", surveyStart + buttonWidth + spacing, 800, buttonWidth, 30, buttonClicked, 2);
    drawButton("3", surveyStart + 2 * (buttonWidth + spacing), 800, buttonWidth, 30, buttonClicked, 3);
    drawButton("4", surveyStart + 3 * (buttonWidth + spacing), 800, buttonWidth, 30, buttonClicked, 4);
    drawButton("5", surveyStart + 4 * (buttonWidth + spacing), 800, buttonWidth, 30, buttonClicked, 5);

    drawText("Very Unsatisfied", surveyStart - 50, 750, textWidth, 0.0f, 0.0f, 0.0f);
    drawText("Unsatisfied", surveyStart + 165, 750, textWidth, 0.0f, 0.0f, 0.0f);
    drawText("Neutral", surveyStart + 370, 750, textWidth, 0.0f, 0.0f, 0.0f);
    drawText("Satisfied", surveyStart + 550, 750, textWidth, 0.0f, 0.0f, 0.0f);
    drawText("Very Satisfied", surveyStart + surveyWidth - 110, 750, surveyWidth, 0.0f, 0.0f, 0.0f);

    drawText("Please explain your answer:", surveyStart, 450, surveyWidth, 0.0f, 0.0f, 0.0f);
    drawTextField(surveyStart, 400, surveyWidth, 300, survey1);
    if (getSelectedButtonLabel() != "") {
        drawButton("Next", 100, surveyStart, surveyWidth, 50, [](int){logAction(std::stoi(getSelectedButtonLabel()), "How satisfied were you with the overall design process you experienced?", survey1.text); changeScreen(10);}, 0);
    }

}

void surveyScreen2() {
    drawText("Survey", surveyStart, 1000, surveyWidth, 0.0f, 0.0f, 0.0f, true);
    drawText("Question 2/6", surveyStart + surveyWidth - 100, 1000, surveyWidth, 0.0f, 0.0f, 0.0f);

    drawText("How easy was it to use the design tools provided?", surveyStart, 900, surveyWidth, 0.0f, 0.0f, 0.0f);

    drawButton("1", surveyStart, 800, buttonWidth, 30, buttonClicked, 1);
    drawButton("2", surveyStart + buttonWidth + spacing, 800, buttonWidth, 30, buttonClicked, 2);
    drawButton("3", surveyStart + 2 * (buttonWidth + spacing), 800, buttonWidth, 30, buttonClicked, 3);
    drawButton("4", surveyStart + 3 * (buttonWidth + spacing), 800, buttonWidth, 30, buttonClicked, 4);
    drawButton("5", surveyStart + 4 * (buttonWidth + spacing), 800, buttonWidth, 30, buttonClicked, 5);

    drawText("Very Difficult", surveyStart - 40, 750, textWidth, 0.0f, 0.0f, 0.0f);
    drawText("Difficult", surveyStart + 180, 750, textWidth, 0.0f, 0.0f, 0.0f);
    drawText("Neutral", surveyStart + 370, 750, textWidth, 0.0f, 0.0f, 0.0f);
    drawText("Easy", surveyStart + 570, 750, textWidth, 0.0f, 0.0f, 0.0f);
    drawText("Very Easy", surveyStart + surveyWidth - 60, 750, surveyWidth, 0.0f, 0.0f, 0.0f);

    drawText("Please explain your answer:", surveyStart, 450, surveyWidth, 0.0f, 0.0f, 0.0f);
    drawTextField(surveyStart, 400, surveyWidth, 300, survey2);
    if (getSelectedButtonLabel() != "") {
        drawButton("Next", 100, surveyStart, surveyWidth, 50, [](int){logAction(std::stoi(getSelectedButtonLabel()), "How easy was it to use the design tools provided?", survey2.text); changeScreen(11);}, 0);
    }
}

void surveyScreen3() {
    drawText("Survey", surveyStart, 1000, surveyWidth, 0.0f, 0.0f, 0.0f, true);
    drawText("Question 3/6", surveyStart + surveyWidth - 100, 1000, surveyWidth, 0.0f, 0.0f, 0.0f);

    drawText("How satisfied are you with the design decisions you made?", surveyStart, 900, surveyWidth, 0.0f, 0.0f, 0.0f);

    drawButton("1", surveyStart, 800, buttonWidth, 30, buttonClicked, 1);
    drawButton("2", surveyStart + buttonWidth + spacing, 800, buttonWidth, 30, buttonClicked, 2);
    drawButton("3", surveyStart + 2 * (buttonWidth + spacing), 800, buttonWidth, 30, buttonClicked, 3);
    drawButton("4", surveyStart + 3 * (buttonWidth + spacing), 800, buttonWidth, 30, buttonClicked, 4);
    drawButton("5", surveyStart + 4 * (buttonWidth + spacing), 800, buttonWidth, 30, buttonClicked, 5);

    drawText("Very Unsatisfied", surveyStart - 50, 750, textWidth, 0.0f, 0.0f, 0.0f);
    drawText("Unsatisfied", surveyStart + 165, 750, textWidth, 0.0f, 0.0f, 0.0f);
    drawText("Neutral", surveyStart + 370, 750, textWidth, 0.0f, 0.0f, 0.0f);
    drawText("Satisfied", surveyStart + 550, 750, textWidth, 0.0f, 0.0f, 0.0f);
    drawText("Very Satisfied", surveyStart + surveyWidth - 110, 750, surveyWidth, 0.0f, 0.0f, 0.0f);

    drawText("Please explain your answer:", surveyStart, 450, surveyWidth, 0.0f, 0.0f, 0.0f);
    drawTextField(surveyStart, 400, surveyWidth, 300, survey3);
    if (getSelectedButtonLabel() != "") {
        drawButton("Next", 100, surveyStart, surveyWidth, 50, [](int){logAction(std::stoi(getSelectedButtonLabel()), "How satisfied are you with the design decisions you made?", survey3.text); changeScreen(12);}, 0);
    }
}

void surveyScreen4() {
    drawText("Survey", surveyStart, 1000, surveyWidth, 0.0f, 0.0f, 0.0f, true);
    drawText("Question 4/6", surveyStart + surveyWidth - 100, 1000, surveyWidth, 0.0f, 0.0f, 0.0f);

    drawText("How do you think having AI assistance would have changed your design process?", surveyStart, 900, surveyWidth, 0.0f, 0.0f, 0.0f);

    drawText("Please explain your answer:", surveyStart, 450, surveyWidth, 0.0f, 0.0f, 0.0f);
    drawTextField(surveyStart, 400, surveyWidth, 300, survey4);

    drawButton("Next", 100, surveyStart, surveyWidth, 50, [](int){logAction(0, "How do you think having AI assistance would have changed your design process?", survey4.text); changeScreen(13);}, 0);

}

void surveyScreen5() {
    drawText("Survey", surveyStart, 1000, surveyWidth, 0.0f, 0.0f, 0.0f, true);
    drawText("Question 5/6", surveyStart + surveyWidth - 100, 1000, surveyWidth, 0.0f, 0.0f, 0.0f);

    drawText("Would you choose to use this design process in future projects based on your current experience?", surveyStart, 900, surveyWidth, 0.0f, 0.0f, 0.0f);

    drawText(" Please explain why or why not.", surveyStart, 450, surveyWidth, 0.0f, 0.0f, 0.0f);
    drawTextField(surveyStart, 400, surveyWidth, 300, survey5);

    drawButton("Next", 100, surveyStart, surveyWidth, 50, [](int){logAction(0, "Would you choose to use this design process in future projects based on your current experience?", survey5.text); changeScreen(14);}, 0);

}

void surveyScreen6() {
    drawText("Survey", surveyStart, 1000, surveyWidth, 0.0f, 0.0f, 0.0f, true);
    drawText("Question 6/6", surveyStart + surveyWidth - 100, 1000, surveyWidth, 0.0f, 0.0f, 0.0f);

    drawText("Please provide any additional comments or suggestions on how we could improve the design process used in this experiment.", surveyStart, 900, surveyWidth, 0.0f, 0.0f, 0.0f);

    drawText("Please explain your answer:", surveyStart, 450, surveyWidth, 0.0f, 0.0f, 0.0f);
    drawTextField(surveyStart, 400, surveyWidth, 300, survey6);

    drawButton("Next", 100, surveyStart, surveyWidth, 50, [](int){logAction(0, "Please provide any additional comments or suggestions on how we could improve the design process used in this experiment.", survey6.text); changeScreen(15);}, 0);
}

void outroScreen() {
    glEnable(GL_LIGHTING); // Enable to show image becomes black
    glEnable(GL_LIGHT0); // Enable to prevent image becomes black

    float picWidth = screenWidth / 1.28f; // Width of the picture as specified.
    float picHeight = screenHeight;
    displayTexture(imgVilla, 0, 0, picWidth, picHeight);

    glDisable(GL_LIGHTING); //Disbale for other GUI elements
    glDisable(GL_LIGHT0); //Disbale for other GUI elements

    // Draw the title
    drawText("Simulation of Co-evolutionary Design Processes", startText, 1000, textWidth, 0.0f, 0.0f, 0.0f, true);
    drawText("You have just completed the assignment. Thank you very much for your participation! If you would like to receive the results of this research and be included in the acknowledgments, please leave your email below.", startText, 900, textWidth, 0.0f, 0.0f, 0.0f);

    drawTextField(startText, 700, textWidth, 25, email);

    drawText("This is the end of the assignment. Don't forget to follow the 'after the assignment' steps in the set-up guide.", startText, 300, textWidth, 0.0f, 0.0f, 0.0f);

    drawButton("Finish assignment", startText, 80, textWidth, 50, [](int){logAction(0, "Email", email.text); writeToProcessFile("assignment_5_SCDP_human.csv"); exit(0);}, 0);
}

int main(int argc, char** argv) {
    // Initialize GLUT
    glutInit(&argc, argv);
    initializeModels();
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(screenWidth, screenHeight);
    glutCreateWindow("Assignment 5: SCDP Human Design");
    glutFullScreen(); // Make the window fullscreen right from the start

    initializeTextures();

    // Set callback functions
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(reshape);
    glutMouseFunc(onMouseClick);
    glutMotionFunc(motion);
    glutPassiveMotionFunc(passive_motion);
    glutSpecialFunc(handleSpecialKeypress);

    glShadeModel(GL_SMOOTH);

    // Main loop
    glutMainLoop();
    return 0;
}
