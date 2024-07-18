#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#endif

#include "glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "glut.h"


//	This is a sample OpenGL / GLUT program
//
//	The objective is to draw a 3d object and change the color of the axes
//		with a glut menu
//
//	The left mouse button does rotation
//	The middle mouse button does scaling
//	The user interface allows:
//		1. The axes to be turned on and off
//		2. The color of the axes to be changed
//		3. Debugging to be turned on and off
//		4. Depth cueing to be turned on and off
//		5. The projection to be changed
//		6. The transformations to be reset
//		7. The program to quit
//
//	Author:			Joe Graphics

// title of these windows:

const char *WINDOWTITLE = { "OpenGL / GLUT Sample -- Project 6  Kazu Ishihara" };
const char *GLUITITLE   = { "User Interface Window" };

// what the glui package defines as true and false:

const int GLUITRUE  = { true  };
const int GLUIFALSE = { false };

// the escape key:

const int ESCAPE = { 0x1b };

// initial window size:

const int INIT_WINDOW_SIZE = { 600 };

// size of the 3d box:

const float BOXSIZE = { 2.f };

// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = { 1. };
const float SCLFACT = { 0.005f };

// minimum allowable scale factor:

const float MINSCALE = { 0.05f };

// scroll wheel button values:

const int SCROLL_WHEEL_UP   = { 3 };
const int SCROLL_WHEEL_DOWN = { 4 };

// equivalent mouse movement when we click a the scroll wheel:

const float SCROLL_WHEEL_CLICK_FACTOR = { 5. };

// active mouse buttons (or them together):

const int LEFT   = { 4 };
const int MIDDLE = { 2 };
const int RIGHT  = { 1 };

// which projection:

enum Projections
{
	ORTHO,
	PERSP
};

// which button:

enum ButtonVals
{
	RESET,
	QUIT
};

// window background color (rgba):

const GLfloat BACKCOLOR[ ] = { 0., 0., 0., 1. };

// line width for the axes:

const GLfloat AXES_WIDTH   = { 3. };

// the color numbers:
// this order must match the radio button order

enum Colors
{
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA,
	WHITE,
	BLACK
};

char * ColorNames[ ] =
{
	(char *)"Red",
	(char*)"Yellow",
	(char*)"Green",
	(char*)"Cyan",
	(char*)"Blue",
	(char*)"Magenta",
	(char*)"White",
	(char*)"Black"
};

// the color definitions:
// this order must match the menu order

const GLfloat Colors[ ][3] = 
{
	{ 1., 0., 0. },		// red
	{ 1., 1., 0. },		// yellow
	{ 0., 1., 0. },		// green
	{ 0., 1., 1. },		// cyan
	{ 0., 0., 1. },		// blue
	{ 1., 0., 1. },		// magenta
	{ 1., 1., 1. },		// white
	{ 0., 0., 0. },		// black
};

// fog parameters:

const GLfloat FOGCOLOR[4] = { .0, .0, .0, 1. };
const GLenum  FOGMODE     = { GL_LINEAR };
const GLfloat FOGDENSITY  = { 0.30f };
const GLfloat FOGSTART    = { 1.5 };
const GLfloat FOGEND      = { 4. };


// what options should we compile-in?
// in general, you don't need to worry about these
// i compile these in to show class examples of things going wrong
//#define DEMO_Z_FIGHTING
//#define DEMO_DEPTH_BUFFER

// non-constant global variables:

int		ActiveButton;			// current button that is down
GLuint	AxesList;				// list to hold the axes
int		AxesOn;					// != 0 means to draw the axes
GLuint	BoxList;				// object display list
int		DebugOn;				// != 0 means to print debugging info
int		DepthCueOn;				// != 0 means to use intensity depth cueing
int		DepthBufferOn;			// != 0 means to use the z-buffer
int		DepthFightingOn;		// != 0 means to force the creation of z-fighting
int		MainWindow;				// window id for main graphics window
float	Scale;					// scaling factor
float	Time;					// timer in the range [0.,1.)
int		WhichColor;				// index into Colors[ ]
int		WhichProjection;		// ORTHO or PERSP
int		Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;				// rotation angles in degrees

int     controlPoints = 1;          // Bezier curve control point  0 = off 1 = on
int     controlLines = 1;           // Bezier curve control line   0 = off 1 = on

bool Frozen;

// function prototypes:

void	Animate( );
void	Display( );
void	DoAxesMenu( int );
void	DoColorMenu( int );
void	DoDepthBufferMenu( int );
void	DoDepthFightingMenu( int );
void	DoDepthMenu( int );
void	DoDebugMenu( int );
void	DoMainMenu( int );
void	DoProjectMenu( int );
void	DoShadowMenu();
void    DoControlPointMenu(int);
void    DoControlLineMenu(int);
void	DoRasterString( float, float, float, char * );
void	DoStrokeString( float, float, float, float, char * );
float	ElapsedSeconds( );
void	InitGraphics( );
void	InitLists( );
void	InitMenus( );
void	Keyboard( unsigned char, int, int );
void	MouseButton( int, int, int, int );
void	MouseMotion( int, int );
void	Reset( );
void	Resize( int, int );
void	Visibility( int );

void			Axes( float );
unsigned char *	BmpToTexture( char *, int *, int * );
void			HsvRgb( float[3], float [3] );
int				ReadInt( FILE * );
short			ReadShort( FILE * );

void			Cross(float[3], float[3], float[3]);
float			Dot(float [3], float [3]);
float			Unit(float [3], float [3]);
                                                      //Rotating a Point an Angle about the Z Axis Around a Center


struct Point
{
	float x0, y0, z0;       // initial coordinates
	float x, y, z;        // animated coordinates
};

struct Curve
{
	float r, g, b;
	Point p0, p1, p2, p3;
};

void
RotateX(Point* p, float deg, float xc, float yc, float zc);

int NUMCURVES;
int NUMPOINTS;
//Curve Curves[NUMCURVES];		// if you are creating a pattern of curves
Curve Stem;				// if you are not


// main program:

int
main( int argc, char *argv[ ] )
{
	// turn on the glut package:
	// (do this before checking argc and argv since it might
	// pull some command line arguments out)

	glutInit( &argc, argv );

	// setup all the graphics stuff:

	InitGraphics( );

	// init all the global variables used by Display( ):

	Reset( );

	// create the display structures that will not change:

	InitLists( );

	// setup all the user interface stuff:

	InitMenus( );

	// draw the scene once and wait for some interaction:
	// (this will never return)

	glutSetWindow( MainWindow );
	glutMainLoop( );

	// glutMainLoop( ) never returns
	// this line is here to make the compiler happy:

	return 0;
}


// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutMainLoop( ) do it

