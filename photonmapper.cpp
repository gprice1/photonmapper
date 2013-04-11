#include "photonmapper.h"
#include <sstream>
#include <math.h>

using std::rand;
using std::string;
using std::ostream;
using std::stringstream;
using cs40::Photon;
using cs40::PhotonMapper;
using cs40::Scene;
using cs40::Ray;

PhotonMapper::PhotonMapper( ){
    total_photons = 1000;
    indirect_positions = new KDTree::kd_point[ total_photons ];
    epsilon = 0.0f;
    current_photons = 0;
}

PhotonMapper::PhotonMapper( int num_photons ){
    total_photons = num_photons;
    indirect_positions = new KDTree::kd_point[ total_photons ];
    epsilon = 0.0f;
    current_photons = 0;
}

PhotonMapper::PhotonMapper( int num_photons, float e ){
    total_photons = num_photons;
    indirect_positions = new KDTree::kd_point[ total_photons ];     
    epsilon = e;
    current_photons = 0;
}


PhotonMapper::~PhotonMapper(){
    indirect_photons.clear();
    caustic_photons.clear();
    delete [] indirect_positions;
    //delete [] caustic_positions;
}

void PhotonMapper::mapScene( Scene &scene ){
    float theta, phi;
    Photon * photon; // euclidean form of theta, phi
    vec3 direction;

    while ( current_photons < total_photons ){
        if ( current_photons % 10000 == 0 ){
            std::cout << "photon number: " << current_photons << std::endl;
        }
        //std::cout << "shooting photon: " << current_photons << std::endl;
        //shoots a random ray out from a point in any direction.
        //I may want to get a better generator
        
        theta = 2 * M_PI * (float)rand()/(float)RAND_MAX;
        phi   = M_PI     * float( rand() ) / (float)RAND_MAX ;

        direction.setX( sin( phi ) * cos( theta ) );
        direction.setY( sin( phi ) * sin( theta ) );
        direction.setZ( cos( phi ) );

        photon = new Photon( direction );
        
        vec3 origin( scene.lights[0].position );
        //this is a hack and will break with a high complexity
        bool ok = checkIntersection( photon, origin, scene);
        
        if ( ok ){
            //std::cout << "collision occurred" << std::endl;
            current_photons ++;
            indirect_photons.push_back( photon );
        }
    }

    //this makes sure that the correct total gets set at the end.
    if ( total_photons != current_photons){
        std::cout << "total_photons is not equal to current photons"<< std::endl;
        exit( 1 );
    }
    
    build();
}

//builds the trees from the datasets.
void PhotonMapper::build(){
    indirect_tree.build( indirect_positions , current_photons );
    //caustic_tree.build( caustic_positions ,  caustic_positions.size() );
    
}

//TODO I may need to set somthing about not using the current test for collision
//detection
//this function returns false if no collision occurred
//this should input the origin of the light and the scene.
bool PhotonMapper::checkIntersection( Photon * photon, 
                                      vec3 & origin,
                                      Scene & scene ){
    float currentTime , minTime;
    minTime = INFINITY;
    
    KDTree::kd_point point;
    
    Ray incidentRay;
    incidentRay.direction = (*photon).direction;
    incidentRay.origin = origin;
    for ( int i = 0 ; i < scene.objects.size(); i ++ ){

        currentTime = scene.objects[i]->hitTime( incidentRay );
        //std::cout << "current Time: " << currentTime << std::endl;
        //std::cout << "direction: " << incidentRay.direction << std::endl;
        
        if ( (currentTime < minTime ) && ( currentTime > 0 )){
            minTime = currentTime;
        }
    }

    if ( minTime != INFINITY ){
        vec3 hitpoint = incidentRay( minTime );
        indirect_positions[ current_photons ][0] = hitpoint.x();
        indirect_positions[ current_photons ][1] = hitpoint.y();
        indirect_positions[ current_photons ][2] = hitpoint.z();

        current_photons ++ ;
        return true;
    }
    
    return false;
}

//TODO make this do the correct thing;
//returns a color value that will be added in as something
vec3 PhotonMapper::getIllumination( vec3 & point ){
    KDTree::kd_point p;
    p[0] = point.x();
    p[1] = point.y();
    p[2] = point.z();
    
    std::vector<KDTree::kd_neighbour> neighbours;
        
    indirect_tree.knn( p , 1 , neighbours, epsilon );
    if ( neighbours[0].squared_distance > 0.001 ){
        return vec3( 0 , 0 , 0);
    }
    std::cout << "photon hit " << std::endl;
    return vec3( 1,1,1);

}




