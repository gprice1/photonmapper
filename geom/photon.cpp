#include "photon.h"
#include <sstream>

using std::string;
using std::ostream;
using std::stringstream;
using cs40::Photon;

Photon::Photon():direction(0,0,-1),
        color( 1 ,1 ,1), isCaustic( true ){ };

Photon::Photon(const Photon& other):
    direction(other.direction), color( other.color ) { };

Photon::Photon(const vec3& dir):
    direction(dir), color( 1 ,1 ,1) { };

Photon::Photon(const vec3& dir, const vec3 & clr ):
    direction(dir), color( clr ){ };

