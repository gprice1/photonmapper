//C++ STL
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <math.h>
#include <typeinfo>

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
#include "glm.h"
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


RayTracer::RayTracer(){
    /* do nothing? */
    maxDepth = 0;
    srand( time(NULL));
}

RayTracer::~RayTracer(){
    for ( int i = 0; i < m_scene.objects.size() ; i ++){
        delete m_scene.objects[i];
    }
}

/* trace - iterates through all the pixels of our view box, traces the ray 
 * coming from the eye, and adjusts
 * the color of the pixel accordingly
 * Inputs: img, an RGBImage passed by reference to be adjusted pixel by pixel
 */
void RayTracer::trace(RGBImage & img){

    cout << "Starting Photon Mapping " << endl;
    getPhotonMap();
    cout << "Finished Photon Mapping" << endl;

    for ( float i = 0.0; i < m_scene.view.ncols; i += 1.0 ){
        for ( float j = 0.0; j < m_scene.view.nrows; j += 1.0 ){

            Ray viewRay;
            viewRay.origin = m_scene.view.eye;

            vec3 h_disp = m_scene.view.horiz * ( i / float(m_scene.view.ncols) );
            vec3 v_disp = m_scene.view.vert * ( j / float(m_scene.view.nrows) );
            vec3 pixel_position = m_scene.view.origin + h_disp + v_disp;

            viewRay.direction = pixel_position - viewRay.origin;
            viewRay.direction.normalize();

            vec3 color (traceOnce( viewRay , -1 , 0 ) );
            img( m_scene.view.nrows - int(j) - 1, int(i) ) = 
                                        convertColor( color );
        }
    }
}

/* traceOnce -  Recursive helper function that takes in an incidentRay
 *               and the current shape index of the ray.
 *              The depth is incremented by 1 for every recursive call.
 *              This returns a color based on the phong lighting model, 
 *              reflection, and transmission.
 */
