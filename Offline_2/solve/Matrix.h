#include <bits/stdc++.h>
using namespace std;

#define DEG2RAD(x) ((x) * M_PI / 180.0)

const int MAX_ROW = 5;
const int MAX_COL = 5;
 
struct Matrix{
    int row,col;
    double **a;
    Matrix(int n,int m){
        row = n;
        col = m;
        a = new double*[n];
        for(int i = 0;i<n;i++){
            a[i] = new double[m];
            for(int j = 0;j<m;j++){
                if(j == i) a[i][j] = 1;
                else a[i][j] = 0;
            }
        }
    }
    Matrix operator*(Matrix &M){
        Matrix res(row,M.col);
        for(int i = 0;i<row;i++){
            for(int j = 0;j<M.col;j++){
                res.a[i][j] = 0.0;
                for(int k = 0;k<col;k++){
                    res.a[i][j] += (a[i][k] * M.a[k][j]);
                }
            }
        }
        return res;
    }
    Matrix operator^(int n){
        if(n == 1) return *this;
        Matrix res(row,col);
        res = (*this)^(n/2);
        res = res*res;
        if(n&1) res = res*(*this);
        return res; 
    }
    // void make_identity(){
    //     for(int i = 0;i<row;i++){
    //         for(int j = 0;j<col;j++){
    //             if(i == j) a[i][j] = 1;
    //             else a[i][j] = 0;
    //         }
    //     }
    // }
    void print(){
        cout << fixed << setprecision(6);
        for(int i = 0;i<row;i++){
            for(int j = 0; j<col; j++){
                cout << a[i][j] << " ";
            }
            cout << "\n";
        }
        cout << "\n";
    }

};

struct Vector{
    double x,y,z;
    Vector(){
        this->x = this->y = this->z = 0;
    }
    Vector(double x,double y,double z){
        this->x = x;
        this->y = y;
        this->z = z;
    }
    Vector(const Vector &v){
        this->x = v.x;
        this->y = v.y;
        this->z = v.z;
    }
    double length(){
        return sqrtl(x*x + y*y + z*z);
    }
    void normalize(){
        double len = this->length();
        x /= len;
        y /= len;
        z /= len;
    }
    Vector cross(const Vector& v) const {
        return Vector(
            y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x
        );
    }

    double dot(const Vector& v) const {
        return x * v.x + y * v.y + z * v.z;
    }

    Vector operator*(double s) const {
        return Vector(x * s, y * s, z * s);
    }

    Vector operator+(const Vector& v) const {
        return Vector(x + v.x, y + v.y, z + v.z);
    }

    Vector operator-(const Vector& v) const {
        return Vector(x - v.x, y - v.y, z - v.z);
    }

};

Vector R(const Vector& v, const Vector& a, double angle_deg) {
    double angle = DEG2RAD(angle_deg);
    double cos_theta = cos(angle);
    double sin_theta = sin(angle);

    Vector term1 = v * cos_theta;
    Vector term2 = a * ((a.dot(v)) * (1 - cos_theta));
    Vector term3 = (a.cross(v)) * sin_theta;

    return term1 + term2 + term3;
}


