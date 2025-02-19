/******************************************************************************
 * OpenGL / GLUT Tree simulation
 *
 * Author:    Hoi Tan
 *
 * Description:
 * -----------
 * This program demonstrates a 3D tree simulation using OpenGL, GLUT, and an L-system
 * to generate tree branches. Turtle graphics are used for interpreting the L-system
 * string into geometry. Leaves are then placed at the branch ends and rendered with
 * a simple leaf shader. Various keyboard and menu options allow for toggling axes,
 * changing colors, projections, etc.
 *
 * Controls:
 * ---------
 * - Left Mouse Button:    Rotate
 * - Middle Mouse Button:  Scale
 * - 'o' or 'O':           Use orthographic projection
 * - 'p' or 'P':           Use perspective projection
 * - 'r' or 'R':           Toggle through different L-system rules
 * - 'q' or 'Q' or ESC:    Quit
 *
 * Menus:
 * ------
 * Right-click opens a menu to toggle axes, depth cue, projections, etc.
 ******************************************************************************/

//=============================================================================
//  1. Includes
//=============================================================================
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <unordered_map>
#include <math.h>
#include "glm/gtc/type_ptr.hpp"

// For Apple vs. Windows/OpenGL
#ifdef __APPLE__
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#else
    #include "glew.h"
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif

#include "glut.h"
#include <GL/glui.h>

// Project headers
#include "TreeBody/LSystem.hpp"
#include "TreeBody/Turtle.hpp"

//=============================================================================
//  2. Macros/Defines
//=============================================================================
#define GLM
#ifndef F_PI
    #define F_PI        ((float)(M_PI))
    #define F_2_PI      ((float)(2.f*F_PI))
    #define F_PI_2      ((float)(F_PI/2.f))
#endif

#define _USE_MATH_DEFINES
#define WINDOWTITLE "OpenGL / GLUT Tree simulation -- Hoi Tan"
#define GLUITITLE   "User Interface Window"

// The escape key:
#define ESCAPE      0x1b

// Initial window size:
#define INIT_WINDOW_SIZE 800

// Size of the 3D box to be drawn:
#define BOXSIZE     2.f

// Multiplication factors for input interaction:
#define ANGFACT     1.f
#define SCLFACT     0.005f

// Minimum allowable scale factor:
#define MINSCALE    0.05f

// Scroll wheel button values:
#define SCROLL_WHEEL_UP       3
#define SCROLL_WHEEL_DOWN     4
#define SCROLL_WHEEL_CLICK_FACTOR 5.f

// Active mouse buttons (or them together):
#define LEFT    4
#define MIDDLE  2
#define RIGHT   1

//=============================================================================
//  3. Constants / Enums
//=============================================================================
enum Projections {
    ORTHO,
    PERSP
};

enum ButtonVals {
    RESET,
    QUIT
};

enum Colors {
    RED,
    YELLOW,
    GREEN,
    CYAN,
    BLUE,
    MAGENTA
};

// Window background color (RGBA):
static const GLfloat BACKCOLOR[] = { 0.f, 0.f, 0.f, 1.f };

// Axes line width:
static const GLfloat AXES_WIDTH = 3.f;

// Color names:
static char * ColorNames[] = {
    (char *)"Red",
    (char *)"Yellow",
    (char *)"Green",
    (char *)"Cyan",
    (char *)"Blue",
    (char *)"Magenta"
};

// Color definitions (same order as above):
static const GLfloat Colors[][3] = {
    { 1.f, 0.f, 0.f },  // red
    { 1.f, 1.f, 0.f },  // yellow
    { 0.f, 1.f, 0.f },  // green
    { 0.f, 1.f, 1.f },  // cyan
    { 0.f, 0.f, 1.f },  // blue
    { 1.f, 0.f, 1.f },  // magenta
};

// Fog parameters:
static const GLfloat FOGCOLOR[4] = { 0.f, 0.f, 0.f, 1.f };
static const GLenum  FOGMODE     = GL_LINEAR;
static const GLfloat FOGDENSITY  = 0.30f;
static const GLfloat FOGSTART    = 1.5f;
static const GLfloat FOGEND      = 4.f;

// White color for lighting:
static const float WHITE[] = { 1.f, 1.f, 1.f, 1.f };

// Animation cycle:
static const int MS_PER_CYCLE = 10000; // 10 seconds

//=============================================================================
//  4. Global Variables
//=============================================================================
int ActiveButton;         // current button that is down
GLuint AxesList;          // display list for axes
int AxesOn;               // != 0 means to draw the axes
GLuint BoxList;           // object display list
int DebugOn;              // != 0 means to print debugging info
int DepthCueOn;           // != 0 means to use intensity depth cueing
int DepthBufferOn;        // != 0 means to use the z-buffer
int DepthFightingOn;      // != 0 means to force the creation of z-fighting
int MainWindow;           // window id for main graphics window
int NowColor;             // index into Colors[]
int NowProjection;        // ORTHO or PERSP
float Scale;              // scaling factor
int ShadowsOn;            // != 0 means to turn shadows on
float Time;               // used for animation (0..1)
int Xmouse, Ymouse;       // mouse values
float Xrot, Yrot;         // rotation angles in degrees
float LightX, LightY, LightZ; // light position

// Camera position (initially matches the original gluLookAt values)
static float camX = -50.f;
static float camY =  54.f;
static float camZ =  53.f;
// Movement speed for WASD
static float moveSpeed = 4.f;

// use glui
int mainWindow;
GLUI *glui;


