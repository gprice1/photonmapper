#ifndef CS40SHAPE_H
#define CS40SHAPE_H

#include "ray.h"
#include <iostream>

using namespace std;

namespace cs40{

inline vec3 reflect( const vec3 & incidentRay, const vec3 & normal ){

     return incidentRay - 2 * normal.dotProduct( normal, incidentRay) * normal;

}


inline vec3 transmit( const vec3 & incidentRay, const vec3 & normal,
                      const float n1, const float n2 ){

        Material material = currentShape->material;
        vec3 transmission;

        //these are constants that make calculating the transmission direction
        //easier
        double n1_over_n2, cosTheta1 , cosTheta2Sqrd;;

        n1_over_n2 = n1 / n2;
        cosTheta1 = normal.dotProduct( -incidentRay.direction , normal );
        cosTheta2Sqrd = 1 - (n1_over_n2 * n1_over_n2) *
                      ( 1 - cosTheta1*cosTheta1);

        //if cosTheta sqrd is less than 1, it is some special case 
        // of emission that decays exponentially in intesity
        //it did not seem worth it to render such a quickly decaying
        //emission, so we return an invalid direction.
        if ( cosTheta2Sqrd < 0 ){
            transmission = vec3( INFINITY , INFINITY , INFINITY);
            return transmission;
        }

        //calculated the direction of the transmission arryr
        transmission  = n1_over_n2 * incidentRay.direction + (
                        n1_over_n2 * cosTheta1 - sqrt(cosTheta2Sqrd)
                        ) * normal;

        if (transmission.length() != 1.0 ){ 
            cout << "transission length : " << transmission << endl;
            transmission.normalize()
        }

        return transmission;

}

//to keep with convection, direction should be the direction of the
//ray going into the surface
inline vec3 fresnel( const vec3 & direction, const vec3 & normal,
                     float n1, float n2 ){
    float R0 = ( n1 - n2 ) / ( n1 + n2 );
    R0 = R0 * R0;

    float cosTheta = normal.dotProduct( -direction , normal );
    cosTheta = 1 - cosTheta;
    float cosTheta_5 = cosTheta * cosTheta;
    cosTheta_5 *= cosTheta_5;
    cosTheta_5 *= cosTheta;

    return R0 + (1 - R0) * cosTheta_5;
}
    
inline vec3 angle_to_direction( float theta, float phi ){
    vec3 direction;
    direction.setX( sin( phi ) * cos( theta ) );
    direction.setY( sin( phi ) * sin( theta ) );
    direction.setZ( cos( phi ) );

    return direction;

}


}//namespace

#endif
