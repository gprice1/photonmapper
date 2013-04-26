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
    total_indirect = 0;
    total_caustic  = 200000;
    total_shadow   = 0;

    if ( total_indirect > 0 ){

        indirect_positions = new KDTree::kd_point[ total_indirect ];
        indirect_photons.reserve( total_indirect );
    }
    else {
        indirect_positions = NULL;
    }
    
    if ( total_caustic > 0 ){
        caustic_positions = new KDTree::kd_point[ total_caustic ];
        caustic_photons.reserve( total_caustic );
    }
    else {
        caustic_positions = NULL;
    }
    
    if ( total_shadow > 0 ){
        shadow_positions = new KDTree::kd_point[ total_shadow ];
        isShadow = new bool[ total_shadow ]; 
    }
    else {
        shadow_positions = NULL;
        isShadow         = NULL;
    }

    epsilon = 0.0f;
    current_indirect = 0;
    current_caustic  = 0;
    current_shadow   = 0;

    indirect_emitted = 0;
    caustic_emitted = 0;

    caustic_N = 50;
    indirect_N = 500;
    shadow_N = 40;

    range = 0.001;

    caustic_power = 0.1;
    indirect_power = 0.001;

    print = true;
    visualize = false;
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
    if ( total_indirect > 0 ){
        delete [] indirect_positions;
        indirect_photons.clear();
    }

    if ( total_caustic > 0 ){
        caustic_photons.clear();
        delete [] caustic_positions;
    }
    if( current_shadow > 0 ){
        delete [] isShadow;
        delete [] shadow_positions;
    }
}

void PhotonMapper::mapScene( const Scene &scene ){


    //each photon should account for less of hte lighting if there are more
    //photons, so light_reduction takes care of that.
    //TODO how do I make light_reduction more physically based?
    
    while ( current_indirect < total_indirect ||
            current_caustic < total_caustic   ||
            current_shadow  < total_shadow    ){
    
        if ( print ){
            int percentIndirect, percentCaustic;
            int indirectDivisor, causticDivisor;

            indirectDivisor = total_indirect / 100 + 1;
            causticDivisor  = total_caustic / 100 + 1;

            //get the percent done
            if ( current_indirect % indirectDivisor == 0 ||
                 current_caustic  % causticDivisor  == 0 ){

                percentIndirect = current_indirect / indirectDivisor;
                percentCaustic  = current_caustic  / causticDivisor;
                std::cout << "indirect: " << percentIndirect << "\t\t";
                std::cout << "caustic: " <<  percentCaustic  << endl;
            
            }
        }

        Ray incidentRay = scene.emitPhoton();

        if (current_indirect < total_indirect){
            indirect_emitted ++;
        }
        if ( current_caustic < total_caustic ){
            caustic_emitted ++;
        }

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
        exit( -1 );
    }
    //this makes sure that the correct total gets set at the end.
    if ( total_caustic != current_caustic){
        std::cout << "total_caustic is not equal to current photons"<< std::endl;
        exit( -1 );
    }

    cout << "emitted Indirect: " << indirect_emitted << "\n";
    cout << "emitted Caustic: " << caustic_emitted << "\n";    
    
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

    if ( depth > 20 ){ return ; }

    vec3 hitPoint;

    // if this is the first tracing directly from the light source,
    // calculate the shadow rays
    if ( depth == 0 && current_shadow < total_shadow){
        vector< vec3 > intersectionPoints;
        shapeIndex = scene.checkAllIntersections( incidentRay,
                                                  intersectionPoints );

        addShadow( intersectionPoints );
        if ( intersectionPoints.size() > 0 ) {
            hitPoint = intersectionPoints.back();
        }

        if ( current_indirect >= total_indirect &&
             current_caustic >= total_caustic ){
            return ;
        }

    }else{
        shapeIndex = scene.checkIntersection( incidentRay,
                                              shapeIndex,
                                              hitPoint );
    }

    if (shapeIndex == INT_MAX || shapeIndex < 0 ){
        return;
    }

    Shape * shape = scene.objects[ shapeIndex ];
    Material mat = shape->material;
    vec3 normal = shape->normal( hitPoint );
    incidentRay.origin = hitPoint;

    //the photon will either be reflected or absorbed
    //TODO : find a better model of absorption.
    //right now I am using the darkness and lightness of a material to determine
    //absorbtion. 
    float absorbtionValue = mat.color.x() + mat.color.y() +  mat.color.z();
    absorbtionValue /= 3;

    //A totally white surface will never absorb a photon, and a totally black
    //surface will always absorb photons
    if ( absorbtionValue < randf() ){
        //photon was absorbed and not reflected
        return;
    }
    
    //photons only stick on diffuse surfaces, so the higher the alpha, the
    //higher the likelihood the photon will stick.
    //TODO I am only using this for indirect lighting, maybe this is not 
    //how it is meant to be used?????
    if ( mat.alpha > randf() && depth != 0 ){
        if( isCaustic){
            addCaustic( incidentRay.direction, hitPoint, incidentColor );
            addIndirect( incidentRay.direction, hitPoint, incidentColor );
            isCaustic = false;
        }
        else{
            addIndirect( incidentRay.direction, hitPoint, incidentColor );
        }
    }



    //TODO MAKE PROTON REFLECTANCE BASED ON THE BRDF !!!!!!!!!!!!!!!
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    
    float randVal = randf();

    //reflect diffusely
    if ( current_indirect < total_indirect &&
         mat.alpha > randVal ){
        
        incidentRay.direction = cs40::randomHemisphereDirection( normal );
        
        tracePhoton( scene, incidentRay, incidentColor * mat.color,
                     false, depth+1, shapeIndex ); 
    }

    //reflect specularly
    else if( mat.alpha < randVal ){
        
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
             cs40::fresnel(incidentRay.direction, normal, n1, n2 ) < randf() ) {
         
            //the light is refracted
            incidentRay.direction = cs40::transmit( incidentRay.direction,
                                                    normal,
                                                    n1, n2 );
            incidentRay.isInsideObject = !( incidentRay.isInsideObject );

            tracePhoton( scene, incidentRay,
                         incidentColor * mat.color,
                         isCaustic, depth+1 , shapeIndex);
        }
        //the photon is reflected
        else{

            incidentRay.direction = cs40::reflect( incidentRay.direction,
                            normal);
            tracePhoton( scene, incidentRay,
                         incidentColor * mat.color,
                         isCaustic, depth+1, shapeIndex );

        }
    }
    
}



