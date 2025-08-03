#include "2005024_classes.h"
#include "bitmap_image.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Vector3D eye(100.0f, 60.0f, 40.0f);
Vector3D center(0.0f,0.0f,0.0f);
Vector3D up(0.0f, 1.0f, 0.0f);

Camera camera(eye, center, up);

vector<Object*> objects;
vector<PointLight> pointLights;
vector<SpotLight> spotLights;
int recursion_level;
int imageWidth, imageHeight;
int animationSpeed = 16;
// Texture data for the floor
unsigned char* textureData = nullptr;
int textureWidth = 0, textureHeight = 0, textureChannels = 0;
bool useTexture = false;


void initGL();
void display();
void reshapeListener(GLsizei width, GLsizei height);
void keyboardListener(unsigned char key, int x, int y);
void specialKeyListener(int key, int x, int y);

bool loadFloorTexture(const char* filename) {
    if (textureData) {
        stbi_image_free(textureData);
        textureData = nullptr;
    }
    textureData = stbi_load(filename, &textureWidth, &textureHeight, &textureChannels, 3);
    if (!textureData) {
        printf("Failed to load texture: %s\n", filename);
        useTexture = false;
        return false;
    }
    useTexture = true;
    printf("Loaded texture: %s (%d x %d)\n", filename, textureWidth, textureHeight);
    return true;
}

void loadData() {
    ifstream file("scene_test.txt");
    if (!file.is_open()) {
        cout << "Error: Could not open scene_test.txt file" << endl;
        return;
    }
    
    cout << "Loading scene..." << "\n";
    file >> recursion_level;

    file >> imageWidth;
    imageHeight = imageWidth;

    int numObjects;
    file >> numObjects;

    for (int i = 0; i < numObjects; i++) {
        string objectType;
        file >> objectType;
        
        Object* obj = nullptr;
        
        if (objectType == "sphere") {
            double centerX, centerY, centerZ, radius;
            file >> centerX >> centerY >> centerZ >> radius;
            
            Vector3D center(centerX, centerY, centerZ);
            obj = new Sphere(center, radius);
            
        } 
        else if (objectType == "triangle") {
            double x1, y1, z1, x2, y2, z2, x3, y3, z3;
            file >> x1 >> y1 >> z1 >> x2 >> y2 >> z2 >> x3 >> y3 >> z3;
            
            Vector3D a(x1, y1, z1);
            Vector3D b(x2, y2, z2);
            Vector3D c(x3, y3, z3);
            obj = new Triangle(a, b, c);
            
        } 
        else if (objectType == "general") {
            double A, B, C, D, E, F, G, H, I, J;
            file >> A >> B >> C >> D >> E >> F >> G >> H >> I >> J;

            double refX, refY, refZ, length, width, height;
            file >> refX >> refY >> refZ >> length >> width >> height;
            
            Vector3D reference(refX, refY, refZ);
            obj = new GeneralQuadric(A, B, C, D, E, F, G, H, I, J, 
                                   reference, length, width, height);
        }
        
        if (obj != nullptr) {
            double r, g, b;
            file >> r >> g >> b;
            obj->setColor(r, g, b);
            
            double ambient, diffuse, specular, reflection;
            file >> ambient >> diffuse >> specular >> reflection;
            obj->setCoEfficients(ambient, diffuse, specular, reflection);
            
            int shine;
            file >> shine;
            obj->setShine(shine);
            
            objects.push_back(obj);
        }
    }
    
    Object* floor = new Floor(1000, 20);
    floor->setColor(1.0, 1.0, 1.0);
    floor->setCoEfficients(0.3, 0.3, 0.2, 0.2);
    floor->setShine(40);
    objects.push_back(floor);

    int numPointLights;
    file >> numPointLights;

    for (int i = 0; i < numPointLights; i++) {
        double posX, posY, posZ;
        double colorR, colorG, colorB;
        
        file >> posX >> posY >> posZ;
        file >> colorR >> colorG >> colorB;
        
        Vector3D position(posX, posY, posZ);
        PointLight pl(position, colorR, colorG, colorB);
        pointLights.push_back(pl);
    }
    
    int numSpotLights;
    file >> numSpotLights;
    
    for (int i = 0; i < numSpotLights; i++) {
        double posX, posY, posZ;
        double colorR, colorG, colorB;
        double dirX, dirY, dirZ;
        double cutoffAngle;
        
        file >> posX >> posY >> posZ;
        file >> colorR >> colorG >> colorB;
        file >> dirX >> dirY >> dirZ;
        file >> cutoffAngle;
        
        Vector3D position(posX, posY, posZ);
        Vector3D direction(dirX, dirY, dirZ);
        direction.normalize();

        SpotLight sl(position, direction, cutoffAngle, colorR, colorG, colorB);
        spotLights.push_back(sl);
    }
    
    file.close();
    cout << "Scene loaded successfully!" << endl;
    cout << "Objects: " << objects.size() << endl;
    cout << "Point Lights: " << pointLights.size() << endl;
    cout << "Spot Lights: " << spotLights.size() << endl;
    cout << "Recursion Level: " << recursion_level << endl;
}

