//C++ STL
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <math.h>
#include <typeinfo>
#include <thread>

//QT
#include <QHash>      //Qt Dictionary
#include <QString>    //Appease QT with its strings

//Our stuff
#include "raytracer.h"
#include "view.h"     //image, eye properties
#include "shape.h"    //base class for hitable objects
#include "light.h"    //basic light struct (pos, intensity)
#include "parser.h"   //helper functions for reading input file
#include "ray.h"      //things we trace
#include "scene.h"    //things we hit
#include "rgbImage.h" //things to which we save output
#include "sphere.h"
#include "rectangle.h"
#include "triangle.h"
//#include "glm.h"
#include <time.h>
#include "functions.h"

#include "photonmapper.h"
#include "kd-tree.h"


using namespace cs40;
using std::cout;
using std::endl;
using std::string;
using std::ifstream;
using std::vector;

using cs40::RayTracer;
using cs40::RGBColor;
using cs40::Material;
using cs40::Scene;
using cs40::View;

//get stuck? use the nuclear option..
//using namespace cs40;

#define imax(a,b) (a>b?a:b)

RayTracer::RayTracer(){
    /* do nothing? */
    maxDepth = 4;
    srand( time(NULL));

    //do you want to print the current row?
    print = false;

    //what types of lighting do I want to happen?
    get_indirect= true;
    get_direct  = false;
    get_reflect = false;
    get_raycast = false;
    get_caustic = false;

    num_threads = 1;

}

RayTracer::~RayTracer(){
    for ( int i = 0; i < m_scene.objects.size() ; i ++){
        delete m_scene.objects[i];
    }
}

void RayTracer::threadedTrace( RGBImage & img, int threadID ){

    for ( float i = threadID; i < m_scene.view.ncols; i += num_threads ){
        for ( float j = 0.0; j < m_scene.view.nrows; j += 1.0 ){

            vec3 color = tracePixel( i, j );

            img( m_scene.view.nrows - int(j) - 1, int(i) ) = 
                                        convertColor( color );
        }
    }
}



/* trace - iterates through all the pixels of our view box, traces the ray 
 * coming from the eye, and adjusts
 * the color of the pixel accordingly
 * Inputs: img, an RGBImage passed by reference to be adjusted pixel by pixel
 */
void RayTracer::trace(RGBImage & img){

    //
    if (get_indirect || get_direct || get_caustic){
        cout << "Starting Photon Mapping " << endl;
        getPhotonMap();
        cout << "Finished Photon Mapping" << endl;
    }

    //spawn threads in the multithreaded case
    if ( num_threads > 1 ){
        vector< std::thread > threads; 

        //spawn threads.
        for ( int i = 1; i < num_threads; i ++ ){
            threads.emplace_back( &RayTracer::threadedTrace, this,
                                      std::ref( img ), i );
            //threads.emplace_back( threadedTrace, std::ref( img ), i );
        }
        //set the parent to doing stuff as well.
        threadedTrace( std::ref( img ), 0 );

        for( int i = 0; i < threads.size() ; i ++ ){
            threads[i].join();
        }
    }

    //trace each pixel in the non-multithreaded place
    else{
        for ( float i = 0.0; i < m_scene.view.ncols; i += 1.0 ){
            if (print ){ cout << "Pixel i : " << i << "\n"; }

            for ( float j = 0.0; j < m_scene.view.nrows; j += 1.0 ){

                vec3 color = tracePixel( i, j );

                img( m_scene.view.nrows - int(j) - 1, int(i) ) = 
                                            convertColor( color );
            }
        }
    }
}

//this traces a single pixel i,j. It takes floats, not integers so that
//pixels can be traced more than once.
vec3 RayTracer::tracePixel( float i, float j ){
    Ray viewRay;
    viewRay.origin = m_scene.view.eye;

    vec3 h_disp = m_scene.view.horiz * ( i / float(m_scene.view.ncols) );
    vec3 v_disp = m_scene.view.vert * ( j / float(m_scene.view.nrows) );
    vec3 pixel_position = m_scene.view.origin + h_disp + v_disp;

    viewRay.direction = pixel_position - viewRay.origin;
    viewRay.direction.normalize();

    return traceOnce( viewRay , -1 , 0 );

}

