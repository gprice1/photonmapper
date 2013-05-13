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
using cs40::ShadowMap;
using cs40::IlluminationMap;
using cs40::Scene;
using cs40::Ray;

PhotonMapper::PhotonMapper() : shadow(0), caustic( 1000 ), indirect( 0 ){
    caustic.N = 100;
    indirect.N = 1;
    shadow.N = 10;
    range = 0.01;

    print = true;
    visualize = false;

    //if this value is set to true, then the photon map will not store photons
    //if they have not been reflected in some way first.
    exclude_direct = false;
}
PhotonMapper::PhotonMapper( int num_indirect, int num_caustic, int num_shadow ) :
    shadow( num_shadow ) , caustic( num_caustic ), indirect( num_indirect ){

    caustic.N = 10;
    indirect.N = 10;
    shadow.N = 0;
    range = 0.01;

    photon_power = 1;

    print = true;
    visualize = false;

    //if this value is set to true, then the photon map will not store photons
    //if they have not been reflected in some way first.
    exclude_direct = false;

    num_threads = 1;
}



PhotonMapper::~PhotonMapper(){
}
        
            
void PhotonMapper::mapScene( const Scene &scene ){
    cout << "Caustic: " << caustic.total << endl;
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
        while ( !shadow.isFull() || !caustic.isFull() || !indirect.isFull() ){
            
            Ray incidentRay = scene.emitPhoton();

            if (!indirect.isFull() ){ indirect.emitted ++; }
            if (!caustic.isFull()  ){ caustic.emitted ++;  }

            tracePhoton( scene, incidentRay );
        }
    }

    cout << "emitted Indirect: " << indirect.emitted << "\n";
    cout << "emitted Caustic: " << caustic.emitted << "\n";    
    
}


void PhotonMapper::threaded_mapScene( const Scene &scene, int threadID ){
    
    while ( !shadow.isFull() || !caustic.isFull() || !indirect.isFull() ){
    

        Ray incidentRay = scene.emitPhoton();

        if (!indirect.isFull() ){ indirect_emitted_array[threadID] ++; }
        if (!caustic.isFull()  ){  caustic_emitted_array[threadID] ++;  }

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
        caustic.emitted  +=  caustic_emitted_array[i];
        indirect.emitted += indirect_emitted_array[i];
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

        if ( indirect.isFull() && !isCaustic ){ return ; }

        vec3 hitPoint;

        // if this is the first tracing directly from the light source,
        // calculate the shadow rays
        if ( i == 0 && !shadow.isFull() ){
            vector< vec3 > intersectionPoints;
            shapeIndex = scene.checkAllIntersections( incidentRay,
                                                      intersectionPoints );

            shadow.addShadow( intersectionPoints );
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
            return;
        }

        //extract information from the hit object such as the material and normal
        Shape * shape = scene.objects[ shapeIndex ];
        Material mat = shape->material;
        vec3 normal = shape->normal( hitPoint );
        incidentRay.origin = hitPoint;

        // the photon will either be reflected or absorbed right now I am
        // using the darkness and lightness of a material to determine
        // absorbtion. Furthermore, the more similar that the colors are, the more 
        // likely that reflection will occur.
        float absorbtionValue = incidentColor.dotProduct( mat.color, incidentColor)
                                / incidentColor.length();

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
        //if exclude_direct is true, then the depth does not matter, the or section
        //will always evaluate to true.
        if ( mat.alpha > randf() && ( !exclude_direct || i != 0) ){
            if( isCaustic && i != 0 ){
                caustic.addPhoton( incidentRay.direction,
                                   hitPoint, incidentColor );
                isCaustic = false;
            }

            indirect.addPhoton( incidentRay.direction, hitPoint, incidentColor );
        }



        //TODO MAKE PROTON REFLECTANCE BASED ON THE BRDF !!!!!!!!!!!!!!!
        //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

        
        float randVal = randf();

        //reflect diffusely
        if ( !indirect.isFull() && mat.alpha > randVal ){
            
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
            return;
        }

        incidentColor *= mat.color;
        if (incidentColor.lengthSquared() > 3 ){
            cout << "Incident Color" << incidentColor;
        }
    }
}



vec3 PhotonMapper::visualizePhotons( const vec3 & point , float clearance){
    KDTree::kd_point p = vec3_to_kdPoint( point );
    
    std::vector<KDTree::kd_neighbour> neighbours;
    
    if ( indirect.total > 0 ){
        indirect.tree.knn( p , 1 , neighbours, indirect.epsilon );
    
        if ( neighbours[0].squared_distance < clearance ){
            return indirect.photons[ neighbours[0].index ]->color;
        }

        neighbours.pop_back();
    }
    if (caustic.total > 0 ){
        caustic.tree.knn( p , 1 , neighbours, caustic.epsilon );
    
        if ( neighbours[0].squared_distance < clearance ){
            return caustic.photons[ neighbours[0].index ]->color;
        
        }
    }

    return vec3();
}


vec3 PhotonMapper::getIllumination( const vec3 & point,
                                    const vec3 & incident,
                                    const vec3 & normal,
                                    const Material & mat, 
                                    const string tag){
    //if the visualize flag is set to true, then use the visualize photons function
    //for the illumination.
    if ( visualize ){
        return visualizePhotons( point, range );
    }
    
    if (tag == "caustic" ){
        return caustic.getColor( point, incident, normal, mat ) * photon_power;
    }

    return indirect.getColor( point, incident, normal, mat ) * photon_power;

}


    //this function returns 1 if the point is totally illuminated,
    //                      0 if the point is imperfectly illuminated,
    //                      -1 if the point is totally shadowed.

int PhotonMapper::isInShadow( const vec3 & point ){
    return shadow.isInShadow( point );
}


//this performs a simple test to see if the kd tree is working like I think it
//should.
void PhotonMapper::kdTest( ){
    int number_wrong = 0;
    int misordered = 0;
    int K = 5;
    for( int i = 0; i < caustic.total ; i ++ ){

        KDTree::kd_point p = caustic.positions[ i ];
    
        std::vector<KDTree::kd_neighbour> neighbours;
        
        caustic.tree.knn( p , K , neighbours, 0.0f );

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

//a smalll utility function to help me out.
inline KDTree::kd_point PhotonMapper::vec3_to_kdPoint( const vec3 & vector ){
    KDTree::kd_point p;
    p[0] = vector.x();
    p[1] = vector.y();
    p[2] = vector.z();
    return p;
}

