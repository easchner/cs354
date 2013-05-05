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
  Vec3d retVal = ke(i) + prod(ka(i), scene->ambient());

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
  int x = coord[0] * width;
  int y = coord[1] * height;

  return Vec3d(getPixelAt(x,y));
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

