//C++ STL
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <vector>

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


using std::cout;
using std::endl;
using std::string;
using std::ifstream;
using std::vector;

using cs40::Ray;
using cs40::RayTracer;
using cs40::RGBColor;
using cs40::Material;
using cs40::Scene;
using cs40::View;
//get stuck? use the nuclear option..
//using namespace cs40;


RayTracer::RayTracer(){
    /* do nothing? */
    background_color = RGBColor( 0.0, 0.0, 0.0 );
    max_depth = 10;
}

RayTracer::~RayTracer(){
    /* do nothing? */
}

void RayTracer::trace( RGBImage & img ){
  
  for ( float i = 0.0; i < m_scene.view.nrows; i += 1.0 ){
    for ( float j = 0.0; j < m_scene.view.ncols; j += 1.0 ){

      Ray viewRay;
      viewRay.origin = m_scene.view.eye;

      vec3 h_disp = m_scene.view.horiz * ( i / m_scene.view.nrows );
      vec3 v_disp = m_scene.view.vert * ( j / m_scene.view.ncols );
      vec3 pixel_position = m_scene.view.origin + h_disp + v_disp;

      viewRay.direction = pixel_position - viewRay.origin;

      RGBImage( int(i), int(j) ) = traceOnce( viewRay, 0 );
    }
  }
}

//TODO 
//right now this fuction has lots of extra stuff that does not function
//at this point, none of hte light modes actually work. 
RGBColor RayTracer::traceOnce( Ray & incidentRay, int depth ){
  //if the recursion depth has been maxed, then return the bg color
  if (depth > max_depth ){ return m_scene.view.background ; }

  //get the closest colliding shape and 
  Shape * collidedObject = checkIntersection( incidentRay );

  //return if the ray hits a light source or there is no intersection
  if (incidentRay.status == HIT_LIGHT_SOURCE ){
    return RBGColor( 1.0, 1.0, 1.0); //light_color;
  }
  else if (incidentRay.status == NO_INTERSECTION ){
    return m_scene.view.background;
  }

  //get the normal to calculate phong stuff and reflection
  vec3 normal = collidedObject->normal( p );
   
  //get the phong lighting modelling.
  RGBColor local = phong( incidentRay , normal, collidedObject->material );

  //the following lines are optional stuff.
  //Ray reflectionRay = reflect( incidentRay, normal);
  //Ray transmissionRay = transmit( incidentRay, normal);
  //RGBColor reflected = traceOnce( reflectionRay, step + 1 );
  //RGBColor transmitted = traceOnce( transmissionRay, step + 1 );

  return local + reflected + transmitted;
}

//TODO
//this function should take a ray and check it for intersection with
//all of the shapes in the scene.
//it should set the status of the ray and set its position to the
//point of intersection.
//there may be some errors with ray status
Shape RayTracer::checkIntersection( Ray & incidentRay ){
  float currentTime , minTime;
  minTime = 999999999999999999;
  Shape * closestShape = NULL;

  for ( int i = 0 ; i < m_scene.objects.size(); i ++ ){

    currentTime = m_scene.objects[i]->hitTime( incidentRay );
    if ( (currentTime < minTime ) && ( currentTime > 0 )){
      minTime = currentTime;
      closestShape = m_scene.objects[i];
    }
  }

  if (closestShape == NULL){ incidentRay.status == NO_INTERSECTION; }
  else { 
    incidentRay.status == HIT_OBJECT;
    incidentRay.origin = incidentRay( currentTime );
  }

  return closestShape;

}

//TODO
//actually apply the phong light stuff.
//send out a rayTrace to each of the lights and see if there is 
//a light in the way. if there is, then we can ignore the diffuse and
//specular components.
RGBColor RayTracer::phong( const Ray & incidentRay ,
                           const vec3 & normal,
                           const Material & material ){
  RGBColor ambient, diffuse, specular; 

  //the following line is used for a test to check that collision is
  //happening.
  ambient = RGBColor( 1.0, 1.0, 1.0 );

  //iterate over each of the lights to see if the item is is shadow.
  for ( int i = 0; i < m_scene.lights.size(); i ++ ){
    Ray lightRay;
    lightRay.origin = incidentRay.origin;
    lightRay.direction = m_scene.lights[i].position - lightRay.origin;

    float time = collisionTime( lightRay );

    //the ray has hit an object before the light source
    //reset the diffuse and specular components to zero
    if ( time < 1.0 ){
      cout << "Light Obstructed"<< endl;
      cout << "But no shadows were created due to phong being incomplete";
      cout << endl;
    }
  }

  return ambient + diffuse + specular;
}


float RayTracer::collisionTime( Ray & incidentRay ){
  float currentTime , minTime;
  minTime = 999999999999999;

  //check all of the intersection times for the shapes
  for ( int i = 0 ; i < m_scene.objects.size(); i ++ ){

    currentTime = m_scene.objects[i]->hitTime( incidentRay );
    if ( (currentTime < minTime ) && ( currentTime > 0 )){
      minTime = currentTime;
    }
  }

  return minTime;

}


//TODO
//the normal must have unit magnitude for this to work
Ray RayTracer::reflect( const Ray & incidentRay, const vec3 & normal ){
  Ray reflectedRay;

  r=d¿2(d¿n^)n^
  reflectedRay.origin = incidentRay.origin;

  reflectedRay.direction = ( reflectedRay.direction - 
                        2 * dotProduct( normal, reflectedRay.direction) * 
                        normal );
  return reflectedRay;
}

//TODO
Ray RayTracer::transmit( const Ray & incidentRay, const vec3 & normal ){
  Ray transmitRay;
  transmitRay.origin = incidentRay.origin;
  return transmitRay;
}

void RayTracer::save(){
    View vw = m_scene.view;

    RGBImage img(vw.nrows, vw.ncols, convertColor(vw.background));

    //TODO: ray trace away!

    img.saveAs(m_scene.view.fname, true);
    cout << "Saved result to " << m_scene.view.fname << endl;
}

/* convert from 0-1 rgb space to 0-255 */
RGBColor RayTracer::convertColor(const vec3& clr){
    int r,g,b;
    r=(int)(255*clr.x());
    g=(int)(255*clr.y());
    b=(int)(255*clr.z());
    return RGBColor(r,g,b);
}

void RayTracer::parseFile(const string& fname){
    ifstream infile;
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
        //use current material as material for sphere
        //get rid of this error after you parse the sphere
        throw parser_error("Parse your own sphere");
    }
    else if (cmd == "rectangle"){
        checksize(words, 9);
        //use current material as material for rect
        throw parser_error("Parse your own rectangle");
    }
    else if (cmd == "amblight"){
        checksize(words,1);
        m_scene.ambient = parseFloat(words[1]);
    }
    else if (cmd == "light"){
        checksize(words,4);
        //Light l;
        throw parser_error("Parse your own light");
        //m_scene.lights.push_back(l);
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
