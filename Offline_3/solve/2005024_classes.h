#include<bits/stdc++.h>
#ifdef __linux__
#include <GL/glut.h> // For Linux systems
#elif defined(_WIN32) || defined(WIN32)
#include <windows.h>
#include <GL/glut.h> // For Windows systems
#elif defined(__APPLE__)
#include <GLUT/glut.h> // For macOS systems
#else
#include <GL/glut.h> // Default fallback
#endif
using namespace std;

const double EPSILON = 1e-6;

extern unsigned char* textureData;
extern int textureWidth, textureHeight, textureChannels;
extern bool useTexture;

void sampleFloorTexture(double u, double v, double* outColor) {
    if (!textureData || textureWidth <= 0 || textureHeight <= 0) {
        outColor[0] = outColor[1] = outColor[2] = 0.5;
        return;
    }
    u = max(0.0, min(1.0, u));
    v = max(0.0, min(1.0, v));
    int pixel_x = (int)(u * (textureWidth - 1));
    int pixel_y = (int)((1.0 - v) * (textureHeight - 1));
    int idx = (pixel_y * textureWidth + pixel_x) * 3;
    outColor[0] = textureData[idx] / 255.0;
    outColor[1] = textureData[idx + 1] / 255.0;
    outColor[2] = textureData[idx + 2] / 255.0;
}

struct Vector3D{
    double x,y,z;
    Vector3D(double x, double y, double z){
        this->x = x;
        this->y = y;
        this->z = z;
    }
    Vector3D(){
        this->x = 0;
        this->y = 0;
        this->z = 0;
    }
    Vector3D(const Vector3D &p){
        this->x = p.x;
        this->y = p.y;
        this->z = p.z;
    }
    void add(const Vector3D &p){
        this->x += p.x;
        this->y += p.y;
        this->z += p.z;
    }
    void sub(const Vector3D &p){
        this->x -= p.x;
        this->y -= p.y;
        this->z -= p.z;
    }
    void mul(double a){
        this->x *= a;
        this->y *= a;
        this->z *= a;
    }
    Vector3D cross(Vector3D p){
        return Vector3D(this->y * p.z - this->z * p.y,
                     this->z * p.x - this->x * p.z,
                     this->x * p.y - this->y * p.x);
    }
    double dot(Vector3D p){
        return this->x * p.x + this->y * p.y + this->z * p.z;
    }
    double length(){
        return sqrt(x*x + y*y + z*z);
    }
    double getAngle(Vector3D p){
        double dotProduct = this->dot(p);
        return acos(dotProduct / (this->length()* p.length()));
    }
    void rotate(Vector3D axis, double angle){
        double axisLength = axis.length();
        axis.x /= axisLength;
        axis.y /= axisLength;
        axis.z /= axisLength;

        double cosTheta = cos(angle);
        double sinTheta = sin(angle);
        double dotProduct = this->dot(axis);

        Vector3D p = this->cross(axis);
        p.mul(sinTheta);
        axis.mul(dotProduct * (1 - cosTheta));
        this->mul(cosTheta);
        this->add(p);
        this->add(axis);
    }
    void normalize(){
        double len = this->length();
        if(len == 0) return;
        this->x /= len;
        this->y /= len;
        this->z /= len;
    }
    Vector3D operator+(const Vector3D &p) const {
        return Vector3D(this->x + p.x, this->y + p.y, this->z + p.z);
    }
    Vector3D operator-(const Vector3D &p) const {
        return Vector3D(this->x - p.x, this->y - p.y, this->z - p.z);
    }
    Vector3D operator*(double a) const {
        return Vector3D(this->x * a, this->y * a, this->z * a);
    }
    Vector3D operator/(double a) const {
        if (a == 0) return Vector3D(0, 0, 0);
        return Vector3D(this->x / a, this->y / a, this->z / a);
    }  
    Vector3D operator-() const {
        return Vector3D(-x, -y, -z);
    }  
};