/* traceOnce -  Recursive helper function that takes in an incidentRay
 *              and the current shape index of the ray.
 *              The depth is incremented by 1 for every recursive call.
 *              This returns a color based on the phong lighting model, 
 *              reflection, and transmission.
 */
vec3 RayTracer::traceOnce( const Ray & incidentRay, int shapeIndex, int depth ){

    //if the recursion depth has been maxed, then return the bg color
    if (depth > maxDepth ){ return m_scene.view.background ; }


    //the point of intersection with an object
    vec3 hitPoint;                                

    //get the closest colliding shape and set the hitpoint
    shapeIndex = m_scene.checkIntersection( incidentRay, shapeIndex, hitPoint);

    //return if there is no intersection
    if (shapeIndex < 0 ){
        return m_scene.view.background;
    }

    // get the current object    
    Shape * collidedObject = m_scene.objects[ shapeIndex ];
    Material mat = collidedObject->material;

    //get the normal to calculate phong, reflection, and transmission
    vec3 normal = collidedObject->normal( hitPoint );
    //reverse the direction of the normal if the vector is going the wrong way.
    if ( incidentRay.isInsideObject ){ normal *= -1; }

    vec3 directLight(0,0,0)   , reflectedLight(0,0,0);
    vec3 indirectLight(0,0,0) , castLight(0,0,0);


    if (get_direct){
        int shadowVal = p_map.isInShadow( hitPoint );

        //if the point is not totally in shadows, continue with the 
        //direct lighting calculations
        if ( shadowVal != -1 ){
            bool checkForIntersections;

            //if the point is totally illuminated
            if ( shadowVal == 1){ checkForIntersections = false ; }
            //if the point is partially illuminated
            else if ( shadowVal == 0 ){ checkForIntersections = true; }

            for ( int i = 0; i < m_scene.lights.size(); i ++ ){
                directLight += m_scene.getDirect( incidentRay.direction, normal,
                                                 hitPoint, shapeIndex, i,
                                                 checkForIntersections );
            }
        }
    }

    if ( get_reflect ){
        reflectedLight = reflect(incidentRay, hitPoint, normal,
                                  mat, shapeIndex, depth );
    }
    if ( get_indirect ){
        indirectLight = p_map.getIllumination( hitPoint, 
                                          incidentRay.direction,
                                          normal,
                                          mat );
    }
    if (get_raycast && depth == 0){
        castLight = rayCast(incidentRay.direction, hitPoint, 
                            normal, mat, shapeIndex, depth );
    }
    if ( get_reflect && get_direct ){
        float reflected_to_local = sqrt( mat.alpha );
        directLight *= reflected_to_local;
        reflectedLight *= (1 - reflected_to_local);
    }

    return directLight + reflectedLight + castLight + indirectLight ;
}

