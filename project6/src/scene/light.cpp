#include <cmath>

#include "light.h"



using namespace std;

double min(double a, double b)
{
  if (a < b)
    return a;
  else
    return b;
}

double DirectionalLight::distanceAttenuation( const Vec3d& P ) const
{
  // distance to light is infinite, so f(di) goes to 0.  Return 1.
  return 1.0;
}


Vec3d DirectionalLight::shadowAttenuation( const Vec3d& P ) const
{
  // YOUR CODE HERE:
  // You should implement shadow-handling code here.

  return Vec3d(1,1,1);

}

Vec3d DirectionalLight::getColor( const Vec3d& P ) const
{
  // Color doesn't depend on P 
  return color;
}

Vec3d DirectionalLight::getDirection( const Vec3d& P ) const
{
  return -orientation;
}

double PointLight::distanceAttenuation( const Vec3d& P ) const
{
    // These three values are the a, b, and c in the distance
  // attenuation function (from the slide labelled 
  // "Intensity drop-off with distance"):
  //    f(d) = min( 1, 1/( a + b d + c d^2 ) )
  Vec3d ans = P - position;
  double d = sqrt(pow(ans[0],2) + pow(ans[1],2) + pow(ans[2],2));
  
  return min( 1, 1/(constantTerm + linearTerm * d + quadraticTerm * pow(d,2)));
}

Vec3d PointLight::getColor( const Vec3d& P ) const
{
  // Color doesn't depend on P 
  return color;
}

Vec3d PointLight::getDirection( const Vec3d& P ) const
{
  Vec3d ret = position - P;
  ret.normalize();
  return ret;
}


Vec3d PointLight::shadowAttenuation(const Vec3d& P) const
{
  // YOUR CODE HERE:
  // You should implement shadow-handling code here.

  return Vec3d(1,1,1);

}