struct Camera{
    Vector3D eye,center,up;
    double v = 0.1;
    double rotationAngle = 2*M_PI / 180.0;
    Camera(Vector3D eye,Vector3D center,Vector3D up){
        this->eye = eye;
        this->center = center;
        this->up = up;
    }
    Camera(){
        this->eye = Vector3D(0,0,0);
        this->center = Vector3D(0,0,-1);
        this->up = Vector3D(0,1,0);
    }
    void increaseVelocity(double a){
        this->v += a;
    }
    void moveForward(){
        Vector3D dir = Vector3D(center.x - eye.x, center.y - eye.y, center.z - eye.z);
        dir.mul(v);
        eye.add(dir);
        center.add(dir);
    }
    void moveBackward(){
        Vector3D dir = Vector3D(center.x - eye.x, center.y - eye.y, center.z - eye.z);
        dir.mul(-v);
        eye.add(dir);
        center.add(dir);
    }
    void moveUp(){
        Vector3D dir = Vector3D(up);
        dir.mul(v);
        eye.add(dir);
        center.add(dir);
    }
    void moveDown(){
        Vector3D dir = Vector3D(up);
        dir.mul(-v);
        eye.add(dir);
        center.add(dir);
    }
    void moveUpFixRefVector3D(){
        Vector3D forward = Vector3D(center.x - eye.x, center.y - eye.y, center.z - eye.z);
        Vector3D left = up.cross(forward);
        forward.rotate(left, rotationAngle);
        up.rotate(left, rotationAngle);
        eye.x = center.x - forward.x;
        eye.y = center.y - forward.y;
        eye.z = center.z - forward.z;
    }
    void moveDownFixRefVector3D(){
        Vector3D forward = Vector3D(center.x - eye.x, center.y - eye.y, center.z - eye.z);
        Vector3D left = up.cross(forward);
        forward.rotate(left, -rotationAngle);
        up.rotate(left, -rotationAngle);
        eye.x = center.x - forward.x;
        eye.y = center.y - forward.y;
        eye.z = center.z - forward.z;
    }
    void moveLeft(){
        Vector3D forward = Vector3D(center.x - eye.x, center.y - eye.y, center.z - eye.z);
        Vector3D left = up.cross(forward);
        left.mul(v);
        eye.add(left);
        center.add(left);
    }
    void moveRight(){
        Vector3D forward = Vector3D(center.x - eye.x, center.y - eye.y, center.z - eye.z);
        Vector3D right = forward.cross(up);
        right.mul(v);
        eye.add(right);
        center.add(right);
    }
    void lookLeft(){
        Vector3D forward = Vector3D(center.x - eye.x, center.y - eye.y, center.z - eye.z);
        forward.rotate(up, rotationAngle);
        center.x = eye.x + forward.x;
        center.y = eye.y + forward.y;
        center.z = eye.z + forward.z;
    }
    void lookRight(){
        Vector3D forward = Vector3D(center.x - eye.x, center.y - eye.y, center.z - eye.z);
        forward.rotate(up, -rotationAngle);
        center.x = eye.x + forward.x;
        center.y = eye.y + forward.y; 
        center.z = eye.z + forward.z;
    }
    void lookUp(){
        Vector3D forward = Vector3D(center.x - eye.x, center.y - eye.y, center.z - eye.z);
        Vector3D left = forward.cross(up);
        forward.rotate(left, -rotationAngle);
        up.rotate(left, -rotationAngle);
        center.x = eye.x + forward.x;
        center.y = eye.y + forward.y;
        center.z = eye.z + forward.z;
    }
    void lookDown(){        
        Vector3D forward = Vector3D(center.x - eye.x, center.y - eye.y, center.z - eye.z);
        Vector3D left = forward.cross(up);
        forward.rotate(left, rotationAngle);
        up.rotate(left, rotationAngle);
        center.x = eye.x + forward.x;
        center.y = eye.y + forward.y;
        center.z = eye.z + forward.z;
    }
    void tiltClockWise(){
        Vector3D forward = Vector3D(center.x - eye.x, center.y - eye.y, center.z - eye.z);
        up.rotate(forward, -rotationAngle);
    }    
    void tiltCounterClockWise(){
        Vector3D forward = Vector3D(center.x - eye.x, center.y - eye.y, center.z - eye.z);
        up.rotate(forward, rotationAngle);
    }
};

struct Ray {
    Vector3D start;
    Vector3D dir;
    
    Ray(Vector3D s, Vector3D d) {
        start = s;
        dir = d;
        dir.normalize();
    }
};


struct Object {
    Vector3D reference_point;
    double height, width, length;
    double color[3];
    double coEfficients[4];
    int shine;
    
