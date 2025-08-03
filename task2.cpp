/**
 * OpenGL 3D Drawing Demo
 *
 * This program demonstrates basic 3D rendering with OpenGL and GLUT including:
 * - Camera positioning with gluLookAt
 * - Drawing 3D shapes (cube and pyramid)
 * - Keyboard navigation for camera control
 * - Perspective projection
 * - Object toggling
 */

// --- Includes ---
// Standard Headers
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "helper.h"
// OpenGL / GLUT Headers
#ifdef __APPLE__
#include <GLUT/glut.h> // Use GLUT framework on macOS
#else
#include <GL/glut.h> // Use standard GLUT location on Linux/Windows
#endif


// --- Global Variables ---
// Camera position and orientation
GLfloat cubeLength = 4.0f;

Point eye(5.0f, 5.0f, 5.0f);
Point center(4.8f,4.8f,4.8f);
Point up(0.0f, 1.0f, 0.0f);

Camera camera(eye, center, up);
Ball ball;
// Object visibility flags
// bool isAxes = true;     // Toggle for coordinate axes
// bool isCube = true;    // Toggle for cube
// bool isPyramid = false; // Toggle for pyramid
bool simulation = false; // Toggle for simulation
// --- Function Declarations ---
void initGL();
void display();
void reshapeListener(GLsizei width, GLsizei height);
void keyboardListener(unsigned char key, int x, int y);
void specialKeyListener(int key, int x, int y);
void drawAxes();
void drawCube();
void drawPyramid();
void drawCheckerboard();
void drawBall();
/**
 * Initialize OpenGL settings
 * Sets up background color and enables depth testing
 */
void initGL()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black background
    glEnable(GL_DEPTH_TEST);              // Enable depth testing for z-culling
}

bool isInsideCube(){
    return (fabs(camera.eye.x) < cubeLength/2 
            && fabs(camera.eye.y) < cubeLength/2 
            && fabs(camera.eye.z) < cubeLength/2);
}

/**
 * Main display function
 * Sets up the camera and renders visible objects
 */
void display()
{
    // Clear color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up the model-view matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Position camera using the eye, center and up vectors
    gluLookAt(camera.eye.x, camera.eye.y, camera.eye.z,          // Camera position
              camera.center.x, camera.center.y, camera.center.z, // Look-at point
              camera.up.x, camera.up.y, camera.up.z);            // Up vector

    // Draw objects based on visibility flags
    drawCube();
    drawBall();
    // if (isAxes)
    //     drawAxes();
    // if (isPyramid)
    //     drawPyramid();

    // Swap buffers (double buffering)
    glutSwapBuffers();
}

/**
 * Window reshape callback
 * Handles window resizing and maintains aspect ratio
 */
void reshapeListener(GLsizei width, GLsizei height)
{
    // Prevent division by zero
    if (height == 0)
        height = 1;

    // Calculate aspect ratio
    GLfloat aspect = (GLfloat)width / (GLfloat)height;

    // Set viewport to cover entire window
    glViewport(0, 0, width, height);

    // Set up perspective projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // 45-degree field of view, aspect ratio, near and far clipping planes
    gluPerspective(45.0f, aspect, 0.1f, 100.0f);
}

/**
 * Keyboard input handler for standard keys
 * Manages camera position, object visibility, and program exit
 */
void keyboardListener(unsigned char key, int x, int y)
{

    switch (key)
    {
    // --- Camera Position Controls (eye coordinates) ---
    case '1':
        camera.lookLeft();
        break; // Move eye right
    case '2':
        camera.lookRight();
        break; // Move eye left
    case '3':
        camera.lookUp();
        break; // Move eye up
    case '4':
        camera.lookDown();
        break; // Move eye down
    case '5':
        camera.tiltClockWise();
        break; // Move eye forward
    case '6':
        camera.tiltCounterClockWise();
        break; // Move eye backward

    // --- Look-at Point Controls (center coordinates) ---
    case 'w':
        camera.moveUpFixRefPoint();
        break;
    case 's':
        camera.moveDownFixRefPoint();
        break;
    case 'r':
        
        break; // Move look-at point down

    // --- Object Visibility Toggles ---
    case ' ':
        simulation = !simulation;
        break; // Toggle simulation

    // --- Program Control ---
    case 27:
        exit(0);
        break; // ESC key: exit program
    }

    glutPostRedisplay(); // Request a screen refresh
}

