#include "ray.h"
#include "material.h"
#include "light.h"

#include "../fileio/bitmap.h"
#include "../fileio/pngimage.h"

using namespace std;
extern bool debugMode;

double max(double a, double b)
{
  if (a > b)
    return a;
  else
    return b;
}


// Apply the phong model to this point on the surface of the object, returning
// the color of that point.
Vec3d Material::shade( Scene *scene, const ray& r, const isect& i ) const
{
  // YOUR CODE HERE

  // For now, this method just returns the diffuse color of the object.
  // This gives a single matte color for every distinct surface in the
  // scene, and that's it.  Simple, but enough to get you started.
  // (It's also inconsistent with the phong model...)

  // Your mission is to fill in this method with the rest of the phong
  // shading model, including the contributions of all the light sources.
  // You will need to call both distanceAttenuation() and shadowAttenuation()
  // somewhere in your code in order to compute shadows and light falloff.
/*  if( debugMode ) {
    cout << "Debugging Phong code..." << endl;
    Vec3d test = scene->ambient();
    cout << "scene amb " << test[0] << " - " << test[1] << " - " << test[2] << endl;
    test = prod(ka(i), scene->ambient());
    cout << "amb val " << test[0] << " - " << test[1] << " - " << test[2] << endl;
  }
*/
  Vec3d retVal = ke(i) + prod(ka(i), scene->ambient());

  // When you're iterating through the lights,
  // you'll want to use code that looks something
  // like this:
  //
  for (vector<Light*>::const_iterator litr = scene->beginLights(); 
       litr != scene->endLights(); ++litr) {
    Vec3d point = r.getPosition() + r.getDirection() * i.t;
    Light* pLight = *litr;
    Vec3d reflectionAngle = 2 * (i.N * pLight->getDirection(point)) * i.N - pLight->getDirection(point);

    Vec3d diffIntensity = kd(i) * (max(0, i.N * pLight->getDirection(point)));

    Vec3d viewerAngle = scene->getCamera().getEye() - point;
    viewerAngle.normalize();
    Vec3d specIntensity = ks(i) * pow(max(0, viewerAngle * reflectionAngle), shininess(i));

    Vec3d lcolor = pLight->getColor(point);

    Vec3d totalColor = prod(diffIntensity + specIntensity, lcolor);
    totalColor = totalColor * pLight->distanceAttenuation(point);
    totalColor = prod(totalColor, pLight->shadowAttenuation(point));

    retVal = retVal + totalColor;	

/*    if (debugMode) {
      cout << "retVal " << retVal[0] << " - " << retVal[1] << " - " << retVal[2] << endl;
      cout << "diff " << diffIntensity[0] << " - " << diffIntensity[1] << " - " << diffIntensity[2] << endl;
      cout << "spec " << specIntensity[0] << " - " << specIntensity[1] << " - " << specIntensity[2] << endl;
      cout << "reflection " << reflectionAngle[0] << " - " << reflectionAngle[1] << " - " << reflectionAngle[2] << endl;
      cout << "lcolor " << lcolor[0] << " - " << lcolor[1] << " - " << lcolor[2] << endl;
      cout << "distAtten " << pLight->distanceAttenuation(point) << endl;
    }*/
  }
  return retVal;
}

TextureMap::TextureMap( string filename ) {

  int start = filename.find_last_of('.');
  int end = filename.size() - 1;
  if (start >= 0 && start < end) {
    string ext = filename.substr(start, end);
    if (!ext.compare(".png")) {
      png_cleanup(1);
      if (!png_init(filename.c_str(), width, height)) {
        double gamma = 2.2;
        int channels, rowBytes;
        unsigned char* indata = png_get_image(gamma, channels, rowBytes);
        int bufsize = rowBytes * height;
        data = new unsigned char[bufsize];
        for (int j = 0; j < height; j++)
          for (int i = 0; i < rowBytes; i += channels)
            for (int k = 0; k < channels; k++)
              *(data + k + i + j * rowBytes) = *(indata + k + i + (height - j - 1) * rowBytes);
        png_cleanup(1);
      }
    }
    else
      if (!ext.compare(".bmp")) data = readBMP(filename.c_str(), width, height);
      else data = NULL;
  } else data = NULL;
  if (data == NULL) {
    width = 0;
    height = 0;
    string error("Unable to load texture map '");
    error.append(filename);
    error.append("'.");
    throw TextureMapException(error);
  }
}

Vec3d TextureMap::getMappedValue( const Vec2d& coord ) const
{
  // YOUR CODE HERE

  // In order to add texture mapping support to the 
  // raytracer, you need to implement this function.
  // What this function should do is convert from
  // parametric space which is the unit square
  // [0, 1] x [0, 1] in 2-space to bitmap coordinates,
  // and use these to perform bilinear interpolation
  // of the values.
  
  return Vec3d(1.0, 1.0, 1.0);
}


Vec3d TextureMap::getPixelAt( int x, int y ) const
{
  // This keeps it from crashing if it can't load
  // the texture, but the person tries to render anyway.
  if (0 == data)
    return Vec3d(1.0, 1.0, 1.0);

  if( x >= width )
    x = width - 1;
  if( y >= height )
    y = height - 1;

  // Find the position in the big data array...
  int pos = (y * width + x) * 3;
  return Vec3d( double(data[pos]) / 255.0, 
                double(data[pos+1]) / 255.0,
                double(data[pos+2]) / 255.0 );
}

Vec3d MaterialParameter::value( const isect& is ) const
{
  if( 0 != _textureMap )
    return _textureMap->getMappedValue( is.uvCoordinates );
  else
    return _value;
}

double MaterialParameter::intensityValue( const isect& is ) const
{
  if( 0 != _textureMap )
  {
    Vec3d value( _textureMap->getMappedValue( is.uvCoordinates ) );
    return (0.299 * value[0]) + (0.587 * value[1]) + (0.114 * value[2]);
  }
  else
    return (0.299 * _value[0]) + (0.587 * _value[1]) + (0.114 * _value[2]);
}