//=============================================================================
//  5. Forward Declarations (Function Prototypes)
//=============================================================================
void Animate();
void Display();
void DoAxesMenu(int);
void DoColorMenu(int);
void DoDepthBufferMenu(int);
void DoDepthFightingMenu(int);
void DoDepthMenu(int);
void DoDebugMenu(int);
void DoMainMenu(int);
void DoProjectMenu(int);
void DoRasterString(float, float, float, char *);
void DoStrokeString(float, float, float, float, char *);
float ElapsedSeconds();
void InitGraphics();
void InitLists();
void InitMenus();
void Keyboard(unsigned char, int, int);
void MouseButton(int, int, int, int);
void MouseMotion(int, int);
void Reset();
void Resize(int, int);
void Visibility(int);

void Axes(float);
void HsvRgb(float[3], float[3]);
void Cross(float[3], float[3], float[3]);
float Dot(float[3], float[3]);
float Unit(float[3], float[3]);
float Unit(float[3]);

// Utility array creation functions
float* Array3(float a, float b, float c);
float* MulArray3(float factor, float array0[]);
float* MulArray3(float factor, float a, float b, float c);

// Texture helper
void SetUpTexture(char* filename, GLuint* texture);

//=============================================================================
//  6. Utility / Helper Functions
//=============================================================================

// Create an array of 4 floats from three separate values:
float* Array3(float a, float b, float c)
{
    static float array[4];
    array[0] = a;
    array[1] = b;
    array[2] = c;
    array[3] = 1.f;
    return array;
}

// Create an array of 4 floats from a multiplier and an array:
float* MulArray3(float factor, float array0[])
{
    static float array[4];
    array[0] = factor * array0[0];
    array[1] = factor * array0[1];
    array[2] = factor * array0[2];
    array[3] = 1.f;
    return array;
}

// Overloaded version for direct floats:
float* MulArray3(float factor, float a, float b, float c )
{
    static float array[4];
    float* abc = Array3(a, b, c);
    array[0] = factor * abc[0];
    array[1] = factor * abc[1];
    array[2] = factor * abc[2];
    array[3] = 1.f;
    return array;
}

// Other helper source files (material, light, geometry, etc.)
#include "setmaterial.cpp"
#include "setlight.cpp"
#include "osusphere.cpp"
// #include "osucone.cpp"
// #include "osutorus.cpp"
#include "bmptotexture.cpp"
#include "loadobjfile.cpp"
#include "keytime.cpp"
#include "glslprogram.cpp"

//=============================================================================
// Additional Global Variables for L-system / Leaf rendering
//=============================================================================
int changeRule = 0; // switch by key
float NowS0, NowT0, NowD, NowKa, NowKd, NowKs, NowShine, NowAlpha;
float NowLeafColor[3] = { 0.f, 0.f, 0.f };

unsigned char* mpLeaf2Texture;
GLuint OSUSphere;
GLuint Leaf2DL;
GLuint Leaf2Tex;
GLuint SphereDL;
GLuint GridDL;

// Keytime objects
Keytimes Kamp, Kfreq, Kspeed;

// GLSL program
GLSLProgram LeafProgram;
GLSLProgram GetDepth;
GLSLProgram RenderWithShadows;
// shadow texture
GLuint DepthFramebuffer;
GLuint DepthTexture;
const int SHADOW_WIDTH = 1024;
const int SHADOW_HEIGHT = 1024;

// Display the scene
void DisplayOneScene( GLSLProgram program );

//glui
void GluiControlCallback(int controlID);
void initGlui();

// Random float helper (if needed)
float Ranf(float low, float high)
{
    float r = (float)rand(); // 0 - RAND_MAX
    float t = r / (float)RAND_MAX; // 0. - 1.
    return low + t * (high - low);
}

//----------------------------------------------------------------------------
//  7. Main Program
//----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    // Turn on the glut package:
    glutInit(&argc, argv);

    // Setup all the graphics stuff:
    InitGraphics();

    // Create the display lists that do not change:
    InitLists();

    // Init all the global variables used by Display():
    Reset();

    // Setup all the user interface stuff:
    InitMenus();

    // Draw the scene once and wait for interaction:
    glutSetWindow(MainWindow);
    glutMainLoop();

    // glutMainLoop() never returns
    return 0; // just to satisfy the compiler
}

//=============================================================================
//  8. Callback and Event Functions
//=============================================================================

