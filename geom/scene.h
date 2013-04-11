#ifndef CS40SCENE_H
#define CS40SCENE_H

#include "view.h"
#include "light.h"
#include "shape.h"
#include <vector>

namespace cs40{

//A little struct to store everything to raytrace
typedef struct scene_s{
    cs40::View view;
    float ambient;           //global ambient intensity
    std::vector<cs40::Light> lights;    //other positional lights
    std::vector<cs40::Shape*> objects;  //things to raytrace
} Scene;

}

#endif // SCENE_H
