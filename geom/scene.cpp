#include "scene.h"
#include <math.h>
#include <limits.h>
#include <stdio.h>
#include "functions.h"

using cs40::Ray;
using cs40::Shape;
using cs40::Light;
using cs40::Scene;

/* checkIntersection -  Checks an incident ray for intersection with all of 
 * the shapes in the scene.
 *  Takes in a reference to a vec3 and stores the point of
 *       intersection at that reference (hitPoint).   
 *  Takes in an index corresponding to an object that it should not
 *       test for intersection.
 *  Returns an int corresponding to the index of the closest intersected object
 *  Returns -1 if there is no intersection.
 */

int Scene::checkIntersection( const Ray & incidentRay, 
                              int shapeIndex, 
                              vec3 & hitPoint ) const{
    float currentTime , minTime;
    //if the ray is inside the object, then it means that
    //I only want to intersect with the current object.
    if ( incidentRay.isInsideObject ){
        currentTime = objects[shapeIndex]->hitTime( incidentRay );
        hitPoint = incidentRay( currentTime );
        return shapeIndex;
    } 

    minTime = INFINITY;
    int minIndex = -2;

    for ( int i = 0 ; i < objects.size(); i ++ ){
        if ( shapeIndex == i ){ continue ;}

        currentTime = objects[i]->hitTime( incidentRay );
        if ( (currentTime < minTime ) && ( currentTime > 0 )){
            minTime = currentTime;
            minIndex = i;
        }
    }

    if ( minIndex >= 0 ){
        hitPoint = incidentRay( minTime );
    }

    return minIndex;
}

//returns false if there is an object with a hit time between 0.0 and 1.0
bool Scene::isObstructed(const Ray & incidentRay, int shapeIndex ) const{
    float currentTime;

    //if ( incidentRay.isInsideObject ){
    //    return true;
    //}

    //check all of the intersection times for the shapes
    for ( int i = 0 ; i < objects.size(); i ++ ){
        if ( i == shapeIndex ){ continue; }

        currentTime = objects[i]->hitTime( incidentRay );
        if ( currentTime < 1.0 && currentTime > 0.0 ){
            //std::cout << "True\n";
            return true;

        }
    }

    return false;

}


float Scene::collisionTime(const Ray & incidentRay, int shapeIndex ) const{
    float currentTime , minTime;
    minTime = INFINITY;
    //if ( incidentRay.isInsideObject ){
    //    return 0.0;
    //}

    //check all of the intersection times for the shapes
    for ( int i = 0 ; i < objects.size(); i ++ ){
        if ( i == shapeIndex ){ continue; }

        currentTime = objects[i]->hitTime( incidentRay );
        if ( currentTime < minTime && currentTime > 0.0 ){
            minTime = currentTime;
        }
    }

    return minTime;

}


//this is not designed to work for a large number of lights
Ray Scene::emitPhoton( ) const {

    float lightVal = cs40::randf( lightMapping.back() );

    int i = 0;
    while( lightVal > lightMapping[ i ] ){
        i ++;
    }

    return lights[i].emitPhoton();

}

//based on how intense a light is, it will either emit more or less photons
void Scene::createLightMapping( ) {

    float totalLight = 0;

    for ( int i = 0; i < lights.size() ; i ++ ){
        totalLight += lights[i].intensity;
        lightMapping.push_back( totalLight );
    }
}

