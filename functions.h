#ifndef CS40FUNCTIONS_H
#define CS40FUNCTIONS_H

#include "ray.h"
#include <iostream>
#include <math.h>
#include "common.h"


namespace cs40{

inline vec3 reflect( const vec3 & incidentRay, const vec3 & normal ){
     return incidentRay - 2 * normal.dotProduct( normal, incidentRay) * normal;
}


inline vec3 transmit( const vec3 & direction, const vec3 & normal,
                      const float n1, const float n2 ){

    vec3 transmission;

    //these are constants that make calculating the transmission direction
    //easier
    double n1_over_n2, cosTheta1 , cosTheta2Sqrd;;

    n1_over_n2 = n1 / n2;
    cosTheta1 = normal.dotProduct( -direction , normal );
    cosTheta2Sqrd = 1 - (n1_over_n2 * n1_over_n2) *
                      ( 1 - cosTheta1*cosTheta1);

    //if cosTheta sqrd is less than 1, it is some special case 
    // of emission that decays exponentially in intesity
    //it did not seem worth it to render such a quickly decaying
    //emission, so we return an invalid direction.
    if ( cosTheta2Sqrd < 0 ){
        float inf = std::numeric_limits<float>::infinity();
        transmission = vec3( inf , inf , inf);
        return transmission;
    }

    //calculated the direction of the transmission arryr
    transmission  = n1_over_n2 * direction + (
                    n1_over_n2 * cosTheta1 - sqrt(cosTheta2Sqrd)
                    ) * normal;

    if (transmission.length() != 1.0 ){ 
        std::cout<< "transission length: " << transmission.length() << std::endl;
        transmission.normalize();
    }

    return transmission;
}

//incoming is the incoing direction
//outgoing is the outgoing direction
//
//
//T may not be the correct quantity.
//There may also be sign errors in the incoming vector.
//The incoming light should be from a photon
//the outgoing light is heading to the viewer.
inline float brdf(float alpha, float beta, const vec3 & in,
                                    const vec3 & out,
                                    const vec3 & normal ){
    vec3 outgoing = - out;
    vec3 incoming = - in;

    outgoing.normalize();
    incoming.normalize();

    float v1 = normal.dotProduct( normal, outgoing ) ;
    float v2 = normal.dotProduct( normal, incoming ) ;

    vec3 H = (outgoing + incoming).normalized() ;
    vec3 H_bar = H.crossProduct( normal , H.crossProduct( H, normal ) 
                 ).normalized();
    
    vec3 T = outgoing.crossProduct( normal ,
             outgoing.crossProduct( outgoing , normal ) ).normalized();

    if ( T.dotProduct( T, normal ) != 0 || T.dotProduct( normal, H_bar ) != 0){

        std::cout << "error _______________________________________________________________________________" << "\n" ;
        std::cout << T.dotProduct( T, normal ) << "\n";
        std::cout << T.dotProduct( H_bar, normal ) << "\n";
        
    }
    float t = normal.dotProduct( normal , H );
    float w = T.dotProduct( T, H_bar );

    float Gv1 = v1 / ( alpha - alpha * v1 + v1 );
    float Gv2 = v2 / ( alpha - alpha * v2 + v2 );
    float Gv1_by_Gv2 = Gv1 * Gv2;

    //t *= t;
    float denominator = 1 + alpha*t*t - t*t;
    float Zt = alpha / (denominator * denominator );

    //w *= w;
    float Aw = sqrt( beta / ( beta*beta - beta*beta*w*w + w*w ) );

    float first = Gv1_by_Gv2 * Aw * Zt / (4 * M_PI * v1 * v2 );
    float second = (1 - Gv1_by_Gv2 )  / (4 * M_PI * v1 * v2 );
    std::cout << "\t alpha: " << alpha << "\n";
    std::cout << "\t beta: " << beta << "\n";
    
    std::cout << "\t first: " << first << "\n";
    std::cout << "\t second: " << second << "\n";
    std::cout << "\t Aw: " << Aw << "\n";
    std::cout << "\t Zt: " << Zt << "\n";
    std::cout << "\t Gv1_by_Gv2 : " << Gv1_by_Gv2 << "\n";
    std::cout << "\t 4 * M_PI * v1 * v2 : " << 4 * M_PI * v1 * v2 << "\n";
    
    

    return ( Gv1_by_Gv2 * Aw * Zt + (1 - Gv1_by_Gv2 ) )/ (4 * M_PI * v1 * v2 ); 
}



//to keep with my other convections, direction should be the direction of the
//ray going into the surface
inline float fresnel( const vec3 & direction, const vec3 & normal,
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

    direction.normalize();
    return direction;

}


}//namespace

#endif