//builds the trees from the datasets.
void PhotonMapper::build(){
    cout << "Building" << endl;
    if ( current_indirect > 0 ){
        indirect_tree.build( indirect_positions , current_indirect );
    }
    if ( current_caustic > 0 ){
        caustic_tree.build( caustic_positions , current_caustic );
    }
    if ( current_shadow > 0 ){
        shadow_tree.build( shadow_positions , current_shadow );
    }

    //kdTest();
    
}

vec3 PhotonMapper::visualizePhotons( const vec3 & point , float clearance){
    KDTree::kd_point p = vec3_to_kdPoint( point );
    
    std::vector<KDTree::kd_neighbour> neighbours;
        
    indirect_tree.knn( p , 1 , neighbours, epsilon );
    
    if ( neighbours[0].squared_distance < clearance ){
        return indirect_photons[ neighbours[0].index ]->color;
    }

    neighbours.pop_back();
    caustic_tree.knn( p , 1 , neighbours, epsilon );
    
    if ( neighbours[0].squared_distance < clearance ){
        return caustic_photons[ neighbours[0].index ]->color;
        
    }

    return vec3();
}



vec3 PhotonMapper::getIllumination( const vec3 & point,
                                    const vec3 & incident,
                                    const vec3 & normal,
                                    const Material & mat ){

    vec3 color(0,0,0);

    if ( current_caustic > 0 ){
        color = getCausticIllumination( point, incident, normal, mat );
    }
    if ( current_indirect > 0 ){
        color += getIndirectIllumination( point, incident, normal, mat );
    }
    if ( visualize ){
        color = visualizePhotons( point, range );
    }
    return color ;
}



//TODO make this do the correct thing;
//returns a color value that will be added in as something
vec3 PhotonMapper::getIndirectIllumination( const vec3 & point,
                                        const vec3 & incident,
                                        const vec3 & normal,
                                        const Material & mat ){
    vec3 total_color, color;
    int currentIndex;
    float maxDistSqrd;

    KDTree::kd_point p = vec3_to_kdPoint( point );
    
    std::vector<KDTree::kd_neighbour> neighbours;
        
    if ( indirect_N > 0 ){
        indirect_tree.knn( p , indirect_N , neighbours, epsilon );
        maxDistSqrd = neighbours[ N - 1 ].squared_distance;
    }else {
        indirect_tree.all_in_range( p, range, neighbours );
        maxDistSqrd = range * range;
    }

    for ( int i = 0; i < neighbours.size() ; i ++){
        currentIndex = neighbours[i].index;
        color = phong( indirect_photons[ currentIndex ],
                       normal, incident, mat);

        float filterVal = gaussianFilter( neighbours[i].squared_distance, 
                                          maxDistSqrd );
        total_color += color * filterVal;
    } 


    //divide out by the number of photons emitted.
    total_color /= M_PI * maxDistSqrd;
    //total_color /= indirect_emitted;
    total_color *= indirect_power;

    //total_color *= indirect_power;
    //cs40::clampTop( total_color , 1.0 );

    return total_color;

}


