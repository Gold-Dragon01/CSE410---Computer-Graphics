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

Point eye(4.5f, 4.5f, 4.5f);
Point center(2.0f,2.0f,2.0f);
Point up(0.0f, 1.0f, 0.0f);

Camera camera(eye, center, up);
Ball ball;
int animationSpeed = 10;
const double GRAVITY_CONST = 9.8;
const double RESTITUTION = 0.75;
// Object visibility flags
// bool isAxes = true;     // Toggle for coordinate axes
// bool isCube = true;    // Toggle for cube
// bool isPyramid = false; // Toggle for pyramid
bool simulation = false; // Toggle for simulation
bool show_arrow = false;
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
void drawArrow();
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
 * Updates the ball's position and handles collisions with the cube walls.
 */

void updateBall(double deltaTime)
{
    // Update ball position based on velocity
    if(ball.velocity.y > 0)
        ball.velocity.y -= GRAVITY_CONST * deltaTime; // Gravity effect
    
    ball.center.x += ball.velocity.x * deltaTime;
    ball.center.y += ball.velocity.y * deltaTime;
    ball.center.z += ball.velocity.z * deltaTime;

    double halfCube = cubeLength / 2.0;

    // Check for collisions with the cube walls and reflect velocity
    if (ball.center.x + ball.radius >= halfCube || ball.center.x - ball.radius <= -halfCube)
    {
        ball.velocity.x = -ball.velocity.x; // Reflect velocity in X direction
        ball.center.x = (ball.center.x > 0) ? halfCube - ball.radius : -halfCube + ball.radius;
    }
    if (ball.center.y + ball.radius > halfCube || ball.center.y - ball.radius < -halfCube)
    {
        ball.velocity.y = -ball.velocity.y; // Reflect velocity in Y direction
        ball.center.y = (ball.center.y > 0) ? halfCube - ball.radius : -halfCube + ball.radius;
    }
    if (ball.center.z + ball.radius >= halfCube || ball.center.z - ball.radius <= -halfCube)
    {
        ball.velocity.z = -ball.velocity.z; // Reflect velocity in Z direction
        ball.center.z = (ball.center.z > 0) ? halfCube - ball.radius : -halfCube + ball.radius;
    }

    ball.rotationAxis = ball.velocity.cross(Point(0.0, 1.0, 0.0)); // Cross product with Y-axis
    ball.rotationAngle += ball.velocity.length() * deltaTime * 360.0 / (2 * M_PI * ball.radius); // Angle in degrees
    if (ball.rotationAngle > 360.0) {
        ball.rotationAngle -= 360.0; // Keep angle within 0-360 degrees
    }
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
    
    if(simulation){
        double deltaTime = 0.02; // Time step for simulation
        updateBall(deltaTime);
    }
    // updateBallRotation();
    //drawAxes();
    drawCube();
    drawBall();
    if (show_arrow)
        drawArrow();
    // if (isAxes)
    //     drawAxes();
    // if (isPyramid)
    //     drawPyramid();

    // Swap buffers (double buffering)
    glutSwapBuffers();
}

void timerFunction(int value)
{
    glutPostRedisplay(); // Request a screen refresh
    glutTimerFunc(animationSpeed, timerFunction, 0); // Call this function again after 16 ms (~60 FPS)
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
    gluPerspective(90.0f, aspect, 0.1f, 4000.0f);
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
        if(!simulation)
            ball.setRandomCenter();
        break;
    case 'v':
        show_arrow = !show_arrow;
        break;

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

/**
 * Draws an arrow in the direction of the velocity vector from the center of the ball.
 *
 * @param center The center of the ball
 * @param velocity The velocity vector of the ball
 */
void drawArrow()
{
    Point center = ball.center;
    Point velocity = ball.velocity;
    // Normalize the velocity vector to get the direction
    double length = velocity.length();
    Point direction = Point(velocity.x / length, velocity.y / length, velocity.z / length);

    // Scale the arrow length (you can adjust the scale factor)
    double arrowLength = 0.5; // Length of the arrow
    Point arrowTip = Point(
        center.x + direction.x * arrowLength,
        center.y + direction.y * arrowLength,
        center.z + direction.z * arrowLength
    );

    // Draw the arrow shaft (line)
    glColor3f(1.0f, 1.0f, 0.0f); // Yellow color for the arrow
    glBegin(GL_LINES);
    glVertex3f(center.x, center.y, center.z); // Start at the ball's center
    glVertex3f(arrowTip.x, arrowTip.y, arrowTip.z); // End at the arrow tip
    glEnd();

    // Draw the arrowhead (triangle)
    glPushMatrix();
    {
        // Translate to the arrow tip
        glTranslatef(arrowTip.x, arrowTip.y, arrowTip.z);

        // Rotate to align with the velocity vector
        double angle = acos(direction.z) * 180.0 / M_PI; // Angle with the Z-axis
        Point rotationAxis = Point(-direction.y, direction.x, 0.0); // Perpendicular to the velocity vector
        glRotatef(angle, rotationAxis.x, rotationAxis.y, rotationAxis.z);

        // Draw a cone for the arrowhead
        glutSolidCone(0.02, 0.05, 10, 10); // Base radius = 0.1, height = 0.2
    }
    glPopMatrix();
}

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
        glRotatef(-ball.rotationAngle, ball.rotationAxis.x, ball.rotationAxis.y, ball.rotationAxis.z);
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

    drawCheckerBoard(15, cubeLength / 15.0, cubeLength);

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

    glutTimerFunc(animationSpeed, timerFunction, 0);


    // Initialize OpenGL settings
    initGL();

    // Enter the GLUT event loop
    glutMainLoop();

    return 0;
}