/**
 * Special key input handler (arrow keys, function keys)
 * Provides camera orbit functionality
 */
void specialKeyListener(int key, int x, int y)
{
    switch (key)
    {
    case GLUT_KEY_LEFT:
        camera.moveLeft();
        break;

    case GLUT_KEY_RIGHT:
        camera.moveRight();
        break;

    case GLUT_KEY_UP:
        camera.moveForward();
        break;

    case GLUT_KEY_DOWN:
        camera.moveBackward();
        break;

    case GLUT_KEY_PAGE_UP:
        camera.moveUp();
        break;

    case GLUT_KEY_PAGE_DOWN:
        camera.moveDown();
        break;
    }

    glutPostRedisplay(); // Request a screen refresh
}

/**
 * Draw coordinate axes
 * X axis: red, Y axis: green, Z axis: blue
 */

void drawCheckerBoard(int boardSize, double squareSize, double cubeLength)
{
    double halfCube = cubeLength / 2.0;
    for (int i = 0; i < boardSize; i++){
        for (int j = 0; j < boardSize; j++){
            if ((i + j) % 2 == 0){
                glColor3f(1.0f, 1.0f, 1.0f);
            }
            else{
                glColor3f(0.0f, 0.0f, 0.0f);
            }
            // Calculate the corners of the square
            double x = -halfCube + i * squareSize;
            double z = -halfCube + j * squareSize;

            // Draw the square
            glBegin(GL_QUADS);
            {
                glVertex3f(x, -halfCube, z);                 // Bottom-left
                glVertex3f(x + squareSize, -halfCube, z);   // Bottom-right
                glVertex3f(x + squareSize, -halfCube, z + squareSize); // Top-right
                glVertex3f(x, -halfCube, z + squareSize);   // Top-left
            }
            glEnd();
        }
    }
}

void drawBall()
{
    glPushMatrix();
    {
        glTranslatef(ball.center.x, ball.center.y, ball.center.z);
        int slices = 20, stacks = 20,col = 0;

        for (int j = 0; j < slices; j++){
            col = j % 2;
            // Draw a longitude strip (slice)
            glBegin(GL_QUAD_STRIP);
            for (int i = 0; i <= stacks; i++){
                if(i == stacks/2) col ^= 1;
                if(col == 0){
                    glColor3f(1.0f, 0.0f, 0.0f);
                }
                else{
                    glColor3f(0.0f, 1.0f, 0.0f);
                }
                double phi = (double)i / stacks * M_PI;        // Latitude angle
                double theta1 = (double)j / slices * 2.0 * M_PI; // Longitude angle (current slice)
                double theta2 = (double)(j + 1) / slices * 2.0 * M_PI; // Longitude angle (next slice)

                // Vertices for the current and next slice
                double x1 = ball.radius * sin(phi) * cos(theta1);
                double y1 = ball.radius * cos(phi);
                double z1 = ball.radius * sin(phi) * sin(theta1);

                double x2 = ball.radius * sin(phi) * cos(theta2);
                double y2 = ball.radius * cos(phi);
                double z2 = ball.radius * sin(phi) * sin(theta2);

                glVertex3f(x1, y1, z1); // Vertex on the current slice
                glVertex3f(x2, y2, z2); // Vertex on the next slice
            }
            glEnd();
        }
    }
    glPopMatrix();
}

/**
 * Draw a colored cube centered at the origin
 * Each face has a different color
 */