void
Animate( )
{
	// put animation stuff in here -- change some global variables
	// for Display( ) to find:

	const int MS_IN_THE_ANIMATION_CYCLE = 10000;	// milliseconds in the animation loop
	int ms = glutGet(GLUT_ELAPSED_TIME);			// milliseconds since the program started
	ms %= MS_IN_THE_ANIMATION_CYCLE;				// milliseconds in the range 0 to MS_IN_THE_ANIMATION_CYCLE-1
	Time = (float)ms / (float)MS_IN_THE_ANIMATION_CYCLE;        // [ 0., 1. )

	// force a call to Display( ) next time it is convenient:

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// draw the complete scene:

void
Display( )
{
	if( DebugOn != 0 )
	{
		fprintf( stderr, "Display\n" );
	}

	// set which window we want to do the graphics into:

	glutSetWindow( MainWindow );

	// erase the background:

	glDrawBuffer( GL_BACK );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glEnable( GL_DEPTH_TEST );
#ifdef DEMO_DEPTH_BUFFER
	if( DepthBufferOn == 0 )
		glDisable( GL_DEPTH_TEST );
#endif

	// specify shading to be flat:

	glShadeModel( GL_FLAT );

	// set the viewport to a square centered in the window:

	GLsizei vx = glutGet( GLUT_WINDOW_WIDTH );
	GLsizei vy = glutGet( GLUT_WINDOW_HEIGHT );
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = ( vx - v ) / 2;
	GLint yb = ( vy - v ) / 2;
	glViewport( xl, yb,  v, v );

	// set the viewing volume:
	// remember that the Z clipping  values are actually
	// given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	//if( WhichProjection == ORTHO )
		glOrtho( -3., 3.,     -3., 3.,     0.1, 1000. );
//	else
		//gluPerspective( 90., 1.,	0.1, 1000. );

	// place the objects into the scene:

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );

	// set the eye position, look-at position, and up-vector:

	gluLookAt( 0., 0., 3.,     0., 0., 0.,     0., 1., 0. );

	// rotate the scene:

	glRotatef( (GLfloat)Yrot, 0., 1., 0. );
	glRotatef( (GLfloat)Xrot, 1., 0., 0. );

	// uniformly scale the scene:

	if( Scale < MINSCALE )
		Scale = MINSCALE;
	glScalef( (GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale );

	// set the fog parameters:
	// (this is really here to do intensity depth cueing)

	if( DepthCueOn != 0 )
	{
		glFogi( GL_FOG_MODE, FOGMODE );
		glFogfv( GL_FOG_COLOR, FOGCOLOR );
		glFogf( GL_FOG_DENSITY, FOGDENSITY );
		glFogf( GL_FOG_START, FOGSTART );
		glFogf( GL_FOG_END, FOGEND );
		glEnable( GL_FOG );
	}
	else
	{
		glDisable( GL_FOG );
	}

	// possibly draw the axes:

	if( AxesOn != 0 )
	{
		glColor3fv( &Colors[WhichColor][0] );
		glCallList( AxesList );
	}

	// since we are using glScalef( ), be sure normals get unitized:

	glEnable( GL_NORMALIZE );

	// draw the current object:

	//glCallList( BoxList );

	/*glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_LINE_STRIP);
	for (int it = 0; it <= 3; it++)
	{
		glVertex3d(0.1, 0.1, 0.0);
		glVertex3d(0.2, 0.2, 0.0);
		glVertex3d(0.3, 0.3, 0.0);
		glVertex3d(0.4, 0.4, 0.0);
}
	glEnd();*/

	/*struct Point
	{
		float x0, y0, z0;       // initial coordinates
		float x, y, z;        // animated coordinates
	};

	struct Curve
	{
		float r, g, b;
		Point p0, p1, p2, p3;
	};*/

	struct Curve oneC {};      //the first curve (upper right)
	
	oneC.r = 0.0;
	oneC.g = 0.8;
	oneC.b = 0.0;
	oneC.p0.x = 0.2;
	oneC.p0.y = 0.2;
	oneC.p0.z = 0.0;
	oneC.p1.x = 1.0 * sin(Time);
	oneC.p1.y = 1.3 * cos(Time);
	oneC.p1.z = 0.0 - sin(Time)*3;
	oneC.p2.x = 2.4;
	oneC.p2.y = 1.8;
	oneC.p2.z = 1.0;
	oneC.p3.x = 0.0;
	oneC.p3.y = 0.0;
	oneC.p3.z = 0.2;

	struct Point p;
	struct Point* oneB = &p;
	//p = &oneC.Point;
	oneB->x0 = oneC.p2.x;
	oneB->y0 = oneC.p2.y;
	oneB->z0 = oneC.p2.z;
	oneB->x;
	oneB->y;
	oneB->z;

	NUMPOINTS = 20;
	glLineWidth(4.);
	glColor3f(oneC.r, oneC.g, oneC.b);
	glBegin(GL_LINE_STRIP);
	for (int it = 0; it <= NUMPOINTS; it++)
	{
		float t = (float)it / (float)NUMPOINTS;
		float omt = 1.f - t;
		float x = omt * omt * omt * oneC.p0.x + 3.f * t * omt * omt * oneC.p1.x + 3.f * t * t * omt * oneC.p2.x + t * t * t * oneC.p3.x;
		float y = omt * omt * omt * oneC.p0.y + 3.f * t * omt * omt * oneC.p1.y + 3.f * t * t * omt * oneC.p2.y + t * t * t * oneC.p3.y;
		float z = omt * omt * omt * oneC.p0.z + 3.f * t * omt * omt * oneC.p1.z + 3.f * t * t * omt * oneC.p2.z + t * t * t * oneC.p3.z;
		glVertex3f(x, y, z);
		
	}
	
	glEnd();
	

	struct Curve secondC {}; //the second curb  (upper left)

	secondC.r = 0.0;
	secondC.g = 0.8;
	secondC.b = 0.0;
	secondC.p0.x = 0.0;
	secondC.p0.y = 0.2;
	secondC.p0.z = 0.0;
	secondC.p1.x = -0.2 * sin(Time);
	secondC.p1.y = 1.3 * cos(Time);
	secondC.p1.z = 0.0 - sin(Time)*3;
	secondC.p2.x = -1.4;
	secondC.p2.y = 1.8;
	secondC.p2.z = 1.0;
	secondC.p3.x = 0.0;
	secondC.p3.y = 0.0;
	secondC.p3.z = 0.2;

	int NUMPOINTS1 = 20;
	glLineWidth(4.);
	glColor3f(secondC.r, secondC.g, secondC.b);
	glBegin(GL_LINE_STRIP);
	for (int it = 0; it <= NUMPOINTS1; it++)
	{
		float t1 = (float)it / (float)NUMPOINTS1;
		float omt = 1.f - t1;
		float x = omt * omt * omt * secondC.p0.x + 3.f * t1 * omt * omt * secondC.p1.x + 3.f * t1 * t1 * omt * secondC.p2.x + t1 * t1 * t1 * secondC.p3.x;
		float y = omt * omt * omt * secondC.p0.y + 3.f * t1 * omt * omt * secondC.p1.y + 3.f * t1 * t1 * omt * secondC.p2.y + t1 * t1 * t1 * secondC.p3.y;
		float z = omt * omt * omt * secondC.p0.z + 3.f * t1 * omt * omt * secondC.p1.z + 3.f * t1 * t1 * omt * secondC.p2.z + t1 * t1 * t1 * secondC.p3.z;
		glVertex3f(x, y, z);

	}

	glEnd();
	

	struct Curve thirdC {}; //the third curb (lower left)

	thirdC.r = 0.0;
	thirdC.g = 0.8;
	thirdC.b = 0.0;
	thirdC.p0.x = 0.0;
	thirdC.p0.y = -0.2;
	thirdC.p0.z = 0.0;
	thirdC.p1.x = -0.2 * sin(Time);
	thirdC.p1.y = -1.3 * cos(Time);
	thirdC.p1.z = 0.0 + sin(Time)*2.8;
	thirdC.p2.x = -1.4 * sin(Time);
	thirdC.p2.y = -1.8;
	thirdC.p2.z = 1.0;
	thirdC.p3.x = 0.0;
	thirdC.p3.y = 0.0;
	thirdC.p3.z = 0.2;

	int NUMPOINTS3 = 20;
	glLineWidth(4.);
	glColor3f(thirdC.r, thirdC.g, thirdC.b);
	glBegin(GL_LINE_STRIP);
	for (int it = 0; it <= NUMPOINTS3; it++)
	{
		float t3 = (float)it / (float)NUMPOINTS3;
		float omt = 1.f - t3;
		float x = omt * omt * omt * thirdC.p0.x + 3.f * t3 * omt * omt * thirdC.p1.x + 3.f * t3 * t3 * omt * thirdC.p2.x + t3 * t3 * t3 * secondC.p3.x;
		float y = omt * omt * omt * thirdC.p0.y + 3.f * t3 * omt * omt * thirdC.p1.y + 3.f * t3 * t3 * omt * thirdC.p2.y + t3 * t3 * t3 * secondC.p3.y;
		float z = omt * omt * omt * thirdC.p0.z + 3.f * t3 * omt * omt * thirdC.p1.z + 3.f * t3 * t3 * omt * thirdC.p2.z + t3 * t3 * t3 * secondC.p3.z;
		glVertex3f(x, y, z);

	}

	glEnd();
	

	struct Curve fourthC {}; //the fourth curb (lower right)

	fourthC.r = 0.0;
	fourthC.g = 0.8;
	fourthC.b = 0.0;
	fourthC.p0.x = 0.0;
	fourthC.p0.y = -0.2;
	fourthC.p0.z = 0.0;
	fourthC.p1.x = 1.2 * sin(Time);
	fourthC.p1.y = -1.3 * cos(Time);
    fourthC.p1.z = 0.0 + sin(Time) * 2.8;
	fourthC.p2.x = 2.4 * sin(Time);
    fourthC.p2.y = -1.8;
	fourthC.p2.z = 1.0;
	fourthC.p3.x = 0.0;
	fourthC.p3.y = 0.0;
	fourthC.p3.z = 0.2;

	int NUMPOINTS4 = 20;
	glLineWidth(4.);
	glColor3f(fourthC.r, fourthC.g, fourthC.b);
	glBegin(GL_LINE_STRIP);
	for (int it = 0; it <= NUMPOINTS4; it++)
	{
		float t4 = (float)it / (float)NUMPOINTS4;
		float omt = 1.f - t4;
		float x = omt * omt * omt * fourthC.p0.x + 3.f * t4 * omt * omt * fourthC.p1.x + 3.f * t4 * t4 * omt * fourthC.p2.x + t4 * t4 * t4 * fourthC.p3.x;
		float y = omt * omt * omt * fourthC.p0.y + 3.f * t4 * omt * omt * fourthC.p1.y + 3.f * t4 * t4 * omt * fourthC.p2.y + t4 * t4 * t4 * fourthC.p3.y;
		float z = omt * omt * omt * fourthC.p0.z + 3.f * t4 * omt * omt * fourthC.p1.z + 3.f * t4 * t4 * omt * fourthC.p2.z + t4 * t4 * t4 * fourthC.p3.z;
		glVertex3f(x, y, z);

	}

	glEnd();
	

	struct Curve stemC {}; //the stem curb (lower center)

	stemC.r = 0.0;
	stemC.g = 0.8;
	stemC.b = 0.0;
	stemC.p0.x = 0.0;
	stemC.p0.y = 0.0;
	stemC.p0.z = 0.0;
	stemC.p1.x = 0.0;
	stemC.p1.y = -1.3 * sin(Time);
	stemC.p1.z = 0.8 + cos(Time);
	stemC.p2.x = 0.0 * sin(Time);
	stemC.p2.y = -2.0 + cos(Time);
	stemC.p2.z = 0.8 + sin(Time)*2;
	stemC.p3.x = 0.0;
	stemC.p3.y = -2.2;
	stemC.p3.z = 1.3 + Time;
	
	int NUMPOINTS_T = 20;
	glLineWidth(4.);
	glColor3f(stemC.r, stemC.g, stemC.b);
	glBegin(GL_LINE_STRIP);
	for (int it = 0; it <= NUMPOINTS_T; it++)
	{
		float tS = (float)it / (float)NUMPOINTS_T;
		float omt = 1.f - tS;
		float x = omt * omt * omt * stemC.p0.x + 3.f * tS * omt * omt * stemC.p1.x + 3.f * tS * tS * omt * stemC.p2.x + tS * tS * tS * stemC.p3.x;
		float y = omt * omt * omt * stemC.p0.y + 3.f * tS * omt * omt * stemC.p1.y + 3.f * tS * tS * omt * stemC.p2.y + tS * tS * tS * stemC.p3.y;
		float z = omt * omt * omt * stemC.p0.z + 3.f * tS * omt * omt * stemC.p1.z + 3.f * tS * tS * omt * stemC.p2.z + tS * tS * tS * stemC.p3.z;
		glVertex3f(x, y, z);

	}

	glEnd();
	

	struct Curve heart1C {}; //the heart 1 curb (upper right)

	heart1C.r = 0.0;
	heart1C.g = 0.8;
	heart1C.b = 0.0;
	heart1C.p0.x = 1.8;
	heart1C.p0.y = 1.0;
	heart1C.p0.z = 0.0;
	heart1C.p1.x = 1.5;
	heart1C.p1.y = 1.3;
	heart1C.p1.z = 0.2;
	heart1C.p2.x = 1.3;
	heart1C.p2.y = 0.8;
	heart1C.p2.z = 0.3;
	heart1C.p3.x = 1.8;
	heart1C.p3.y = 0.5;
	heart1C.p3.z = 0.4;

	int NUMPOINTS_H1 = 20;
	glLineWidth(4.);
	glColor3f(heart1C.r, heart1C.g, heart1C.b);
	glBegin(GL_LINE_STRIP);
	for (int it = 0; it <= NUMPOINTS_H1; it++)
	{
		float tH1 = (float)it / (float)NUMPOINTS_H1;
		float omt = 1.f - tH1;
		float x = omt * omt * omt * heart1C.p0.x + 3.f * tH1 * omt * omt * heart1C.p1.x + 3.f * tH1 * tH1 * omt * heart1C.p2.x + tH1 * tH1 * tH1 * heart1C.p3.x;
		float y = omt * omt * omt * heart1C.p0.y + 3.f * tH1 * omt * omt * heart1C.p1.y + 3.f * tH1 * tH1 * omt * heart1C.p2.y + tH1 * tH1 * tH1 * heart1C.p3.y;
		float z = omt * omt * omt * heart1C.p0.z + 3.f * tH1 * omt * omt * heart1C.p1.z + 3.f * tH1 * tH1 * omt * heart1C.p2.z + tH1 * tH1 * tH1 * heart1C.p3.z;
		glVertex3f(x, y, z);

	}

	glEnd();
	

	struct Curve heart2C {}; //the heart 2 curb (upper right)

	heart2C.r = 0.0;
	heart2C.g = 0.8;
	heart2C.b = 0.0;
	heart2C.p0.x = 1.8;
	heart2C.p0.y = 1.0;
	heart2C.p0.z = 0.0;
	heart2C.p1.x = 2.1;
	heart2C.p1.y = 1.3;
	heart2C.p1.z = 0.3;
	heart2C.p2.x = 2.3;
	heart2C.p2.y = 0.8;
	heart2C.p2.z = 0.4;
	heart2C.p3.x = 1.5;
	heart2C.p3.y = 0.5;
	heart2C.p3.z = 0.8;

	int NUMPOINTS_H2 = 20;
	glLineWidth(4.);
	glColor3f(heart2C.r, heart2C.g, heart2C.b);
	glBegin(GL_LINE_STRIP);
	for (int it = 0; it <= NUMPOINTS_H2; it++)
	{
		float tH2 = (float)it / (float)NUMPOINTS_H2;
		float omt = 1.f - tH2;
		float x = omt * omt * omt * heart2C.p0.x + 3.f * tH2 * omt * omt * heart2C.p1.x + 3.f * tH2 * tH2 * omt * heart2C.p2.x + tH2 * tH2 * tH2 * heart2C.p3.x;
		float y = omt * omt * omt * heart2C.p0.y + 3.f * tH2 * omt * omt * heart2C.p1.y + 3.f * tH2 * tH2 * omt * heart2C.p2.y + tH2 * tH2 * tH2 * heart2C.p3.y;
		float z = omt * omt * omt * heart2C.p0.z + 3.f * tH2 * omt * omt * heart2C.p1.z + 3.f * tH2 * tH2 * omt * heart2C.p2.z + tH2 * tH2 * tH2 * heart2C.p3.z;
		glVertex3f(x, y, z);

	}

	glEnd();
	//glLineWidth(2.0);


	if (controlPoints == 1)
	{
		
		glPointSize(10);                //control points for curb 1 (the rightmost)
		glColor3f(0.3, 1.0, 0.8);
		glBegin(GL_POINTS);
		glVertex3f(oneC.p0.x, oneC.p0.y, oneC.p0.z);
		glVertex3f(oneC.p1.x, oneC.p1.y, oneC.p1.z);
		glVertex3f(oneC.p2.x, oneC.p2.y, oneC.p2.z);
		glVertex3f(oneC.p3.x, oneC.p3.y, oneC.p3.z);

		glEnd();


		glPointSize(10);                //control points for curb 2 (the leftmost)
		glColor3f(0.3, 1.0, 0.8);
		glBegin(GL_POINTS);
		glVertex3f(secondC.p0.x, secondC.p0.y, secondC.p0.z);
		glVertex3f(secondC.p1.x, secondC.p1.y, secondC.p1.z);
		glVertex3f(secondC.p2.x, secondC.p2.y, secondC.p2.z);
		glVertex3f(secondC.p3.x, secondC.p3.y, secondC.p3.z);

		glEnd();

		glPointSize(10);                //control points for curb 3 (the lowerleft)
		glColor3f(0.3, 1.0, 0.8);
		glBegin(GL_POINTS);
		glVertex3f(thirdC.p0.x, thirdC.p0.y, thirdC.p0.z);
		glVertex3f(thirdC.p1.x, thirdC.p1.y, thirdC.p1.z);
		glVertex3f(thirdC.p2.x, thirdC.p2.y, thirdC.p2.z);
		glVertex3f(thirdC.p3.x, thirdC.p3.y, thirdC.p3.z);

		glEnd();

		glPointSize(10);                //control points for curb 4 (the lower right)
		glColor3f(0.3, 1.0, 0.8);
		glBegin(GL_POINTS);
		glVertex3f(fourthC.p0.x, fourthC.p0.y, fourthC.p0.z);
		glVertex3f(fourthC.p1.x, fourthC.p1.y, fourthC.p1.z);
		glVertex3f(fourthC.p2.x, fourthC.p2.y, fourthC.p2.z);
		glVertex3f(fourthC.p3.x, fourthC.p3.y, fourthC.p3.z);

		glEnd();

		glPointSize(10);                //control points for stem (lower center)
		glColor3f(0.3, 1.0, 0.8);
		glBegin(GL_POINTS);
		glVertex3f(stemC.p0.x, stemC.p0.y, stemC.p0.z);
		glVertex3f(stemC.p1.x, stemC.p1.y, stemC.p1.z);
		glVertex3f(stemC.p2.x, stemC.p2.y, stemC.p2.z);
		glVertex3f(stemC.p3.x, stemC.p3.y, stemC.p3.z);

		glEnd();

		glPointSize(10);                //control points for heart curb1 (upper right)
		glColor3f(0.3, 1.0, 0.8);
		glBegin(GL_POINTS);
		glVertex3f(heart1C.p0.x, heart1C.p0.y, heart1C.p0.z);
		glVertex3f(heart1C.p1.x, heart1C.p1.y, heart1C.p1.z);
		glVertex3f(heart1C.p2.x, heart1C.p2.y, heart1C.p2.z);
		glVertex3f(heart1C.p3.x, heart1C.p3.y, heart1C.p3.z);

		glEnd();

		glPointSize(10);                //control points for heart curb2 (upper right)
		glColor3f(0.3, 1.0, 0.8);
		glBegin(GL_POINTS);
		glVertex3f(heart2C.p0.x, heart2C.p0.y, heart2C.p0.z);
		glVertex3f(heart2C.p1.x, heart2C.p1.y, heart2C.p1.z);
		glVertex3f(heart2C.p2.x, heart2C.p2.y, heart2C.p2.z);
		glVertex3f(heart2C.p3.x, heart2C.p3.y, heart2C.p3.z);

		glEnd();

	}

	if (controlLines == 1)
	{

		glLineWidth(2);                 //control line for curb 1
		glColor3f(0.3, 1.0, 0.8);
		glBegin(GL_LINE_STRIP);
		glVertex3f(oneC.p0.x, oneC.p0.y, oneC.p0.z);
		glVertex3f(oneC.p1.x, oneC.p1.y, oneC.p1.z);
		glVertex3f(oneC.p2.x, oneC.p2.y, oneC.p2.z);
		glVertex3f(oneC.p3.x, oneC.p3.y, oneC.p3.z);

		glEnd();


		glLineWidth(2);
		glColor3f(0.3, 1.0, 0.8);
		glBegin(GL_LINE_STRIP);
		glVertex3f(secondC.p0.x, secondC.p0.y, secondC.p0.z);
		glVertex3f(secondC.p1.x, secondC.p1.y, secondC.p1.z);
		glVertex3f(secondC.p2.x, secondC.p2.y, secondC.p2.z);
		glVertex3f(secondC.p3.x, secondC.p3.y, secondC.p3.z);

		glEnd();
		//glEnable(GL_DEPTH_TEST);



		glLineWidth(2);                //control line for curb 3
		glColor3f(0.3, 1.0, 0.8);
		glBegin(GL_LINE_STRIP);
		glVertex3f(thirdC.p0.x, thirdC.p0.y, thirdC.p0.z);
		glVertex3f(thirdC.p1.x, thirdC.p1.y, thirdC.p1.z);
		glVertex3f(thirdC.p2.x, thirdC.p2.y, thirdC.p2.z);
		glVertex3f(thirdC.p3.x, thirdC.p3.y, thirdC.p3.z);

		glEnd();



		glLineWidth(2);                //control line for curb 4
		glColor3f(0.3, 1.0, 0.8);
		glBegin(GL_LINE_STRIP);
		glVertex3f(fourthC.p0.x, fourthC.p0.y, fourthC.p0.z);
		glVertex3f(fourthC.p1.x, fourthC.p1.y, fourthC.p1.z);
		glVertex3f(fourthC.p2.x, fourthC.p2.y, fourthC.p2.z);
		glVertex3f(fourthC.p3.x, fourthC.p3.y, fourthC.p3.z);

		glEnd();



		glLineWidth(2);                //control line for curb 4
		glColor3f(0.3, 1.0, 0.8);
		glBegin(GL_LINE_STRIP);
		glVertex3f(stemC.p0.x, stemC.p0.y, stemC.p0.z);
		glVertex3f(stemC.p1.x, stemC.p1.y, stemC.p1.z);
		glVertex3f(stemC.p2.x, stemC.p2.y, stemC.p2.z);
		glVertex3f(stemC.p3.x, stemC.p3.y, stemC.p3.z);

		glEnd();



		glLineWidth(2);                //control line for heart curb1
		glColor3f(0.3, 1.0, 0.8);
		glBegin(GL_LINE_STRIP);
		glVertex3f(heart1C.p0.x, heart1C.p0.y, heart1C.p0.z);
		glVertex3f(heart1C.p1.x, heart1C.p1.y, heart1C.p1.z);
		glVertex3f(heart1C.p2.x, heart1C.p2.y, heart1C.p2.z);
		glVertex3f(heart1C.p3.x, heart1C.p3.y, heart1C.p3.z);

		glEnd();



		glLineWidth(2);                //control line for heart curb1
		glColor3f(0.3, 1.0, 0.8);
		glBegin(GL_LINE_STRIP);
		glVertex3f(heart2C.p0.x, heart2C.p0.y, heart2C.p0.z);
		glVertex3f(heart2C.p1.x, heart2C.p1.y, heart2C.p1.z);
		glVertex3f(heart2C.p2.x, heart2C.p2.y, heart2C.p2.z);
		glVertex3f(heart2C.p3.x, heart2C.p3.y, heart2C.p3.z);

		glEnd();

	}
	
	

	//Rotating a Point an Angle about the Z Axis Around a Center
	

#ifdef DEMO_Z_FIGHTING
	if( DepthFightingOn != 0 )
	{
		glPushMatrix( );
			glRotatef( 90.,   0., 1., 0. );
			glCallList( BoxList );
		glPopMatrix( );
	}
#endif

	// draw some gratuitous text that just rotates on top of the scene:

	//glDisable( GL_DEPTH_TEST );
	//glColor3f( 0, 1., 1. );
	//DoRasterString( 0., 1., 0., (char *)"Text That Moves" );

	// draw some gratuitous text that is fixed on the screen:
	//
	// the projection matrix is reset to define a scene whose
	// world coordinate system goes from 0-100 in each axis
	//
	// this is called "percent units", and is just a convenience
	//
	// the modelview matrix is reset to identity as we don't
	// want to transform these coordinates

	/* glDisable( GL_DEPTH_TEST );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	gluOrtho2D( 0., 100.,     0., 100. );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );
	glColor3f( 1., 1., 1. );
	DoRasterString( 5., 5., 0., (char *)"Text That Doesn't" );*/

	// swap the double-buffered framebuffers:
	

	




	glutSwapBuffers( );

	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush( );
}


void
DoAxesMenu( int id )
{
	AxesOn = id;
	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoColorMenu( int id )
{
	WhichColor = id - RED;
	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}

void
DoControlPointMenu(int id)
{
	controlPoints = id;
	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void
DoControlLineMenu(int id)
{
	controlLines = id;
	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


void
DoDebugMenu( int id )
{
	DebugOn = id;
	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthBufferMenu( int id )
{
	DepthBufferOn = id;
	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthFightingMenu( int id )
{
	DepthFightingOn = id;
	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthMenu( int id )
{
	DepthCueOn = id;
	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// main menu callback:

void
DoMainMenu( int id )
{
	switch( id )
	{
		case RESET:
			Reset( );
			break;

		case QUIT:
			// gracefully close out the graphics:
			// gracefully close the graphics window:
			// gracefully exit the program:
			glutSetWindow( MainWindow );
			glFinish( );
			glutDestroyWindow( MainWindow );
			exit( 0 );
			break;

		default:
			fprintf( stderr, "Don't know what to do with Main Menu ID %d\n", id );
	}

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoProjectMenu( int id )
{
	WhichProjection = id;
	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// use glut to display a string of characters using a raster font:

void
DoRasterString( float x, float y, float z, char *s )
{
	glRasterPos3f( (GLfloat)x, (GLfloat)y, (GLfloat)z );
	char c;			// one character to print
	for( ; ( c = *s ) != '\0'; s++ )
	{
		glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, c );
	}
}


// use glut to display a string of characters using a stroke font:

void
DoStrokeString( float x, float y, float z, float ht, char *s )
{
	glPushMatrix( );
		glTranslatef( (GLfloat)x, (GLfloat)y, (GLfloat)z );
		float sf = ht / ( 119.05f + 33.33f );
		glScalef( (GLfloat)sf, (GLfloat)sf, (GLfloat)sf );
		char c;			// one character to print
		for( ; ( c = *s ) != '\0'; s++ )
		{
			glutStrokeCharacter( GLUT_STROKE_ROMAN, c );
		}
	glPopMatrix( );
}


// return the number of seconds since the start of the program:

float
ElapsedSeconds( )
{
	// get # of milliseconds since the start of the program:
	int ms = glutGet( GLUT_ELAPSED_TIME );

	// convert it to seconds:
	return (float)ms / 1000.f;
}


// initialize the glui window:

void
InitMenus( )
{
	glutSetWindow( MainWindow );

	int numColors = sizeof( Colors ) / ( 3*sizeof(int) );
	int colormenu = glutCreateMenu( DoColorMenu );
	for( int i = 0; i < numColors; i++ )
	{
		glutAddMenuEntry( ColorNames[i], i );
	}

	int axesmenu = glutCreateMenu( DoAxesMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthcuemenu = glutCreateMenu( DoDepthMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthbuffermenu = glutCreateMenu( DoDepthBufferMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthfightingmenu = glutCreateMenu( DoDepthFightingMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int debugmenu = glutCreateMenu( DoDebugMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int controlpointmenu = glutCreateMenu(DoControlPointMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int controllinemenu = glutCreateMenu(DoControlLineMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int projmenu = glutCreateMenu( DoProjectMenu );
	glutAddMenuEntry( "Orthographic",  ORTHO );
	glutAddMenuEntry( "Perspective",   PERSP );

	int mainmenu = glutCreateMenu( DoMainMenu );
	glutAddSubMenu(   "Axes",          axesmenu);
	glutAddSubMenu(   "Colors",        colormenu);

#ifdef DEMO_DEPTH_BUFFER
	glutAddSubMenu(   "Depth Buffer",  depthbuffermenu);
#endif

#ifdef DEMO_Z_FIGHTING
	glutAddSubMenu(   "Depth Fighting",depthfightingmenu);
#endif

	glutAddSubMenu(   "Depth Cue",     depthcuemenu);
	glutAddSubMenu(   "Projection",    projmenu );
	glutAddMenuEntry( "Reset",         RESET );
	glutAddSubMenu(   "Debug",         debugmenu);
	glutAddMenuEntry( "Quit",          QUIT );
	glutAddSubMenu("Control Points", controlpointmenu);
	glutAddSubMenu("Control Lines", controllinemenu);

// attach the pop-up menu to the right mouse button:

	glutAttachMenu( GLUT_RIGHT_BUTTON );
}



// initialize the glut and OpenGL libraries:
//	also setup display lists and callback functions

void
InitGraphics( )
{
	// request the display modes:
	// ask for red-green-blue-alpha color, double-buffering, and z-buffering:

	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );

	// set the initial window configuration:

	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( INIT_WINDOW_SIZE, INIT_WINDOW_SIZE );

	// open the window and set its title:

	MainWindow = glutCreateWindow( WINDOWTITLE );
	glutSetWindowTitle( WINDOWTITLE );

	// set the framebuffer clear values:

	glClearColor( BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3] );

	// setup the callback functions:
	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow( MainWindow );
	glutDisplayFunc( Display );
	glutReshapeFunc( Resize );
	glutKeyboardFunc( Keyboard );
	glutMouseFunc( MouseButton );
	glutMotionFunc( MouseMotion );
	glutPassiveMotionFunc(MouseMotion);
	//glutPassiveMotionFunc( NULL );
	glutVisibilityFunc( Visibility );
	glutEntryFunc( NULL );
	glutSpecialFunc( NULL );
	glutSpaceballMotionFunc( NULL );
	glutSpaceballRotateFunc( NULL );
	glutSpaceballButtonFunc( NULL );
	glutButtonBoxFunc( NULL );
	glutDialsFunc( NULL );
	glutTabletMotionFunc( NULL );
	glutTabletButtonFunc( NULL );
	glutMenuStateFunc( NULL );
	glutTimerFunc( -1, NULL, 0 );
	glutIdleFunc( Animate );

	// init glew (a window must be open to do this):

#ifdef WIN32
	GLenum err = glewInit( );
	if( err != GLEW_OK )
	{
		fprintf( stderr, "glewInit Error\n" );
	}
	else
		fprintf( stderr, "GLEW initialized OK\n" );
	fprintf( stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif
}


// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void
InitLists( )
{
	float dx = BOXSIZE / 2.f;
	float dy = BOXSIZE / 2.f;
	float dz = BOXSIZE / 2.f;
	glutSetWindow( MainWindow );

	// create the object:

	BoxList = glGenLists( 1 );
	glNewList( BoxList, GL_COMPILE );

		glBegin( GL_QUADS );

			glColor3f( 0., 0., 1. );
				glVertex3f( -dx, -dy,  dz );
				glVertex3f(  dx, -dy,  dz );
				glVertex3f(  dx,  dy,  dz );
				glVertex3f( -dx,  dy,  dz );

				glVertex3f( -dx, -dy, -dz );
				glVertex3f( -dx,  dy, -dz );
				glVertex3f(  dx,  dy, -dz );
				glVertex3f(  dx, -dy, -dz );

			glColor3f( 1., 0., 0. );
				glVertex3f(  dx, -dy,  dz );
				glVertex3f(  dx, -dy, -dz );
				glVertex3f(  dx,  dy, -dz );
				glVertex3f(  dx,  dy,  dz );

				glVertex3f( -dx, -dy,  dz );
				glVertex3f( -dx,  dy,  dz );
				glVertex3f( -dx,  dy, -dz );
				glVertex3f( -dx, -dy, -dz );

			glColor3f( 0., 1., 0. );
				glVertex3f( -dx,  dy,  dz );
				glVertex3f(  dx,  dy,  dz );
				glVertex3f(  dx,  dy, -dz );
				glVertex3f( -dx,  dy, -dz );

				glVertex3f( -dx, -dy,  dz );
				glVertex3f( -dx, -dy, -dz );
				glVertex3f(  dx, -dy, -dz );
				glVertex3f(  dx, -dy,  dz );

		glEnd( );

	glEndList( );

	// create the axes:

	AxesList = glGenLists( 1 );
	glNewList( AxesList, GL_COMPILE );
		glLineWidth( AXES_WIDTH );
			Axes( 1.5 );
		glLineWidth( 1. );
	glEndList( );
}


// the keyboard callback:

void
Keyboard( unsigned char c, int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Keyboard: '%c' (0x%0x)\n", c, c );

	switch( c )
	{
	    case 'f':
	    case 'F':
		  Frozen = ! Frozen;
		  if (Frozen)
			glutIdleFunc(NULL);
		  else
			glutIdleFunc(Animate);
		  break;

	    case 'o':
		case 'O':
			WhichProjection = ORTHO;
			break;

		case 'p':
		case 'P':
			WhichProjection = PERSP;
			break;

		case 'q':
		case 'Q':
		case ESCAPE:
			DoMainMenu( QUIT );	// will not return here
			break;				// happy compiler

		default:
			fprintf( stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c );
	}

	// force a call to Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// called when the mouse button transitions down or up:

void
MouseButton( int button, int state, int x, int y )
{
	int b = 0;			// LEFT, MIDDLE, or RIGHT

	if( DebugOn != 0 )
		fprintf( stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y );

	// get the proper button bit mask:

	switch( button )
	{
		case GLUT_LEFT_BUTTON:
			b = LEFT;		break;

		case GLUT_MIDDLE_BUTTON:
			b = MIDDLE;		break;

		case GLUT_RIGHT_BUTTON:
			b = RIGHT;		break;

		case SCROLL_WHEEL_UP:
			Scale += SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
			// keep object from turning inside-out or disappearing:
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;

		case SCROLL_WHEEL_DOWN:
			Scale -= SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
			// keep object from turning inside-out or disappearing:
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;

		default:
			b = 0;
			fprintf( stderr, "Unknown mouse button: %d\n", button );
	}

	// button down sets the bit, up clears the bit:

	if( state == GLUT_DOWN )
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;		// set the proper bit
	}
	else
	{
		ActiveButton &= ~b;		// clear the proper bit
	}

	glutSetWindow(MainWindow);
	glutPostRedisplay();

}


// called when the mouse moves while a button is down:

void
MouseMotion( int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "MouseMotion: %d, %d\n", x, y );

	int dx = x - Xmouse;		// change in mouse coords
	int dy = y - Ymouse;

	if( ( ActiveButton & LEFT ) != 0 )
	{
		Xrot += ( ANGFACT*dy );
		Yrot += ( ANGFACT*dx );
	}

	if( ( ActiveButton & MIDDLE ) != 0 )
	{
		Scale += SCLFACT * (float) ( dx - dy );

		// keep object from turning inside-out or disappearing:

		if( Scale < MINSCALE )
			Scale = MINSCALE;
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene

void
Reset( )
{
	ActiveButton = 0;
	AxesOn = 1;
	DebugOn = 0;
	DepthBufferOn = 1;
	DepthFightingOn = 0;
	DepthCueOn = 0;
	Scale  = 1.0;
	WhichColor = WHITE;
	WhichProjection = PERSP;
	Xrot = Yrot = 0.;
	Frozen = false;
}


// called when user resizes the window:

void
Resize( int width, int height )
{
	if( DebugOn != 0 )
		fprintf( stderr, "ReSize: %d, %d\n", width, height );

	// don't really need to do anything since the window size is
	// checked each time in Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// handle a change to the window's visibility:

void
Visibility ( int state )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Visibility: %d\n", state );

	if( state == GLUT_VISIBLE )
	{
		glutSetWindow( MainWindow );
		glutPostRedisplay( );
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}



///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////


// the stroke characters 'X' 'Y' 'Z' :

static float xx[ ] = {
		0.f, 1.f, 0.f, 1.f
	      };

static float xy[ ] = {
		-.5f, .5f, .5f, -.5f
	      };

static int xorder[ ] = {
		1, 2, -3, 4
		};

static float yx[ ] = {
		0.f, 0.f, -.5f, .5f
	      };

static float yy[ ] = {
		0.f, .6f, 1.f, 1.f
	      };

static int yorder[ ] = {
		1, 2, 3, -2, 4
		};

static float zx[ ] = {
		1.f, 0.f, 1.f, 0.f, .25f, .75f
	      };

static float zy[ ] = {
		.5f, .5f, -.5f, -.5f, 0.f, 0.f
	      };

static int zorder[ ] = {
		1, 2, 3, 4, -5, 6
		};

// fraction of the length to use as height of the characters:
const float LENFRAC = 0.10f;

// fraction of length to use as start location of the characters:
const float BASEFRAC = 1.10f;

//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)

void
Axes( float length )
{
	glBegin( GL_LINE_STRIP );
		glVertex3f( length, 0., 0. );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., length, 0. );
	glEnd( );
	glBegin( GL_LINE_STRIP );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., 0., length );
	glEnd( );

	float fact = LENFRAC * length;
	float base = BASEFRAC * length;

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 4; i++ )
		{
			int j = xorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( base + fact*xx[j], fact*xy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 5; i++ )
		{
			int j = yorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( fact*yx[j], base + fact*yy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 6; i++ )
		{
			int j = zorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( 0.0, fact*zy[j], base + fact*zx[j] );
		}
	glEnd( );

}

// read a BMP file into a Texture:

#define VERBOSE		false
#define BMP_MAGIC_NUMBER	0x4d42
#ifndef BI_RGB
#define BI_RGB			0
#define BI_RLE8			1
#define BI_RLE4			2
#endif


// bmp file header:
struct bmfh
{
	short bfType;		// BMP_MAGIC_NUMBER = "BM"
	int bfSize;		// size of this file in bytes
	short bfReserved1;
	short bfReserved2;
	int bfOffBytes;		// # bytes to get to the start of the per-pixel data
} FileHeader;

// bmp info header:
struct bmih
{
	int biSize;		// info header size, should be 40
	int biWidth;		// image width
	int biHeight;		// image height
	short biPlanes;		// #color planes, should be 1
	short biBitCount;	// #bits/pixel, should be 1, 4, 8, 16, 24, 32
	int biCompression;	// BI_RGB, BI_RLE4, BI_RLE8
	int biSizeImage;
	int biXPixelsPerMeter;
	int biYPixelsPerMeter;
	int biClrUsed;		// # colors in the palette
	int biClrImportant;
} InfoHeader;

// read a BMP file into a Texture:

unsigned char *
BmpToTexture( char *filename, int *width, int *height )
{
	FILE *fp;
#ifdef _WIN32
        errno_t err = fopen_s( &fp, filename, "rb" );
        if( err != 0 )
        {
		fprintf( stderr, "Cannot open Bmp file '%s'\n", filename );
		return NULL;
        }
#else
	FILE *fp = fopen( filename, "rb" );
	if( fp == NULL )
	{
		fprintf( stderr, "Cannot open Bmp file '%s'\n", filename );
		return NULL;
	}
#endif

	FileHeader.bfType = ReadShort( fp );

	// if bfType is not BMP_MAGIC_NUMBER, the file is not a bmp:

	if( VERBOSE ) fprintf( stderr, "FileHeader.bfType = 0x%0x = \"%c%c\"\n",
			FileHeader.bfType, FileHeader.bfType&0xff, (FileHeader.bfType>>8)&0xff );
	if( FileHeader.bfType != BMP_MAGIC_NUMBER )
	{
		fprintf( stderr, "Wrong type of file: 0x%0x\n", FileHeader.bfType );
		fclose( fp );
		return NULL;
	}

	FileHeader.bfSize = ReadInt( fp );
	if( VERBOSE )	fprintf( stderr, "FileHeader.bfSize = %d\n", FileHeader.bfSize );

	FileHeader.bfReserved1 = ReadShort( fp );
	FileHeader.bfReserved2 = ReadShort( fp );

	FileHeader.bfOffBytes = ReadInt( fp );
	if( VERBOSE )	fprintf( stderr, "FileHeader.bfOffBytes = %d\n", FileHeader.bfOffBytes );

	InfoHeader.biSize = ReadInt( fp );
	if( VERBOSE )	fprintf( stderr, "InfoHeader.biSize = %d\n", InfoHeader.biSize );
	InfoHeader.biWidth = ReadInt( fp );
	if( VERBOSE )	fprintf( stderr, "InfoHeader.biWidth = %d\n", InfoHeader.biWidth );
	InfoHeader.biHeight = ReadInt( fp );
	if( VERBOSE )	fprintf( stderr, "InfoHeader.biHeight = %d\n", InfoHeader.biHeight );

	const int nums = InfoHeader.biWidth;
	const int numt = InfoHeader.biHeight;

	InfoHeader.biPlanes = ReadShort( fp );
	if( VERBOSE )	fprintf( stderr, "InfoHeader.biPlanes = %d\n", InfoHeader.biPlanes );

	InfoHeader.biBitCount = ReadShort( fp );
	if( VERBOSE )	fprintf( stderr, "InfoHeader.biBitCount = %d\n", InfoHeader.biBitCount );

	InfoHeader.biCompression = ReadInt( fp );
	if( VERBOSE )	fprintf( stderr, "InfoHeader.biCompression = %d\n", InfoHeader.biCompression );

	InfoHeader.biSizeImage = ReadInt( fp );
	if( VERBOSE )	fprintf( stderr, "InfoHeader.biSizeImage = %d\n", InfoHeader.biSizeImage );

	InfoHeader.biXPixelsPerMeter = ReadInt( fp );
	InfoHeader.biYPixelsPerMeter = ReadInt( fp );

	InfoHeader.biClrUsed = ReadInt( fp );
	if( VERBOSE )	fprintf( stderr, "InfoHeader.biClrUsed = %d\n", InfoHeader.biClrUsed );

	InfoHeader.biClrImportant = ReadInt( fp );

	// fprintf( stderr, "Image size found: %d x %d\n", ImageWidth, ImageHeight );

	// pixels will be stored bottom-to-top, left-to-right:
	unsigned char *texture = new unsigned char[ 3 * nums * numt ];
	if( texture == NULL )
	{
		fprintf( stderr, "Cannot allocate the texture array!\n" );
		return NULL;
	}

	// extra padding bytes:

	int requiredRowSizeInBytes = 4 * ( ( InfoHeader.biBitCount*InfoHeader.biWidth + 31 ) / 32 );
	if( VERBOSE )	fprintf( stderr, "requiredRowSizeInBytes = %d\n", requiredRowSizeInBytes );

	int myRowSizeInBytes = ( InfoHeader.biBitCount*InfoHeader.biWidth + 7 ) / 8;
	if( VERBOSE )	fprintf( stderr, "myRowSizeInBytes = %d\n", myRowSizeInBytes );

	int oldNumExtra =  4*(( (3*InfoHeader.biWidth)+3)/4) - 3*InfoHeader.biWidth;
	if( VERBOSE )	fprintf( stderr, "Old NumExtra padding = %d\n", oldNumExtra );

	int numExtra = requiredRowSizeInBytes - myRowSizeInBytes;
	if( VERBOSE )	fprintf( stderr, "New NumExtra padding = %d\n", numExtra );

	// this function does not support compression:

	if( InfoHeader.biCompression != 0 )
	{
		fprintf( stderr, "Wrong type of image compression: %d\n", InfoHeader.biCompression );
		fclose( fp );
		return NULL;
	}
	
	// we can handle 24 bits of direct color:
	if( InfoHeader.biBitCount == 24 )
	{
		rewind( fp );
		fseek( fp, FileHeader.bfOffBytes, SEEK_SET );
		int t;
		unsigned char *tp;
		for( t = 0, tp = texture; t < numt; t++ )
		{
			for( int s = 0; s < nums; s++, tp += 3 )
			{
				*(tp+2) = fgetc( fp );		// b
				*(tp+1) = fgetc( fp );		// g
				*(tp+0) = fgetc( fp );		// r
			}

			for( int e = 0; e < numExtra; e++ )
			{
				fgetc( fp );
			}
		}
	}

	// we can also handle 8 bits of indirect color:
	if( InfoHeader.biBitCount == 8 && InfoHeader.biClrUsed == 256 )
	{
		struct rgba32
		{
			unsigned char r, g, b, a;
		};
		struct rgba32 *colorTable = new struct rgba32[ InfoHeader.biClrUsed ];

		rewind( fp );
		fseek( fp, sizeof(struct bmfh) + InfoHeader.biSize - 2, SEEK_SET );
		for( int c = 0; c < InfoHeader.biClrUsed; c++ )
		{
			colorTable[c].r = fgetc( fp );
			colorTable[c].g = fgetc( fp );
			colorTable[c].b = fgetc( fp );
			colorTable[c].a = fgetc( fp );
			if( VERBOSE )	fprintf( stderr, "%4d:\t0x%02x\t0x%02x\t0x%02x\t0x%02x\n",
				c, colorTable[c].r, colorTable[c].g, colorTable[c].b, colorTable[c].a );
		}

		rewind( fp );
		fseek( fp, FileHeader.bfOffBytes, SEEK_SET );
		int t;
		unsigned char *tp;
		for( t = 0, tp = texture; t < numt; t++ )
		{
			for( int s = 0; s < nums; s++, tp += 3 )
			{
				int index = fgetc( fp );
				*(tp+0) = colorTable[index].r;	// r
				*(tp+1) = colorTable[index].g;	// g
				*(tp+2) = colorTable[index].b;	// b
			}

			for( int e = 0; e < numExtra; e++ )
			{
				fgetc( fp );
			}
		}

		delete[ ] colorTable;
	}

	fclose( fp );

	*width = nums;
	*height = numt;
	return texture;
}

int
ReadInt( FILE *fp )
{
	const unsigned char b0 = fgetc( fp );
	const unsigned char b1 = fgetc( fp );
	const unsigned char b2 = fgetc( fp );
	const unsigned char b3 = fgetc( fp );
	return ( b3 << 24 )  |  ( b2 << 16 )  |  ( b1 << 8 )  |  b0;
}

short
ReadShort( FILE *fp )
{
	const unsigned char b0 = fgetc( fp );
	const unsigned char b1 = fgetc( fp );
	return ( b1 << 8 )  |  b0;
}


// function to convert HSV to RGB
// 0.  <=  s, v, r, g, b  <=  1.
// 0.  <= h  <=  360.
// when this returns, call:
//		glColor3fv( rgb );

void
HsvRgb( float hsv[3], float rgb[3] )
{
	// guarantee valid input:

	float h = hsv[0] / 60.f;
	while( h >= 6. )	h -= 6.;
	while( h <  0. ) 	h += 6.;

	float s = hsv[1];
	if( s < 0. )
		s = 0.;
	if( s > 1. )
		s = 1.;

	float v = hsv[2];
	if( v < 0. )
		v = 0.;
	if( v > 1. )
		v = 1.;

	// if sat==0, then is a gray:

	if( s == 0.0 )
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}

	// get an rgb from the hue itself:
	
	float i = (float)floor( h );
	float f = h - i;
	float p = v * ( 1.f - s );
	float q = v * ( 1.f - s*f );
	float t = v * ( 1.f - ( s * (1.f-f) ) );

	float r=0., g=0., b=0.;			// red, green, blue
	switch( (int) i )
	{
		case 0:
			r = v;	g = t;	b = p;
			break;
	
		case 1:
			r = q;	g = v;	b = p;
			break;
	
		case 2:
			r = p;	g = v;	b = t;
			break;
	
		case 3:
			r = p;	g = q;	b = v;
			break;
	
		case 4:
			r = t;	g = p;	b = v;
			break;
	
		case 5:
			r = v;	g = p;	b = q;
			break;
	}

	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}

void
Cross(float v1[3], float v2[3], float vout[3])
{
	float tmp[3];
	tmp[0] = v1[1] * v2[2] - v2[1] * v1[2];
	tmp[1] = v2[0] * v1[2] - v1[0] * v2[2];
	tmp[2] = v1[0] * v2[1] - v2[0] * v1[1];
	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}

float
Dot(float v1[3], float v2[3])
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

float
Unit(float vin[3], float vout[3])
{
	float dist = vin[0] * vin[0] + vin[1] * vin[1] + vin[2] * vin[2];
	if (dist > 0.0)
	{
		dist = sqrtf(dist);
		vout[0] = vin[0] / dist;
		vout[1] = vin[1] / dist;
		vout[2] = vin[2] / dist;
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}
	return dist;
}

void //Rotating a Point an Angle about the X Axis Around a Center
RotateX(Point* p, float deg, float xc, float yc, float zc)
{
	float rad = deg * (M_PI / 180.f);         // radians
	float x = p->x0 - xc;
	float y = p->y0 - yc;
	float z = p->z0 - zc;

	float xp = x;
	float yp = y * cos(rad) - z * sin(rad);
	float zp = y * sin(rad) + z * cos(rad);

	p->x = xp + xc;
	p->y = yp + yc;
	p->z = zp + zc;
}

void //Rotating a Point an Angle about the Y Axis Around a Center
RotateY(Point* p, float deg, float xc, float yc, float zc)
{
	float rad = deg * (M_PI / 180.f);         // radians
	float x = p->x0 - xc;
	float y = p->y0 - yc;
	float z = p->z0 - zc;

	float xp = x * cos(rad) + z * sin(rad);
	float yp = y;
	float zp = -x * sin(rad) + z * cos(rad);

	p->x = xp + xc;
	p->y = yp + yc;
	p->z = zp + zc;
}

void //Rotating a Point an Angle about the Z Axis Around a Center
RotateZ(Point* p, float deg, float xc, float yc, float zc)
{
	float rad = deg * (M_PI / 180.f);         // radians
	float x = p->x0 - xc;
	float y = p->y0 - yc;
	float z = p->z0 - zc;

	float xp = x * cos(rad) - y * sin(rad);
	float yp = x * sin(rad) + y * cos(rad);
	float zp = z;

	p->x = xp + xc;
	p->y = yp + yc;
	p->z = zp + zc;
}


