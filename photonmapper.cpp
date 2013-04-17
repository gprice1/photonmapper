#include "photonmapper.h"
#include <sstream>
#include <math.h>
#include "functions.h"
#include "common.h"
#include <limits.h>

using std::rand;
using std::string;
using std::ostream;
using std::vector;
using std::stringstream;
using std::cout;
using std::endl;
using cs40::Photon;
using cs40::PhotonMapper;
using cs40::Scene;
using cs40::Ray;

PhotonMapper::PhotonMapper( ){
    total_indirect = 2;
    total_caustic  = 100000;
    indirect_photons.reserve( total_indirect );
    caustic_photons.reserve( total_caustic );
    indirect_positions = new KDTree::kd_point[ total_indirect ];
    caustic_positions = new KDTree::kd_point[ total_caustic ];
    epsilon = 0.2;
    current_indirect = 0;
    current_caustic  = 0;
    N = 10;
    caustic_power = 0.1;
    indirect_power = 0.1;
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
    delete [] caustic_positions;
}

void PhotonMapper::mapScene( const Scene &scene ){
    vec3 direction;

    //based on how intense a light is, it will either emit more or less photons
    float totalLight = 0;
    vector< float > intensityMapping;
    for ( int i = 0; i < scene.lights.size() ; i ++ ){
        totalLight += scene.lights[i].intensity;
        intensityMapping.push_back( totalLight );
        cout << "int asldkfm: " << intensityMapping[i] << "\n";
    }
    cout <<  "Intensity: " << totalLight << "\n";
    


    //each photon should account for less of hte lighting if there are more
    //photons, so light_reduction takes care of that.
    //TODO how do I make light_reduction more physically based?
    float lightVal;
    

    while ( current_indirect < total_indirect ||
            current_caustic < total_caustic ){

        if ( current_indirect + current_caustic % 10000 == 0 ){
            std::cout << "photon number: " << current_indirect << std::endl;
        }
        //std::cout << "shooting photon: " << current_photons << std::endl;
        
        //gets a random light from which to emit
        lightVal = (float)rand()/(float)RAND_MAX * totalLight;
        int i = 0;
        while( lightVal > intensityMapping[ i ] ){
            i ++;
        }

        vec3 origin( scene.lights[i].position );
        direction = getRandomDirection();

        Ray incidentRay( origin, direction );
        
        tracePhoton( scene, 
                     incidentRay, 
                     vec3(1, 1, 1 ),
                     true,
                     0,
                     INT_MAX);
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
void PhotonMapper::tracePhoton(const Scene &scene ,
                               Ray & incidentRay,
                               const vec3 & incidentColor,
                               bool isCaustic,
                               int depth,
                               int shapeIndex){
    vec3 hitPoint;
    float randVal;

    shapeIndex = scene.checkIntersection( incidentRay,
                                          shapeIndex,
                                          hitPoint );
    if (shapeIndex == INT_MAX || shapeIndex < 0 ){
        return;
    }

    Shape * shape = scene.objects[ shapeIndex ];
    Material mat = shape->material;
    vec3 normal = shape->normal( hitPoint );
    incidentRay.origin = hitPoint;
    

    randVal = (float)rand()/(float)RAND_MAX;

    //photons only stick on diffuse surfaces, so the higher the alpha, the
    //higher the likelihood the photon will stick.
    //TODO I am only using this for indirect lighting, maybe this is not 
    //how it is meant to be used?????
    if ( mat.alpha > randVal && depth != 0 ){
        if( isCaustic){
            addCaustic( incidentRay.direction, hitPoint, incidentColor );
            isCaustic = false;
        }
        else{
            addIndirect( incidentRay.direction, hitPoint, incidentColor );
        }
    }

    //the photon will either be reflected or absorbed
    //The lower alpha is, the more specular it is, and therefore more likely to
    //reflect a photon.
    randVal = (float)rand()/(float)RAND_MAX;
    if ( mat.alpha > randVal ){
        //photon was absorbed and not reflected
        return;
    }


    //TODO MAKE PROTON REFLECTANCE BASED ON THE BRDF !!!!!!!!!!!!!!!
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    
    //if the photon is reflected, trace another photon
    
    //reflect diffusely
    if ( total_indirect > current_indirect &&
         mat.alpha > (float)rand()/(float)RAND_MAX ){

        incidentRay.direction = cs40::randomHemisphereDirection( normal );
        tracePhoton( scene, incidentRay, incidentColor * mat.diffuse,
                     false, depth+1, shapeIndex );
    }

    //reflect specularly
    else{
        float n1, n2;
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
        //For some reason, the fresnel equation is preventing my program from 
        //seg faulting.
        if ( mat.refractionIndex >  0.1 &&
             cs40::fresnel(incidentRay.direction, normal, n1, n2 ) <
             (float)rand()/(float)RAND_MAX) {
         
            //the light is refracted
            incidentRay.direction = cs40::transmit( incidentRay.direction,
                                                    normal,
                                                    n1, n2 );
            incidentRay.isInsideObject = !( incidentRay.isInsideObject );

            tracePhoton( scene, incidentRay,
                         incidentColor * mat.specular,
                         isCaustic, depth+1 , shapeIndex);
        }
        //the photon is reflected
        else{

            incidentRay.direction = cs40::reflect( incidentRay.direction,
                                                   normal);
            tracePhoton( scene, incidentRay,
                         incidentColor * mat.specular,
                         isCaustic, depth+1, shapeIndex );

        }
    }
}



//builds the trees from the datasets.
void PhotonMapper::build(){
    cout << "Building" << endl;
    //indirect_tree.build( indirect_positions , current_indirect );
    caustic_tree.build( caustic_positions , current_caustic );
    //kdTest();
    
}

vec3 PhotonMapper::getIllumination( const vec3 & point,
                                    const vec3 & incident,
                                    const vec3 & normal,
                                    const Material & mat ){
    vec3 total_color( 0,0,0);
    total_color += getCausticIllumination( point, incident, normal, mat );
    //total_color += getIndirectIllumination( point, incident, normal, mat );

    return total_color ;
}



//TODO make this do the correct thing;
//returns a color value that will be added in as something
vec3 PhotonMapper::getIndirectIllumination( const vec3 & point,
                                        const vec3 & incident,
                                        const vec3 & normal,
                                        const Material & mat ){

    KDTree::kd_point p = vec3_to_kdPoint( point );
    
    std::vector<KDTree::kd_neighbour> neighbours;
        
    indirect_tree.knn( p , N , neighbours, epsilon );

    vec3 total_color, color;
    int currentIndex;

    for ( int i = 0; i < neighbours.size() ; i ++){
        currentIndex = neighbours[i].index;
        color = phong( indirect_photons[ currentIndex ],
                       point, normal, incident, mat);
        //color /= neighbours[i].squared_distance * M_PI;
        total_color += color;
    } 
    total_color /= neighbours[ N - 1 ].squared_distance * M_PI;
    cs40::clampTop( total_color , 1.0 );

    return total_color;

}


vec3 PhotonMapper::getCausticIllumination( const vec3 & point,
                                        const vec3 & incident,
                                        const vec3 & normal,
                                        const Material & mat ){
    KDTree::kd_point p = vec3_to_kdPoint( point );
    
    std::vector<KDTree::kd_neighbour> neighbours;
        
    caustic_tree.knn( p , N , neighbours, epsilon );

    vec3 total_color, color;
    int currentIndex;

    for ( int i = 0; i < neighbours.size() ; i ++){
        currentIndex = neighbours[i].index;
        color = phong( caustic_photons[ currentIndex ],
                       point, normal, incident, mat);
        //color /= neighbours[i].squared_distance * M_PI;
        total_color += color;
    } 
    total_color /= neighbours[ N - 1 ].squared_distance * M_PI;
    cs40::clampTop( total_color , 1.0 );


    return total_color;
}

void PhotonMapper::addIndirect( const vec3 & direction, const vec3 & hitPoint,
                                const vec3 & incidentColor ){
    //prevents the addition of too many photons
    if ( current_indirect >= total_indirect ){
        return;
    }
    current_indirect ++;

    Photon * photon = new Photon( direction,
                                  incidentColor * indirect_power / N );
    indirect_photons.push_back( photon );

    indirect_positions[ current_indirect ] = vec3_to_kdPoint( hitPoint );
        
}

void PhotonMapper::addCaustic( const vec3 & direction, const vec3 & hitPoint,
                     const vec3 & incidentColor ){
    //prevents the addition of too many photons
    if ( current_caustic >= total_caustic){
        return;
    }
    current_caustic ++;

    Photon * photon = new Photon( direction,
                                  incidentColor * caustic_power / N);
    caustic_photons.push_back( photon );

    caustic_positions[ current_caustic ] = vec3_to_kdPoint( hitPoint );
        
}



inline KDTree::kd_point PhotonMapper::vec3_to_kdPoint( const vec3 & vector ){
    KDTree::kd_point p;
    p[0] = vector.x();
    p[1] = vector.y();
    p[2] = vector.z();
    return p;
}

inline vec3 PhotonMapper::phong( const Photon * photon, const vec3 & hitPoint,
                                              const vec3 & normal,
                                              const vec3 & incident,
                                              const Material & mat){

    vec3 color( 0,0,0);
    double k_diffuse, k_specular;

    k_diffuse = cs40::phongDiffuse( photon->direction, normal);
    color += k_diffuse * photon->color;

    k_specular = cs40::phongSpecular( photon->direction,
                                      normal ,
                                      incident,
                                      10);

    color += k_specular * photon->color;

    return color;
}

//this performs a simple test to see if the kd tree is working like I think it
//should.
void PhotonMapper::kdTest( ){
    int number_wrong = 0;
    int misordered = 0;
    int K = 5;
    for( int i = 0; i < current_caustic ; i ++ ){

        KDTree::kd_point p = caustic_positions[ i ];
    
        std::vector<KDTree::kd_neighbour> neighbours;
        
        caustic_tree.knn( p , K , neighbours, 0.0f );

        float previousDist = -1;
        for ( int j = 0; j < K ; j ++){
            if ( previousDist > neighbours[j].squared_distance ){
                cout << "Misordered" << "\n";
                misordered ++;
            }
            previousDist = neighbours[j].squared_distance;
        }


        if ( neighbours[0].index != i ){
            number_wrong ++;
            cout << "incorrect matching at index " << i << "\n";
        }
    }
    cout << "Total # wrong : " << number_wrong << "\n" ;
}