    Object() {
        color[0] = color[1] = color[2] = 0.0;
        coEfficients[0] = coEfficients[1] = coEfficients[2] = coEfficients[3] = 0.0;
        shine = 0;
    }
    
    virtual ~Object() {}
    
    virtual void draw() {}
    
    virtual double intersect(Ray* r, double* color, int level) {
        return -1.0;
    }
    
    void setColor(double r, double g, double b) {
        color[0] = r;
        color[1] = g;
        color[2] = b;
    }
    
    void setShine(int s) {
        shine = s;
    }
    
    void setCoEfficients(double ambient, double diffuse, double specular, double reflection) {
        coEfficients[0] = ambient;
        coEfficients[1] = diffuse;
        coEfficients[2] = specular;
        coEfficients[3] = reflection;
    }
    
    virtual Vector3D getNormalAt(Vector3D point) = 0;
    virtual double* getColorAt(Vector3D point) = 0;
};


struct PointLight {
    Vector3D position;
    double color[3];
    double intensity;
    
    PointLight(Vector3D pos, double r, double g, double b, double intensity = 1.5) {
        position = pos;
        color[0] = r;
        color[1] = g;
        color[2] = b;
        this->intensity = intensity;
    }

    PointLight(const PointLight& other) {
        position = other.position;
        color[0] = other.color[0];
        color[1] = other.color[1];
        color[2] = other.color[2];
        intensity = other.intensity;
    }
    
    void setColor(double r, double g, double b) {
        color[0] = r;
        color[1] = g;
        color[2] = b;
    }
    
    void setIntensity(double intensity) {
        this->intensity = intensity;
    }
};

struct SpotLight {
    Vector3D position;
    Vector3D direction;
    double angle; 
    double color[3];
    double intensity;
    
    SpotLight(Vector3D pos, Vector3D dir, double angle, double r, double g, double b, double intensity = 1.5) {
        position = pos;
        direction = dir;
        this->angle = angle;
        color[0] = r;
        color[1] = g;
        color[2] = b;
        this->intensity = intensity;
    }
    
    SpotLight(const SpotLight& other) {
        position = other.position;
        direction = other.direction;
        angle = other.angle;
        color[0] = other.color[0];
        color[1] = other.color[1];
        color[2] = other.color[2];
        intensity = other.intensity;
    }



    void setColor(double r, double g, double b) {
        color[0] = r;
        color[1] = g;
        color[2] = b;
    }
    
    void setIntensity(double intensity) {
        this->intensity = intensity;
    }
};

extern vector<Object*> objects;
extern vector<PointLight> pointLights;
extern vector<SpotLight> spotLights;
extern int recursion_level;

