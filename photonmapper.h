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

using std::vector;

typedef kd_tree< float , 3 > KDTree;

/* A basic Ray class.*/
namespace cs40{

class PhotonMapper{

public:
    
    PhotonMapper();
    PhotonMapper( int num_photons );
    PhotonMapper( int num_photons, float e );

    ~PhotonMapper();

    void mapScene( const Scene &scene );

    //bool checkIntersection( Photon * photon,
    //                         vec3 & origin,
    //                         Scene & scene );

    vec3 getIllumination( const vec3 & point,
                          const vec3 & incident,
                          const vec3 & normal,
                          const cs40::Material & mat );

    int total_indirect, total_caustic;
    
    float epsilon;
    
    //the numnber of nearest neighbors.
    int N;
    //the range of photon collection if both this, and N are non-zero, then
    //the program will use KNN rather than in range.
    float range;

    float caustic_power, indirect_power;

    bool print;

    
private:

//_________________________Private Variables__________________________________
    KDTree indirect_tree;
    KDTree caustic_tree;
    
    KDTree::kd_point * indirect_positions;
    KDTree::kd_point * caustic_positions;
    
    //stores the other data about the photons
    vector<Photon *> indirect_photons;
    vector<Photon *> caustic_photons;

    int current_indirect, current_caustic;

//___________________________Private Functions________________________________
    void build();

    void tracePhoton(const cs40::Scene & scene , cs40::Ray & incidentRay,
                     const vec3 & incidentColor, bool isCaustic , int depth,
                     int shapeIndex);

    void addIndirect( const vec3 & direction, const vec3 & hitPoint,
                    const vec3 & incidentColor );

    void addCaustic( const vec3 & direction, const vec3 & hitPoint,
                     const vec3 & incidentColor );

    inline KDTree::kd_point vec3_to_kdPoint( const vec3 & vector );


    vec3 getIndirectIllumination( const vec3 & point,
                          const vec3 & incident,
                          const vec3 & normal,
                          const cs40::Material & mat );

    vec3 getCausticIllumination( const vec3 & point,
                                 const vec3 & incident,
                                 const vec3 & normal,
                                 const cs40::Material & mat );

    void kdTest();

    vec3 visualizePhotons( const vec3 & point , float clearance);
    
    inline vec3 brdf( const cs40::Photon * photon, const vec3 & normal,
                                                   const vec3 & incident,
                                                   const cs40::Material & mat);
    inline vec3 phong( const cs40::Photon * photon,
                                              const vec3 & normal,
                                              const vec3 & incident,
                                              const cs40::Material & mat);

};

} //namespace
#endif
