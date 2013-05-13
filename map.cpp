#include "map.h"
#include "functions.h"

using cs40::Map;
using cs40::IlluminationMap;
using cs40::ShadowMap;
using cs40::Material;
using cs40::Photon;

//a smalll utility function to help me out.
inline KDTree::kd_point vec3_to_kdPoint( const vec3 & vector ){
    KDTree::kd_point p;
    p[0] = vector.x();
    p[1] = vector.y();
    p[2] = vector.z();
    return p;
}



//____________________________the base map class__________________________________
Map::Map( int total_photons ):
     N( 0 ), total( total_photons ), emitted(0), epsilon( 0.0f), current( 0 ) {

    if (total_photons > 0 ){
        positions = new KDTree::kd_point[ total ];
        _isFull = false;
    }else{
        positions = NULL;
        _isFull = true;
    } 
}

Map::~Map(){

    if (total > 0 ){
        delete [] positions;
    }
}

void Map::_buildTree(){
    _isFull = true;    
    tree.build( positions, current );
}




//________________Implementation of illumination maps____________________________
IlluminationMap::IlluminationMap( int total_photons ) : Map(total_photons ){
    if ( total > 0 ){
        photons.reserve( total_photons );
    }
}
IlluminationMap::~IlluminationMap(){
    if ( total > 0 ){
        photons.clear();
    }
}
void IlluminationMap::addPhoton( const vec3 & direction,
                                 const vec3 & hitPoint,
                                 const vec3 & incidentColor ){

    //prevents the addition of too many photons and allows for an early
    //bail out
    if ( isFull() ){ return; } 

  lock.lock();

    if ( isFull() ){ return; }
    
    Photon * photon = new Photon( direction, incidentColor );
    photons.push_back( photon );
    positions[ current ] = vec3_to_kdPoint( hitPoint );

    current ++;

    if ( current == total ){ 
        _buildTree();
    }

  lock.unlock();       
}

vec3 IlluminationMap::getColor( const vec3 & point, const vec3 & view,
                                const vec3 & normal, const Material & mat) const{

    if ( total <= 0 ){ return vec3( 0,0,0); }

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

        color = mat.getLight( -photons[ currentIndex ]->direction, view,
                                  normal );
        filterVal = gaussianFilter( neighbours[i].squared_distance,
                                          maxDistSqrd );
        total_color += filterVal * color * photons[ currentIndex ]->color ;
    } 

    return total_color / emitted;
}





//__________________________shadow map implementation____________________________
ShadowMap::ShadowMap( int total_photons) : Map(total_photons ){
    if ( total > 0 ){
        isShadow = new bool[ total_photons ];
    }
}
ShadowMap::~ShadowMap(){
    if (total > 0 ){
        delete [] isShadow;
    }
}

void ShadowMap::addShadow( const vector< vec3 > & pos ){

    //this allows for a early bailout
    if ( isFull() ){ return ; }

  lock.lock();
    if ( isFull() ){ return ; }
    
    for ( int i = 0; i < pos.size(); i ++ ){
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

//this function returns 1 if the point is totally illuminated,
//                      0 if the point is imperfectly illuminated,
//                      -1 if the point is totally shadowed.

int ShadowMap::isInShadow( const vec3 & point ){
    if ( total == 0 ){ return 0; }

    KDTree::kd_point p = vec3_to_kdPoint( point );
    std::vector<KDTree::kd_neighbour> neighbours;
        
    if ( N > 0 ){
        tree.knn( p , N , neighbours, epsilon );
    }

    int shadowed = 0;

    //add up the amount of shadow photons collected
    for ( int i = 0; i < neighbours.size() ; i ++){
        int currentIndex = neighbours[i].index;

        if ( isShadow[ currentIndex ] ){
            shadowed ++;
        }
    } 

    if ( shadowed == N ){ return -1; } //the point is totally shadowed
    if ( shadowed == 0        ){ return  1; } //the point is totally illuminated
    return 0;                                 //the point is partially illuminated
}



