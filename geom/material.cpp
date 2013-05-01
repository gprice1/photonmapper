#include "material.h"
#include "functions.h"

using cs40::Material;

inline vec3 Material::brdfLight( const vec3 & incident,
                                const vec3 & outgoing,
                                const vec3 & normal    ) const {

    float brdfVal = brdf( alpha, beta, incident, outgoing, normal );
    return color * brdfVal;
}

//make this based on the alpha value.
//the incident is the incident light, and the outgoing is the view ray or the
//outgoing light
inline vec3 Material::phongLight(const vec3 & incident,
                                const vec3 & outgoing,
                                const vec3 & normal    ) const {
    
    float k_diff, k_spec;
    k_diff = phongDiffuse(  incident, normal );
    k_spec = phongSpecular( incident, normal,
                            outgoing, 10 );


    return color * (k_diff + k_spec);

}

vec3 Material::getLight(const vec3 & incident,
                        const vec3 & outgoing,
                        const vec3 & normal    ) const {

    //return phongLight( incident, outgoing, normal );
    //
    //for indirect photons, the correct thing is to negate outgoing
    return brdfLight( incident, -outgoing, normal );
}

//returns the "getLight" function weighted by the cosine of the incident light.
vec3 Material::getCosLight(const vec3 & incident,
                           const vec3 & outgoing,
                           const vec3 & normal    ) const {

    float cosine = normal.dotProduct( normal, incident );
    return cosine * getLight( incident, outgoing, normal );
}



