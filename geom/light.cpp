#include "light.h"
#include "functions.h"

using cs40::Light;

Light::Light(): 
    position( 0,0,0), intensity(0), radius( 0 ), type( POINT )
{}

Light::Light( vec3 pos, float intense ):
    position( pos), intensity(intense), type( POINT )
{}

Light::Light( vec3 pos, float intense , float r, LightType t):
    position( pos ), intensity( intense ), radius( r ), type( t ) 
{}

//this is a circle light constructor 
Light::Light( vec3 pos, float intense , float r, vec3 n, LightType t):
    position( pos ), intensity( intense ), radius( r ),
    normal( n ), type( t ) 
{
    normal.normalize();

    vec3 randVec;
    do {
        randVec = vec3( rand(), rand(), rand() );
    }while( normal.dotProduct( randVec, normal ) < 0 );

    basis1 = normal.crossProduct( normal, 
                   normal.crossProduct( randVec, normal ) );
    basis1.normalize();

    basis2 = normal.crossProduct( normal, basis1);
    basis2.normalize();
}


Light::Light( vec3 point1, vec3 point2, vec3 point3, 
             float intense,  LightType t ):
   position( point2 ), intensity( intense ), type( t )
{

    basis1 = point1 - position;
    basis2 = point3 - position;

    normal = basis1.crossProduct( basis2, basis1 ).normalized();

}

cs40::Ray Light::emitPhoton() const {

    vec3 origin, direction;
    float A, B;

    switch( type ){

        case POINT :
            origin = position ;
            direction = getRandomDirection();
            break;

        case SPHERE : 
            origin = position + getRandomDirection() * radius;
            direction = randomHemisphereDirection( origin - position );
            break;

        case RECTANGLE :
            origin = position + basis1 * randf() + basis2 * randf() ;
            direction = randomHemisphereDirection( normal );
            break;

        case TRIANGLE :
            do{
                A = randf();
                B = randf();
            }while( A + B > 1.0 );

            origin = position + basis1 * A + basis2 * B ;
            direction = randomHemisphereDirection( normal );
            break;

        case CIRCLE :
            do{
                A = randf( -1.0, 1.0);
                B = randf (-1.0, 1.0);

                //reject the sample if they are not in the unit circle
            }while( A*A + B*B > 1.0 );
            origin = position + basis1 * A + basis2 * B;
            direction = randomHemisphereDirection( normal );
            break;
    }
    

    return cs40::Ray( origin, direction );
}

