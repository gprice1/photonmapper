#ifndef CS40RAYTRACER_H
#define CS40RAYTRACER_H

#include <vector>
#include <string>
#include <QHash>      //Qt Dictionary
#include <QString>    //Appease QT with its strings
#include "scene.h"
#include "material.h"
#include "rgbColor.h"
#include "rgbImage.h"

namespace cs40{

class RayTracer{

public:
    RayTracer();
    ~RayTracer();

    /* parse an input file given by a filename and build the global
     * scene object from the parsed input */
    void parseFile(const std::string& fname);

    /* Do the raytracing */
    void trace(RGBImage & img);

    /* Save output image */
    void save();

private:
    cs40::Scene m_scene;
    QHash<QString, vec3> m_colors;
    int maxDepth;

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
    cs40::RGBColor convertColor( vec3& clr);

    vec3 traceOnce( const cs40::Ray & incidentRay, int shapeIndex, int step );

    /* take an incident ray, and find what it intersects
     * set the status of the ray based on what happened
     * set the position of the ray to the point of intersection
     * finally, return the shape that it intersected with */
    int checkIntersection( const cs40::Ray & incidentRay, vec3 & hitPoint, int shapeIndex );

    //takes an incident ray and returns the collision time of the closest
    //colliding object.
    float collisionTime( const cs40::Ray & incidentRay, int shapeIndex);

    //does the phong coloration.
    //also takes care of shadowing.
    vec3 phong( const cs40::Ray & incidentRay ,
                vec3 & normal,
                vec3 & hitPoint,
                int shapeIndex);

    /* takes an incident ray and a normal vector and comutes a
     * reflection ray. */
    vec3 reflect( const vec3 & incidentRay,
                  const vec3 & normal );

    /* takes an incident ray and a normal vector and comutes a
     * transmission ray. This function likely needs a refraction index*/
    cs40::Ray transmit( const cs40::Ray & incidentRay,
                        vec3 normal,
                        Shape * currentShape );

    void loadModel( char * objectFile );
    


};

}
#endif // RAYTRACER_H