#ifndef VISUALIZATION_HPP
#define VISUALIZATION_HPP

#ifdef __unix__
// see http://stackoverflow.com/questions/31579243/segmentation-fault-before-main-when-using-glut-and-stdstring
#include <pthread.h>
void* simpleFunc(void*) {return NULL;}
void forcePThreadLink() {pthread_t t1; pthread_create(&t1,NULL,&simpleFunc,NULL);}

#elif defined(_WIN32) || defined(WIN32)
#include <time.h>
#endif

#include <bso/visualization/user_interface/viewportmanager.hpp>
#include <bso/visualization/utilities/camera.hpp>

#ifdef CF_BUILDING_HPP
#include <bso/visualization/models/cf_building.hpp>
#endif // CF_BUILDING_HPP

#ifdef BP_MODEL_HPP
#include <bso/building_physics/bp_model.hpp>
#include <bso/visualization/models/bp_model.hpp>
#endif // BP_MODEL_HPP

#ifdef SD_MODEL_HPP
#include <bso/structural_design/sd_model.hpp>
#include <bso/visualization/models/sd_model.hpp>
#endif // SD_MODEL_HPP

#ifdef DL_MODEL_HPP
#include <bso/daylight/dl_model.hpp>
#include <bso/visualization/models/dl_model.hpp>
#endif // DL_MODEL_HPP

// Visualisation models
#include <bso/visualization/models/ms_building.hpp>
#include <bso/visualization/models/sc_building.hpp>
#include <bso/visualization/models/geometry.hpp>
#include <bso/visualization/models/pn_building.hpp>

#include <stdlib.h>

