
#include "common.h"
#include "body.h"
#include <freeglut/GL/freeglut.h>


namespace
{
	int32 width = 800;
	int32 height = 600;
	int32 framePeriod = 5;
	float32 viewZoom = 1.0f;
	b2Vec2 viewCenter(0.0f, 0.0f);
	int32 mainWindow;
	float settingsHz = 60.0;
	Body* body;
	Settings settings;
}

b2Vec2 ConvertScreenToWorld(int32 x, int32 y)
{
	int tw = width;
	int th = height;

	float32 u = x / float32(tw);
	float32 v = (th - y) / float32(th);

	float32 ratio = float32(tw) / float32(th);
	b2Vec2 extents(ratio * 25.0f, 25.0f);

	float32 viewZoom = 1.0f;

	extents *= viewZoom;

	b2Vec2 viewCenter(0.0f, 0.0f);

	b2Vec2 lower = viewCenter - extents;
	b2Vec2 upper = viewCenter + extents;

	b2Vec2 p;
	p.x = b2Abs( (1.0f - u) * lower.x + u * upper.x );
	p.y = b2Abs( (1.0f - v) * lower.y + v * upper.y );
	return p;
}

void resize(int32 w, int32 h)
{
	width = w;
	height = h;

	glViewport(0, 0, w, h);

	settings.worldExtents = ConvertScreenToWorld(w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float32 ratio = float32(w) / float32(h);

	b2Vec2 extents(ratio * 25.0f, 25.0f);
	extents *= viewZoom;

	b2Vec2 lower = viewCenter - extents;
	b2Vec2 upper = viewCenter + extents;

	/// L/R/B/T
	gluOrtho2D(0, upper.x, 0, upper.y);
}

/// This is used to control the frame rate (60Hz).
void timer(int)
{
	glutSetWindow(mainWindow);
	glutPostRedisplay();
	glutTimerFunc(framePeriod, timer, 0);
}

void simulation_loop()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (body)
	{
		body->SetTextLine(30);
		body->Step();
		body->DrawTitle(5, 15, g_entities[0].name);
	}

	glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y)
{
	REF(x);
	REF(y);

	switch (key)
	{
	case 27:
		exit(0);
		break;

	// Press 'z' to zoom out.
	case 'z':
		viewZoom = b2Min(1.1f * viewZoom, 20.0f);
		resize(width, height);
		break;

	// Press 'x' to zoom in.
	case 'x':
		viewZoom = b2Max(0.9f * viewZoom, 0.02f);
		resize(width, height);
		break;

	default:
		if (body)
		{
			body->Keyboard(key);
		}
	}
}

void keyboard_special(int key, int x, int y)
{
	REF(x);
	REF(y);

	switch (key)
	{
	case 0:
	default:
		if (body)
		{
			body->Keyboard(key);
		}
	}
}

void mouse(int32 button, int32 state, int32 x, int32 y)
{
	REF(button);
	REF(state);
	REF(x);
	REF(y);
}

void mouse_motion(int32 x, int32 y)
{
	REF(x);
	REF(y);
}

void mouse_wheel(int wheel, int direction, int x, int y)
{
	REF(wheel);
	REF(direction);
	REF(x);
	REF(y);
}

void glexit()
{
	if (body)
	{
		delete body;
		body = NULL;
	}
	glutLeaveMainLoop();
	glutDestroyWindow(mainWindow);
	glutExit();
}

int glmain(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(width, height);

	char title[48];
	sprintf(title, "network-game (Box2D Version %d.%d.%d)", 
		b2_version.major, b2_version.minor, b2_version.revision);
	mainWindow = glutCreateWindow(title);

	glutDisplayFunc(simulation_loop);
	glutReshapeFunc(resize);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(keyboard_special);
	glutMouseFunc(mouse);
#ifdef FREEGLUT
	glutMouseWheelFunc(mouse_wheel);
#endif
	glutMotionFunc(mouse_motion);

	/// Use a timer to control the frame rate.
	glutTimerFunc(framePeriod, timer, 0);

	settings.hz = settingsHz;
	settings.worldExtents = ConvertScreenToWorld(width, height);
	body = g_entities[0].createFcn(&settings);

	atexit(glexit);
	glutMainLoop();

	return 0;
}
