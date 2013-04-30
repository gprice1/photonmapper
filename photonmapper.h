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

using std::vector;

typedef kd_tree< float , 3 > KDTree;

/* A basic Ray class.*/
namespace cs40{

inline KDTree::kd_point vec3_to_kdPoint( const vec3 & vector );
    

class Map;
class IlluminationMap;
class ShadowMap;

class PhotonMapper{

public:
    
    PhotonMapper();
    PhotonMapper( int num_photons );
    PhotonMapper( int num_photons, float e );

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

    int total_indirect, total_caustic, total_shadow;
    
    float epsilon;
    
    //the numnber of nearest neighbors.
    int caustic_N, shadow_N, indirect_N;
    //the range of photon collection if both this, and N are non-zero, then
    //the program will use KNN rather than in range.
    float range;
    
    float caustic_power, indirect_power, photon_power;

    bool print, visualize, exclude_direct;

    int num_threads;

    std::mutex caustic_lock;
    std::mutex indirect_lock;
    std::mutex shadow_lock;

    
private:

//_________________________Private Variables__________________________________
    
    ShadowMap shadow;
    IlluminationMap caustic;
    IlluminationMap indirect;



    KDTree indirect_tree;
    KDTree caustic_tree;
    KDTree shadow_tree;
    
    KDTree::kd_point * indirect_positions;
    KDTree::kd_point * caustic_positions;
    KDTree::kd_point * shadow_positions;
    
    //stores the other data about the photons
    vector<Photon *> indirect_photons;
    vector<Photon *> caustic_photons;
    bool * isShadow;  //this array will store whether the photon is a shadow
                      //or illumination photon.

    int current_indirect, current_caustic, current_shadow;
    int emitted_caustic, emitted_indirect;

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
    
    inline vec3 brdfLight( const cs40::Photon * photon, const vec3 & normal,
                                                   const vec3 & incident,
                                                   const cs40::Material & mat);
    inline vec3 phong( const cs40::Photon * photon,
                                              const vec3 & normal,
                                              const vec3 & incident,
                                              const cs40::Material & mat);

    inline vec3 getColor( const KDTree & tree, int N,
                                         const vector< cs40::Photon *> & photons,
                                         const vec3 & point,
                                         const vec3 & normal,
                                         const vec3 & incident,
                                         const cs40::Material & mat);
    void printInfo();

};


class Map{

    public:

        Map( int total_photons );      
        ~Map();
        bool isFull(){ return _isFull ; }

        unsigned int N, total, emitted;
        float epsilon;
        std::mutex lock;
        KDTree tree;
        KDTree::kd_point * positions;

   private:
        unsigned int current;
        bool _isFull;
        void _buildTree();
        
};

class IlluminationMap : public Map {
    public:
        IlluminationMap( int total_photons ) : Map(total_photons);
        ~IlluminationMap();
        void addPhoton( const vec3 & direction,
                        const vec3 & hitPoint,
                        const vec3 & incidentColor );

        vector<Photon *> photons;
};


class ShadowMap : public Map{
    public:
        ShadowMap( int total_photons) : Map(total_photons );
        ~ShadowMap();
        void addShadow( const vector< vec3 > & pos );

        bool * isShadow;
};

} //namespace
#endif
