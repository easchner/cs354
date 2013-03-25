#include <iostream>
#include <vector>

#include "./mesh.h"

using namespace std;

Mesh::Mesh() {
  _cur_mtl = -1;
}

// This will be called by the obj parser
void Mesh::AddVertex(const Vec3f& v) {
  // TODO
  vert_list.push_back(v);
  cout << "AddVertex: (" << v[0] << "," << v[1] << "," << v[2] << ")" << endl;
  // updates the bounding box
  _bb(v);
}

// This will be called by the obj parser
void Mesh::AddTextureVertex(const Vec3f& v) {
  // TODO
  tex_vert_list.push_back(v);
  cout << "AddTextureVertex: (" << v[0] << "," << v[1] << "," << v[2]
  << ")" << endl;
}

// p is the list of indices of vertices for this polygon.  For example,
// if p = {0, 1, 2} then the polygon is a triangle with the zeroth, first and
// second vertices added using AddVertex.
//
// pt is the list of texture indices for this polygon, similar to the
// actual vertices described above.
void Mesh::AddPolygon(const std::vector<int>& p, const std::vector<int>& pt) {
  // TODO
  vector <int> v;
  vector <int> vt;

  for (int i = 0; i < p.size(); i++) {
    v.push_back(p[i]);
  }
  for (int i = 0; i < pt.size(); i++) {
    vt.push_back(pt[i]);
  }

  poly poly = {v, vt};
  poly_list.push_back(poly);
  cout << "AddPolygon" << endl;
  // updates the poly2mat map
  _polygon2material.push_back(_cur_mtl);
}

// Computes a normal for each vertex.
void Mesh::compute_normals() {
  // TODO don't forget to normalize your normals!
  for (int i = 0; i < vert_list.size(); i++) {
    float mag = sqrt(pow(vert_list[i][0], 2) +
                     pow(vert_list[i][1], 2) +
                     pow(vert_list[i][0], 2));

    Vec3f normal;
    normal[0] = vert_list[i][0]/mag;
    normal[1] = vert_list[i][1]/mag;
    normal[2] = vert_list[i][2]/mag;
    vert_norm_list.push_back(normal);
  }
  cout << "Computed Normals" << endl;
}