//TODO use importance sampling for the phong model.
vec3 RayTracer::reflect(  const Ray & incidentRay,
                             const vec3 & hitPoint,
                             const vec3 & normal,
                             const Material & mat,
                             int shapeIndex,
                             int depth ){

    vec3 reflectedLight , transmittedLight;
    //get the reflection ray and recurse if the 
    //material has reflection properties
    if ( mat.alphaInv >= 0.11 ){
        vec3 reflection = cs40::reflect( incidentRay.direction, normal);
        Ray reflectionRay;
        reflectionRay.origin = hitPoint;
        reflectionRay.direction = reflection;
        reflectionRay.isInsideObject = incidentRay.isInsideObject;
        reflectedLight = traceOnce( reflectionRay, shapeIndex, depth + 1 );
    }

    //if the material has transmission properties, 
    //get the transmission ray and recurse
    if ( mat.refractionIndex != 0.0){
        float n1, n2; 
        if (incidentRay.isInsideObject){
            n1 = mat.refractionIndex;
            n2 = 1.0;
        }else {
            n1 = 1.0;
            n2 = mat.refractionIndex;
        }

        Ray transmissionRay;
        transmissionRay.direction = cs40::transmit( incidentRay.direction,
                                                     normal, n1, n2);
        transmissionRay.origin = hitPoint;
        transmissionRay.isInsideObject = !( incidentRay.isInsideObject );

        //if the transmit was invalid, do not process the transmission
        if ( transmissionRay.direction.x() < 2 ){
            transmittedLight = traceOnce( transmissionRay,
                                          shapeIndex, depth + 1 );
        }

        float reflectance = cs40::fresnel( incidentRay.direction, normal, 
                                           n1, n2);
        reflectedLight *= reflectance ;
        transmittedLight *= 1 - reflectance;

        if ( reflectance < 0 || reflectance > 1.0){
            cout << "reflectance: " << reflectance << "\n";

            cout << "Total reflect: " << reflectedLight << "\n";
            cout << "Total transmit: " << transmittedLight << "\n";
        }
    }

    return (reflectedLight + transmittedLight ) * mat.alphaInv;

}


//TODO use importance sampling for the phong model.
vec3 RayTracer::rayCast( const vec3 & view,
                         const vec3 & hitPoint,
                         const vec3 & normal,
                         const Material & mat,
                         int shapeIndex,
                         int depth ){

    vec3 direction, color, indirect;

    int num_samples = 100;
    for( int i = 0; i < num_samples; i ++){
        direction = cosWeightedRandomHemisphereDirection( normal );
        Ray castRay( hitPoint, direction );

        //change this to be more dynamic
        color = traceOnce( castRay, shapeIndex, maxDepth - 1 );
        color *= mat.getLight( direction, view, normal );
        indirect += color;
    }
    indirect /= num_samples;

    return indirect;
}


void RayTracer::save(){
    View vw = m_scene.view;

    RGBImage img(vw.nrows, vw.ncols, convertColor(vw.background));
    cout << "before trace" << endl;
    trace(img );
    cout << "before save" << endl;
    img.saveAs(m_scene.view.fname, true);
    cout << "Saved result to " << m_scene.view.fname << endl;
}


/* convert from 0-1 rgb space to 0-255 */
RGBColor RayTracer::convertColor( vec3& clr){
    int r,g,b;
    clr.setX( std::max( std::min( clr.x(), 1.0), 0.0 ) );
    clr.setY( std::max( std::min( clr.y(), 1.0), 0.0 ) );
    clr.setZ( std::max( std::min( clr.z(), 1.0), 0.0 ) );
    r=(int)(255*clr.x());
    g=(int)(255*clr.y());
    b=(int)(255*clr.z());
    return RGBColor(r, g, b);
}

void RayTracer::parseFile(const string& fname){
    ifstream infile;
    infile.clear();
    infile.open(fname.c_str());

    if(!infile.good()){
        cout << "error opening file " << fname << endl;
        return;
    }

    int lno = 1; //line number
    string line; //current line
    vector<string> words;


    while(!infile.eof()){
        getline(infile, line);
        //cout << lno << ": " << line.length() << " " << line <<  endl;
        words = split(line);
        if(words.size()>0){
            try{
                parseLine(words);
            }
            catch(parser_error e){
                cout << "Error on line " << lno << ": " << line << endl;
                cout << e.what() << endl;
            }
        }
        lno++;
    }

    infile.close();
}