namespace bso { namespace visualization
{

viewportmanager vpmanager;
orbitalcamera   cam;
int prevx, prevy;



void reshape(int w, int h)
{
	vpmanager.resize(w,h);
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	vpmanager.render(cam);

	glutSwapBuffers();
}

void mouse(int button, int state, int x, int y)
{
	prevx = x;
	prevy = y;

	vpmanager.mouse_event(button, state,x, y, glutGet(GLUT_ELAPSED_TIME));
}

void motion(int x, int y)
{
	double dx = prevx-x,
				 dy = prevy-y;

	cam.setrotation(cam.getrotation() + (dx*0.5));
	cam.setelevation(cam.getelevation() + (dy*0.5));

	prevx = x;
	prevy = y;

	vpmanager.mousemove_event(x, y);

	glutPostRedisplay();
}

void passive_motion(int x, int y)
{
	vpmanager.mousemove_event(x, y);
}

void keyboard(unsigned char key, int x, int y)
{
	(void)x; //does nothing
	(void)y; //does nothing
	switch (key) {
	case ' ' :
		// do nothing
		break;
	case 'q' :
	case 'Q' :
		// In freeglut, leave the main loop so control returns to caller
		// instead of terminating the entire process. Fall back to exit(0)
		// when this option is not available.
		#ifdef GLUT_ACTION_ON_WINDOW_CLOSE
			glutLeaveMainLoop();
		#else
			exit(0);
		#endif
	default:
		if (vpmanager.key_event(key, 0)) glutPostRedisplay();
		break;
	};
}


void initVisualization(int argc, char *argv[])
{

	/*
	//trace memory allocation (leak testing)
	setenv("MALLOC_TRACE", "mtrace.log", 1);
	mtrace();

	createbsp();

	cout << "deleting" << endl;
	delete_bsp(testbsp);
	*/

	//init random seed with the current time
	srand(time(NULL));

	//init glut
	glutInit(&argc, argv);
	//glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(600, 300);
	glutInitWindowPosition(100, 100);
	glutCreateWindow(argv[0]);

	// If using freeglut, make sure that closing the window causes
	// glutMainLoop() to return instead of calling exit().
	#ifdef GLUT_ACTION_ON_WINDOW_CLOSE
		glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	#endif
	
	//maximize window (not fullscreen)
	int scrw = glutGet(GLUT_SCREEN_WIDTH)/1.5,
			scrh = glutGet(GLUT_SCREEN_HEIGHT)/1.5;
	if (scrw > 0 && scrh > 0) glutReshapeWindow(scrw, scrh);

	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutPassiveMotionFunc(passive_motion);

	//init gl
	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	if(1 == 1){glClearColor(1.0, 1.0, 1.0, 1.0);} // toggles white background, I deleted the input possibility
	else      {glClearColor(0.8, 0.8, 0.8, 1.0);}

	glClearDepth(1000.0);
} // initVisualization()


void endVisualization()
{
	glutMainLoop();
} // end_visualization()

void visualize(const spatial_design::ms_building& ms, const std::string& type = "spaces", const std::string& title = "ms_building", const double& lineWidth = 1.0)
{
	vpmanager.addviewport(new viewport(new MS_Model(ms, type, title,lineWidth)));
}

void visualize(const spatial_design::ms_N_building& msN, const std::string& type = "spaces", const std::string& title = "ms_N_building", const double& lineWidth = 1.0)
{
	vpmanager.addviewport(new viewport(new MS_Model(msN, type, title,lineWidth)));
}

void visualize(const utilities::clustering& clus, const std::string& title = "ms_N_clustering", const double& linewidth = 1.0)
{
	vpmanager.addviewport(new viewport(new MS_Model(clus, title, linewidth)));
}

void visualize(const std::vector<utilities::data_point>& spaceData, const std::vector<double>& resultWeightFactor, float transparency = 1.0, const std::string& title = "ms_N_performances", const double& linewidth = 1.0)
{
	vpmanager.addviewport(new viewport(new MS_Model(spaceData, resultWeightFactor, transparency, title, linewidth)));
}

void visualize(const std::string& fileName, const spatial_design::ms_N_building& msN, const std::string& type = "spaces", const std::string& title = "ms_N_building", const double& lineWidth = 1.0)
{
	vpmanager.addviewport(new viewport(new MS_Model(fileName, msN, type, title,lineWidth)));
}

void visualize(const spatial_design::sc_building& sc_building, const std::string& title = "sc_building")
{
	vpmanager.addviewport(new viewport(new SC_Model(sc_building, title)));
}

#ifdef CF_BUILDING_HPP
void visualize(const spatial_design::cf_building& cf, std::string type, std::string title = "sc_building")
{
	vpmanager.addviewport(new viewport(new Conformal_Model(cf, type, title)));
}
#endif // CF_BUILDING_HPP

#ifdef BP_MODEL_HPP
void visualize(const bso::building_physics::bp_model& bp_model, const std::string& title = "bp_model")
{
	vpmanager.addviewport(new viewport(new BP_Model(bp_model,title)));
}
#endif // BP_MODEL_HPP

#ifdef DL_MODEL_HPP
void visualize(const bso::daylight::dl_model& dl_model, const std::string& type = "", const std::string& title = "dl_model")
{
	vpmanager.addviewport(new viewport(new DL_Model(dl_model, type, title)));
}
#endif // DL_MODEL_HPP

#ifdef SD_MODEL_HPP
void visualize(const bso::structural_design::sd_model& sd, const std::string& type ="component",
							 const std::string& title ="sd_model", const bool& ghostly = false,
							 const std::vector<std::pair<bso::utilities::geometry::vertex,
							 bso::utilities::geometry::vector>>& cuttingPlanes = {})
{
	vpmanager.addviewport(new viewport(new SD_Model(sd, type, title, ghostly, cuttingPlanes)));
}
#endif // SD_MODEL_HPP

void visualize(const std::vector<bso::utilities::geometry::quad_hexahedron> QHs,
	const std::vector<bso::utilities::geometry::triangular_prism> triPrisms, 
	const std::vector<bso::utilities::geometry::quadrilateral> quads, 
	const std::vector<bso::utilities::geometry::triangle> triangles,
	const std::vector<bso::utilities::geometry::line_segment> lines,
	const std::string& title = "geometry_model",
	const double& linewidth = 1.0)
{
	vpmanager.addviewport(new viewport(new GEOMETRY_Model(QHs,triPrisms,quads,triangles,lines,title,linewidth)));
}

void visualize(const std::vector<bso::utilities::geometry::triangular_prism> triPrisms, 
	const std::string& title = "geometry_model",
	const double& linewidth = 1.0)
{
	vpmanager.addviewport(new viewport(new GEOMETRY_Model(triPrisms,title,linewidth)));
}

void visualize(const std::string& colorFileName, const std::string& PNFileName, 
	const std::string& title = "pn_building", const double& linewidth = 1.0)
{
	vpmanager.addviewport(new viewport(new PN_BUILDING_Model(colorFileName,PNFileName,title,linewidth)));
}

void visualize(const std::string& PNFileName, const double& linewidth = 1.0, 
	const std::string& title = "pn_building")
{
	vpmanager.addviewport(new viewport(new PN_BUILDING_Model(PNFileName,linewidth,title)));
}

} // namespace visualization
} // namespace BSO

#endif // VISUALIZATION_HPP
