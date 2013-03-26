#include <iostream>
#include <vector>

#include "./mesh.h"
#include "./common.h"

using namespace std;

Mesh::Mesh() {
  _cur_mtl = -1;
}

// This will be called by the obj parser
void Mesh::AddVertex(const Vec3f& v) {
  // TODO
  vert_list.push_back(v);
  // cout << "AddVertex: (" << v[0] << "," << v[1] << "," << v[2] << ")"
  //  << endl;
  // updates the bounding box
  _bb(v);
}

// This will be called by the obj parser
void Mesh::AddTextureVertex(const Vec3f& v) {
  // TODO
  tex_vert_list.push_back(v);
  // cout << "AddTextureVertex: (" << v[0] << "," << v[1] << "," << v[2]
  //   << ")" << endl;
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

  // cout << "AddPolygon: (";

  for (int i = 0; i < p.size(); i++) {
    v.push_back(p[i]);
    // cout << p[i];
    // if (i < p.size() - 1)
      // cout << ", ";
  }
  // cout << ") -> (";
  for (int i = 0; i < pt.size(); i++) {
    vt.push_back(pt[i]);
    // cout << pt[i];
    // if (i < pt.size() - 1)
      // cout << ", ";
  }

  // cout << ")" << endl;

  poly poly = {v, vt};
  poly_list.push_back(poly);
  // updates the poly2mat map
  _polygon2material.push_back(_cur_mtl);
}

// Computes a normal for each vertex.
void Mesh::compute_normals() {
  // TODO don't forget to normalize your normals!
  // This is wrong, the normal of a plane should be the cross product of
  // two vectors in the plane Can get two vectors by subtracting points
  // Vertex normals are the same of each adjacent plane normal
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
  // cout << "Computed Normals" << endl;
}

// Draw our mesh to the world view
void Mesh::draw_mesh() {
  // cout << "Drawing mesh" << endl;

  for (int i = 0; i < poly_list.size(); i++) {
    glBegin(GL_POLYGON);
    glColor3f(1.0, 0.0, 0.0);
    for (int j = 0; j < poly_list[i].v.size(); j++) {
      glVertex3f(vert_list[poly_list[i].v[j]][0],
                 vert_list[poly_list[i].v[j]][1],
                 vert_list[poly_list[i].v[j]][2]);
    }
    glEnd();
  }
}