// Animate the scene
void Animate()
{
    int ms = glutGet(GLUT_ELAPSED_TIME);
    ms %= MS_PER_CYCLE; // 0..MS_PER_CYCLE-1
    Time = (float)ms / (float)MS_PER_CYCLE; // 0..1

    // Force a call to Display():
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

// Display callback
void Display()
{
    if(DebugOn != 0)
        fprintf(stderr, "Starting Display.\n");

    // Set which window to render into:
    glutSetWindow(MainWindow);

    // Erase the background:
    glDrawBuffer(GL_BACK);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
#ifdef DEMO_DEPTH_BUFFER
    if(DepthBufferOn == 0)
        glDisable(GL_DEPTH_TEST);
#endif

    // Use flat shading
    glShadeModel(GL_FLAT);

    // Set viewport to a square centered in the window:
    GLsizei vx = glutGet(GLUT_WINDOW_WIDTH);
    GLsizei vy = glutGet(GLUT_WINDOW_HEIGHT);
    GLsizei v = (vx < vy) ? vx : vy; // minimum dimension
    GLint xl = (vx - v) / 2;
    GLint yb = (vy - v) / 2;
    glViewport(xl, yb, v, v);

    // Projection:
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if(NowProjection == ORTHO)
        glOrtho(-2.f, 2.f, -2.f, 2.f, 0.1f, 1000.f);
    else
        gluPerspective(70.f, 1.f, 0.1f, 1000.f);

    // Place objects:
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Eye position, look-at, up-vector
    // Use the camera variables:
	gluLookAt(camX, camY, camZ,  0.f, 5.f, 0.f,   0.f, 1.f, 0.f);


    // Rotate the scene:
    glRotatef(Yrot, 0.f, 1.f, 0.f);
    glRotatef(Xrot, 1.f, 0.f, 0.f);

    // Scale the scene:
    if(Scale < MINSCALE)
        Scale = MINSCALE;
    glScalef(Scale, Scale, Scale);

    // Fog:
    if(DepthCueOn != 0) {
        glFogi(GL_FOG_MODE, FOGMODE);
        glFogfv(GL_FOG_COLOR, FOGCOLOR);
        glFogf(GL_FOG_DENSITY, FOGDENSITY);
        glFogf(GL_FOG_START, FOGSTART);
        glFogf(GL_FOG_END, FOGEND);
        glEnable(GL_FOG);
    } else {
        glDisable(GL_FOG);
    }

    // Draw axes:
    if(AxesOn != 0) {
        glColor3fv(&Colors[NowColor][0]);
        glCallList(AxesList);
    }

    // Normalize:
    glEnable(GL_NORMALIZE);

    // Animation time in seconds:
    int msec = glutGet(GLUT_ELAPSED_TIME) % MS_PER_CYCLE;
    float nowTime = (float)msec / 1000.f; // 0..10.

	// Set up light position
	LightX = 0.f;
	LightY = 30.f;
	LightZ = 0.f;
	////////////////////////////////////////////////////////////
    // Setup light
    glPushMatrix();
        glTranslatef(-10.f, 30.f, 0.f);
        glColor3f(1.f, 1.f, 1.f);
        glScalef(.5f, .5f, .5f);
        glCallList(OSUSphere);
        glRotatef(90.f, 0.f, 0.f, 1.f);
        SetPointLight(GL_LIGHT0, 0, 0, 0, 1, 1, 1);
    glPopMatrix();

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    LeafProgram.SetUniformVariable((char*)"uLightPos", LightX, LightY, LightZ);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Leaf2Tex);
    LeafProgram.SetUniformVariable((char*)"uTexUnit", 0);

	////////////////////////////////////////////////////////////
	//set up shadow
	//first pass, render from light's perspective, store depth of scene in texture
	glBindFramebuffer(GL_FRAMEBUFFER, DepthFramebuffer);
	glClear(GL_DEPTH_BUFFER_BIT);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_FLAT);
	glDisable(GL_NORMALIZE);
	// these matrices are the equivalent of projection and view matrices
	glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.f, 20.f);
	glm::vec3 lightPos(LightX, LightY, LightZ);
	glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0., 0., 0.), glm::vec3(0., 1., 0.));
	//this matrix is the transformation matrix that the vertex shader will use instead of glModelViewProjectionMatrix:
	glm::mat4 lightSpaceMatrix = lightProjection * lightView;
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	GetDepth.Use( );
	GetDepth.SetUniformVariable((char*)"uLightSpaceMatrix", lightSpaceMatrix);
	glm::vec3 color = glm::vec3(0., 1., 1.);
	GetDepth.SetUniformVariable((char*)"uColor", color);
	DisplayOneScene(GetDepth);
	GetDepth.UnUse( );
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//second pass, render from camera's perspective, use depth map to calculate shadows
	RenderWithShadows.Use();
	RenderWithShadows.SetUniformVariable((char*)"uShadowMap", 0 );
	RenderWithShadows.SetUniformVariable((char*)"uLightX", LightX);
	RenderWithShadows.SetUniformVariable((char*)"uLightY", LightY);
	RenderWithShadows.SetUniformVariable((char*)"uLightZ", LightZ);
	RenderWithShadows.SetUniformVariable((char*)"uLightSpaceMatrix", lightSpaceMatrix);
	glm::vec3 eye = glm::vec3(0., 0., 8.);
	glm::vec3 look = glm::vec3(0., 0., 0.);
	glm::vec3 up = glm::vec3(0., 1., 0.);
	glm::mat4 modelview = glm::lookAt(eye, look, up);
	if (Scale < MINSCALE) Scale = MINSCALE;
	glm::vec3 scale = glm::vec3(Scale, Scale, Scale);
	modelview = glm::scale(modelview, scale);
	glm::vec3 xaxis = glm::vec3(1., 0., 0.);
	glm::vec3 yaxis = glm::vec3(0., 1., 0.);
	modelview = glm::rotate(modelview, glm::radians(Yrot), yaxis);
	modelview = glm::rotate(modelview, glm::radians(Xrot), xaxis);
	RenderWithShadows.SetUniformVariable((char*)"uModelView", modelview);
	glm::mat4 proj = glm::perspective(glm::radians(75.f), 1.f, .1f, 100.f);
	RenderWithShadows.SetUniformVariable((char*)"uProj", proj);
	DisplayOneScene(RenderWithShadows);
	RenderWithShadows.UnUse( );


	    // Floor
    glPushMatrix();
        glTranslatef(0.f, -5.f, 0.f);
        glScalef(5.f, 5.f, 5.f);
        glCallList(GridDL);
    glPopMatrix();
    glDisable(GL_LIGHTING);


    // Draw box list for demonstration (if needed)
    glCallList(BoxList);

