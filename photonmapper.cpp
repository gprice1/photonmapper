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
    total_caustic  = 10000;
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

    emitted_indirect = 0;
    emitted_caustic = 0;

    caustic_N = 50;
    indirect_N = 200;
    shadow_N = 40;

    range = 0.001;

    caustic_power = 0.1;
    indirect_power = 0.03;
    photon_power = 10000;

    print = true;
    visualize = true;

    //if this value is set to true, then the photon map will not store photons
    //if they have not been reflected in some way first.
    exclude_direct = false;
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
    if ( num_threads > 1 ){

        initThread();

        vector< std::thread > threads; 

        //spawn threads.
        for ( int i = 1; i < num_threads; i ++ ){
            threads.emplace_back( &PhotonMapper::threaded_mapScene, this,
                                  std::ref( scene ), i );
        }

        threaded_mapScene( scene, 0 );

        for( int i = 0; i < threads.size() ; i ++ ){
            threads[i].join();
        }

        endThread();
    }


    //each photon should account for less of hte lighting if there are more
    //photons, so light_reduction takes care of that.
    //TODO how do I make light_reduction more physically based?
    else{
        while ( current_indirect < total_indirect ||
                current_caustic < total_caustic   ||
                current_shadow  < total_shadow    ){
            
            if ( print ){
                printInfo();
            }

            Ray incidentRay = scene.emitPhoton();

            if (current_indirect < total_indirect){
                emitted_indirect ++;
            }
            if ( current_caustic < total_caustic ){
                emitted_caustic ++;
            }

            tracePhoton( scene, incidentRay );
        }
    }

    cout << "emitted Indirect: " << emitted_indirect << "\n";
    cout << "emitted Caustic: " << emitted_caustic << "\n";    
    
    build();
}


void PhotonMapper::threaded_mapScene( const Scene &scene, int threadID ){

    //each photon should account for less of hte lighting if there are more
    //photons, so light_reduction takes care of that.
    //TODO how do I make light_reduction more physically based?
    
    while ( current_indirect < total_indirect ||
            current_caustic < total_caustic   ||
            current_shadow  < total_shadow    ){
    

        Ray incidentRay = scene.emitPhoton();

        if (current_indirect < total_indirect){
            indirect_emitted_array[threadID] ++;
        }
        if ( current_caustic < total_caustic ){
            caustic_emitted_array[threadID] ++;
        }

        tracePhoton( scene, incidentRay);
    }
}


void PhotonMapper::initThread(){
    caustic_emitted_array = new int[num_threads];
    indirect_emitted_array = new int[num_threads];

    for ( int i = 0; i < num_threads ; i ++ ){
        caustic_emitted_array[i] = 0;
        indirect_emitted_array[i] = 0;
    }
}

void PhotonMapper::endThread(){
    for ( int i = 0; i < num_threads ; i ++ ){
        emitted_caustic += caustic_emitted_array[i];
        emitted_indirect += indirect_emitted_array[i];
    }

    delete [] indirect_emitted_array;
    delete [] caustic_emitted_array;
}



