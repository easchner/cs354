// The main ray tracer.

#pragma warning (disable: 4786)

#include "RayTracer.h"
#include "scene/light.h"
#include "scene/material.h"
#include "scene/ray.h"

#include "parser/Tokenizer.h"
#include "parser/Parser.h"

#include "ui/TraceUI.h"
#include <cmath>
#include <algorithm>

extern TraceUI* traceUI;

#include <iostream>
#include <fstream>

using namespace std;

// Use this variable to decide if you want to print out
// debugging messages.  Gets set in the "trace single ray" mode
// in TraceGLWindow, for example.
bool debugMode = false;

// Trace a top-level ray through normalized window coordinates (x,y)
// through the projection plane, and out into the scene.  All we do is
// enter the main ray-tracing method, getting things started by plugging
// in an initial ray weight of (0.0,0.0,0.0) and an initial recursion depth of 0.
Vec3d RayTracer::trace( double x, double y )
{
  // Clear out the ray cache in the scene for debugging purposes,
  scene->intersectCache.clear();

  ray r( Vec3d(0,0,0), Vec3d(0,0,0), ray::VISIBILITY );

  scene->getCamera().rayThrough( x,y,r );
  Vec3d ret = traceRay( r, Vec3d(1.0,1.0,1.0), 0 );
  ret.clamp();
  return ret;
}

// Do recursive ray tracing!  You'll want to insert a lot of code here
// (or places called from here) to handle reflection, refraction, etc etc.
Vec3d RayTracer::traceRay( const ray& r, const Vec3d& thresh, int depth )
{
  isect i;

  // if depth > max depth, return background color
  if(depth > traceUI->getDepth())
    return Vec3d( 0.0, 0.0, 0.0);

  if( scene->intersect( r, i ) ) {
        
    const Material& m = i.getMaterial();

    Vec3d iphong = Vec3d(0);
    Vec3d ireflect = Vec3d(0);
    Vec3d itransmit = Vec3d(0);
    Vec3d transmission_dir;
    ray transmission_ray (r.at(i.t), i.N, ray::REFRACTION);
    
    iphong = m.shade(scene,r,i);

    // calculate reflection
    Vec3d ray_dir = -r.getDirection();
    Vec3d reflection_dir = 2 * (ray_dir * i.N) * i.N - ray_dir;
    
    // create reflection_ray
    ray reflection_ray (r.at(i.t), reflection_dir, ray::REFLECTION);
    
    //recurse the reflection ray
    ireflect = traceRay(reflection_ray, thresh, depth+1);

    // Check for transmissive rays
    double val = m.kt(i)[0] + m.kt(i)[1] + m.kt(i)[2];
    if (val > 0) {
      // Check are we coming in or going out
      double test = ray_dir * i.N;

      double tranIndex, thetaI, thetaT;
      
      if (test > 0) {
        // calculate transmission direction
        tranIndex = 1 / m.index(i);
        thetaI = i.N * ray_dir;
        thetaT = 1 - pow(tranIndex, 2) * (1 - pow(thetaI, 2));
        thetaT = sqrt(thetaT);
        transmission_dir = i.N * (tranIndex * thetaI - thetaT) - ray_dir * tranIndex;
      } else {
        // calculate transmission direction
        tranIndex = m.index(i);
        thetaI = i.N * ray_dir;
        thetaT = (1 - pow(tranIndex, 2) * (1 - pow(thetaI, 2)));
        thetaT = sqrt(thetaT);
        transmission_dir = i.N * (tranIndex * thetaI + thetaT) - ray_dir * tranIndex;
      }
     
      // create the transmission ray
      if (thetaT > 0)	{
        ray temp_ray (r.at(i.t), transmission_dir, ray::REFRACTION);
        transmission_ray = temp_ray;
      }
      
      itransmit = traceRay(transmission_ray, thresh, depth + 1);
    }
    return iphong + prod(m.kr(i), ireflect) + prod(m.kt(i), itransmit);
  } else {
    // No intersection.  This ray travels to infinity, so we color
    // it according to the background color, which in this (simple) case
    // is just black.
    return Vec3d( 0.0, 0.0, 0.0 );
  }
}

RayTracer::RayTracer()
    : scene( 0 ), buffer( 0 ), buffer_width( 256 ), buffer_height( 256 ), m_bBufferReady( false )
{
  
}

RayTracer::~RayTracer()
{
  delete scene;
  delete [] buffer;
}

void RayTracer::getBuffer( unsigned char *&buf, int &w, int &h )
{
  buf = buffer;
  w = buffer_width;
  h = buffer_height;
}

double RayTracer::aspectRatio()
{
  return sceneLoaded() ? scene->getCamera().getAspectRatio() : 1;
}

bool RayTracer::loadScene( char* fn )
{
  ifstream ifs( fn );
  if( !ifs ) {
    string msg( "Error: couldn't read scene file " );
    msg.append( fn );
    traceUI->alert( msg );
    return false;
  }
	
  // Strip off filename, leaving only the path:
  string path( fn );
  if( path.find_last_of( "\\/" ) == string::npos )
    path = ".";
  else
    path = path.substr(0, path.find_last_of( "\\/" ));

  // Call this with 'true' for debug output from the tokenizer
  Tokenizer tokenizer( ifs, false );
  Parser parser( tokenizer, path );
  try {
    delete scene;
    scene = 0;
    scene = parser.parseScene();
  } 
  catch( SyntaxErrorException& pe ) {
    traceUI->alert( pe.formattedMessage() );
    return false;
  }
  catch( ParserException& pe ) {
    string msg( "Parser: fatal exception " );
    msg.append( pe.message() );
    traceUI->alert( msg );
    return false;
  }
  catch( TextureMapException e ) {
    string msg( "Texture mapping exception: " );
    msg.append( e.message() );
    traceUI->alert( msg );
    return false;
  }


  if( ! sceneLoaded() )
    return false;

  return true;
}

void RayTracer::traceSetup( int w, int h )
{
  if( buffer_width != w || buffer_height != h )
  {
    buffer_width = w;
    buffer_height = h;

    bufferSize = buffer_width * buffer_height * 3;
    delete [] buffer;
    buffer = new unsigned char[ bufferSize ];


  }
  memset( buffer, 0, w*h*3 );
  m_bBufferReady = true;
}

void RayTracer::tracePixel( int i, int j )
{
  Vec3d col;

  if( ! sceneLoaded() )
    return;

  int numRays = traceUI->getRays();

  double x, y;  
  double pixelSpacing = 1.0 / numRays;

  for (int stepX = 0 * numRays; stepX < numRays; stepX++) {
    for (int stepY = 0 * numRays; stepY < numRays; stepY++) {
      x = double(i + pixelSpacing * (double(stepX) + 0.5)) / double(buffer_width);
      y = double(j + pixelSpacing * (double(stepY) + 0.5)) / double(buffer_height);
      col = col + trace(x, y);
    }
  }

  unsigned char *pixel = buffer + ( i + j * buffer_width ) * 3;

  numRays = numRays * numRays;

  pixel[0] = (int)(255.0 * col[0] / numRays);
  pixel[1] = (int)(255.0 * col[1] / numRays);
  pixel[2] = (int)(255.0 * col[2] / numRays);
}