#ifdef DEMO_Z_FIGHTING
    if(DepthFightingOn != 0)
    {
        glPushMatrix();
            glRotatef(90.f, 0.f, 1.f, 0.f);
            glCallList(BoxList);
        glPopMatrix();
    }
#endif

    // 2D overlay text:
    glDisable(GL_DEPTH_TEST);
    glColor3f(0.f, 1.f, 1.f);
    // DoRasterString(0.f, 1.f, 0.f, (char *)"Text That Moves");

    // Another overlay text
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.f, 100.f, 0.f, 100.f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor3f(1.f, 1.f, 1.f);
    // DoRasterString(5.f, 5.f, 0.f, (char *)"Text That Doesn't");

    // Swap buffers
    glutSwapBuffers();
    glFlush();
}

// Keyboard callback
 void Keyboard(unsigned char c, int x, int y)
{
	// // Convert degrees to radians
	// float radX = glm::radians(Xrot);
	// float radY = glm::radians(Yrot);

	// // "Forward" vector in your scene:
	// float fx = sinf(radY);
	// float fz = -cosf(radY);
	// float fy = sinf(radX);   // If you want up/down from Xrot

	// // Then pressing 'W':
	// camX += fx * moveSpeed;
	// camY += fy * moveSpeed;
	// camZ += fz * moveSpeed;
	// // etc.
	moveSpeed = 7;

    if(DebugOn != 0)
        fprintf(stderr, "Keyboard: '%c' (0x%0x)\n", c, c);

    switch(c)
    {
        // Toggle Orthographic Projection
        case 'o':
        case 'O':
            NowProjection = ORTHO;
            break;

        // Toggle Perspective Projection
        case 'p':
        case 'P':
            NowProjection = PERSP;
            break;

        // Quit
        case 'q':
        case 'Q':
        case ESCAPE:
            DoMainMenu(QUIT);
            break;

        // Cycle L-system rules
        case 'r':
        case 'R':
            changeRule++;
            break;

        // ======== WASD MOVEMENT ========
        case 'w':
        case 'W':
            // Move 'forward' along -Z
            camZ -= moveSpeed;
            break;

        case 's':
        case 'S':
            // Move 'backward' along +Z
            camZ += moveSpeed;
            break;

        case 'a':
        case 'A':
            // Move 'left' along -X
            camX -= moveSpeed;
            break;

        case 'd':
        case 'D':
            // Move 'right' along +X
            camX += moveSpeed;
            break;

        default:
            fprintf(stderr, "Unknown key: '%c' (0x%0x)\n", c, c);
            break;
    }

    // Tell GLUT to redraw after changes:
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}


