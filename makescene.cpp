#include "raytracer.h"
#include <iostream>

using namespace std;

using cs40::RayTracer;

int main(int argc, char* argv[]){
    if(argc < 2){ //Check it users are using program correctly
        std::cout << "Usage: " << argv[0] << " <inputfile>\n";
        return 1;
    }
    RayTracer rt;
    rt.parseFile(argv[1]);
    //rt.trace(); //TODO: ray trace away!
    rt.save();

    return 0;
}