vec3 RayTracer::traceOnce( const Ray & incidentRay, int shapeIndex, int depth ){

    //if the recursion depth has been maxed, then return the bg color
    if (depth > maxDepth ){ return m_scene.view.background ; }

    //colors for the local, reflected, and transmitted light
    vec3 local, reflectedLight, transmittedLight;

    //the point of intersection with an object
    vec3 hitPoint;                                

    //get the closest colliding shape and set the hitpoint
    shapeIndex = m_scene.checkIntersection( incidentRay, shapeIndex, hitPoint);

    //return if there is no intersection
    if (shapeIndex < 0 ){
        return m_scene.view.background;
    }

    Shape * collidedObject = m_scene.objects[ shapeIndex ]; 
    // get the current object

    //get the normal to calculate phong, reflection, and transmission
    vec3 normal = collidedObject->normal( hitPoint );
    //reverse the direction of the normal if the vector is going the wrong way.
    if ( incidentRay.isInsideObject ){ normal *= -1; }

    //get the phong lighting modelling.
    local = phong( incidentRay , normal, hitPoint, shapeIndex );

    //get the reflection ray and recurse if the 
    //material has reflection properties
    if ( collidedObject->material.reflection != 0.0){
        vec3 reflection = cs40::reflect( incidentRay.direction, normal);
        Ray reflectionRay;
        reflectionRay.origin = hitPoint;
        reflectionRay.direction = reflection;
        reflectionRay.isInsideObject = incidentRay.isInsideObject;
        reflectedLight = traceOnce( reflectionRay, shapeIndex, depth + 1 );
    }

    //if the material has transmission properties, 
    //get the transmission ray and recurse
    if (collidedObject->material.transmission != 0.0 || 
        collidedObject->material.refractionIndex != 0.0){
        float n1, n2; 
        if (incidentRay.isInsideObject){
            n1 = collidedObject->material.refractionIndex;
            n2 = 1.0;
        }else {
            n1 = 1.0;
            n2 = collidedObject->material.refractionIndex;
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
        reflectedLight *= reflectance;
        transmittedLight *= 1 - reflectance;

        if ( reflectance < 0 || reflectance > 1.0){
            cout << "reflectance: " << reflectance << "\n";

            cout << "Total reflect: " << reflectedLight << "\n";
            cout << "Total transmit: " << transmittedLight << "\n";
        }
        

    }
    else{
    
        reflectedLight  *= collidedObject->material.reflection;
        transmittedLight *= collidedObject->material.transmission;
    }

    reflectedLight  *= collidedObject->material.reflection;
    transmittedLight *= collidedObject->material.transmission;

    local *= collidedObject->material.local;
    vec3 indirect = p_map.getIllumination( hitPoint, 
                                          incidentRay.direction,
                                          normal,
                                          collidedObject->material );
    return indirect ; //local + reflectedLight + transmittedLight ; //+ indirect;
}



vec3 RayTracer::phong(const Ray & incidentRay ,
                            vec3 & normal,
                            vec3 & hitPoint,
                            int shapeIndex ){

    Material material = m_scene.objects[ shapeIndex ]->material;
    vec3 viewRay = incidentRay.direction * -1;

    vec3 specular, diffuse;

    //iterate over each of the lights to see if the item is is shadow.
    for ( int i = 0; i < m_scene.lights.size(); i ++ ){

        Ray lightRay;
        lightRay.origin = hitPoint;
        lightRay.direction = m_scene.lights[i].position - hitPoint;

        float time = m_scene.collisionTime( lightRay, shapeIndex );

        //the ray has hit an object before the light source
        //reset the diffuse and specular components to zero
        if ( time < 1.0 ){
          continue;
        }


        vec3 direction_to_light = m_scene.lights[i].position - hitPoint;
        direction_to_light.normalize();

        vec3 reflection = reflect( -direction_to_light, normal);


        //diffuse
        double k_diffuse = std::max(
                           normal.dotProduct( normal, direction_to_light ), 0.0);
        diffuse += m_scene.lights[i].intensity * k_diffuse * material.diffuse;

        //specular
        double k_specular = std::max(
                            viewRay.dotProduct( viewRay, reflection) , 0.0 );

        k_specular = pow( k_specular, 10); // material.shinyness );
        specular += m_scene.lights[i].intensity * k_specular * material.specular;

    }

    vec3 ambient = m_scene.ambient * material.ambient;
    vec3 color = diffuse + specular + ambient;

    return color;
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
    else if (cmd == "outsize"){
        checksize(words,2);
        m_scene.view.nrows = parseInt(words[1]);
        m_scene.view.ncols = parseInt(words[2]);
    }
    else if (cmd == "origin"){
        m_scene.view.origin = vec3(parseInt(words[1]),parseInt(words[2]),parseInt(words[3]));
    }
    else if (cmd == "horiz"){
        m_scene.view.horiz = vec3(parseInt(words[1]),parseInt(words[2]),parseInt(words[3]));
    }
    else if (cmd == "vert"){
        m_scene.view.vert = vec3(parseInt(words[1]),parseInt(words[2]),parseInt(words[3]));
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

        vec3 center( parseFloat(words[1]), parseFloat(words[2]), parseFloat(words[3]) );
        cout << "sphere center:" << center << endl;
        Sphere* object = new Sphere(center, parseFloat(words[4]), m_materials["current_"]);
        m_scene.objects.push_back(object);
    }
    else if (cmd == "rectangle"){
        checksize(words, 9);
        vec3 ll ( parseFloat(words[1]),parseFloat(words[2]),parseFloat(words[3]) );
        vec3 lr ( parseFloat(words[4]),parseFloat(words[5]),parseFloat(words[6]) );
        vec3 ur ( parseFloat(words[7]),parseFloat(words[8]),parseFloat(words[9]) );
        Rectangle* object = new Rectangle(ll, lr, ur, m_materials["current_"]);
        m_scene.objects.push_back(object);
    }
    else if (cmd == "triangle"){
        checksize(words,9);
        vec3 pt1 ( parseFloat(words[1]),parseFloat(words[2]),parseFloat(words[3]) );
        vec3 pt2 ( parseFloat(words[4]),parseFloat(words[5]),parseFloat(words[6]) );
        vec3 pt3 ( parseFloat(words[7]),parseFloat(words[8]),parseFloat(words[9]) );
        Triangle* object = new Triangle(pt1,pt2, pt3, m_materials["current_"]);
        m_scene.objects.push_back(object);
    }
    else if (cmd == "amblight"){
        checksize(words,1);
        m_scene.ambient = parseFloat(words[1]);
    }
    else if (cmd == "light"){
        checksize(words,4);
        Light light;
        light.position = parseVec3(words,1);
        light.intensity = parseFloat(words[4]);
        m_scene.lights.push_back(light);
    }
    else if ( cmd == "reflect"){
        checksize( words, 1);
        m_materials["current_"].reflection = parseFloat(words[1]);
    }
    else if ( cmd == "transmit"){
        checksize( words, 2);
        m_materials["current_"].transmission = parseFloat( words[1]);
        m_materials["current_"].refractionIndex = parseFloat( words[2]);
    }
    else if ( cmd == "local"){
        checksize( words, 1);
        m_materials["current_"].local = parseFloat( words[1]);

    }
    else if ( cmd == "alpha"){
        checksize( words, 1);
        m_materials["current_"].alpha = parseFloat(words[1]);
    }
    else if ( cmd == "beta"){
        checksize( words, 1);
        m_materials["current_"].beta = parseFloat(words[1]);
    }

    else{
        throw parser_error("Unknown command: "+cmd);
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
    else if(subcmd != "amb" && subcmd != "diff" && subcmd != "spec"){
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
        if(subcmd=="amb"){m_materials["current_"].ambient=clr;}
        else if(subcmd=="diff"){m_materials["current_"].diffuse=clr;}
        else if(subcmd=="spec"){m_materials["current_"].specular=clr;}
    }
}


void RayTracer::getPhotonMap(){
    p_map.mapScene( m_scene );

}



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

/*After this code is executed, the model will have been loaded, 
 * normals will have been created and smoothed, and the model will be
 *  ready to render. To draw the loaded model, add the following code
 *   into your display function:
 */
//glmDraw(objmodel_ptr, GLM_SMOOTH | GLM_MATERIAL);
