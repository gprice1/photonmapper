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
                      float n1, float n2 ){

    vec3 transmission;

    //these are constants that make calculating the transmission direction
    //easier
    double n, cosTheta1 , sinTheta2;

    n = n1 / n2;
    cosTheta1 = normal.dotProduct( direction , normal );
    sinTheta2 = (n * n) * ( 1.0 - cosTheta1 * cosTheta1 );

    //if cosTheta sqrd is negative, then there is total internal
    //reflectance and this function spits out an error.
    if ( sinTheta2 >= 1 ){
        return vec3( INFINITY , INFINITY , INFINITY);
    }

    //calculated the direction of the transmission ray
    if (cosTheta1 > 0){
        transmission = (n * direction) - (n * cosTheta1
                        - sqrt( 1 - sinTheta2 ) ) * normal;
    }
    else{
        transmission = (n * direction) - (n * cosTheta1 
                        + sqrt( 1 - sinTheta2 ) ) * normal;
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
inline float schlick_brdf(float alpha, float beta, const vec3 & incoming,
                                    const vec3 & outgoing,
                                    const vec3 & normal ){

    float v1 = normal.dotProduct( normal, outgoing ) ;
    if ( v1 <= 0 ){
        return 0;
    }
    float v2 = normal.dotProduct( normal, incoming ) ;
    if (v2 <= 0){
        return 0;
    }

    vec3 H = (-outgoing - incoming).normalized() ;
    vec3 H_bar = H.crossProduct( normal , H.crossProduct( H, normal ));
    H_bar.normalize();

    vec3 T = outgoing.crossProduct( normal ,
             outgoing.crossProduct( -outgoing , normal ) );
    T.normalize();

    float t = normal.dotProduct( normal , H );
    float w = T.dotProduct( T, H_bar );

    float Gv1 = v1 / ( alpha - alpha * v1 + v1 );
    float Gv2 = v2 / ( alpha - alpha * v2 + v2 );
    float Gv1_by_Gv2 = Gv1 * Gv2;

    t *= t;
    float denominator = 1 + alpha*t - t;
    float Zt = alpha / (denominator * denominator );

    w *= w;
    float Aw = sqrt( beta / ( beta*beta - beta*beta*w + w ) );

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

    if ( first + second <= 0 ){
        return 0 ;
    }

    return first + second;
}



inline float brdf(float alpha, float beta, const vec3 & in,
                                           const vec3 & out,
                                           const vec3 & normal ){
    vec3 outgoing = -out;
    vec3 incoming = -in;

    outgoing.normalize();
    incoming.normalize();

    float v1 = normal.dotProduct( normal, outgoing ) ;
    if ( v1 <= 0 ){ return 0; }

    float v2 = normal.dotProduct( normal, incoming ) ;
    if ( v2 <= 0 ){ return 0; }

    vec3 H = (outgoing + incoming).normalized() ;

    float t = normal.dotProduct( normal , H );

    float Gv1 = v1 / ( alpha - alpha * v1 + v1 );
    float Gv2 = v2 / ( alpha - alpha * v2 + v2 );
    float Gv1_by_Gv2 = Gv1 * Gv2;

    float denominator = 1 + alpha*t*t - t*t;
    float Zt = alpha / (denominator * denominator );

    float first = Gv1_by_Gv2 * Zt / (4 * M_PI * v1 * v2 );
    float second = (1 - Gv1_by_Gv2 )  / M_PI ;
    float brdfVal = first + second;
    if ( brdfVal > 1.0 || brdfVal < 0.0 ){
        std::cout << "\t v1: " << v1 << "\n";
        std::cout << "\t v2: " << v2 << "\n";
        std::cout << "\t alpha: " << alpha << "\n";
        std::cout << "\t first: " << first << "\n";
        std::cout << "\t second: " << second << "\n";
        std::cout << "\t Zt: " << Zt << "\n";
        std::cout << "\t Gv1_by_Gv2 : " << Gv1_by_Gv2 << "\n";
        std::cout << "\t 4 * M_PI * v1 * v2 : " << 4 * M_PI * v1 * v2 << "\n";
        std::cout << "brdf: " << first + second << "\n";

    }

    return brdfVal; 
}


//to keep with my other convections, direction should be the direction of the
//ray going into the surface
inline float fresnel( const vec3 & direction, const vec3 & normal,
                     float n1, float n2 ){

    float cosTheta_i = normal.dotProduct( -direction , normal );
    float sinTheta_i = sqrt( 1  - cosTheta_i * cosTheta_i );

    float radicand = n1/n2 * sinTheta_i;
    radicand = radicand * radicand;

    float component = sqrt( 1 - radicand );

    float Rs = (n1 * cosTheta_i - n2 * component) / 
               (n1 * cosTheta_i + n2 * component);
    Rs = Rs * Rs;

    float Rp = (n1 * component - n2 * cosTheta_i) / 
               (n1 * component + n2 * cosTheta_i);
    Rp = Rp * Rp;


    float R = ( Rs + Rp ) / 2;

    if ( R > 1.0 ){
        return 1.0;
    }

    return R;

}
    


//to keep with my other convections, direction should be the direction of the
//ray going into the surface
inline float approxFresnel( const vec3 & direction, const vec3 & normal,
                     float n1, float n2 ){
    float R0 = ( n1 - n2 ) / ( n1 + n2 );
    R0 = R0 * R0;

    float cosTheta = normal.dotProduct( -direction , normal );
    cosTheta = 1 - cosTheta;
    float cosTheta_5 = cosTheta * cosTheta;
    cosTheta_5 *= cosTheta_5;
    cosTheta_5 *= cosTheta;

    float value = R0 + (1 - R0) * cosTheta_5;

    if ( value > 1.0 ){
        return 1.0;
    }
    if( value < 0.0 ){
        return 0.0;
    }

    return value;

}
    
inline vec3 angle_to_direction( float theta, float phi ){
    vec3 direction;
    direction.setX( sin( phi ) * cos( theta ) );
    direction.setY( sin( phi ) * sin( theta ) );
    direction.setZ( cos( phi ) );

    return direction;

}

inline vec2 direction_to_angle( const vec3 & direction ){
    vec2 theta_phi;
    theta_phi.setX( atan2( direction.y() , direction.x() ) );
    theta_phi.setY( acos( direction.z() ) );
    return theta_phi;
}


inline vec3 getRandomDirection(){
    float theta, phi;
    theta = 2 * M_PI * (float)rand()/(float)RAND_MAX;
    phi   = M_PI     * float( rand() ) / (float)RAND_MAX ;

    return angle_to_direction( theta, phi );
}

//this is a really stupid, but very easy way of getting a random vector
//on the hemisphere.
inline vec3 randomHemisphereDirection( const vec3 & normal ){
    while( true ){
        vec3 direction( getRandomDirection() );
        if ( normal.dotProduct( normal, direction ) > 0 ){
            return direction;
        }
    }
}


inline vec3 getRandomDirection( float lowerTheta , float upperTheta,
                                float lowerPhi   , float upperPhi   ){

    float theta, phi;
    theta = lowerTheta + ( upperTheta - lowerTheta)
            * (float)rand()/(float)RAND_MAX;
    phi   = lowerPhi + ( upperPhi - lowerPhi)
            * float( rand() ) / (float)RAND_MAX ;

    return angle_to_direction( theta, phi );
}


inline float phongDiffuse( const vec3 & direction_to_light,
                           const vec3 & normal){
    return std::max( normal.dotProduct( normal, direction_to_light ), 0.0 );
}

inline float phongSpecular( const vec3 & direction_to_light,
                            const vec3 & normal,
                            const vec3 & view,
                            float shinyness) {

    vec3 reflection = reflect( -direction_to_light, normal);

    return pow( std::max( view.dotProduct(view, reflection) , 0.0 ) , shinyness);
}

inline void clampTop( vec3 & vec, float top ){
    if ( vec.x() > top ){ vec.setX( top ) ; }
    if ( vec.y() > top ){ vec.setY( top ) ; }
    if ( vec.z() > top ){ vec.setZ( top ) ; }
}
    
    

}//namespace

#endif
