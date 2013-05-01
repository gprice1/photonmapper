#ifndef CS40MAP_H
#define CS40MAP_H

#include <string>
#include <iostream>
#include "common.h"
#include "kd-tree.h"
#include "photon.h"
#include <vector>
#include <mutex>
#include "material.h"

using std::vector;

typedef kd_tree< float , 3 > KDTree;

/* A basic Ray class.*/
namespace cs40{


class Map{

    public:

        Map( int total_photons );      
        ~Map();
        bool isFull(){ return _isFull ; }
        

        int N, total, emitted;
        float epsilon;
        std::mutex lock;
        KDTree tree;
        KDTree::kd_point * positions;

    protected:
        int current;        
        void _buildTree();
        
    private:

        bool _isFull;
        
};

class IlluminationMap : public Map {
    public:
        IlluminationMap( int total_photons );
        ~IlluminationMap();
        void addPhoton( const vec3 & direction,
                               const vec3 & hitPoint,
                               const vec3 & incidentColor );
        vec3 getColor( const vec3 & point, const vec3 & view,
                              const vec3 & normal, 
                              const cs40::Material & mat) const ;

        vector<Photon *> photons;
};


class ShadowMap : public Map{
    public:
        ShadowMap( int total_photons);
        ~ShadowMap();
        void addShadow( const vector< vec3 > & pos );

        //this function returns 1 if the point is totally illuminated,
        //                      0 if the point is imperfectly illuminated,
        //                      -1 if the point is totally shadowed.
        int isInShadow( const vec3 & point );

        bool * isShadow;
};

} //namespace
#endif

