//these are notes to myself about changes.
//
//rita should copy over my changes to raytrace.h and cpp
//also possible ray.h
//
//there is a set of text at the top of raytrace.h that holds the circle
//hitTime function.
//
//the following code is the planar hitTime code:
//
//
float C = - dotProduct( points[0], normal )
float time = (dotProduct( p.origin, normal ) + points[0] + C ) /
              dotProduct( normal, p.direction );

//now we must check to see if the point is in the rectangle we have.
//this is accomplished through projection.

vec3 point = p( time );

//precompute these values for faster processing
vec3 basis1 = points[0] - points[1];
vec3 basis2 = points[2] - points[1];
length1 = basis1.length();
length2 = basis2.length();

float test = dotProduct( basis1, point ) / length1;
if (test > 1.0 || test < 0.0){ return -1.0; }
test = dotProduct( basis2, point ) / length2;
if (test > 1.0 || test < 0.0){ return -1.0; }

return time;