// Mouse button callback
void MouseButton(int button, int state, int x, int y)
{
    if(DebugOn != 0)
        fprintf(stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y);

    int b = 0;
    switch(button)
    {
        case GLUT_LEFT_BUTTON:
            b = LEFT; break;
        case GLUT_MIDDLE_BUTTON:
            b = MIDDLE; break;
        case GLUT_RIGHT_BUTTON:
            b = RIGHT; break;
        case SCROLL_WHEEL_UP:
            Scale += SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
            if(Scale < MINSCALE) Scale = MINSCALE;
            break;
        case SCROLL_WHEEL_DOWN:
            Scale -= SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
            if(Scale < MINSCALE) Scale = MINSCALE;
            break;
        default:
            b = 0;
            fprintf(stderr, "Unknown mouse button: %d\n", button);
            break;
    }

    if(state == GLUT_DOWN)
    {
        Xmouse = x;
        Ymouse = y;
        ActiveButton |= b;
    }
    else
    {
        ActiveButton &= ~b;
    }
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

// Mouse motion callback
void MouseMotion(int x, int y)
{
    int dx = x - Xmouse;
    int dy = y - Ymouse;

    if((ActiveButton & LEFT) != 0)
    {
        Xrot += (ANGFACT * dy);
        Yrot += (ANGFACT * dx);
    }
    if((ActiveButton & MIDDLE) != 0)
    {
        Scale += SCLFACT * (float)(dx - dy);
        if(Scale < MINSCALE)
            Scale = MINSCALE;
    }

    Xmouse = x;
    Ymouse = y;

    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

// Reset transforms and colors
void Reset()
{
    ActiveButton = 0;
    AxesOn = 1;
    DebugOn = 0;
    DepthBufferOn = 1;
    DepthFightingOn = 0;
    DepthCueOn = 0;
    Scale = 1.f;
    ShadowsOn = 0;
    NowColor = YELLOW;
    NowProjection = PERSP;
    Xrot = Yrot = 0.f;
}

// Resize callback
void Resize(int width, int height)
{
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

// Visibility callback
void Visibility(int state)
{
    if(DebugOn != 0)
        fprintf(stderr, "Visibility: %d\n", state);

    if(state == GLUT_VISIBLE)
    {
        glutSetWindow(MainWindow);
        glutPostRedisplay();
    }
    else
    {
        // could pause animation if desired
    }
}

//=============================================================================
//  9. Menu Functions
//=============================================================================
void DoAxesMenu(int id)
{
    AxesOn = id;
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

void DoColorMenu(int id)
{
    NowColor = id - RED;
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

void DoDebugMenu(int id)
{
    DebugOn = id;
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

void DoDepthBufferMenu(int id)
{
    DepthBufferOn = id;
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

void DoDepthFightingMenu(int id)
{
    DepthFightingOn = id;
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

void DoDepthMenu(int id)
{
    DepthCueOn = id;
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

void DoMainMenu(int id)
{
    switch(id)
    {
        case RESET:
            Reset();
            break;
        case QUIT:
            glutSetWindow(MainWindow);
            glFinish();
            glutDestroyWindow(MainWindow);
            exit(0);
            break;
        default:
            fprintf(stderr, "Unknown Main Menu ID %d\n", id);
            break;
    }
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

void DoProjectMenu(int id)
{
    NowProjection = id;
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

//=============================================================================
//  10. GL/GLUT Initialization and Menu Setup
//=============================================================================
void InitMenus()
{
    if(DebugOn != 0)
        fprintf(stderr, "Starting InitMenus.\n");

    glutSetWindow(MainWindow);

    int numColors = sizeof(Colors) / (3 * sizeof(float));
    int colormenu = glutCreateMenu(DoColorMenu);
    for(int i = 0; i < numColors; i++)
    {
        glutAddMenuEntry(ColorNames[i], i + RED);
    }

    int axesmenu = glutCreateMenu(DoAxesMenu);
    glutAddMenuEntry("Off", 0);
    glutAddMenuEntry("On",  1);

    int depthcuemenu = glutCreateMenu(DoDepthMenu);
    glutAddMenuEntry("Off", 0);
    glutAddMenuEntry("On",  1);

    int depthbuffermenu = glutCreateMenu(DoDepthBufferMenu);
    glutAddMenuEntry("Off", 0);
    glutAddMenuEntry("On",  1);

    int depthfightingmenu = glutCreateMenu(DoDepthFightingMenu);
    glutAddMenuEntry("Off", 0);
    glutAddMenuEntry("On",  1);

    int debugmenu = glutCreateMenu(DoDebugMenu);
    glutAddMenuEntry("Off", 0);
    glutAddMenuEntry("On",  1);

    int projmenu = glutCreateMenu(DoProjectMenu);
    glutAddMenuEntry("Orthographic", ORTHO);
    glutAddMenuEntry("Perspective",  PERSP);

    int mainmenu = glutCreateMenu(DoMainMenu);
    glutAddSubMenu("Axes", axesmenu);
    glutAddSubMenu("Axis Colors", colormenu);

#ifdef DEMO_DEPTH_BUFFER
    glutAddSubMenu("Depth Buffer", depthbuffermenu);
#endif

#ifdef DEMO_Z_FIGHTING
    glutAddSubMenu("Depth Fighting", depthfightingmenu);
#endif

    glutAddSubMenu("Depth Cue", depthcuemenu);
    glutAddSubMenu("Projection", projmenu);
    glutAddMenuEntry("Reset", RESET);
    glutAddSubMenu("Debug", debugmenu);
    glutAddMenuEntry("Quit", QUIT);

    // Attach the pop-up menu to the right mouse button:
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void InitGraphics()
{
    if(DebugOn != 0)
        fprintf(stderr, "Starting InitGraphics.\n");

    // 10s animation
    Kamp.Init();
    Kfreq.Init();
    Kspeed.Init();

    Kamp.AddTimeValue(0, 0.5);
    Kfreq.AddTimeValue(0, 1);
    Kspeed.AddTimeValue(0, 5);

    Kamp.AddTimeValue(5, 1);
    Kfreq.AddTimeValue(5, 0.5);
    Kspeed.AddTimeValue(5, 7);

    Kamp.AddTimeValue(7, 0.7f);
    Kfreq.AddTimeValue(7, 2);
    Kspeed.AddTimeValue(7, 3);

    Kamp.AddTimeValue(10, 0.5);
    Kfreq.AddTimeValue(10, 1);
    Kspeed.AddTimeValue(10, 5);

    Kamp.PrintTimeValues();

    // Request display modes:
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

    // Window config:
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(INIT_WINDOW_SIZE, INIT_WINDOW_SIZE);

    // Open window:
    MainWindow = glutCreateWindow(WINDOWTITLE);
    glutSetWindowTitle(WINDOWTITLE);

    // Clear values:
    glClearColor(BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3]);

    // Setup callbacks:
    glutSetWindow(MainWindow);
    glutDisplayFunc(Display);
    glutReshapeFunc(Resize);
    glutKeyboardFunc(Keyboard);
    glutMouseFunc(MouseButton);
    glutMotionFunc(MouseMotion);
    glutPassiveMotionFunc(MouseMotion);
    glutVisibilityFunc(Visibility);

    // Idle callback for animation:
    glutIdleFunc(Animate);

	// Setup GLUI:
	initGlui();


    // Setup texture(s):
	////////////////////////////////////////////////////////////
    SetUpTexture((char*)"LeafProject/mapleleaf2.bmp", &Leaf2Tex);

    // Leaf shader
    LeafProgram.Init();
    bool valid = LeafProgram.Create((char*)"leaf.vert", (char*)"leaf.frag");
    if(!valid)
        fprintf(stderr, "Error compiling leaf shader.\n");
    else
        fprintf(stderr, "Leaf shader compiled.\n");

    // Set initial lighting params in the shader
    NowKa = 0.5f;
    NowKd = 0.8f;
    NowKs = 0.6f;
    NowShine = 20.f;
    NowAlpha = 1.f;
	
    NowLeafColor[0] = 1.f;
    NowLeafColor[1] = 0.5f;
    NowLeafColor[2] = 0.f;

    LeafProgram.Use();
    LeafProgram.SetUniformVariable((char*)"uKa", NowKa);
    LeafProgram.SetUniformVariable((char*)"uKd", NowKd);
    LeafProgram.SetUniformVariable((char*)"uKs", NowKs);
    LeafProgram.SetUniformVariable((char*)"uAlpha", NowAlpha);
    LeafProgram.SetUniformVariable((char*)"uColor", NowLeafColor);
    LeafProgram.SetUniformVariable((char*)"uTranslucency", 1.f);
    LeafProgram.SetUniformVariable((char*)"uShininess", NowShine);
    LeafProgram.UnUse();

	////////////////////////////////////////////////////////////
	// GetDepth shader
    GetDepth.Init();
    valid = GetDepth.Create((char*)"GetDepth.vert", (char*)"GetDepth.frag");
    if(!valid)
        fprintf(stderr, "Error compiling GetDepth shader.\n");
    else
        fprintf(stderr, "GetDepth shader compiled.\n");

	// RenderWithShadows shader
	RenderWithShadows.Init();
	valid = RenderWithShadows.Create((char*)"RenderWithShadows.vert", (char*)"RenderWithShadows.frag");
	if(!valid)
		fprintf(stderr, "Error compiling RenderWithShadows shader.\n");
	else
		fprintf(stderr, "RenderWithShadows shader compiled.\n");
	
	////////////////////////////////////////////////////////////
	//set up shadow texture:
	// 1. Generate a framebuffer object and a depth texture:
    glGenFramebuffers(1, &DepthFramebuffer);
    glGenTextures(1, &DepthTexture);

    // 2. Create a texture to serve as the depth buffer:
    glBindTexture(GL_TEXTURE_2D, DepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 SHADOW_WIDTH, SHADOW_HEIGHT, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    // 3. Set texture parameters:
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // 4. Attach this depth texture to the FBO:
    glBindFramebuffer(GL_FRAMEBUFFER, DepthFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, DepthTexture, 0);

    // 5. We don't need any color buffer for this pass:
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    // Finally, unbind the framebuffer for now:
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
	

#ifdef WIN32
    // init GLEW
    GLenum err = glewInit();
    if(err != GLEW_OK)
    {
        fprintf(stderr, "glewInit Error\n");
    }
    else
    {
        fprintf(stderr, "GLEW initialized OK\n");
        fprintf(stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
    }
#endif
}

// Create display lists
void InitLists()
{
    if(DebugOn != 0)
        fprintf(stderr, "Starting InitLists.\n");

    // Box geometry:
    float dx = BOXSIZE / 2.f;
    float dy = BOXSIZE / 2.f;
    float dz = BOXSIZE / 2.f;
    glutSetWindow(MainWindow);

    // Grid display list
    #define XSIDE   20
    #define X0      (-XSIDE/2.)
    #define NX      400
    #define DX      (XSIDE/(float)NX)

    #define YGRID   0.f
    #define ZSIDE   20
    #define Z0      (-ZSIDE/2.)
    #define NZ      400
    #define DZ      (ZSIDE/(float)NZ)

    GridDL = glGenLists(1);
    glNewList(GridDL, GL_COMPILE);
        glColor3f(.8f, .8f, .8f);
        for(int i = 0; i < NZ; i++)
        {
            glBegin(GL_QUAD_STRIP);
            for(int j = 0; j < NX; j++)
            {
                glVertex3f(X0 + DX*(float)j, YGRID, Z0 + DZ*(float)(i));
                glVertex3f(X0 + DX*(float)j, YGRID, Z0 + DZ*(float)(i+1));
            }
            glEnd();
        }
    glEndList();

    // Sphere
    OSUSphere = glGenLists(1);
    glNewList(OSUSphere, GL_COMPILE);
        glShadeModel(GL_SMOOTH);
        OsuSphere(1.f, 20, 20);
    glEndList();

    // Leaf object
    Leaf2DL = glGenLists(1);
    glNewList(Leaf2DL, GL_COMPILE);
        LoadObjFile((char*)"LeafProject/mapleLeafShape.obj");
    glEndList();

    // Axes
    AxesList = glGenLists(1);
    glNewList(AxesList, GL_COMPILE);
        glLineWidth(AXES_WIDTH);
        Axes(1.5f);
        glLineWidth(1.f);
    glEndList();

    // A simple box list if needed for DEMO_Z_FIGHTING:
    BoxList = glGenLists(1);
    glNewList(BoxList, GL_COMPILE);
        glColor3f(1.f, 1.f, 1.f);
        glutSolidCube(BOXSIZE);
    glEndList();
}

//=============================================================================
//  11. Additional Helper Functions (Raster/Stroke Strings, Axes, etc.)
//=============================================================================
void DoRasterString(float x, float y, float z, char *s)
{
    glRasterPos3f(x, y, z);
    for(char c = *s; c != '\0'; s++, c = *s)
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
}

void DoStrokeString(float x, float y, float z, float ht, char *s)
{
    glPushMatrix();
        glTranslatef(x, y, z);
        float sf = ht / (119.05f + 33.33f);
        glScalef(sf, sf, sf);
        for(char c = *s; c != '\0'; s++, c = *s)
            glutStrokeCharacter(GLUT_STROKE_ROMAN, c);
    glPopMatrix();
}

float ElapsedSeconds()
{
    int ms = glutGet(GLUT_ELAPSED_TIME);
    return (float)ms / 1000.f;
}

void Axes(float length)
{
    glBegin(GL_LINE_STRIP);
        glVertex3f(length, 0.f, 0.f);
        glVertex3f(0.f, 0.f, 0.f);
        glVertex3f(0.f, length, 0.f);
    glEnd();

    glBegin(GL_LINE_STRIP);
        glVertex3f(0.f, 0.f, 0.f);
        glVertex3f(0.f, 0.f, length);
    glEnd();

    float fact = 0.10f * length;
    float base = 1.10f * length;
    static float xx[] = {0.f, 1.f, 0.f, 1.f};
    static float xy[] = {-0.5f, 0.5f, 0.5f, -0.5f};
    static int xorder[] = {1, 2, -3, 4};

    // X
    glBegin(GL_LINE_STRIP);
    for(int i = 0; i < 4; i++)
    {
        int j = xorder[i];
        if(j < 0)
        {
            glEnd();
            glBegin(GL_LINE_STRIP);
            j = -j;
        }
        j--;
        glVertex3f(base + fact*xx[j], fact*xy[j], 0.f);
    }
    glEnd();

    // Y
    static float yx[] = {0.f, 0.f, -0.5f, 0.5f};
    static float yy[] = {0.f, 0.6f, 1.f, 1.f};
    static int yorder[] = {1, 2, 3, -2, 4};
    glBegin(GL_LINE_STRIP);
    for(int i = 0; i < 5; i++)
    {
        int j = yorder[i];
        if(j < 0)
        {
            glEnd();
            glBegin(GL_LINE_STRIP);
            j = -j;
        }
        j--;
        glVertex3f(fact*yx[j], base + fact*yy[j], 0.f);
    }
    glEnd();

    // Z
    static float zx[] = {1.f, 0.f, 1.f, 0.f, 0.25f, 0.75f};
    static float zy[] = {0.5f, 0.5f, -0.5f, -0.5f, 0.f, 0.f};
    static int zorder[] = {1, 2, 3, 4, -5, 6};
    glBegin(GL_LINE_STRIP);
    for(int i = 0; i < 6; i++)
    {
        int j = zorder[i];
        if(j < 0)
        {
            glEnd();
            glBegin(GL_LINE_STRIP);
            j = -j;
        }
        j--;
        glVertex3f(0.f, fact*zy[j], base + fact*zx[j]);
    }
    glEnd();
}

// Convert HSV to RGB
void HsvRgb(float hsv[3], float rgb[3])
{
    float h = hsv[0] / 60.f;
    while(h >= 6.) h -= 6.;
    while(h <  0.) h += 6.;

    float s = hsv[1];
    if(s < 0.) s = 0.f;  if(s > 1.f) s = 1.f;
    float v = hsv[2];
    if(v < 0.) v = 0.f;  if(v > 1.f) v = 1.f;

    if(s == 0.0f)
    {
        rgb[0] = rgb[1] = rgb[2] = v;
        return;
    }

    float i = floor(h);
    float f = h - i;
    float p = v * (1.f - s);
    float q = v * (1.f - s*f);
    float t = v * (1.f - (s*(1.f-f)));

    switch((int)i)
    {
        case 0:  rgb[0] = v; rgb[1] = t; rgb[2] = p; break;
        case 1:  rgb[0] = q; rgb[1] = v; rgb[2] = p; break;
        case 2:  rgb[0] = p; rgb[1] = v; rgb[2] = t; break;
        case 3:  rgb[0] = p; rgb[1] = q; rgb[2] = v; break;
        case 4:  rgb[0] = t; rgb[1] = p; rgb[2] = v; break;
        case 5:  rgb[0] = v; rgb[1] = p; rgb[2] = q; break;
    }
}

// Vector cross product
void Cross(float v1[3], float v2[3], float vout[3])
{
    float tmp[3];
    tmp[0] = v1[1]*v2[2] - v2[1]*v1[2];
    tmp[1] = v2[0]*v1[2] - v1[0]*v2[2];
    tmp[2] = v1[0]*v2[1] - v2[0]*v1[1];
    vout[0] = tmp[0];
    vout[1] = tmp[1];
    vout[2] = tmp[2];
}

// Vector dot product
float Dot(float v1[3], float v2[3])
{
    return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

// Normalize vin into vout
float Unit(float vin[3], float vout[3])
{
    float dist = vin[0]*vin[0] + vin[1]*vin[1] + vin[2]*vin[2];
    if(dist > 0.f)
    {
        dist = sqrtf(dist);
        vout[0] = vin[0]/dist;
        vout[1] = vin[1]/dist;
        vout[2] = vin[2]/dist;
    }
    else
    {
        vout[0] = vin[0];
        vout[1] = vin[1];
        vout[2] = vin[2];
    }
    return dist;
}

// In-place normalization
float Unit(float v[3])
{
    float dist = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
    if(dist > 0.f)
    {
        dist = sqrtf(dist);
        v[0] /= dist;
        v[1] /= dist;
        v[2] /= dist;
    }
    return dist;
}

// Load textures from file
void SetUpTexture(char* filename, GLuint* texture)
{
    int width, height;
    unsigned char* textureData = BmpToTexture(filename, &width, &height);
    if(textureData == NULL)
    {
        fprintf(stderr, "Cannot open texture '%s'\n", filename);
        return;
    }
    else
    {
        fprintf(stderr, "Opened '%s': width = %d ; height = %d\n", filename, width, height);
    }

    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
}

// Set up scene
void DisplayOneScene( GLSLProgram program ){
    // Define the L-system:
    // (Three example rules; we'll pick one via changeRule)
    std::vector<std::string> ruleStringSet = {
        "F[>A][<A][+A][-AvA][^A]L",
        "F[>A][<A][+A][-A][vA][^A]L",
        "F[+A][-A]<A>AvA^AL"
    };
    if(changeRule >= (int)ruleStringSet.size())
        changeRule = 0;
    std::string inputRule = ruleStringSet[changeRule];

    // Axiom & rules
    std::string axiom = "A";
    std::unordered_map<char, std::string> rules = {
        {'A', inputRule}
    };
    int iterations = 5;

    // Build L-system
    LSystem lsystem(axiom, rules, iterations);
    std::string finalString = lsystem.generate();

	////////////////////////////////////////////////////////////
    // Interpret with Turtle, draw tree
    Turtle turtle;
    turtle.setInitialFactor(35.f, 10.f, 5.f, 0.5f);
    glPushMatrix();
        turtle.interpret(finalString);
    glPopMatrix();

    // Draw Leaves
    std::vector<Turtle::Leaf> leafPositions = turtle.GetLeaves();
    for(const auto& leaf : leafPositions)
    {
        glPushMatrix();
            // Translate to leaf position
            glTranslatef(leaf.position.x, leaf.position.y, leaf.position.z);

            // Orientation
            glm::mat3 orientationMatrix;
            orientationMatrix[0] = glm::normalize(leaf.right); 
            orientationMatrix[1] = glm::normalize(leaf.up);
            orientationMatrix[2] = glm::normalize(glm::cross(leaf.right, leaf.up));
            glm::mat4 rotationMatrix = glm::mat4(orientationMatrix);
            glMultMatrixf(glm::value_ptr(rotationMatrix));

            // Scale
            float scale = 5.f;
            glScalef(scale, scale, scale);

            // Color variation
            float randval = leaf.position.y / 60.f;
            if(randval < 0.30f) {
                NowLeafColor[0] = 1.0f;  NowLeafColor[1] = 0.55f; NowLeafColor[2] = 0.0f;
            } else if(randval < 0.50f) {
                NowLeafColor[0] = 1.0f;  NowLeafColor[1] = 0.6f;  NowLeafColor[2] = 0.2f;
            } else if(randval < 0.80f) {
                NowLeafColor[0] = 0.8f;  NowLeafColor[1] = 0.1f;  NowLeafColor[2] = 0.1f;
            } else if(randval < 0.90f) {
                NowLeafColor[0] = 0.85f; NowLeafColor[1] = 0.2f;  NowLeafColor[2] = 0.1f;
            } else {
                NowLeafColor[0] = 0.7f;  NowLeafColor[1] = 0.0f;  NowLeafColor[2] = 0.0f;
            }

            LeafProgram.Use();
            LeafProgram.SetUniformVariable((char*)"uColor", NowLeafColor);
            glCallList(Leaf2DL);
            LeafProgram.UnUse();

        glPopMatrix();
    }
    LeafProgram.UnUse();
    glDisable(GL_TEXTURE_2D);
}

void GluiControlCallback(int controlID)
{
    // This is called whenever a GLUI widget changes a variable
    // For a spinner/slider changing NowKa, the new value is already in NowKa.
    // You can force a redisplay if needed:
    glutSetWindow(mainWindow);
    glutPostRedisplay();
}

void initGlui()
{
    // Create a GLUI subwindow:
    // NOTE: use a const char*, or cast the string
    glui = GLUI_Master.create_glui(const_cast<char *>("Controls"), 0, 800, 50);

    // Optional: add a panel to group your spinners
    GLUI_Panel *panel = glui->add_panel("Material");

    // Ambient (Ka) spinner:
    GLUI_Spinner *kaSpinner = 
       glui->add_spinner_to_panel(panel, "Ambient (Ka)", 
                                  GLUI_SPINNER_FLOAT, &NowKa, 
                                  /*controlID*/ 1, 
                                  GluiControlCallback);
    kaSpinner->set_float_limits(0.0, 1.0, GLUI_LIMIT_CLAMP);

    // Diffuse (Kd):
    GLUI_Spinner *kdSpinner = 
       glui->add_spinner_to_panel(panel, "Diffuse (Kd)", 
                                  GLUI_SPINNER_FLOAT, &NowKd, 
                                  /*controlID*/ 2, 
                                  GluiControlCallback);
    kdSpinner->set_float_limits(0.0, 1.0, GLUI_LIMIT_CLAMP);

    // Specular (Ks):
    GLUI_Spinner *ksSpinner = 
       glui->add_spinner_to_panel(panel, "Specular (Ks)",
                                  GLUI_SPINNER_FLOAT, &NowKs, 
                                  /*controlID*/ 3, 
                                  GluiControlCallback);
    ksSpinner->set_float_limits(0.0, 1.0, GLUI_LIMIT_CLAMP);

    // Shine
    GLUI_Spinner *shineSpinner = 
       glui->add_spinner_to_panel(panel, "Shine", 
                                  GLUI_SPINNER_FLOAT, &NowShine, 
                                  /*controlID*/ 4, 
                                  GluiControlCallback);
    shineSpinner->set_float_limits(0.0, 100.0, GLUI_LIMIT_CLAMP);

    // Alpha
    GLUI_Spinner *alphaSpinner =
       glui->add_spinner_to_panel(panel, "Alpha", 
                                  GLUI_SPINNER_FLOAT, &NowAlpha, 
                                  /*controlID*/ 5,
                                  GluiControlCallback);
    alphaSpinner->set_float_limits(0.0, 1.0, GLUI_LIMIT_CLAMP);

    // Optionally add a Quit button:
    glui->add_button("Quit", 0, (GLUI_Update_CB)exit);

    // Tie the GLUI window to your main GLUT window:
    glui->set_main_gfx_window(MainWindow);
}