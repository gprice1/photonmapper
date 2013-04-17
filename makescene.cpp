#include "raytracer.h"
#include <iostream>

using namespace std;

using cs40::RayTracer;

int main(int argc, char* argv[]){
    RayTracer rt;
    
    if(argc < 2){ //Check it users are using program correctly
        std::cout << "Usage: " << argv[0] << " <inputfile>\n";
        rt.parseFile("input.txt");
    }
    else{ 
        rt.parseFile(argv[1]);
    }

    //rt.trace(); //TODO: ray trace away!
    rt.save();

    return 0;
}
