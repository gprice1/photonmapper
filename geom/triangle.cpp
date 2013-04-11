#include "triangle.h"

using namespace cs40;

Triangle::Triangle(vec3 pt1, vec3 pt2, vec3 pt3, cs40::Material material){
    this->points[0] = pt1;
    this->points[1] = pt2;
    this->points[2] = pt3;

    this->material = material;

    //find the normal of the rectangle by getting the cross product of two
    //lines inside of it.

    vert = points[0] - points[1];
    horiz = points[2] - points[1];

    lengthVert = vert.lengthSquared();
    lengthHoriz = horiz.lengthSquared();

    this->n = vert.crossProduct( vert, horiz ).normalized();

    name = "Triangle";
}

//this is hacked up
//TODO - generalize the rectangle hit time

float Triangle::hitTime(const cs40::Ray& r)const{
        float rDirection_dot_n = n.dotProduct( r.direction , n );

        if( rDirection_dot_n == 0 ){ return -1.0;}
        float time = n.dotProduct( points[1] - r.origin , n ) /
                     rDirection_dot_n;

        //now we must check to see if the point is in the rectangle we have.
        //this is accomplished through projection.
        vec3 hitPoint = r( time );
        vec3 p = hitPoint - points[1];

        float test1, test2;
        test1 = vert.dotProduct( vert, p );
        if (test1 > lengthVert || test1 < 0.0){ return -1.0; }

        test2 = horiz.dotProduct( horiz, p );
        if (test2 > lengthHoriz || test2 < 0.0){ return -1.0; }

        if ( test1 / lengthVert + test2 / lengthHoriz > 1.0 ){ return -1.0; }
        return time;

}

vec3 Triangle::normal(const vec3& p)const{
    return n;
}

