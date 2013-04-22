#ifndef CS40LIGHT_H
#define CS40LIGHT_H

#include "common.h"
#include "ray.h"

namespace cs40{
typedef enum LightType{ POINT,
                        RECTANGLE,
                        SPHERE,
                        CIRCLE,
                        TRIANGLE
}LightType;

/* a little struct to manage light properties */
class Light{

public:
    Light();
    Light( vec3 pos , float intensity);
    Light( vec3 pos, float intensity, float r, LightType type);
    Light( vec3 pos, float intensity, float r, vec3 normal, LightType type);
    Light( vec3 point1, vec3 point2, vec3 point3, float intensity,
                                                  LightType type );

    cs40::Ray emitPhoton() const;

    vec3 position;     //xyz position in world coordinates
    float intensity;   //should be in range 0-1

    //the bases of the sides. this is for a triangle or a rectangle
    vec3 basis1;
    vec3 basis2;
    vec3 normal;

    float radius;


    //stores the type of light this is
    LightType type;



};

}
#endif