void computePhongLighting(Object* obj, const Vector3D& intersectionPoint,double* color,Ray* r,int level) {
    Vector3D normal = obj->getNormalAt(intersectionPoint);

    if(r->dir.dot(normal) > 0) {
        normal = -normal;
    }

    double* intersectionPointColor = obj->getColorAt(intersectionPoint);
    color[0] = intersectionPointColor[0] * obj->coEfficients[0];
    color[1] = intersectionPointColor[1] * obj->coEfficients[0];
    color[2] = intersectionPointColor[2] * obj->coEfficients[0];

    for (const auto& pl : pointLights) {
        Vector3D lightDir = pl.position - intersectionPoint;
        double lightDistance = lightDir.length();
        lightDir.normalize();

        Ray shadowRay(pl.position, intersectionPoint - pl.position);
        bool inShadow = false;
        for (Object* other : objects) {
            if (other != obj) {
                double t = other->intersect(&shadowRay, nullptr, 0);
                if (t > EPSILON && t < (lightDistance - EPSILON)) {
                    inShadow = true;
                    break;
                }
            }
        }
        if (!inShadow) {
            double lambert = max(0.0, normal.dot(lightDir));

            Vector3D reflectDir = lightDir - normal * (2.0 * normal.dot(lightDir));
            reflectDir.normalize();

            Vector3D viewDir = (r->start - intersectionPoint);
            viewDir.normalize();

            double phong = max(0.0, viewDir.dot(reflectDir));

            for (int i = 0; i < 3; i++) {
                color[i] += pl.color[i] * pl.intensity * obj->coEfficients[1] * lambert * intersectionPointColor[i];
                color[i] += pl.color[i] * pl.intensity * obj->coEfficients[2] * pow(phong, obj->shine);
            }
        }
    }

    for (const auto& sl : spotLights) {
        Vector3D lightDir = sl.position - intersectionPoint;
        double lightDistance = lightDir.length();
        lightDir.normalize();
        double angle = acos(lightDir.dot(-sl.direction)) * 180.0 / M_PI;
        if (angle > sl.angle) continue;


        Ray shadowRay(sl.position, intersectionPoint - sl.position);
        bool inShadow = false;
        for (Object* other : objects) {
            if (other != obj) {
                double t = other->intersect(&shadowRay, nullptr, 0);
                if (t > EPSILON && t < (lightDistance - EPSILON)) {
                    inShadow = true;
                    break;
                }
            }
        }
        if (!inShadow) {
            double lambert = max(0.0, normal.dot(lightDir));
            
            Vector3D reflectDir = lightDir - normal * (2.0 * normal.dot(lightDir));
            reflectDir.normalize();

            Vector3D viewDir = (r->start - intersectionPoint);
            viewDir.normalize();

            double phong = max(0.0, viewDir.dot(reflectDir));
            for (int i = 0; i < 3; i++) {
                color[i] += sl.color[i] * sl.intensity * obj->coEfficients[1] * lambert * intersectionPointColor[i];
                color[i] += sl.color[i] * sl.intensity *obj->coEfficients[2] * pow(phong, obj->shine);
            }
        }
    }

    if (level < recursion_level && obj->coEfficients[3] > 0) {
        Vector3D reflectDir = r->dir - normal * (2.0 * r->dir.dot(normal));
        reflectDir.normalize();
        Vector3D reflectStart = intersectionPoint + normal * EPSILON;
        Ray reflectRay(reflectStart, reflectDir);

        double minT = -1.0;
        Object* nearest = nullptr;
        for (Object* other : objects) {
            double t = other->intersect(&reflectRay, nullptr, 0);
            if (t > 0 && (minT < 0 || t < minT)) {
                minT = t;
                nearest = other;
            }
        }
        if (nearest != nullptr) {
            double reflectedColor[3] = {0, 0, 0};
            nearest->intersect(&reflectRay, reflectedColor, level + 1);
            for (int i = 0; i < 3; i++) {
                color[i] += reflectedColor[i] * obj->coEfficients[3];
            }
        }
    }

    for (int i = 0; i < 3; i++) {
        color[i] = max(0.0, min(1.0, color[i]));
    }

}

struct Sphere : public Object {
    double radius;
    Sphere(Vector3D center, double r) {
        reference_point = center;
        radius = r;
        length = radius; 
    }
    
    void draw() override {
        glPushMatrix();
        glTranslatef(reference_point.x, reference_point.y, reference_point.z);
        glColor3f(color[0], color[1], color[2]);
        glutSolidSphere(radius, 100, 100);
        glPopMatrix();
    }
    
    double intersect(Ray* r, double* color, int level) override {
        Vector3D oc = r->start - reference_point;
        
        double a = r->dir.dot(r->dir);
        double b = 2.0 * oc.dot(r->dir);
        double c = oc.dot(oc) - radius * radius;
        
        double discriminant = b * b - 4 * a * c;
        
        if (discriminant < 0) {
            return -1.0; 
        }
        
        double t1 = (-b - sqrt(discriminant)) / (2 * a);
        double t2 = (-b + sqrt(discriminant)) / (2 * a);
        
        double t = (t1 > 0) ? t1 : t2;
        
        if (t <= 0) {
            return -1.0;
        }
        
        if (level == 0) {
            return t;
        }
        
        Vector3D intersectionPoint = r->start + r->dir * t;
        
        computePhongLighting(this, intersectionPoint, color, r, level);
        
        return t;
    }
    
    Vector3D getNormalAt(Vector3D point) override {
        Vector3D normal = point - reference_point;
        normal.normalize();
        return normal;
    }
    
    double* getColorAt(Vector3D point) override {
        return color;
    }
};
struct Triangle : public Object {
    Vector3D a, b, c;
    Vector3D normal;
    Triangle(Vector3D v1, Vector3D v2, Vector3D v3) {
        a = v1;
        b = v2;
        c = v3;
        reference_point = (a + b + c) / 3.0;
        
        Vector3D edge1 = b - a;
        Vector3D edge2 = c - a;
        normal = edge1.cross(edge2);
        normal.normalize();
    }
    