void RayTracer::parseLine(const vector<string>& words){
    string cmd = words[0];
    if(cmd == "output"){
        checksize(words,1);
        m_scene.view.fname = words[1];
    }
    else if (cmd == "direct"){
        get_direct = true;
        cout << "Direct Lighting On" << "\n";
    }
    else if (cmd == "indirect"){
        get_indirect = true;
        cout << "Indirect Lighting On" << "\n";
    }
    else if (cmd == "raycast"){
        get_raycast = true;
        cout << "RayCasting On" << "\n";
    }
    else if (cmd == "reflect"){
        get_reflect = true;
        cout << "Reflection On" << "\n";        
    }
    else if (cmd == "outsize"){
        checksize(words,2);
        m_scene.view.nrows = parseInt(words[1]);
        m_scene.view.ncols = parseInt(words[2]);
    }
    else if (cmd == "origin"){
        m_scene.view.origin = parseVec3( words, 1 );
    }
    else if (cmd == "horiz"){
        m_scene.view.horiz = parseVec3( words, 1 );
    }
    else if (cmd == "vert"){
        m_scene.view.vert = parseVec3( words, 1 );
    }
    else if (cmd == "eye"){
        checksize(words,3);
        m_scene.view.eye = parseVec3(words,1);
    }
    else if (cmd == "background"){
        checksize(words,3);
        m_scene.view.background = parseVec3(words,1);
    }
    else if (cmd == "color"){
        checksize(words,4);
        //add named color to map
        m_colors[words[1].c_str()]=parseVec3(words,2);
    }
    else if (cmd == "mat"){
        //either mat cmd name or mat cmd r g b
        checksize(words,2,4);
        parseMat(words); //this gets complicated
    }
    else if (cmd == "sphere"){
        checksize(words, 4);

        vec3 center = parseVec3( words, 1);
        Sphere* object = new Sphere(center, parseFloat(words[4]),
                             m_materials["current_"]);
        m_scene.objects.push_back(object);
    }
    else if (cmd == "rectangle"){
        checksize(words, 9);
        vec3 ll = parseVec3( words, 1 );
        vec3 lr = parseVec3( words, 4 );
        vec3 ur = parseVec3( words, 7 );
        Rectangle* object = new Rectangle(ll, lr, ur, m_materials["current_"]);
        m_scene.objects.push_back(object);
    }
    else if (cmd == "triangle"){
        checksize(words,9);
        vec3 pt1 = parseVec3( words, 1 );
        vec3 pt2 = parseVec3( words, 4 );
        vec3 pt3 = parseVec3( words, 7 );
        Triangle* object = new Triangle(pt1,pt2, pt3, m_materials["current_"]);
        m_scene.objects.push_back(object);
    }

    else if (cmd == "amblight"){
        checksize(words,1);
        m_scene.ambient = parseFloat(words[1]);
    }
    else if (cmd == "light"){
        //checksize(words,4);
        parseLight( words );
    }
    else if ( cmd == "refract"){
        checksize( words, 1);
        m_materials["current_"].refractionIndex = parseFloat( words[1]);
    }

    else if ( cmd == "alpha"){
        checksize( words, 1);
        m_materials["current_"].alpha = parseFloat(words[1]); 
        m_materials["current_"].alphaInv = 1 - parseFloat(words[1]); 

    }
    else if ( cmd == "beta"){
        checksize( words, 1);
        m_materials["current_"].beta = parseFloat(words[1]);
    }
    else if ( cmd == "photon" ){
        checksize( words, 3 );
        cs40::PhotonMapper p_map (  parseInt( words[1] ),
                                    parseInt( words[2] ),
                                    parseInt( words[3] )  );
    }
    else if ( cmd == "photon_power" ){
        checksize( words, 1 );
        p_map.photon_power = parseFloat( words[1] );
    }
    else if( cmd == "photon_N" ){
        checksize( words, 3 );
        p_map.shadow.N    = parseInt( words[1] );
        p_map.caustic.N   = parseInt( words[2] );
        p_map.indirect.N  = parseInt( words[3] );
    }
    else if( cmd == "photon_visualize" ){
        p_map.visualize = true;
    }
    else if( cmd == "photon_exclude_direct" ){
        p_map.exclude_direct = true;
    }
    else if( cmd == "photon_include_direct" ){
        p_map.exclude_direct = false;
    }

    else if( cmd == "photon_epsilon" ){
        checksize( words, 3 );
        p_map.shadow.epsilon    = parseFloat( words[1] );
        p_map.caustic.epsilon   = parseFloat( words[2] );
        p_map.indirect.epsilon  = parseFloat( words[3] );
    }
    else{
        throw parser_error("Unknown command: "+cmd);
    }
}

