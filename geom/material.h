#ifndef CS40MATERIAL_H
#define CS40MATERIAL_H

#include "common.h"

namespace cs40{

/* a little struct to store material colors */
typedef struct mat_s{
    vec3 ambient; //All in rgb space with 0-1 range
    vec3 diffuse;
    vec3 specular;
    float local;
    float reflection;
    float transmission;
    float refractionIndex;

    //this controls the diffuse/specular continuum
    float alpha;

    //this controls the anisotropic/isotropic continuum
    float beta;

} Material;

} //namespace

#endif