void capture() {

    static int imageCount = 1;
    
    bitmap_image image(imageWidth, imageHeight);
    image.set_all_channels(0, 0, 0);

    double windowHeight = 500.0;
    double windowWidth = 500.0;
    double viewAngle = 80.0 * M_PI / 180.0;

    double planeDistance = (windowHeight / 2.0) / tan(viewAngle / 2.0);

    Vector3D l = camera.center - camera.eye;
    l.normalize();
    
    Vector3D r = l.cross(camera.up);
    r.normalize();
    
    Vector3D u = r.cross(l);     
    u.normalize();

    Vector3D topleft = camera.eye + l * planeDistance - r * (windowWidth / 2) + u * (windowHeight / 2);

    double du = windowWidth / imageWidth;
    double dv = windowHeight / imageHeight;

    topleft = topleft + r * (0.5 * du) - u * (0.5 * dv);
    
    cout << "Capturing image " << imageCount << "..." << endl;
    
    for (int i = 0; i < imageWidth; i++) {
        for (int j = 0; j < imageHeight; j++) {
            Vector3D curPixel = topleft + r * (i * du) - u * (j * dv);
            
            Vector3D rayDir = curPixel - camera.eye;
            rayDir.normalize();
            Ray ray(camera.eye, rayDir);
            
            double tMin = -1.0;
            Object* nearestObject = nullptr;
            
            for (Object* obj : objects) {
                double dummyColor[3] = {0, 0, 0};
                double t = obj->intersect(&ray, dummyColor, 0);
                
                if (t > 0 && (tMin < 0 || t < tMin)) {
                    tMin = t;
                    nearestObject = obj;
                }
            }
            
            double finalColor[3] = {0, 0, 0};
            
            if (nearestObject != nullptr) {
                nearestObject->intersect(&ray, finalColor, 1);
            }

            for (int k = 0; k < 3; k++) {
                finalColor[k] = max(0.0, min(1.0, finalColor[k]));
            }

            unsigned char red = (unsigned char)(finalColor[0] * 255);
            unsigned char green = (unsigned char)(finalColor[1] * 255);
            unsigned char blue = (unsigned char)(finalColor[2] * 255);
            
            image.set_pixel(i, j, red, green, blue);
        }
    }
    
    string filename = "Output_1" + to_string(imageCount) + ".bmp";
    image.save_image(filename);
    
    cout << "Image saved as: " << filename << endl;
    imageCount++;
}


void initGL()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black background
    glEnable(GL_DEPTH_TEST);              // Enable depth testing for z-culling
}

void keyboardListener(unsigned char key, int x, int y)
{

    switch (key)
    {
    // --- Camera Position Controls (eye coordinates) ---
    case '0':
        capture();
        break;
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
    case 't':
        useTexture = !useTexture;
        printf("Floor texture %s.\n", useTexture ? "enabled" : "disabled");
        break;


    // --- Program Control ---
    case 27:
        exit(0);
        break; // ESC key: exit program
    }

    glutPostRedisplay(); // Request a screen refresh
}

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
    gluPerspective(45.0f, aspect, 0.1f, 5000.0f);
}


void timerFunction(int value)
{
    glutPostRedisplay(); // Request a screen refresh
    glutTimerFunc(animationSpeed, timerFunction, 0); // Call this function again after 16 ms (~60 FPS)
}

void display()
{
    // Clear color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up the model-view matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //glRotatef(-90, 1, 0, 0); 
    // Position camera using the eye, center and up vectors
    gluLookAt(camera.eye.x, camera.eye.y, camera.eye.z,          // Camera position
              camera.center.x, camera.center.y, camera.center.z, // Look-at point
              camera.up.x, camera.up.y, camera.up.z);            // Up vector

    
    for(Object* obj : objects){
        obj->draw();
    }
    // Swap buffers (double buffering)
    glutSwapBuffers();
}

void cleanup() {
    if (textureData) {
        stbi_image_free(textureData);
        textureData = nullptr;
    }

    for (Object* obj : objects) {
        Floor* floor = dynamic_cast<Floor*>(obj);
        if (floor && floor->textureID > 0) {
            glDeleteTextures(1, &floor->textureID);
        }
    }
    for (Object* obj : objects) {
        delete obj;
    }
    objects.clear();
    pointLights.clear();
    spotLights.clear();
}

int main(int argc, char **argv){

    loadData();
    loadFloorTexture("../texture/floor_texture2.jpg");

    if (useTexture) {
        for (Object* obj : objects) {
            Floor* floor = dynamic_cast<Floor*>(obj);
            if (floor) {
                floor->loadTextureForOpenGL();
                break;
            }
        }
    }

    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 800);
    glutInitWindowPosition(50, 50);
    glutCreateWindow("OpenGL 3D Drawing");

    glutDisplayFunc(display);
    glutReshapeFunc(reshapeListener);
    glutKeyboardFunc(keyboardListener);
    glutSpecialFunc(specialKeyListener);

    glutTimerFunc(animationSpeed, timerFunction, 0);


    initGL();

    atexit(cleanup); 

    glutMainLoop();

    return 0;
}