/* parse a material command in the vector words, 
 * using matmap to load/store/modify current and other maps */
void RayTracer::parseLight(const vector<string>& words){

    float intensity = parseFloat(words[2]);


    //we have a more complex type of light
    if ( words[1] == "point"){
        vec3 position = parseVec3(words,3); 
        Light light( position, intensity );
        m_scene.lights.push_back( light );
    }

    else if ( words[1] == "rectangle"){
        vec3 ll, ul, ur;
        ll = parseVec3( words, 3 );
        ul = parseVec3( words, 6 );
        ur = parseVec3( words, 9 );
        
        Light light( ll, ul, ur, intensity, RECTANGLE );
        m_scene.lights.push_back( light );
    }
    else if ( words[1] == "triangle"){

        vec3 ll, ul, ur;
        ll = parseVec3( words, 3 );
        ul = parseVec3( words, 6 );
        ur = parseVec3( words, 9 );
        
        Light light( ll, ul, ur, intensity, TRIANGLE );
        m_scene.lights.push_back( light );
        
    }
    else if ( words[1] == "circle"){

        vec3 position = parseVec3( words, 3 );
        float radius = parseFloat( words[6] );
        vec3 normal  = parseVec3( words, 7 );
        Light light( position, intensity, radius, normal, CIRCLE );
        m_scene.lights.push_back( light );
        
    }
    else if ( words[1] == "sphere"){
        vec3 position = parseVec3( words, 3 );
        float radius = parseFloat( words[6] );
        Light light( position, intensity, radius, SPHERE );
        m_scene.lights.push_back( light );
        
    }
    else{
        throw parser_error("No command found for light");
    }
}

/* parse a material command in the vector words, 
 * using matmap to load/store/modify current and other maps */
void RayTracer::parseMat(const vector<string>& words){
    string subcmd = words[1];
    if(subcmd == "save"){
        m_materials[words[2].c_str()] = m_materials[QString("current_")];
    }
    else if(subcmd == "load"){
        if(m_materials.contains(words[2].c_str())){
            m_materials["current_"]=m_materials[words[2].c_str()];
        }
        else{
            throw parser_error("No Material " + words[2] + " found");
        }
    }
    else if(subcmd != "color"){
        throw parser_error("Unknown material subcommand: " + subcmd);
    }
    else{
        //looks like mat <type> <name> or mat <type> r g b
        //where <type> is amb, diff, or spec
        vec3 clr;
        if(words.size()>3 ){ //mat <type> r g b
            clr = parseVec3(words, 2);
        }
        else{ //mat <type> name, where name refers to existing color
            string clrname = words[2];
            if(m_colors.contains(clrname.c_str())){
                clr=m_colors[clrname.c_str()];
            }
            else{
                throw parser_error("No color " + clrname + " found");
            }
        }
        if(subcmd=="color"){m_materials["current_"].color=clr;}
    }
}


void RayTracer::getPhotonMap(){
    m_scene.createLightMapping();
    p_map.num_threads = num_threads;
    p_map.mapScene( m_scene );

}


/*
void RayTracer::loadModel( char * objectFile )
{
    GLMmodel *objmodel_ptr;
    objmodel_ptr = glmReadOBJ( objectFile );

    //Then, use this code to load the model and prepare it to be rendered

    if (!objmodel_ptr)
    {
        objmodel_ptr = glmReadOBJ("bunny.obj");
        if (!objmodel_ptr){ exit(0); }
    }

    glmUnitize(objmodel_ptr);
    glmFacetNormals(objmodel_ptr);
    glmVertexNormals(objmodel_ptr, 90.0);
}

*/
/*After this code is executed, the model will have been loaded, 
 * normals will have been created and smoothed, and the model will be
 *  ready to render. To draw the loaded model, add the following code
 *   into your display function:
 */
//glmDraw(objmodel_ptr, GLM_SMOOTH | GLM_MATERIAL);