void drawCube()
{
    glBegin(GL_QUADS);

    // Top face (y = 1.0f) - Green
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(cubeLength/2, cubeLength/2, -cubeLength/2);
    glVertex3f(-cubeLength/2, cubeLength/2, -cubeLength/2);
    glVertex3f(-cubeLength/2, cubeLength/2, cubeLength/2);
    glVertex3f(cubeLength/2, cubeLength/2, cubeLength/2);

    // Bottom face (y = -1.0f) - Orange
    // glColor3f(1.0f, 0.5f, 0.0f);
    // glVertex3f(cubeLength/2, -cubeLength/2,cubeLength/2);
    // glVertex3f(-cubeLength/2, -cubeLength/2, cubeLength/2);
    // glVertex3f(-cubeLength/2,-cubeLength/2, -cubeLength/2);
    // glVertex3f(cubeLength/2, -cubeLength/2, -cubeLength/2);

    // Front face  (z = 1.0f) - Red
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(cubeLength/2, cubeLength/2, cubeLength/2);
    glVertex3f(-cubeLength/2, cubeLength/2, cubeLength/2);
    glVertex3f(-cubeLength/2, -cubeLength/2, cubeLength/2);
    glVertex3f(cubeLength/2, -cubeLength/2, cubeLength/2);

    // Back face (z = -1.0f) - Yellow
    glColor3f(1.0f, 1.0f, 0.0f);
    glVertex3f(cubeLength/2, -cubeLength/2, -cubeLength/2);
    glVertex3f(-cubeLength/2, -cubeLength/2, -cubeLength/2);
    glVertex3f(-cubeLength/2, cubeLength/2, -cubeLength/2);
    glVertex3f(cubeLength/2, cubeLength/2, -cubeLength/2);

    // Left face (x = -1.0f) - Blue
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-cubeLength/2, cubeLength/2, cubeLength/2);
    glVertex3f(-cubeLength/2, cubeLength/2, -cubeLength/2);
    glVertex3f(-cubeLength/2, -cubeLength/2, -cubeLength/2);
    glVertex3f(-cubeLength/2, -cubeLength/2, cubeLength/2);

    // Right face (x = 1.0f) - Magenta
    glColor3f(1.0f, 0.0f, 1.0f);
    glVertex3f(cubeLength/2, cubeLength/2, -cubeLength/2);
    glVertex3f(cubeLength/2, cubeLength/2, cubeLength/2);
    glVertex3f(cubeLength/2, -cubeLength/2, cubeLength/2);
    glVertex3f(cubeLength/2, -cubeLength/2, -cubeLength/2);

    drawCheckerBoard(10, cubeLength / 10.0, cubeLength);

    glEnd();
}

/**
 * Draw a pyramid with color gradients
 * Base at y=-1, apex at y=1
 */
void drawPyramid()
{
    glBegin(GL_TRIANGLES);

    // Front face - Red to green to blue gradient
    glColor3f(1.0f, 0.0f, 0.0f); // Red (apex)
    glVertex3f(0.0f, 1.0f, 0.0f);
    glColor3f(0.0f, 1.0f, 0.0f); // Green (front-left)
    glVertex3f(-1.0f, -1.0f, 1.0f);
    glColor3f(0.0f, 0.0f, 1.0f); // Blue (front-right)
    glVertex3f(1.0f, -1.0f, 1.0f);

    // Right face - Red to blue to green gradient
    glColor3f(1.0f, 0.0f, 0.0f); // Red (apex)
    glVertex3f(0.0f, 1.0f, 0.0f);
    glColor3f(0.0f, 0.0f, 1.0f); // Blue (front-right)
    glVertex3f(1.0f, -1.0f, 1.0f);
    glColor3f(0.0f, 1.0f, 0.0f); // Green (back-right)
    glVertex3f(1.0f, -1.0f, -1.0f);

    // Back face - Red to green to blue gradient
    glColor3f(1.0f, 0.0f, 0.0f); // Red (apex)
    glVertex3f(0.0f, 1.0f, 0.0f);
    glColor3f(0.0f, 1.0f, 0.0f); // Green (back-right)
    glVertex3f(1.0f, -1.0f, -1.0f);
    glColor3f(0.0f, 0.0f, 1.0f); // Blue (back-left)
    glVertex3f(-1.0f, -1.0f, -1.0f);

    // Left face - Red to blue to green gradient
    glColor3f(1.0f, 0.0f, 0.0f); // Red (apex)
    glVertex3f(0.0f, 1.0f, 0.0f);
    glColor3f(0.0f, 0.0f, 1.0f); // Blue (back-left)
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glColor3f(0.0f, 1.0f, 0.0f); // Green (front-left)
    glVertex3f(-1.0f, -1.0f, 1.0f);

    glEnd();
}

/**
 * Main function: Program entry point
 */
int main(int argc, char **argv)
{
    // Initialize GLUT
    glutInit(&argc, argv);

    // Configure display mode and window
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 800);
    glutInitWindowPosition(50, 50);
    glutCreateWindow("OpenGL 3D Drawing");

    // Register callback functions
    glutDisplayFunc(display);
    glutReshapeFunc(reshapeListener);
    glutKeyboardFunc(keyboardListener);
    glutSpecialFunc(specialKeyListener);

    // Initialize OpenGL settings
    initGL();

    // Enter the GLUT event loop
    glutMainLoop();

    return 0;
}