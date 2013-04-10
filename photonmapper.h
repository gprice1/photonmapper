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


/* A basic Ray class.*/
namespace cs40{

class PhotonMapper{

public:
    
    PhotonMapper();
    PhotonMapper( int num_photons );
    PhotonMapper( int num_photons, float e );

    ~PhotonMapper();

    mapScene( Scene &scene );
    
private:

//_________________________Private Variables__________________________________
    typedef kd_tree< float , 3 > KDTree;

    KDTree indirect_tree;
    KDTree caustic_tree;
    
    //stores the positions of the photons
    vector< KDTree::kd_point > indirect_positions;
    vector< KDTree::kd_point > caustic_positions;

    //stores the other data about the photons
    vector< Photon > indirect_photons;
    vector< Photon > caustic_photons;

    int total_photons, current_photons;


//___________________________Private Functions________________________________
    void build();

};

} //namespace
#endif