    void draw() override {
        glColor3f(color[0], color[1], color[2]);
        glBegin(GL_TRIANGLES);
        glVertex3f(a.x, a.y, a.z);
        glVertex3f(b.x, b.y, b.z);
        glVertex3f(c.x, c.y, c.z);
        glEnd();
    }
    
    double intersect(Ray* r, double* color, int level) override {
        Vector3D edge1 = b - a;
        Vector3D edge2 = c - a;
        Vector3D h = r->dir.cross(edge2);
        double det = edge1.dot(h);
        
        if (det > -EPSILON && det < EPSILON) {
            return -1.0;
        }
        
        double invDet = 1.0 / det;
        Vector3D s = r->start - a;
        double u = invDet * s.dot(h);
        
        if (u < 0.0 || u > 1.0) {
            return -1.0;
        }
        
        Vector3D q = s.cross(edge1);
        double v = invDet * r->dir.dot(q);
        
        if (v < 0.0 || u + v > 1.0) {
            return -1.0;
        }
        
        double t = invDet * edge2.dot(q);
        
        if (t <= EPSILON) {
            return -1.0;
        }
        
        if (level == 0) {
            return t;
        }

        Vector3D intersectionPoint = r->start + r->dir * t;   
        
        computePhongLighting(this, intersectionPoint, color, r, level);

        return t;
    }
    
    Vector3D getNormalAt(Vector3D point) override {
        return normal;
    }
    
    double* getColorAt(Vector3D point) override {
        return color;
    }
};

struct GeneralQuadric : public Object {
    double A, B, C, D, E, F, G, H, I, J;
    Vector3D cubeRef;
    double cubeLength, cubeWidth, cubeHeight;
    GeneralQuadric(double a, double b, double c, double d, double e, double f,
                   double g, double h, double i, double j,
                   Vector3D ref, double len, double wid, double hei) {
        A = a; B = b; C = c; D = d; E = e; F = f;
        G = g; H = h; I = i; J = j;
        cubeRef = ref;
        cubeLength = len;
        cubeWidth = wid;
        cubeHeight = hei;
        reference_point = ref;
    }
    
    void draw() override {}
    
    double intersect(Ray* r, double* color, int level) override {
        double x0 = r->start.x, y0 = r->start.y, z0 = r->start.z;
        double dx = r->dir.x, dy = r->dir.y, dz = r->dir.z;
        
        double a = A*dx*dx + B*dy*dy + C*dz*dz + D*dx*dy + E*dx*dz + F*dy*dz;
        double b = 2*A*x0*dx + 2*B*y0*dy + 2*C*z0*dz + D*(x0*dy + y0*dx) + 
                   E*(x0*dz + z0*dx) + F*(y0*dz + z0*dy) + G*dx + H*dy + I*dz;
        double c = A*x0*x0 + B*y0*y0 + C*z0*z0 + D*x0*y0 + E*x0*z0 + F*y0*z0 + 
                   G*x0 + H*y0 + I*z0 + J;
        
        double discriminant = b*b - 4*a*c;
        
        if (discriminant < 0) {
            return -1.0;
        }
        
        double t1 = (-b - sqrt(discriminant)) / (2*a);
        double t2 = (-b + sqrt(discriminant)) / (2*a);
        
        auto isInBounds = [&](double t) -> bool {
            Vector3D point = r->start + r->dir * t;
            
            if (cubeLength > 0) {
                if (point.x < cubeRef.x || point.x > cubeRef.x + cubeLength) return false;
            }
            if (cubeWidth > 0) {
                if (point.y < cubeRef.y || point.y > cubeRef.y + cubeWidth) return false;
            }
            if (cubeHeight > 0) {
                if (point.z < cubeRef.z || point.z > cubeRef.z + cubeHeight) return false;
            }
            return true;
        };
        
        double t = -1.0;
        if (t1 > 0 && isInBounds(t1)) {
            t = t1;
        } 
        else if (t2 > 0 && isInBounds(t2)) {
            t = t2;
        }
        
        if (t <= 0) {
            return -1.0;
        }
        
        if (level == 0) {
            return t;
        }
        
        Vector3D intersectionPoint = r->start + r->dir * t;
        
        computePhongLighting(this, intersectionPoint, color, r, level);

        return t;
    }
    
