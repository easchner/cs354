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
  vert_list.push_back(v);
  // updates the bounding box
  _bb(v);
}

// This will be called by the obj parser
void Mesh::AddTextureVertex(const Vec3f& v) {
  tex_vert_list.push_back(v);
}

void Mesh::AddPolygon(const std::vector<int>& p, const std::vector<int>& pt) {
  // Store the polys vertices and texture coordinates
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
  // updates the poly2mat map
  _polygon2material.push_back(_cur_mtl);
}

// Computes a normal for each vertex.
void Mesh::compute_normals() {
  // What we are going to do is save a list of each vertex then iterate
  // through each face and compute its normal and add it to that face's
  // vertices.  Then normalize each vertex's normal.

  // Create our list of vertices
  vert_norm_list.resize(vert_list.size());
  Vec3f edge1, edge2, normal;

  // Iterate through each face
  for (int i = 0; i < poly_list.size(); i++) {
    // Compute the normal by taking cross product of V1 - V0 and V2 - V1
    // Actual vertices used doesn't matter as long as direction is right
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
void Mesh::draw_mesh(GLuint* texture_ids, bool normals) {
  int currentMaterial = -1;
  glCullFace(GL_BACK);
  glEnable(GL_CULL_FACE);

  // Iterate through each face
  for (int i = 0; i < poly_list.size(); i++) {
    // Change material properties if different from last face
    if (_polygon2material[i] != currentMaterial) {
      Material cur_mat = _materials[_polygon2material[i]];
      Vec3f vec = cur_mat.ambient();
      GLfloat ambient[] = {vec[0], vec[1], vec[2], vec[3]};
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
      vec = cur_mat.diffuse();
      GLfloat diffuse[] = {vec[0], vec[1], vec[2], vec[3]};
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
      vec = cur_mat.specular();
      GLfloat specular[] = {vec[0], vec[1], vec[2], vec[3]};
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
      glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, cur_mat.specular_coeff());
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, texture_ids[cur_mat.texture_id() - 1]);
      currentMaterial = _polygon2material[i];
    }

    // Draw the face
    glBegin(GL_POLYGON);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    for (int j = 0; j < poly_list[i].v.size(); j++) {
      glNormal3f(vert_norm_list[poly_list[i].v[j]][0],
                 vert_norm_list[poly_list[i].v[j]][1],
                 vert_norm_list[poly_list[i].v[j]][2]);
                 if (tex_vert_list.size() > 0) {
                   glTexCoord2f(tex_vert_list[poly_list[i].vt[j]][0],
                     tex_vert_list[poly_list[i].vt[j]][1]);
                 }
      glVertex3f(vert_list[poly_list[i].v[j]][0],
                 vert_list[poly_list[i].v[j]][1],
                 vert_list[poly_list[i].v[j]][2]);
    }
    glEnd();
  }

  // This is to show normals
  if (normals) {
    glDisable(GL_LIGHTING);
    glLineWidth(2);
    glBegin(GL_LINES);
    glColor3f(1.0, 0.0, 0.0);
    for (int i = 0; i < vert_list.size(); i++) {
      glVertex3f(vert_list[i][0], vert_list[i][1], vert_list[i][2]);
      glVertex3f(vert_list[i][0] + vert_norm_list[i][0],
                 vert_list[i][1] + vert_norm_list[i][1],
                 vert_list[i][2] + vert_norm_list[i][2]);
    }
    glEnd();
    glEnable(GL_LIGHTING);
  }
}
