#include "photonmapper.h"
#include <sstream>

using std::rand;
using std::string;
using std::ostream;
using std::stringstream;
using cs40::Photon;
using cs40::PhotonMapper;
using cs40::Scene;

PhotonMapper::PhotonMapper( ){
    total_photons = 50000;
    epsilon = 0.0f;
    current_photons = 0;
}

PhotonMapper::PhotonMapper( int num_photons ){
    total_photons = num_photons;
    epsilon = 0.0f;
    current_photons = 0;
}

PhotonMapper::PhotonMapper( int num_photons, float e ){
    total_photons = num_photons;
    epsilon = e;
    current_photons = 0;
}


PhotonMapper::~PhotonMapper(){
 //get rid of stuff   
}

void PhotonMapper::mapScene( Scene &scene ){
    float theta, phi;
    vec3 direction; // euclidean form of theta, phi

    while ( current_photons < total_photons ){

        //shoots a random ray out from a point in any direction.
        //I may want to get a better generator
        //
        theta = 2 * M_PI / float( rand() ) / RAND_MAX ;
        phi   = M_PI     / float( rand() ) / RAND_MAX ;

        direction.x = sin( phi ) * cos( theta );
        direction.y = sin( phi ) * sin( theta );
        direction.z = cos( phi );


        std::cout << "trace photon is not implemented";
        break ;

    }

    //this makes sure that the correct total gets set at the end.
    total_photons = current_photons;

}

//builds the trees from the datasets.
void PhotonMapper::build(){
    indirect_tree.build( &indirect_positions[0] , indirect_positions.size() );
    caustic_tree.build( &caustic_positions[0] ,  caustic_positions.size() );
    
}




