#include "common.h"
#include "iostream"
using std::ostream;

ostream& operator<<(ostream& str, const QVector3D& vec){
    str << (float) vec.x() << ", "
        << (float) vec.y() << ", "
        << (float) vec.z();
    return str;
}
 

