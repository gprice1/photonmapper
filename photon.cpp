#include "photon.h"
#include <sstream>

using std::string;
using std::ostream;
using std::stringstream;
using cs40::Photon;

Photon::Photon():origin(0,0,0),direction(0,0,-1),
        color( 1 ,1 ,1), isCaustic( true ){ };

Photon::Photon(const Photon& other):
    origin(other.origin), direction(other.direction),
    color( other.color ), isCaustic( other.isCaustic ) { };

Photon::Photon(const vec3& src, const vec3& dir):
    origin(src), direction(dir),
    color( 1 ,1 ,1), isCaustic( true ) { };

Photon::Photon(const vec3& src, const vec3& dir, const vec3 & clr ):
    origin(src), direction(dir),
    color( clr ), isCaustic( true ) { };

Photon::Photon(const vec3& src, const vec3& dir, const vec3 & clr,
               const bool & caustic ):
    origin(src), direction(dir),
    color( clr ), isCaustic( caustic ) { };

/* convert Photon to string representation */
string Photon::str() const {
    stringstream ss;
    ss << "src: " << origin << " dir: " << direction;
    return ss.str();
}

/* allow cout << ray */
ostream& operator<<(ostream& os, const Photon& r){
    return os << r.str();
}
