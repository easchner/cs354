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
  // cout << "AddTextureVertex: (" << v[0] << "," << v[1] << ")" << endl;
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
  if (p.size() > 3)
    cout << "polygon more than 3 vertex" << endl;

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
  // What we are going to do is save a list of each vertex then iterate
  // through each face and compute its normal and add it to that face's
  // vertices.  Then normalize each vertex's normal

  // Create our list of vertices
  vert_norm_list.resize(vert_list.size());

  Vec3f edge1, edge2, normal;

  // Iterate through each face
  for (int i = 0; i < poly_list.size(); i++) {
    // Compute the normal by taking cross product of V1 - V0 and V2 - V1
    edge1 = vert_list[poly_list[i].v[1]] - vert_list[poly_list[i].v[0]];
    edge2 = vert_list[poly_list[i].v[2]] - vert_list[poly_list[i].v[1]];
    normal = edge1 ^ edge2;
    normal = normal.unit();

    // Add it to each vertex on the polygon
    for (int j = 0; j < poly_list[i].v.size(); j++) {
      vert_norm_list[poly_list[i].v[j]] += normal;
    }
  }

  // Now normalize each vertex's normal
  for (int i = 0; i < vert_norm_list.size(); i++) {
    vert_norm_list[i] = vert_norm_list[i].unit();
  }
}

// Draw our mesh to the world view
void Mesh::draw_mesh(GLuint* texture_ids) {
  glTranslatef(0 , -bb().dim(1)/2, 0);
  // cout << "Drawing mesh" << endl;
  int currentMaterial = -1;
  glCullFace(GL_BACK);
  glEnable(GL_CULL_FACE);
  glColor4f(1, 1, 1, 1);

  for (int i = 0; i < poly_list.size(); i++) {
    if (_polygon2material[i] != currentMaterial) {
      Material cur_mat = _materials[_polygon2material[i]];
      Vec3f vec = cur_mat.ambient();
      GLfloat ambient[] = {vec[0], vec[1], vec[2], vec[3]};
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
      vec = cur_mat.diffuse();
      GLfloat diffuse[] = {vec[0], vec[1], vec[2], vec[3]};
      // glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
      vec = cur_mat.specular();
      GLfloat specular[] = {vec[0], vec[1], vec[2], vec[3]};
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
      glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, cur_mat.specular_coeff());
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, texture_ids[cur_mat.texture_id() - 1]);
      currentMaterial = _polygon2material[i];
    }

    glBegin(GL_POLYGON);
    glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);

    for (int j = 0; j < poly_list[i].v.size(); j++) {
      glNormal3f(vert_norm_list[poly_list[i].v[j]][0],
                 vert_norm_list[poly_list[i].v[j]][1],
                 vert_norm_list[poly_list[i].v[j]][2]);
      glTexCoord2f(tex_vert_list[poly_list[i].vt[j]][0],
                   tex_vert_list[poly_list[i].vt[j]][1]);
      glVertex3f(vert_list[poly_list[i].v[j]][0],
                 vert_list[poly_list[i].v[j]][1],
                 vert_list[poly_list[i].v[j]][2]);
    }
    glEnd();
  }
  glTranslatef(0.0, bb().dim(1)/2, 0.0);

  // This is to show normals
  /* glDisable(GL_LIGHTING);
  glLineWidth(2);
  glBegin(GL_LINES);
  glColor3f(1.0, 0.0, 0.0);
  for (int i = 0; i < vert_list.size(); i++) {
    glVertex3f(vert_list[i][0], vert_list[i][1], vert_list[i][2]);
    glVertex3f(vert_list[i][0] + vert_norm_list[i][0],
               vert_list[i][1] + vert_norm_list[i][1],
               vert_list[i][2] + vert_norm_list[i][2]);
  }
  glEnable(GL_LIGHTING); */
}
