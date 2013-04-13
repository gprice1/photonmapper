#include "scene.h"

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
  minTime = std::numeric_limits<float>::infinity();
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


float Scene::collisionTime(const Ray & incidentRay, int shapeIndex ) const{
  float currentTime , minTime;
  minTime = std::numeric_limits<float>::infinity();

  //check all of the intersection times for the shapes
  for ( int i = 0 ; i < objects.size(); i ++ ){
    if ( i == shapeIndex ){ continue; }

    currentTime = objects[i]->hitTime( incidentRay );
    if ( currentTime < minTime && currentTime > 0 ){
      minTime = currentTime;
    }
  }

  return minTime;

}