    Vector3D getNormalAt(Vector3D point) override {
        double x = point.x, y = point.y, z = point.z;
        
        Vector3D normal(
            2*A*x + D*y + E*z + G,
            2*B*y + D*x + F*z + H,
            2*C*z + E*x + F*y + I
        );
        
        normal.normalize();
        return normal;
    }
    
    double* getColorAt(Vector3D point) override {
        return color;
    }
};

struct Floor : public Object{
    double tileWidth;
    double color2[3] = {0.0, 0.0, 0.0};
    double colorTex[3];
    GLuint textureID; 

    Floor(double w, double tw){
        length = width = w;
        tileWidth = tw;
        reference_point = Vector3D(-w/2, -w/2, 0.0);
        textureID = 0;
    }

    void draw() override {
        int boardSize = width / tileWidth;
        
        if (useTexture && textureID > 0) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glColor3f(1.0f, 1.0f, 1.0f);
            
            for (int i = 0; i < boardSize; i++) {
                for (int j = 0; j < boardSize; j++) {
                    double x = reference_point.x + i * tileWidth;
                    double y = reference_point.y + j * tileWidth;
                    
                    glBegin(GL_QUADS);
                    
                    glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y, 0);
                    glTexCoord2f(1.0f, 0.0f); glVertex3f(x + tileWidth, y, 0);
                    glTexCoord2f(1.0f, 1.0f); glVertex3f(x + tileWidth, y + tileWidth, 0);
                    glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y + tileWidth, 0);
                    
                    glEnd();
                }
            }
            
            glBindTexture(GL_TEXTURE_2D, 0);
            glDisable(GL_TEXTURE_2D);
        } 
        else {
            for (int i = 0; i < boardSize; i++) {
                for (int j = 0; j < boardSize; j++) {
                    if ((i + j) % 2 == 0) {
                        glColor3f(0.0f, 0.0f, 0.0f);
                    } else {
                        glColor3f(1.0f, 1.0f, 1.0f);
                    }
                    
                    double x = reference_point.x + i * tileWidth;
                    double y = reference_point.y + j * tileWidth;
                    
                    glBegin(GL_QUADS);
                    glVertex3f(x, y, 0);
                    glVertex3f(x + tileWidth, y, 0);
                    glVertex3f(x + tileWidth, y + tileWidth, 0);
                    glVertex3f(x, y + tileWidth, 0);
                    glEnd();
                }
            }
        }
    }

    void loadTextureForOpenGL() {
        if (!textureData || textureWidth <= 0 || textureHeight <= 0) {
            return;
        }
        
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 
                    0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
        
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    Vector3D getNormalAt(Vector3D point) override {
        return Vector3D(0,0,1);
    }

    double* getColorAt(Vector3D point) override {

        if (useTexture) {
            int i = (point.x - reference_point.x) / tileWidth;
            int j = (point.y - reference_point.y) / tileWidth;

            double tileOriginX = reference_point.x + i * tileWidth;
            double tileOriginY = reference_point.y + j * tileWidth;

            double localX = point.x - tileOriginX;
            double localY = point.y - tileOriginY;

            double u = localX / tileWidth;
            double v = localY / tileWidth;

            sampleFloorTexture(u, v, colorTex);
            return colorTex;
        } 
        else {
            int i = (point.x - reference_point.x) / tileWidth;
            int j = (point.y - reference_point.y) / tileWidth;
            if ((i + j) % 2 == 0) return color2;
            else return color;
        }
    }


    double intersect(Ray* r, double* color, int level) override {
        Vector3D normal = getNormalAt(reference_point);
        double denom = normal.dot(r->dir);

        if (fabs(denom) < EPSILON) {
            return -1.0;
        }

        double t = -r->start.z / r->dir.z;

        if (t < 0) {
            return -1.0;
        }

        Vector3D intersectionPoint = r->start + r->dir * t;

        if (intersectionPoint.x < reference_point.x || intersectionPoint.x > reference_point.x + width ||
            intersectionPoint.y < reference_point.y || intersectionPoint.y > reference_point.y + width) {
            return -1.0;
        }

        if (level == 0) {
            return t;
        }
        
        computePhongLighting(this, intersectionPoint, color, r, level);

        return t;
    }
};
