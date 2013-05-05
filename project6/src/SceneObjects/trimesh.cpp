#include <cmath>
#include <float.h>
#include "trimesh.h"


using namespace std;

Trimesh::~Trimesh()
{
  for( Materials::iterator i = materials.begin(); i != materials.end(); ++i )
    delete *i;
}

// must add vertices, normals, and materials IN ORDER
void Trimesh::addVertex( const Vec3d &v )
{
  vertices.push_back( v );
}

void Trimesh::addMaterial( Material *m )
{
  materials.push_back( m );
}

void Trimesh::addNormal( const Vec3d &n )
{
  normals.push_back( n );
}

// Returns false if the vertices a,b,c don't all exist
bool Trimesh::addFace( int a, int b, int c )
{
  int vcnt = vertices.size();

  if( a >= vcnt || b >= vcnt || c >= vcnt ) return false;

  TrimeshFace *newFace = new TrimeshFace( scene, new Material(*this->material), this, a, b, c );
  newFace->setTransform(this->transform);
  faces.push_back( newFace );
  return true;
}

char *
Trimesh::doubleCheck()
    // Check to make sure that if we have per-vertex materials or normals
    // they are the right number.
{
  if( !materials.empty() && materials.size() != vertices.size() )
    return "Bad Trimesh: Wrong number of materials.";
  if( !normals.empty() && normals.size() != vertices.size() )
    return "Bad Trimesh: Wrong number of normals.";

  return 0;
}

bool Trimesh::intersectLocal(const ray&r, isect&i) const
{
  double tmin = 0.0;
  double tmax = 0.0;
  typedef Faces::const_iterator iter;
  bool have_one = false;
  for( iter j = faces.begin(); j != faces.end(); ++j ) {
    isect cur;
    if( (*j)->intersectLocal( r, cur ) )
    {
      if( !have_one || (cur.t < i.t) )
      {
        i = cur;
        have_one = true;
      }
    }
  }
  if( !have_one ) i.setT(1000.0);
  return have_one;
}

// Intersect ray r with the triangle abc.  If it hits returns true,
// and puts the t parameter, barycentric coordinates, normal, object id,
// and object material in the isect object
// Using Moller / Trumbore algorithm
bool TrimeshFace::intersectLocal( const ray& r, isect& i ) const
{
  const Vec3d& a = parent->vertices[ids[0]];
  const Vec3d& b = parent->vertices[ids[1]];
  const Vec3d& c = parent->vertices[ids[2]];

  Vec3d B_A = b - a;
  Vec3d C_A = c - a;
  Vec3d PxT = r.getDirection() ^ C_A;
  Vec3d t = r.getPosition() - a;
  Vec3d q = t ^ B_A;

  double determinant = B_A * PxT;
  double invertedDet = 1 / determinant;

  double u = (t * PxT) * invertedDet;
  double v = (r.getDirection() * q) * invertedDet;

  if (determinant == 0) {
    return false;
  } else if (u < 0 || v < 0) {
    return false;
  } else if (u + v > 1) {
    return false;
  }
  double tval = (C_A * q) * invertedDet;
  if (tval < 0){
    cout << "Below Epsilon, r.orig = (" << r.getPosition()[0] << ", " << r.getPosition()[1] << ", " << r.getPosition()[2] << ")  -  Direction = (" << r.getDirection()[0] << ", " << r.getDirection()[1] << ", " << r.getDirection()[2] << ")  -  t = " << tval << endl;
    return false;
  }

  Vec3d calcNormal = parent->normals[ids[0]] * (1-u-v) + parent->normals[ids[1]]
     * u + parent->normals[ids[2]] * v;

  i.setN(calcNormal);
  i.N.normalize();
  i.setMaterial(*(parent->material));
  i.setUVCoordinates(Vec2d(u, v));
  i.setT(tval);
  i.setObject(this);

  return true;
}

void Trimesh::generateNormals()
    // Once you've loaded all the verts and faces, we can generate per
    // vertex normals by averaging the normals of the neighboring faces.
{
  int cnt = vertices.size();
  normals.resize( cnt );
  int *numFaces = new int[ cnt ]; // the number of faces assoc. with each vertex
  memset( numFaces, 0, sizeof(int)*cnt );
    
  for( Faces::iterator fi = faces.begin(); fi != faces.end(); ++fi )
  {
    Vec3d faceNormal = (**fi).getNormal();
        
    for( int i = 0; i < 3; ++i )
    {
      normals[(**fi)[i]] += faceNormal;
      ++numFaces[(**fi)[i]];
    }
  }

  for( int i = 0; i < cnt; ++i )
  {
    if( numFaces[i] )
      normals[i]  /= numFaces[i];
  }

  delete [] numFaces;
  vertNorms = true;
}

