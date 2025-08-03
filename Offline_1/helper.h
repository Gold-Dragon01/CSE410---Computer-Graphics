struct Point{
    double x,y,z;
    Point(double x, double y, double z){
        this->x = x;
        this->y = y;
        this->z = z;
    }
    Point(){
        this->x = 0;
        this->y = 0;
        this->z = 0;
    }
    Point(Point &p){
        this->x = p.x;
        this->y = p.y;
        this->z = p.z;
    }
    void add(Point p){
        this->x += p.x;
        this->y += p.y;
        this->z += p.z;
    }
    void sub(Point p){
        this->x -= p.x;
        this->y -= p.y;
        this->z -= p.z;
    }
    void mul(double a){
        this->x *= a;
        this->y *= a;
        this->z *= a;
    }
    Point cross(Point p){
        return Point(this->y * p.z - this->z * p.y,
                     this->z * p.x - this->x * p.z,
                     this->x * p.y - this->y * p.x);
    }
    double dot(Point p){
        return this->x * p.x + this->y * p.y + this->z * p.z;
    }
    double length(){
        return sqrt(x*x + y*y + z*z);
    }
    double getAngle(Point p){
        double dotProduct = this->dot(p);
        return acos(dotProduct / (this->length()* p.length()));
    }
    void rotate(Point axis, double angle){
        double axisLength = axis.length();
        axis.x /= axisLength;
        axis.y /= axisLength;
        axis.z /= axisLength;

        double cosTheta = cos(angle);
        double sinTheta = sin(angle);
        double dotProduct = this->dot(axis);

        Point p = this->cross(axis);
        p.mul(sinTheta);
        axis.mul(dotProduct * (1 - cosTheta));
        this->mul(cosTheta);
        this->add(p);
        this->add(axis);
    }
};

struct Camera{
    Point eye,center,up;
    double v = 0.1;
    double rotationAngle = 2*M_PI / 180.0;
    Camera(Point eye,Point center,Point up){
        this->eye = eye;
        this->center = center;
        this->up = up;
    }
    Camera(){
        this->eye = Point(0,0,0);
        this->center = Point(0,0,-1);
        this->up = Point(0,1,0);
    }
    void increaseVelocity(double a){
        this->v += a;
    }
    void moveForward(){
        Point dir = Point(center.x - eye.x, center.y - eye.y, center.z - eye.z);
        dir.mul(v);
        eye.add(dir);
        center.add(dir);
    }
    void moveBackward(){
        Point dir = Point(center.x - eye.x, center.y - eye.y, center.z - eye.z);
        dir.mul(-v);
        eye.add(dir);
        center.add(dir);
    }
    void moveUp(){
        Point dir = Point(up);
        dir.mul(v);
        eye.add(dir);
        center.add(dir);
    }
    void moveDown(){
        Point dir = Point(up);
        dir.mul(-v);
        eye.add(dir);
        center.add(dir);
    }
    void moveUpFixRefPoint(){
        Point forward = Point(center.x - eye.x, center.y - eye.y, center.z - eye.z);
        Point left = up.cross(forward);
        forward.rotate(left, rotationAngle);
        up.rotate(left, rotationAngle);
        eye.x = center.x - forward.x;
        eye.y = center.y - forward.y;
        eye.z = center.z - forward.z;
    }
    void moveDownFixRefPoint(){
        Point forward = Point(center.x - eye.x, center.y - eye.y, center.z - eye.z);
        Point left = up.cross(forward);
        forward.rotate(left, -rotationAngle);
        up.rotate(left, -rotationAngle);
        eye.x = center.x - forward.x;
        eye.y = center.y - forward.y;
        eye.z = center.z - forward.z;
    }
    void moveLeft(){
        Point forward = Point(center.x - eye.x, center.y - eye.y, center.z - eye.z);
        Point left = up.cross(forward);
        left.mul(v);
        eye.add(left);
        center.add(left);
    }
    void moveRight(){
        Point forward = Point(center.x - eye.x, center.y - eye.y, center.z - eye.z);
        Point right = forward.cross(up);
        right.mul(v);
        eye.add(right);
        center.add(right);
    }
    void lookLeft(){
        Point forward = Point(center.x - eye.x, center.y - eye.y, center.z - eye.z);
        forward.rotate(up, rotationAngle);
        center.x = eye.x + forward.x;
        center.y = eye.y + forward.y;
        center.z = eye.z + forward.z;
    }
    void lookRight(){
        Point forward = Point(center.x - eye.x, center.y - eye.y, center.z - eye.z);
        forward.rotate(up, -rotationAngle);
        center.x = eye.x + forward.x;
        center.y = eye.y + forward.y; 
        center.z = eye.z + forward.z;
    }
    void lookUp(){
        Point forward = Point(center.x - eye.x, center.y - eye.y, center.z - eye.z);
        Point left = forward.cross(up);
        forward.rotate(left, -rotationAngle);
        up.rotate(left, -rotationAngle);
        center.x = eye.x + forward.x;
        center.y = eye.y + forward.y;
        center.z = eye.z + forward.z;
    }
    void lookDown(){        
        Point forward = Point(center.x - eye.x, center.y - eye.y, center.z - eye.z);
        Point left = forward.cross(up);
        forward.rotate(left, rotationAngle);
        up.rotate(left, rotationAngle);
        center.x = eye.x + forward.x;
        center.y = eye.y + forward.y;
        center.z = eye.z + forward.z;
    }
    void tiltClockWise(){
        Point forward = Point(center.x - eye.x, center.y - eye.y, center.z - eye.z);
        up.rotate(forward, -rotationAngle);
    }    
    void tiltCounterClockWise(){
        Point forward = Point(center.x - eye.x, center.y - eye.y, center.z - eye.z);
        up.rotate(forward, rotationAngle);
    }
};

struct Ball{
    Point center,velocity;
    double radius;
    double speed;
    Point rotationAxis;
    double rotationAngle;
    Ball(Point center,double radius){
        this->center = center;
        this->radius = radius;
        this->velocity = getRandomVelocity();
        this->rotationAxis = Point(1.0, 0.0, 0.0);
        this->rotationAngle = 0.0;
    }
    Ball(){
        this->center = Point(0,0,0);
        this->radius = 0.2;
        this->velocity = getRandomVelocity();
        printf("Velocity: %f %f %f\n", velocity.x, velocity.y, velocity.z);
        //this->velocity = Point(0.1,0.2,0.3);
        this->rotationAxis = Point(1.0, 0.0, 0.0);
        this->rotationAngle = 0.0;
    }
    void setRandomCenter(){
        center.x = (rand()% 350 - 175)/100.0;
        center.y = -2.0;
        center.z = (rand()% 350 - 175)/100.0;
    }
    Point getRandomVelocity(){
        Point V;
        V.x = (rand()% 5 + 10)/100.0;
        V.y = (rand()% 30 + 10)/100.0;
        V.z = (rand()% 5 + 10)/100.0;
        return V;
    }
};