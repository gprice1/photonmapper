#include "rectangle.h"

using namespace cs40;



Rectangle::Rectangle( vec3 ll, vec3 lr, vec3 ur, cs40::Material material){
  this->points[0] = ll;
  this->points[1] = lr;
  this->points[2] = ur;

  //find the normal of the rectangle by getting the cross product of two
  //lines inside of it.

  vert = points[0] - points[1];
  horiz = points[2] - points[1];

  lengthVert = vert.lengthSquared();
  lengthHoriz = horiz.lengthSquared();

  this->n = vert.crossProduct( horiz, vert ).normalized();

  this->material = material;

  name = "Rectangle";

}

//this is hacked up
//TODO - generalize the rectangle hit time
float Rectangle::hitTime(const cs40::Ray& r)const{ 
    float rDirection_dot_n = n.dotProduct( r.direction , n );

    if( rDirection_dot_n == 0 ){ return -1.0;}
    float time = n.dotProduct( points[1] - r.origin , n ) /
                 rDirection_dot_n;

    //now we must check to see if the point is in the rectangle we have.
    //this is accomplished through projection.
    vec3 hitPoint = r( time );
    vec3 p = hitPoint - points[1];

    float test = vert.dotProduct( vert, p );
    if (test > lengthVert || test < 0.0){ return -1; }

    test = horiz.dotProduct( horiz, p );
    if (test > lengthHoriz || test < 0.0){ return -1; }

    return time;

}

vec3 Rectangle::normal(const vec3& p)const{
  return n;
}
