#ifndef CS40TRIANGLE_H
#define CS40TRIANGLE_H
#include "shape.h"
#include "common.h"

namespace cs40{

class Triangle : public Shape {
protected:
    //TODO: add member variables
    vec3 points[3];
    vec3 vert;
    vec3 horiz;
    vec3 n;
    float lengthVert;
    float lengthHoriz;

public:
    Triangle(vec3 pt1, vec3 pt2, vec3 pt3, cs40::Material material);
    ~Triangle() { /*do nothing*/ };

    float hitTime(const cs40::Ray& r) const;
    vec3 normal(const vec3& p) const;
};

}

#endif // TRIANGLE_H
