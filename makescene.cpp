#include "raytracer.h"
#include <iostream>
#include <cstdlib>
using namespace std;

using cs40::RayTracer;

int main(int argc, char* argv[]){
    RayTracer rt;
    
    if(argc < 2 ){ //Check it users are using program correctly
        std::cout << "Usage: " << argv[0] << " <inputfile>  <number_threads>\n";
        rt.parseFile("input.txt");
    }
    else if( argc == 3 && atoi( argv[2] ) != 0 ){
        rt.parseFile(argv[1]);        
        rt.num_threads = atoi( argv[2] );
    }
    else{ 
        rt.parseFile(argv[1]);

    }

    //rt.trace(); //TODO: ray trace away!
    rt.save();

    return 0;
}
