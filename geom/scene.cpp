#include "scene.h"
#include <math.h>
#include <limits.h>
#include <stdio.h>
#include "functions.h"

using cs40::Ray;
using cs40::Shape;
using cs40::Light;
using cs40::Scene;
using cs40::Material;

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


int Scene::checkAllIntersections( const Ray & incidentRay, 
                                  std::vector< vec3 > & intersections ) const{

    float currentTime , minTime;

    minTime = INFINITY;
    int minIndex = -2;

    for ( int i = 0 ; i < objects.size(); i ++ ){

        currentTime = objects[i]->hitTime( incidentRay );

        //if a valid hit occurred
        if ( currentTime > 0 ){

            //the current best is replaced, so save the position of the old
            //best in the vector
            if ( currentTime < minTime ){
                //save the old best position
                intersections.push_back( incidentRay( minTime ) );

                //set the minTime to the new best time
                minTime = currentTime;
                minIndex = i;
            }
            //store the position in the vector.
            else{
                intersections.push_back( incidentRay( currentTime ) );
            }
        }
                
    }

    //store the closest intersection point as the last element of the vector
    if ( minIndex >= 0 ){
        intersections.push_back( incidentRay( minTime ) );
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

vec3 Scene::getDirect(  const vec3 & view,
                 const vec3 & normal,
                 const vec3 & hitPoint,
                 int shapeIndex, int lightIndex,
                 bool checkForIntersections){

    Material mat = objects[ shapeIndex ]->material;

    vec3 lightVal( 0,0,0);
    vec3 direction_to_light;
    Ray lightRay;
    lightRay.origin = hitPoint;

    Light currentLight = lights[lightIndex];

    
    if( currentLight.type == RECTANGLE ){


        //these following lines attenuate the intensity of the light based on
        //the difference in angle between the normal of the light and the
        //direction from the light to the hitPoint.
        vec3 lightCenter =  currentLight.position +
                            currentLight.basis1 * 0.5 +
                            currentLight.basis2 * 0.5;

        vec3 direction_from_center = hitPoint - lightCenter;
        direction_from_center.normalize();
        float attenuation =  normal.dotProduct(  direction_from_center,
                                                 currentLight.normal);

        if (attenuation <= 0 ){
            return vec3();
        }


        vec3 lightPos;
        float increment = 0.1;

        for ( float j = 0.1; j <= 1.0; j += increment ){
            for( float k = 0.1; k <= 1.0; k += increment ){

                lightPos = currentLight.position +
                           currentLight.basis1 * j +
                           currentLight.basis2 * k;

                direction_to_light = lightPos - hitPoint;
                lightRay.direction = direction_to_light;

                //if we are not checking for intersections, then 
                //it does not matter if the light is obstructed or not,
                //we still add in all of the directional coponenets.
                if ( !checkForIntersections ||
                     !( isObstructed( lightRay, shapeIndex )) ){
                    direction_to_light.normalize();
                    lightVal += mat.getLight( direction_to_light,
                                              view,
                                              normal);
                }
            }
        }

        lightVal *= fmax( attenuation, 0.0 ) * increment * increment;
    }


    else{
        direction_to_light = currentLight.position - hitPoint;
        lightRay.direction = direction_to_light;
        
        //the ray has hit an object before the light source
        //reset the diffuse and specular components to zero
        if ( !checkForIntersections ||
             !( isObstructed( lightRay, shapeIndex )) ){

            direction_to_light.normalize(); 
            lightVal += mat.getLight( direction_to_light,
                                      view,
                                      normal);
        }
    }

    //vec3 color = m_scene.color * material.color;
    lightVal *= currentLight.intensity;
    return lightVal;
}


