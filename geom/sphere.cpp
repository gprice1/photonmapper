#include "sphere.h"
#include <math.h>
using namespace std;
using namespace cs40;

Sphere::Sphere(vec3 center, float radius, cs40::Material material){
    this->center = center;
    this->radius = radius;
    this->material = material;
    name = "circle";
}

Sphere::Sphere(){
    radius = 4.0;
}

//TODO
float Sphere::hitTime(const cs40::Ray& r) const{

    vec3 diff = r.origin - center;
    float A, B, C, determinant;
    A = r.direction.dotProduct( r.direction, r.direction );
    B = 2 * r.direction.dotProduct( diff , r.direction );
    C = diff.dotProduct( diff, diff ) - radius * radius;

    determinant = B*B - 4*A*C;

    //the line does not intersect the sphere
    if (determinant < 0 ){
      return -1.0;
    }
    //cout << "r.direction: " << r.direction << "\tA: "<< A << "\tB: " << B << "\tC: " << C;
    //cout << "\tDeterminant1: " << determinant;

    determinant = sqrt( determinant );

    //cout << "\tDeterminant2: " << determinant;
    //cout << "\tA: " << A << endl;


    if ( B > 0 ){
      if (determinant < B){ return -0.5; } // the intersection is in the wrong
                                           //direction
      //cout  << "sphere hit point: " << r((-B + determinant) / (2*A)) << endl;
      return (-B + determinant) / (2*A);
    }

    // if B < 0 :
    if (determinant <= -B) {
        //cout  << "sphere hit point: " << r((-B + determinant) / (2*A)) << endl;
        return (-B - determinant) / (2*A); }


    //cout  << "sphere hit point: " << r((-B + determinant) / (2*A)) << endl;
    return (-B + determinant) / (2*A);

}

vec3 Sphere::normal(const vec3& p) const{
    return (p - center).normalized();

}
