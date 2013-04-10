#ifndef CS40RGBCOLOR_H
#define CS40RGBCOLOR_H

#include <stdexcept>
#include <sstream>

namespace cs40{
typedef struct rgbcolor_s{
    //struct is just a public class in C++
    unsigned char r;
    unsigned char g;
    unsigned char b;

    //struct Constructors allowed in C++
    //Default (black) constructor
    rgbcolor_s(): r(0), g(0), b(0) {};

    rgbcolor_s(unsigned char red, unsigned char green, unsigned char blue):
        r(red), g(green), b(blue){};

    //copy constructor
    rgbcolor_s(const struct rgbcolor_s& other){
        r=other.r; g=other.g; b=other.b;
    }

    //Allow color(0) syntax for returning red component (1,2 for green,blue)
    unsigned char& operator()(int idx){
        std::stringstream emsg;
        if(idx < 0 || idx > 2){
            emsg << "rgb index " << idx << " out of bounds" << std::endl;
            throw std::range_error(emsg.str());
        }
        if(idx==0){ return r; }
        else if(idx==1){ return g; }
        return b;
    }


} RGBColor;

}//namespace

#endif 
