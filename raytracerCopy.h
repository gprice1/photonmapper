#ifndef CS40RAYTRACER_H
#define CS40RAYTRACER_H

#include <vector>
#include <string>
#include <QHash>      //Qt Dictionary
#include <QString>    //Appease QT with its strings
#include "scene.h"
#include "material.h"
#include "rgbColor.h"

//this is the hitTime thing for the sphere 
/*
float diff = p.origin - center;
float A, B, C, determinant;
A = dotProduct( p.direction, p.direction );
B = dotProduct( 2 * diff , p.direction );
C = dotProduct( diff, diff );

determinant = B*B - 4*A*C;
//the line does not intersect the sphere
if (determinant < 0 ){
  return -1.0;

determinant = sqrt( determinant );

//the intersection is in the wrong direction
if ( B > 0 ){
  if (determinant < B){ return -0.5; } // the intersection is in the wrong
                                       //direction
  return (-B + determinant) / (2*A);
}

// if B < 0 :
if (determinant <= -B) { return (-B - determinant) / (2*A); }

return (-B + determinant) / (2*A);

*/
namespace cs40{

class RayTracer{

public:
    RayTracer();
    ~RayTracer();

    /* parse an input file given by a filename and build the global
     * scene object from the parsed input */
    void parseFile(const std::string& fname);

    /* Do the raytracing */
    void trace( cs40::RGBImage & img );

    /* Save output image */
    void save();

private:
    cs40::Scene m_scene;
    QHash<QString, vec3> m_colors;
    int maxDepth;
    RGBColor background_color;


                      

    /* Hash table mapping material names to material structs.
     * materials["current_"] is a special entry refering to the
     * current material. The current material is applied to all newly
     * created objects */
    QHash<QString, cs40::Material> m_materials;

    /* parse a single line in the input file consisting of the given
     * vector of whitespace-delimited words. Non comment non blank lines
     * are of the form <commandname> [arguments] */
    void parseLine(const std::vector<std::string>& words);

    /* parse a material command in the vector words,
     * using m_materials to load/store/modify current and other maps */
    void parseMat(const std::vector<std::string>& words);

    /* convert from 0-1 rgb space to 0-255 */
    cs40::RGBColor convertColor(const vec3& clr);
    
    cs40::RGBColor traceOnce( cs40::Ray & incidentRay, int step );

    /* take an incident ray, and find what it intersects
     * set the status of the ray based on what happened
     * set the position of the ray to the point of intersection
     * finally, return the shape that it intersected with */
    cs40::Shape checkIntersection( cs40::Ray & incidentRay );

    //takes an incident ray and returns the collision time of the closest
    //colliding object.
    float collisionTime( cs40::Ray & incidentRay );
    
    //does the phong coloration.
    //also takes care of shadowing.
    cs40::RGBColor phong( const cs40::Ray & incidentRay ,
                          const cs40::vec3 & normal,
                          const cs40::Material & material );

    /* takes an incident ray and a normal vector and comutes a 
     * reflection ray. */
    cs40::Ray reflect( const cs40::Ray & incidentRay,
                       const cs40::vec3 & normal );

    /* takes an incident ray and a normal vector and comutes a 
     * transmission ray. This function likely needs a refraction index*/
    cs40::Ray transmit( const cs40::Ray & incidentRay,
                        const cs40::vec3 & normal );
    
};

}
#endif // RAYTRACER_H


