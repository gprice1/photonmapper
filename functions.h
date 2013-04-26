#ifndef CS40FUNCTIONS_H
#define CS40FUNCTIONS_H

#include "ray.h"
#include <iostream>
#include <math.h>
#include "common.h"


namespace cs40{

//random float functions
inline float randf(){
    return (float)rand() / (float)RAND_MAX;
}
inline float randf( float upper ){
    return randf() * upper ;
}
inline float randf( float lower, float upper ){
    return randf() * ( upper - lower ) + lower;
}


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

//this is the cook - torrance brdf model
inline float brdf_ct(float alpha, float beta, const vec3 & incoming,
                                    const vec3 & outgoing,
                                    const vec3 & normal ){
    float v1 = normal.dotProduct( normal, -outgoing ) ;
    if ( v1 <= 0 ){
        return 0;
    }

    float v2 = normal.dotProduct( normal, -incoming ) ;
    if (v2 <= 0){
        return 0;
    }

    return 0;
}

inline float brdf(float alpha, float beta, const vec3 & incoming,
                                           const vec3 & outgoing,
                                           const vec3 & normal ){

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

    /*
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
    */

    //if ( brdfVal > 1.0 ){ return 1.0; }
    return  brdfVal; 
}


//incoming is the incoing direction
//outgoing is the outgoing direction
//
//
//T may not be the correct quantity.
//this is the schlick model with both isotropy and diffuse/specular
//this model also handles microfacets
//TODO : The microfacet code may be defective
//TODO : there are some very serious bounds errors and I am not sure if
//       they are supposed to be there.
//TODO : I am unsure of the proper value of T. 
//All of the vectors should be pointing out of the surface of the object.
//
//the incoming should be the incident light.
//the outgoing should be  view vector.
inline float brdf_s(float alpha, float beta, 
                       const vec3 & incoming,
                       const vec3 & outgoing,
                       const vec3 & normal, 
                       const vec3 & surfaceTangent=vec3(-2,-2,-2) ) {

    if (surfaceTangent.x() <= -2 ){
        return brdf( alpha, beta, incoming, outgoing, normal );
    }

    float v1 = normal.dotProduct( normal, outgoing );
    if ( v1 <= 0 ){
        return 0;
    }

    float v2 = normal.dotProduct( normal, incoming ) ;
    if (v2 <= 0){
        
        return 0;
    }

    vec3 H = (outgoing + incoming).normalized() ;
    vec3 H_bar = H.crossProduct( normal , H.crossProduct( H, normal ));
    H_bar.normalize();

    float t = normal.dotProduct( normal , H );
    float w = surfaceTangent.dotProduct( surfaceTangent, H_bar );

    float Gv1 = v1 / ( alpha - alpha * v1 + v1 );
    float Gv2 = v2 / ( alpha - alpha * v2 + v2 );
    float Gv1_by_Gv2 = Gv1 * Gv2;

    t = t*t;
    float denominator = 1 + alpha*t - t;
    float Zt = alpha / (denominator * denominator );

    w = w*w;
    float Aw = sqrt( beta / ( beta*beta - beta*beta*w + w ) );

    //TODO take this out

    float brdfVal = Gv1_by_Gv2 * Aw * Zt;
    brdfVal += (1 - Gv1_by_Gv2 );        //this line enables micro facet
                                         //specular reflections, I may want to
                                         //TODO take this out
    brdfVal /= 4 * M_PI * v1 * v2;
/*
    if (brdfVal > 1.0 ){ 
        std::cout << "BRDF: " << brdfVal << "\n";
        std::cout << "\t alpha: " << alpha << "\n";
        std::cout << "\t beta: " << beta << "\n";
        std::cout << "\t Aw: " << Aw << "\n";
        std::cout << "\t Zt: " << Zt << "\n";
        std::cout << "\t Gv1_by_Gv2 : " << Gv1_by_Gv2 << "\n";
        std::cout << "\t 4 * M_PI * v1 * v2 : " << 4 * M_PI * v1 * v2 << "\n";
    }
*/
    //if ( brdfVal <= 0 ){ return 0; }
    if ( brdfVal >= 1.0 ) { return 1.0 ;}

    return brdfVal;
}






//this is meant to give a montecarlo importance sample for the brdf_s
//function above.
//TODO make this work.
inline vec3 sample_s( float alpha, float beta, const vec3 & in,
                                           const vec3 & out,
                                           const vec3 & normal ){

    float zenith, azimuth;
    float A = randf();
    float B = randf();

    //this controls which quarter-shpere the ray is emitted from.
    //int   C = (rand() % 2);

    B *= B;
    beta *= beta;
    float B_beta = B * beta;
    azimuth = M_PI / 2 * sqrt( B_beta / ( 1 - beta + B_beta ) );

    zenith = acos( sqrt( A / ( alpha - A * alpha + A ) ) );

    return vec3();

}
/*
//for efficient rendering,
//all of the vectors should be pointing out of the surface.
inline float brdf_ABCD(float A, float B, float D, const vec3 & incoming,
                                                  const vec3 & outgoing,
                                                  const vec3 & normal ){

    float v1 = normal.dotProduct( normal, outgoing ) ;
    if ( v1 <= 0 ){ return 0; }

    float v2 = normal.dotProduct( normal, incoming ) ;

    if ( v2 <= 0 ){ return 0; }

    //H is the halfway vector between incoming and outgoing
    vec3 H = (outgoing + incoming) ;

    //H_hat is the normalized halfway vector
    vec3 H_hat = H.normalized();

    //H_bat is the halfway vector projected into the surface plane
    vec3 H_bar = H.crossProduct( normal , H.crossProduct( H, normal ));
    H_bar.normalize();

    //the difference between the reflection vector and the outgoing vector
    vec3 Dp = H - normal.dotProduct( H, normal) * normal;

    vec3 Ti = outgoing.crossProduct( normal ,
             outgoing.crossProduct( incoming , normal ) );
    Ti.normalize();

    vec3 To = outgoing.crossProduct( normal ,
             outgoing.crossProduct( outgoing , normal ) );
    To.normalize();

    float w = T.dotProduct( T, H_bar );
    float t = normal.dotProduct( normal , H );
   
    //this is phi out when we put phi_in at pi that phi_in is pi
    float sin_phi_out , cos_phi_out;
    cos_phi_out = Ti.dotProduct( Ti, To );
    sin_phi_out = sqrt( 1 - cos_phi_out*cos_phi_out );

    float sin_out = sqrt( 1 - v1*v1 );
    float sin_in  = sqrt( 1 - v2*v2 );

    float fx, fy, f, 1_over_gamma;
    fx = ( sin_out * cos_phi_out - sin_out );
    fy = sin_out * sin_phi_out;
    f = sqrt( fx*fx + fy*fy );

    float B, C;
    B = b * b;
    C = (c + 1)/2;
    float spectralDensity = A / pow( 1 + B*f*f , C  );
    
    
    return  brdfVal; 
}

*/



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
    
//theta is the zenith angle, while phi is the azimuth angle
//this corresponds the the general convention of most brdfs 
inline vec3 angle_to_direction( float theta, float phi ){
    vec3 direction;
    direction.setX( cos( theta ) * cos( phi ) );
    direction.setY( cos( theta ) * sin( phi ) );
    direction.setZ( sin( theta ) );

    return direction;

}

//the x component is theta, the zenith angle and the y component is phi, the
//azimuth angle
inline vec2 direction_to_angle( const vec3 & direction ){
    vec2 theta_phi;
    theta_phi.setX( acos( direction.z() ) );
    theta_phi.setY( atan2( direction.y() , direction.x() ) );
    
    return theta_phi;
}


inline vec3 getRandomDirection(){

    
    float A, B, B_2, B_sqrt;
    
    A = randf( 0, 2 * M_PI);
    B = randf(-1 , 1);
    
    B_2 = B * B;
    B_sqrt = sqrt( 1 - B_2 );

    return vec3( cos(A)*B_sqrt,  sin(A)*B_sqrt,  B ) ;
}


//this maps from the unit square to vectors on the unit circle
inline vec3 mapToHemisphere( float A, float B ){
    float A_2pi = 2 * M_PI * A;
    float B_sqrt = sqrt( B );

    return vec3( cos( A_2pi )*B_sqrt, sin( A_2pi ) * B_sqrt, sqrt( 1 - B ) ) ;
}


inline vec3 mapToHemisphericalDirection( float A, float B, const vec3 & normal ){

    vec3 direction = mapToHemisphere( A, B );

    vec2 directionAngle, normalAngle, resultAngle;
    directionAngle = direction_to_angle( direction );
    normalAngle = direction_to_angle( normal );

    resultAngle = directionAngle + normalAngle;
    resultAngle.setX( resultAngle.x() - M_PI/2 );

    return angle_to_direction( resultAngle.x(), resultAngle.y() );

}


inline vec3 getRandomUpDirection(){
 
    float A, B, B_sqrt, A_2pi;
    
    A = randf();
    B = randf();
    
    A_2pi = 2 * M_PI * A;
    B_sqrt = sqrt( B );

    return vec3( cos( A_2pi )*B_sqrt, sin( A_2pi ) * B_sqrt, sqrt( 1 - B ) ) ;
}

//this generates a random, cosine weighted sample of point on the unit sphere.
//the wieghting favors the point ( 0, 0, 1)
inline vec3 cosWeightedHemisphere( ){
    float A = randf();
    float B = randf();

    float r = sqrt( A );
    float phi = 2 * M_PI * B;

    vec3 direction;
    direction.setX( r * cos( phi ));
    direction.setY( r * sin( phi ));
    direction.setZ( sqrt( 1.0 - A ));
    
    return direction;
}


//TODO this is incomplete.
inline vec3 cosWeightedRandomHemisphereDirection( const vec3 & normal ){

    vec3 randomDirection;
    //get two basis vectors for the circle in the plane of the hemisphere.
    do {
        randomDirection = vec3 ( rand(), rand(), rand() );
    }while ( normal.dotProduct( randomDirection, normal) == 0);

    vec3 basis1, basis2;
    basis1 = normal.crossProduct( normal,
                    normal.crossProduct( randomDirection, normal ) );
    basis1.normalize();

    basis2 = normal.crossProduct( normal, basis1 );


    float A = randf();
    float B = randf();

    float r = sqrt( A );
    float phi = 2 * M_PI * B;

    vec3 direction = normal * sqrt( 1.0 - A );    
    direction     += basis1 * r * sin( phi );
    direction     += basis2 * r * cos( phi );
    
    return direction;
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



//all of these rays should point out of the surface
inline float phongDiffuse( const vec3 & direction_to_light,
                           const vec3 & normal){
    return std::max( normal.dotProduct( normal, direction_to_light ), 0.0 );
}


//all of these rays should point out of the surface
inline float phongSpecular( const vec3 & direction_to_light,
                            const vec3 & normal,
                            const vec3 & view,
                            float shinyness) {

    vec3 reflection = reflect( direction_to_light, normal);

    return pow( std::max( view.dotProduct(view, reflection) , 0.0 ) , shinyness);
}


inline void clampTop( vec3 & vec, float top ){
    if ( vec.x() > top ){ vec.setX( top ) ; }
    if ( vec.y() > top ){ vec.setY( top ) ; }
    if ( vec.z() > top ){ vec.setZ( top ) ; }
}    
    
//this is a gaussian filter that makes castic rendering more accurate.
inline float gaussianFilter(float distSqrd, float maxDistSqrd ){
    float alpha, beta;
    alpha = 0.918;
    beta = 1.953;

    float top, bottom, filterVal;

    top = 1 - exp( -beta * distSqrd / ( 2 * maxDistSqrd ) );
    bottom = 1 - exp( -beta );

    filterVal = alpha * ( 1 - top/bottom );

    return filterVal;
}


}//namespace

#endif
