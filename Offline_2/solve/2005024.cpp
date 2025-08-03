#include "Matrix.h"
#include "../bmp_image_codes/bitmap_image.hpp"

const double eps = 1.0e-9;

struct Point {
    double x, y, z;
};

struct Color {
    int r, g, b;
};

struct Triangle {
    Point p[3];
    Color color;
};

Vector eye,up,look;
double fovY,aspectRatio,near,far;

int screen_width, screen_height;
double leftLimitX, rightLimitX;
double topLimitY, bottomLimitY;
double zFrontLimit, zRearLimit;

double dx, dy, topY, leftX;
vector<vector<double>> zBuffer;
bitmap_image image;

vector<Triangle> triangles;

void Modeling_Transformation(const string& input_file,const string& output_file){  
    ifstream in(input_file);
    ofstream out(output_file);
    
    out << fixed << setprecision(7);

    double x,y,z;
    in >> x >> y >> z;
    eye = Vector(x,y,z);

    in >> x >> y >> z;
    look = Vector(x,y,z);

    in >> x >> y >> z;
    up = Vector(x,y,z);

    in >> fovY >> aspectRatio >> near >> far;

    stack<Matrix> S;
    Matrix M(4,4);
    S.push(M);
    string command;
    while(true){
        in >> command;
        if(command == "triangle"){
            double x,y,z;
            Matrix P(4,1);
            //S.top().print();
            for(int i=0;i<3;i++){
                in >> x >> y >> z;
                P.a[0][0] = x, P.a[1][0] = y, P.a[2][0] = z, P.a[3][0] = 1.0;
                //P.print();
                P = S.top()*P;
                double w = P.a[3][0];
                //cout << w << "\n";
                P.a[0][0] /= w;
                P.a[1][0] /= w;
                P.a[2][0] /= w;                
                //P.print();
                for(int j = 0;j<3;j++){
                    out << P.a[j][0] << " ";
                }
                out << "\n";
            }
            out << "\n";
        }
        else if(command == "translate"){
            double tx,ty,tz;
            in >> tx >> ty >> tz;
            Matrix T(4,4);
            T.a[0][3] = tx;
            T.a[1][3] = ty;
            T.a[2][3] = tz;
            Matrix N = S.top()*T;
            S.pop();
            S.push(N);
        }
        else if(command == "scale"){
            double sx,sy,sz;
            in >> sx >> sy >> sz;
            Matrix M(4,4);
            M.a[0][0] = sx;
            M.a[1][1] = sy;
            M.a[2][2] = sz;

            Matrix N = S.top()*M;
            S.pop();
            S.push(N);
        }
        else if(command == "rotate"){
            double angle,ax,ay,az;
            in >> angle >> ax >> ay >> az;
            Vector A(ax,ay,az);
            A.normalize();

            Vector i(1,0,0), j(0,1,0), k(0,0,1);
            Vector c1 = R(i, A, angle);
            Vector c2 = R(j, A, angle);
            Vector c3 = R(k, A, angle);

            Matrix M(4,4);
            M.a[0][0] = c1.x, M.a[0][1] = c2.x, M.a[0][2] = c3.x;
            M.a[1][0] = c1.y; M.a[1][1] = c2.y; M.a[1][2] = c3.y;
            M.a[2][0] = c1.z; M.a[2][1] = c2.z; M.a[2][2] = c3.z;

            M.a[0][3] = M.a[1][3] = M.a[2][3] = 0;
            M.a[3][0] = M.a[3][1] = M.a[3][2] = 0;
            M.a[3][3] = 1;

            Matrix N = S.top()*M;
            S.pop();
            S.push(N);
        }
        else if(command == "push"){
            S.push(S.top());
        }
        else if(command == "pop"){
            S.pop();
        }
        else if(command == "end"){
            break;
        }
    }

    in.close();
    out.close();
}

void View_Transformation(const string& input_file,const string& output_file){
    ifstream in(input_file);
    ofstream out(output_file);
    double x, y, z;
    int pointCount = 0;

    out << fixed << setprecision(7);

    Vector l = look - eye;
    l.normalize();

    Vector r = l.cross(up);
    r.normalize();

    Vector u = r.cross(l);

    Matrix T(4,4);
    T.a[0][3] = -eye.x;
    T.a[1][3] = -eye.y;
    T.a[2][3] = -eye.z;

    Matrix R(4,4);
    R.a[0][0] = r.x, R.a[0][1] = r.y, R.a[0][2] = r.z;
    R.a[1][0] = u.x, R.a[1][1] = u.y, R.a[1][2] = u.z;
    R.a[2][0] = -l.x, R.a[2][1] = -l.y, R.a[2][2] = -l.z;

    Matrix V = R*T;
    Matrix P(4,1);

    while (in >> x >> y >> z) {
        P.a[0][0] = x, P.a[1][0] = y, P.a[2][0] = z, P.a[3][0] = 1;
        P = V*P;
        double w = P.a[3][0];
        //cout << w << "\n";
        P.a[0][0] /= w;
        P.a[1][0] /= w;
        P.a[2][0] /= w;
        out << P.a[0][0] << " " << P.a[1][0] << " " << P.a[2][0] << endl;
        pointCount++;
        if (pointCount % 3 == 0) out << endl;
    }

    in.close();
    out.close();
}

