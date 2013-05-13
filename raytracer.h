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
#include "photonmapper.h"
#include <thread>

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

    void getCommandLineArgs();

    bool get_indirect, get_direct, get_reflect , get_raycast, get_caustic,
         get_monteCarlo;

    int num_threads;

private:
    cs40::Scene m_scene;
    QHash<QString, vec3> m_colors;
    int maxDepth;
    PhotonMapper * p_map;
    bool print;
    int num_raycast;

    //vectors to hold the values of each of these;
    vec3 * indirectColor;
    vec3 * directColor;
    vec3 * reflectColor;
    vec3 * causticColor;
    float indirectPower, directPower, reflectPower, causticPower;
    
    void threadedTrace( RGBImage & img, int threadID );

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

    //parse a light type
    void parseLight(const std::vector<std::string>& words);

    /* convert from 0-1 rgb space to 0-255 */
    cs40::RGBColor convertColor( vec3& clr);

    vec3 traceOnce( const cs40::Ray & incidentRay, int shapeIndex, int depth,
                    int row=-1, int col=-1);

    vec3 monteCarloTrace( const cs40::Ray & incidentRay, int shapeIndex, int depth,
                         float strength, int row=-1, int col=-1 );

    vec3 monteCarloTransmit( const cs40::Ray & incidentRay,
                             const vec3 & hitPoint,
                             const vec3 & normal,
                             const Material & mat,
                             int shapeIndex,
                             int depth,
                             float strength,
                             float & reflectance  );

    vec3 tracePixel( float i, float j );

    vec3 reflect( const cs40::Ray & incidentRay,
                     const vec3 & hitPoint,
                     const vec3 & normal,
                     const Material & mat,
                     int shapeIndex,
                     int depth );


    vec3 rayCast(const vec3 & view,
                 const vec3 & normal,
                 const vec3 & hitPoint,
                 const Material & mat );
    
    //void loadModel( char * objectFile );
    
    void getPhotonMap();

    void adjustStrength( string tag, float factor);
    void multiplyColorVector( vec3 * colorVec, float factor);
    void sumAndSave();

};

}
#endif // RAYTRACER_H
