#ifndef CS40PHOTON_H
#define CS40PHOTON_H

#include <string>
#include <iostream>
#include "common.h"

/* A basic Ray class.*/
namespace cs40{

class Photon{

public:
    //go ahead and mess with these directly, they are public
    vec3 direction;  //a vector
    vec3 color;      // the color

    bool isCaustic;

    Photon();  /*at origin in -z direction*/
    Photon(const Photon& other); /*copy constructor*/
    Photon(const vec3& origin, const vec3& dir);
    Photon(const vec3& origin, const vec3& dir, const vec3 & clr );
    Photon(const vec3& origin, const vec3& dir, const vec3 & clr,
           const bool & caustic );

    /* convert Ray to string representation */
    std::string str() const;

    //Return point in direction of ray at time t
    inline vec3 operator() (float t) const {return origin+t*direction; }

};

} //namespace

/* allow cout << photon */
std::ostream& operator<<(std::ostream& os, const cs40::Photon& r);

#endif
