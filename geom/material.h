#ifndef CS40MATERIAL_H
#define CS40MATERIAL_H

#include "common.h"

namespace cs40{

/* a little struct to store material colors */
class Material{
public:
//_________________________PUBLIC VARIABLES________________________________

    vec3 color; //All in rgb space with 0-1 range

    float refractionIndex;

    //this controls the diffuse/specular continuum
    //1 is perfectly diffuse, while 0 is perfectly specular
    float alpha;

    //alpha Inv is simple 1 - alpha, this cuts down on code size and makes 
    //things easier
    float alphaInv;

    //this controls the anisotropic/isotropic continuum
    //1 is perfectly isotropic while 0 is perfectly anisotropic
    float beta;


//________________________PUBLIC FUNCTIONS__________________________________

    vec3 getLight(const vec3 & incident,
                           const vec3 & outgoing,
                           const vec3 & normal    ) const ;
private:

    inline vec3 brdfLight( const vec3 & incident,
                           const vec3 & outgoing,
                           const vec3 & normal    ) const ;
    inline vec3 phongLight(const vec3 & incident,
                           const vec3 & outgoing,
                           const vec3 & normal    ) const ;



};

} //namespace

#endif