void Projection_Transformation(const string& input_file, const string& output_file){
    ifstream in(input_file);
    ofstream out(output_file);
    double x, y, z;
    int pointCount = 0;

    out << fixed << setprecision(7);

    double fovX = fovY * aspectRatio;
    double t = near * tan(DEG2RAD(fovY/2));
    double r = near * tan(DEG2RAD(fovX/2));

    Matrix M(4,4);

    M.a[0][0] = near/r;
    M.a[1][1] = near/t;
    M.a[2][2] = -(far+near)/(far-near);
    M.a[2][3] = -(2*far*near)/(far-near);
    M.a[3][2] = -1;
    M.a[3][3] = 0;

    M.print();

    Matrix P(4,1);

    while (in >> x >> y >> z) {
        P.a[0][0] = x, P.a[1][0] = y, P.a[2][0] = z, P.a[3][0] = 1;
        P = M*P;
        double w = P.a[3][0];
        //cout << w << "\n";
        P.a[0][0] /= w;
        P.a[1][0] /= w;
        P.a[2][0] /= w;
        out << P.a[0][0] << " " << P.a[1][0] << " " << P.a[2][0] << endl;
        pointCount++;
        if (pointCount % 3 == 0) out << endl;
    }

    in.close();
    out.close();

}

int randColor(int min = 0, int max = 255) {
    return min + rand() % (max - min + 1);
}

void readData(const string& config_file, const string& input_file){
    ifstream in(config_file);

    in >> screen_width >> screen_height;

    in >> leftLimitX;
    rightLimitX = -leftLimitX;

    in >> bottomLimitY;
    topLimitY = -bottomLimitY;

    in >> zFrontLimit >> zRearLimit;

    in.close();

    in.open(input_file);
    Triangle tr;
    while (in >> tr.p[0].x >> tr.p[0].y >> tr.p[0].z
           >> tr.p[1].x >> tr.p[1].y >> tr.p[1].z
           >> tr.p[2].x >> tr.p[2].y >> tr.p[2].z) {
        tr.color = { randColor(), randColor(), randColor() };
        triangles.push_back(tr);
    }
    in.close();
}

void initializeBuffer(){
    dx = (rightLimitX - leftLimitX) / screen_width;
    dy = (topLimitY - bottomLimitY) / screen_height;
    topY = topLimitY - (dy / 2.0);
    leftX = leftLimitX + (dx / 2.0);

    zBuffer = vector<vector<double>>(screen_height, vector<double>(screen_width, zRearLimit));
    
    image.setwidth_height(screen_width, screen_height, true);
}

double checkSide(Point a, Point b, Point c) {
    return (c.x - a.x)*(b.y - a.y) - (c.y - a.y)*(b.x - a.x);
}

void rasterizeTriangle(const Triangle& tri) {
    double minX = min({ tri.p[0].x, tri.p[1].x, tri.p[2].x });
    double maxX = max({ tri.p[0].x, tri.p[1].x, tri.p[2].x });
    double minY = min({ tri.p[0].y, tri.p[1].y, tri.p[2].y });
    double maxY = max({ tri.p[0].y, tri.p[1].y, tri.p[2].y });

    int rowStart = max(0, (int)floor((topY - maxY) / dy));
    int rowEnd   = min(screen_height - 1, (int)floor((topY - minY) / dy));

    int colStart = max(0, (int)floor((minX - leftX) / dx));
    int colEnd   = min(screen_width - 1, (int)floor((maxX - leftX) / dx));

    for (int row = rowStart; row <= rowEnd; ++row) {
        double y = topY - row * dy;
        for (int col = colStart; col <= colEnd; ++col) {
            double x = leftX + col * dx;
            Point p = { x, y, 0 };

            double area = checkSide(tri.p[0], tri.p[1], tri.p[2]);
            double w0 = checkSide(tri.p[1], tri.p[2], p);
            double w1 = checkSide(tri.p[2], tri.p[0], p);
            double w2 = checkSide(tri.p[0], tri.p[1], p);

            if (area == 0) continue;
            w0 /= area; w1 /= area; w2 /= area;

            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                double z = w0 * tri.p[0].z + w1 * tri.p[1].z + w2 * tri.p[2].z;
                if (z < zFrontLimit || z > zRearLimit) continue;
                if (zBuffer[row][col] - z >= eps) {
                    zBuffer[row][col] = z;
                    image.set_pixel(col, row, tri.color.r, tri.color.g, tri.color.b);
                }
            }
        }
    }
}

void saveZBuffer(const string& filename){
    ofstream out(filename);
    out << fixed << setprecision(6);
    for(int row = 0; row < screen_height; ++row) {
        for(int col = 0; col < screen_width; ++col) {
            double z = zBuffer[row][col];
            if(z < zRearLimit){
                out << z << "\t";
            }
        }
        out << endl;
    }
    out.close();
}

void clip_and_scan(const string& config_file,const string& input_file, const string& output_file, const string& output_img){
    srand(time(0));

    readData(config_file, input_file);

    initializeBuffer();

    for (const Triangle& tri : triangles) {
        rasterizeTriangle(tri);
    }

    image.save_image(output_img);
    saveZBuffer(output_file);

    zBuffer.clear();
    triangles.clear();
    image.clear();
}



int main(){
    string input = "../Test Cases/4/scene.txt";
    string output = "stage1.txt";
    Modeling_Transformation(input,output);

    input = "stage1.txt";
    output = "stage2.txt";
    View_Transformation(input,output);

    input = "stage2.txt";
    output = "stage3.txt";
    Projection_Transformation(input,output);

    string config_file = "../Test Cases/4/config.txt";
    input = "stage3.txt";
    output = "z-buffer.txt";
    string img_output = "out.bmp";
    clip_and_scan(config_file,input,output,img_output);

    return 0;
}

