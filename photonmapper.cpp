#include "photonmapper.h"
#include <sstream>
#include <math.h>
#include "functions.h"
#include "common.h"

using std::rand;
using std::string;
using std::ostream;
using std::stringstream;
using cs40::Photon;
using cs40::PhotonMapper;
using cs40::Scene;
using cs40::Ray;

PhotonMapper::PhotonMapper( ){
    total_indirect = 10000;
    total_caustic  = 0;
    indirect_positions = new KDTree::kd_point[ total_indirect ];
    //caustic_positions = new KDTree::kd_point[ total_caustic ];
    epsilon = 0.0f;
    current_indirect = 0;
    current_caustic  = 0;
    N = 2;
}

PhotonMapper::PhotonMapper( int num_photons ){
    total_indirect = num_photons;
    indirect_positions = new KDTree::kd_point[ total_indirect ];
    epsilon = 0.0f;
}

PhotonMapper::PhotonMapper( int num_photons, float e ){
    total_indirect = num_photons;
    indirect_positions = new KDTree::kd_point[ total_indirect ];     
    epsilon = e;
}


PhotonMapper::~PhotonMapper(){
    indirect_photons.clear();
    caustic_photons.clear();
    delete [] indirect_positions;
    //delete [] caustic_positions;
}

void PhotonMapper::mapScene( const Scene &scene ){
    float theta, phi;
    vec3 direction;

    while ( current_indirect < total_indirect ){
        if ( current_indirect % 10000 == 0 ){
            std::cout << "photon number: " << current_indirect << std::endl;
        }
        //std::cout << "shooting photon: " << current_photons << std::endl;
        //shoots a random ray out from a point in any direction.
        //I may want to get a better generator
        
        theta = 2 * M_PI * (float)rand()/(float)RAND_MAX;
        phi   = M_PI     * float( rand() ) / (float)RAND_MAX ;

        direction = cs40::angle_to_direction( theta, phi );
 
        vec3 origin( scene.lights[0].position );
        Ray incidentRay( origin, direction );
        
        tracePhoton( scene, incidentRay, vec3( 1, 1, 1) );
    }

    //this makes sure that the correct total gets set at the end.
    if ( total_indirect != current_indirect){
        std::cout << "total_indirect is not equal to current photons"<< std::endl;
        exit( 1 );
    }
    
    build();
}

//TODO make this handle transmission;
//  right now, it only handles transmission going into a surface, not out of
//  one
//  there also may be sign errors on the incident ray direction.
void PhotonMapper::tracePhoton(const Scene &scene , Ray & incidentRay,
                               const vec3 & incidentColor){
    vec3 hitPoint;
    float randVal;

    int shapeIndex = scene.checkIntersection( incidentRay, -1, hitPoint );
    if (shapeIndex < 0 ){
        return;
    }

    Shape * shape = scene.objects[ shapeIndex ];
    Material mat = shape->material;

    randVal = (float)rand()/(float)RAND_MAX;
    //photons only stick on diffuse surfaces, so the higher the alpha, the
    //higher the likelihood the photon will stick.
    if ( mat.alpha > randVal ){
        addIndirect( incidentRay.direction, hitPoint, incidentColor );
    }

    //the photon will either be reflected or absorbed
    //The lower alpha is, the more specular it is, and therefore more likely to
    //reflect a photon.
    randVal = (float)rand()/(float)RAND_MAX;
    if ( mat.alpha > randVal ){
        return;
    }

    //if the photon is reflected, trace another photon
    
    float n1, n2;
    vec3 normal = shape->normal( hitPoint );
    if (incidentRay.isInsideObject){
        normal *= -1;
        n1 = mat.refractionIndex;
        n2 = 1.0;

    }
    else{
        n1 = 1.0;
        n2 = mat.refractionIndex;
    }

    //If the material is refractive, then it calculates the fresnel equation
    //to see how much light is refracted vs reflected. 
    if ( mat.refractionIndex != 0 &&
         cs40::fresnel(incidentRay.direction, normal, n1, n2 ) <
         (float)rand()/(float)RAND_MAX ){

        //the light is refracted
        incidentRay.direction = cs40::transmit( incidentRay.direction, normal,
                                          n1, n2 );
        incidentRay.origin = hitPoint;
        incidentRay.isInsideObject = !( incidentRay.isInsideObject );
        tracePhoton( scene, incidentRay, incidentColor * mat.specular );
    }
    //the photon is reflected
    else{

        incidentRay.direction = cs40::reflect( incidentRay.direction, normal);
        incidentRay.origin = hitPoint;
        tracePhoton( scene, incidentRay, incidentColor * mat.specular );
    }
}



//builds the trees from the datasets.
void PhotonMapper::build(){
    indirect_tree.build( indirect_positions , current_indirect );
    //caustic_tree.build( caustic_positions , current_caustic );
    
}

//TODO make this do the correct thing;
//returns a color value that will be added in as something
vec3 PhotonMapper::getIllumination( const vec3 & point,
                                    const vec3 & incident,
                                    const vec3 & normal,
                                    const Material & mat ){

    KDTree::kd_point p = vec3_to_kdPoint( point );
    
    std::vector<KDTree::kd_neighbour> neighbours;
        
    indirect_tree.knn( p , N , neighbours, epsilon );

    vec3 color;
    int currentIndex;
    Photon * currentPhoton;
    float brdfVal;

    for ( int i = 0; i < neighbours.size() ; i ++){
        currentIndex = neighbours[i].index;
        currentPhoton = indirect_photons[ currentIndex ];
        brdfVal = cs40::brdf( mat.alpha, mat.beta, currentPhoton->direction,
                                                   incident, 
                                                   normal );
        std::cout << "\tBRDF: " << brdfVal << std::endl;
        color += currentPhoton->color * brdfVal; //  / neighbours[i].squared_distance;
    }
    std::cout << "color: " << color << std::endl ;
    

    return color;

}

//TODO need to make this for caustics
void PhotonMapper::addIndirect( const vec3 & direction, const vec3 & hitPoint,
                                const vec3 & incidentColor ){
    //prevents the addition of too many photons
    if ( current_indirect >= total_indirect ){
        return;
    }
    current_indirect ++;

    Photon * photon = new Photon( direction, incidentColor );
    indirect_photons.push_back( photon );

    indirect_positions[ current_indirect ] = vec3_to_kdPoint( hitPoint );
        
}

void PhotonMapper::addCaustic( const vec3 & direction, const vec3 & hitPoint,
                     const vec3 & incidentColor ){

    std::cout << "addCaustic is unimlpemented " << std::endl;
}


inline KDTree::kd_point PhotonMapper::vec3_to_kdPoint( const vec3 & vector ){
    KDTree::kd_point p;
    p[0] = vector.x();
    p[1] = vector.y();
    p[2] = vector.z();
    return p;
}

