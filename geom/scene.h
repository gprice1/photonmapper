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

    //takes a reference to a vector and stores all of the hitpoints in this 
    //vector.
    //returns the shape index.
    int checkAllIntersections( const cs40::Ray & incidentRay, 
                               std::vector< vec3 > & intersections ) const;


    float collisionTime(const cs40::Ray & incidentRay, int shapeIndex ) const;
    bool  isObstructed( const cs40::Ray & incidentRay, int shapeIndex ) const;



    cs40::Ray emitPhoton( ) const ;

    void createLightMapping( );

    vec3 getDirect( const vec3 & view,
                     const vec3 & normal,
                     const vec3 & hitPoint,
                     int shapeIndex, int lightIndex,
                     bool checkForIntersections=true );

};

}

#endif // SCENE_H
