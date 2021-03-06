//this is a simple program that is meant to test the functionality of the
//brdf functions that I have created


#include "functions.h"
#include "ray.h"
#include <iostream>
#include "rgbImage.h"
#include "common.h"

using namespace std;
using namespace cs40;

RGBColor convertColor( vec3& clr);

int main(){
    vec3 bgcolor( 0, 0, 0);
    RGBImage img(M_PI / 2 / 0.02 +1,
                 M_PI * 2 / 0.02 +1,
                 convertColor( bgcolor ) );

    float alpha, beta, brdfValue, lightVal;
    alpha = 0.01;
    beta = 0.5;

    vec3 incoming, outgoing, normal; 
    incoming = angle_to_direction( M_PI/4, 0 );
    normal   = angle_to_direction( M_PI/2, 0 );
    float cos_thetaI = cos( M_PI/4 );
    
    vec3 color;
    int k , l;
    k = 0;

    for ( float i = 0.0; i < M_PI / 2 ; i += 0.02) {
        l = 0;
        for ( float j = 0.0; j < M_PI * 2 ; j += 0.02){
        
            outgoing = angle_to_direction( i , j );

            //cout << "outgoing: " << outgoing << "\n";

            brdfValue = cs40::brdf( alpha, beta, incoming, outgoing, normal );
            //brdfValue *= cos_thetaI;

            if ( brdfValue > 1.0 ){
                cout << "\tBRDF value: " << brdfValue << "\n";
                cout << "\tTheta: " << i <<"\n";
                cout << "\tPhi: " << j <<"\n";
                color = vec3( sqrt( sqrt( brdfValue )) - 1 , 0 ,1);
            }
            else if (brdfValue < 0.0 ){
                cout << "\tBRDF value: " << brdfValue << "\n";
                cout << "\tTheta: " << i <<"\n";
                cout << "\tPhi: " << j <<"\n";
            }
            else{
                color = vec3( brdfValue, brdfValue, brdfValue );
                //cout << "\tBRDF value: " << brdfValue << "\n";
            }

            img( k , l ) = convertColor( color );
            l ++;
        }
        k++;
    }


    img.saveAs( "profile.png" , true);
    cout << "Saved result to " << "profile.png" << endl;
    cout << "incoming: " << incoming << "\n";
    cout << "normal: " << normal << "\n";

    return 0;
}


RGBColor convertColor( vec3& clr){
    int r,g,b;
    clr.setX( std::max( std::min( clr.x(), 1.0), 0.0 ) );
    clr.setY( std::max( std::min( clr.y(), 1.0), 0.0 ) );
    clr.setZ( std::max( std::min( clr.z(), 1.0), 0.0 ) );
    r=(int)(255*clr.x());
    g=(int)(255*clr.y());
    b=(int)(255*clr.z());
    return RGBColor(r, g, b);
}

              