vec3 PhotonMapper::getCausticIllumination( const vec3 & point,
                                        const vec3 & incident,
                                        const vec3 & normal,
                                        const Material & mat ){
    vec3 total_color, color;
    int currentIndex;
    //constants for gaussian filtering

    float maxDistSqrd;

    KDTree::kd_point p = vec3_to_kdPoint( point );
    
    std::vector<KDTree::kd_neighbour> neighbours;
        
    if ( caustic_N > 0 ){
        caustic_tree.knn( p , caustic_N , neighbours, epsilon );
        maxDistSqrd = neighbours[ N - 1 ].squared_distance;

    }else {
        caustic_tree.all_in_range( p, range, neighbours );
        maxDistSqrd = range * range;
    }


    for ( int i = 0; i < neighbours.size() ; i ++){
        currentIndex = neighbours[i].index;
        //color = phong( caustic_photons[ currentIndex ],
        //               normal, incident, mat);
        color = brdfLight( caustic_photons[ currentIndex ],
                        normal, incident, mat);

        //color /= neighbours[i].squared_distance * M_PI;
        float filterVal = gaussianFilter( neighbours[i].squared_distance, 
                                          maxDistSqrd );
        total_color += color * filterVal;
    } 

    total_color *= caustic_power;
    //cs40::clampTop( total_color , 1.0 );

    return total_color;
}

void PhotonMapper::addIndirect( const vec3 & direction, const vec3 & hitPoint,
                                const vec3 & incidentColor ){
    //prevents the addition of too many photons
    if ( current_indirect >= total_indirect ){
        return;
    }

    Photon * photon = new Photon( direction, incidentColor );

    indirect_photons.push_back( photon );
    indirect_positions[ current_indirect ] = vec3_to_kdPoint( hitPoint );

    current_indirect ++;
    
        
}

void PhotonMapper::addCaustic( const vec3 & direction, const vec3 & hitPoint,
                     const vec3 & incidentColor ){
    //prevents the addition of too many photons
    if ( current_caustic >= total_caustic){
        return;
    }

    Photon * photon = new Photon( direction, incidentColor );

    caustic_photons.push_back( photon );
    caustic_positions[ current_caustic ] = vec3_to_kdPoint( hitPoint );

    current_caustic ++;
        
}

//this takes in a vector of positions for shadow photons.
//the last member of the list is the only item that is in light,
//all the other photons represent a lack of light.

void PhotonMapper::addShadow( const vector< vec3 > & positions ){
    if ( current_shadow >= total_shadow ){ return ; }
    for ( int i = 0; i < positions.size(); i ++ ){
        shadow_positions[ current_shadow ] = vec3_to_kdPoint( positions[i] );
        
        //if the photon is the last one in the list, then it is a light photon,
        //not a shadow photon
        if (i == positions.size() - 1 ){
            isShadow[ current_shadow ] = false;
        }
        else{
            isShadow[ current_shadow ] = true;
        }

        current_shadow ++;
        if ( current_shadow >= total_shadow ){ return ; }
        
    }
}

//if the point has only 
int PhotonMapper::isInShadow( const vec3 & point ){
    if ( current_shadow == 0 ){ return 0; }

    KDTree::kd_point p = vec3_to_kdPoint( point );
    std::vector<KDTree::kd_neighbour> neighbours;
        
    if ( shadow_N > 0 ){
        shadow_tree.knn( p , N , neighbours, epsilon );
    }

    int shadowed = 0;
    int currentIndex;

    //add up the amount of shadow photons collected
    for ( int i = 0; i < neighbours.size() ; i ++){
        currentIndex = neighbours[i].index;

        if ( isShadow[ currentIndex ] ){
            shadowed ++;
        }
    } 

    if ( shadowed == shadow_N ){ return -1; } //the point is totally shadowed
    if ( shadowed == 0        ){ return  1; } //the point is totally illuminated
    return 0;                                 //the point is partially illuminated
}


inline KDTree::kd_point PhotonMapper::vec3_to_kdPoint( const vec3 & vector ){
    KDTree::kd_point p;
    p[0] = vector.x();
    p[1] = vector.y();
    p[2] = vector.z();
    return p;
}

inline vec3 PhotonMapper::phong( const Photon * photon,
                                 const vec3 & normal,
                                 const vec3 & incident,
                                 const Material & mat){

    vec3 color( 0,0,0);
    double k_diffuse, k_specular;

    k_diffuse = cs40::phongDiffuse( -photon->direction, normal);
    color += k_diffuse * photon->color;
    k_specular = cs40::phongSpecular( -photon->direction,
                                       normal ,
                                      -incident,
                                       10);
    color += k_specular * photon->color;
    return color;
}


inline vec3 PhotonMapper::brdfLight(const Photon * photon,                                                             const vec3 & normal,
                                    const vec3 & incident,
                                    const Material & mat){

    double brdfVal = brdf( mat.alpha, mat.beta, -photon->direction, 
                                                -incident,
                                                normal);
    return photon->color * brdfVal;

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




