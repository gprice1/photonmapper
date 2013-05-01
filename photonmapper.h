#ifndef CS40PHOTONMAPPER_H
#define CS40PHOTONMAPPER_H

#include <string>
#include <iostream>
#include "common.h"
#include "kd-tree.h"
#include "photon.h"
#include <vector>
#include <iostream>
#include "scene.h"
#include <mutex>
#include <thread>
#include "map.h"

using std::vector;

typedef kd_tree< float , 3 > KDTree;

/* A basic Ray class.*/
namespace cs40{

inline KDTree::kd_point vec3_to_kdPoint( const vec3 & vector );
    
class PhotonMapper{

public:
    
    PhotonMapper();
    PhotonMapper( int num_shadow, int num_caustic, int num_indirect );
    ~PhotonMapper();

    void mapScene( const Scene &scene );

    vec3 getIllumination( const vec3 & point,
                          const vec3 & incident,
                          const vec3 & normal,
                          const cs40::Material & mat );

    //this function returns 1 if the point is totally illuminated,
    //                      0 if the point is imperfectly illuminated,
    //                      -1 if the point is totally shadowed.
    int isInShadow( const vec3 & point );

//________________________Public Variable____________________________________ 
    
    //the range of photon collection if both this, and N are non-zero, then
    //the program will use KNN rather than in range.
    float range;
    
    float photon_power;

    bool print, visualize, exclude_direct;

    int num_threads;

    cs40::ShadowMap shadow;
    cs40::IlluminationMap caustic;
    cs40::IlluminationMap indirect;


    
private:

//_________________________Private Variables__________________________________
    

    int * caustic_emitted_array;
    int * indirect_emitted_array;



//___________________________Private Functions________________________________
    void build();

    void tracePhoton(const cs40::Scene & scene , cs40::Ray & incidentRay );

    void addIndirect( const vec3 & direction, const vec3 & hitPoint,
                    const vec3 & incidentColor );

    void addCaustic( const vec3 & direction, const vec3 & hitPoint,
                     const vec3 & incidentColor );

    void addShadow( const vector<vec3> & intersectionPoints );

    inline KDTree::kd_point vec3_to_kdPoint( const vec3 & vector );

    vec3 getIndirectIllumination( const vec3 & point,
                          const vec3 & incident,
                          const vec3 & normal,
                          const cs40::Material & mat );

    vec3 getCausticIllumination( const vec3 & point,
                                 const vec3 & incident,
                                 const vec3 & normal,
                                 const cs40::Material & mat );

    void threaded_mapScene( const cs40::Scene &scene, int threadID );
    
    void initThread();

    void endThread();
    
    void kdTest();

    vec3 visualizePhotons( const vec3 & point , float clearance);
    

};

} //namespace
#endif
