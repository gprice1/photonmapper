#ifndef CS40RECTANGLE_H
#define CS40RECTANGLE_H

#include "shape.h"
#include "common.h"

namespace cs40{

class Rectangle : public Shape { 
protected:
  vec3 points[3];
  vec3 n;
  vec3 vert;
  vec3 horiz;
  float lengthHoriz, lengthVert, C;


public:
    Rectangle( vec3 ll, vec3 lr, vec3 ur, cs40::Material material);
    Rectangle();
    ~Rectangle() { /*do nothing*/ };

    //TODO: implement in sphere.cpp
    float hitTime(const cs40::Ray& r) const;
    vec3 normal(const vec3& p) const;
    bool pointIsInRectangle( vec3 point);
};

}

#endif