//TODO make this handle transmission;
//  right now, it only handles transmission going into a surface, not out of
//  one
//  there also may be sign errors on the incident ray direction.
void PhotonMapper::tracePhoton(const Scene &scene ,
                               Ray & incidentRay   ){

    vec3 incidentColor( 1, 1, 1 );
    bool isCaustic = true;
    int shapeIndex = -1;

    //the max depth of photon bouncing is 20.
    for ( int i = 0; i < 20 ; i ++ ){

        if ( current_indirect >= total_indirect && !isCaustic ){ return ; }

        vec3 hitPoint;

        // if this is the first tracing directly from the light source,
        // calculate the shadow rays
        if ( i == 0 && current_shadow < total_shadow){
            vector< vec3 > intersectionPoints;
            shapeIndex = scene.checkAllIntersections( incidentRay,
                                                      intersectionPoints );

            addShadow( intersectionPoints );
            if ( intersectionPoints.size() > 0 ) {
                hitPoint = intersectionPoints.back();
            }

        }
        else{
            shapeIndex = scene.checkIntersection( incidentRay,
                                                  shapeIndex,
                                                  hitPoint );
        }
        
        //if the photon does not collide with anything, then break from the loop
        if (shapeIndex == INT_MAX || shapeIndex < 0 ){
            break;
        }

        //extract information from the hit object such as the material and normal
        Shape * shape = scene.objects[ shapeIndex ];
        Material mat = shape->material;
        vec3 normal = shape->normal( hitPoint );
        incidentRay.origin = hitPoint;

        //the photon will either be reflected or absorbed
        //right now I am using the darkness and lightness of a material to determine
        //absorbtion. Furthermore, the more similar that the colors are, the more 
        //likely that reflection will occur.
        float absorbtionValue = incidentColor.dotProduct( mat.color, incidentColor ) 
                                / incidentColor.length();

        //A totally white surface will never absorb a photon, and a totally black
        //surface will always absorb photons
        if ( absorbtionValue < randf() ){
            //photon was absorbed and not reflected
            break;
        }
        
        //photons only stick on diffuse surfaces, so the higher the alpha, the
        //higher the likelihood the photon will stick.
        //TODO I am only using this for indirect lighting, maybe this is not 
        //how it is meant to be used?????
        //if exclude_direct is true, then the depth does not matter,the or section
        //will always evaluate to true.
        if ( mat.alpha > randf() && ( !exclude_direct || i != 0) ){
            if( isCaustic && i != 0 ){
                addCaustic( incidentRay.direction, hitPoint, incidentColor );
                isCaustic = false;
            }

            addIndirect( incidentRay.direction, hitPoint, incidentColor );
        }



        //TODO MAKE PROTON REFLECTANCE BASED ON THE BRDF !!!!!!!!!!!!!!!
        //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

        
        float randVal = randf();

        //reflect diffusely
        if ( current_indirect <= total_indirect &&
             mat.alpha > randVal ){
            
            incidentRay.direction = cs40::cosWeightedRandomHemisphereDirection(
                                                                         normal );
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

            }
            //the photon is reflected
            else{
                incidentRay.direction = cs40::reflect( incidentRay.direction,
                                        normal);
            }
        }
        //if no reflections occur, then return.
        else{
            break;
        }
        incidentColor *= mat.color;
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
        vec3 caustic = getColor( caustic_tree, caustic_N, caustic_photons,
                                  point, incident, normal, mat );
        color += caustic / emitted_caustic * photon_power;
    }
    if ( current_indirect > 0 ){
        vec3 indirect = getColor( indirect_tree, indirect_N, indirect_photons,
                                  point, incident, normal, mat );
        color += indirect / emitted_indirect * photon_power;
    }
    if ( visualize ){
        color = visualizePhotons( point, range );
    }
    return color ;

}



inline vec3 PhotonMapper::getColor( const KDTree & tree, int N, 
                                              const vector< Photon *> & photons,
                                              const vec3 & point,
                                              const vec3 & normal,
                                              const vec3 & view,
                                              const cs40::Material & mat){
    vec3 total_color;
    float maxDistSqrd;

    KDTree::kd_point p = vec3_to_kdPoint( point );
    std::vector<KDTree::kd_neighbour> neighbours;
        
    tree.knn( p , N , neighbours, epsilon );
    maxDistSqrd = neighbours[ N - 1 ].squared_distance;


    for ( int i = 0; i < neighbours.size() ; i ++){
        float filterVal;
        int currentIndex;
        vec3 color;
        
        currentIndex = neighbours[i].index;

        color = mat.getLight( -view, -photons[ currentIndex ]->direction,
                              normal );
        color *= normal.dotProduct( photons[ currentIndex ]->direction , normal );

        filterVal = gaussianFilter( neighbours[i].squared_distance, 
                                          maxDistSqrd );
        total_color += filterVal * color * photons[ currentIndex ]->color ;
    } 

    return total_color;

}


void PhotonMapper::addIndirect( const vec3 & direction, const vec3 & hitPoint,
                                const vec3 & incidentColor ){


    //prevents the addition of too many photons
    if ( current_indirect >= total_indirect ){
        return;
    }

  if ( num_threads > 1 ) { indirect_lock.lock(); }    

    Photon * photon = new Photon( direction, incidentColor );

    indirect_photons.push_back( photon );
    indirect_positions[ current_indirect ] = vec3_to_kdPoint( hitPoint );

    current_indirect ++;

  if ( num_threads > 1 ) { indirect_lock.unlock(); }
    
}

