#ifndef CS40SCENE_H
#define CS40SCENE_H

#include "view.h"
#include "light.h"
#include "shape.h"
#include <vector>
#include "ray.h"

namespace cs40{

//A little struct to store everything to raytrace
class Scene{

public:
    Scene(){}
    ~Scene(){}

    cs40::View view;
    float ambient;           //global ambient intensity
    std::vector<cs40::Light> lights;    //other positional lights

    std::vector<float> lightMapping;    //this data is used to select which
                                        //light will emit a photon.
    
    std::vector<cs40::Shape*> objects;  //things to raytrace

    int checkIntersection( const cs40::Ray & incidentRay, 
                           int shapeIndex, 
                           vec3 & hitPoint ) const;

    float collisionTime(const cs40::Ray & incidentRay, int shapeIndex ) const;
    bool  isObstructed( const cs40::Ray & incidentRay, int shapeIndex ) const;



    cs40::Ray emitPhoton( ) const ;

    void createLightMapping( );

};

}

#endif // SCENE_H