void PhotonMapper::addCaustic( const vec3 & direction, const vec3 & hitPoint,
                               const vec3 & incidentColor ){


    //prevents the addition of too many photons
    if ( current_caustic >= total_caustic){
        return;
    }
    cout << "starting caustic add" << endl;
    

  if ( num_threads > 1 ) { caustic_lock.lock(); }
    
    Photon * photon = new Photon( direction, incidentColor );

    caustic_photons.push_back( photon );
    caustic_positions[ current_caustic ] = vec3_to_kdPoint( hitPoint );

    current_caustic ++;

  if ( num_threads > 1 ) { caustic_lock.unlock(); }

    cout << "ending caustic add" << endl;
  
        
}

//this takes in a vector of positions for shadow photons.
//the last member of the list is the only item that is in light,
//all the other photons represent a lack of light.

void PhotonMapper::addShadow( const vector< vec3 > & positions ){


    if ( current_shadow >= total_shadow ){ return ; }

  if ( num_threads > 1 ) { shadow_lock.lock(); }
    
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

  if ( num_threads > 1 ) { shadow_lock.unlock(); }

}

//if the point has only 
int PhotonMapper::isInShadow( const vec3 & point ){
    if ( current_shadow == 0 ){ return 0; }

    KDTree::kd_point p = vec3_to_kdPoint( point );
    std::vector<KDTree::kd_neighbour> neighbours;
        
    if ( shadow_N > 0 ){
        shadow_tree.knn( p , shadow_N , neighbours, epsilon );
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


void PhotonMapper::printInfo(){
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





Map::Map( int total_photons ){
    if (total_photons > 0 ){
        positions = new KDTree::kd_point[ total_indirect ];
        isFull = false;
    }else{
        positions = NULL;
        isFull = true;
    }
    total = total_photons;
}

Map::~Map(){

    if (total_photons > 0 ){
        delete [] positions;
    }
}
Map::_buildTree(){
    tree.build( positions, current );
    _isFull = true;
}

//________________Implementation of illumination maps_______________________
IlluminationMap::IlluminationMap : Map(total_photons ){
    if ( total_photons > 0 ){
        photons.reserve( total_indirect );
    }
}
IlluminationMap::~IlluminationMap(){
    if ( total_photons > 0 ){
        photons.clear();
    }
}
void IlluminationMap::addPhoton( const vec3 & direction,
                                 const vec3 & hitPoint,
                                 const vec3 & incidentColor ){

    //prevents the addition of too many photons
    if ( _isFull ){ return; } 

  lock.lock();
    
    Photon * photon = new Photon( direction, incidentColor );
    photons.push_back( photon );
    positions[ current ] = vec3_to_kdPoint( hitPoint );

    current ++;

    if ( current == total ){ 
        _buildTree();
    }

  lock.unlock();       
}



ShadowMap::ShadowMap( int total_photons) : Map(total_photons ){
     isShadow = new bool[ total_photons ];
}
ShadowMap::~ShadowMap(){
    if (total_photons > 0 ){
        delete [] isShadow;
    }
}

void ShadowMap::addShadow( const vector< vec3 > & pos ){

    if ( _isFull ){ return ; }

  lock.lock();
    
    for ( int i = 0; i < positions.size(); i ++ ){
        positions[ current ] = vec3_to_kdPoint( pos[i] );
        
        //if the photon is the last one in the list, then it is a light photon,
        //not a shadow photon
        if (i == pos.size() - 1 ){
            isShadow[ current ] = false;
        }
        else{
            isShadow[ current ] = true;
        }

        current ++;
        if ( current == total ){ 
            _buildTree();
            lock.unlock();
            return ;
        } 
    }

  lock.unlock();

}


//a samll utility function to help me out.
inline KDTree::kd_point vec3_to_kdPoint( const vec3 & vector ){
    KDTree::kd_point p;
    p[0] = vector.x();
    p[1] = vector.y();
    p[2] = vector.z();
    return p;
}

