#include <math.h>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <algorithm>


#include "./common.h"
#include "./bb.h"
#include "./mesh.h"
#include "./io.h"
#include "./texture.h"


using namespace std;

Mesh mesh;

GLuint* texture_ids;

// window parameters
int window_width = 800, window_height = 600;
float window_aspect = window_width / static_cast<float>(window_height);
bool scene_lighting;

/* Our variables */
int old_y;
float x_norm, y_norm;
bool rotating;
bool zooming = false;
int radius = 1;
// (2,2,5) are the values that were originally in the gluLookAt()
Vec3f eye = {2, 2, 5};
BoundingBox bbox = {{-1, -1, -1}, {1, 1, 1}};
Vec3f center = (bbox.max+bbox.min)/2.0f;

void Display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(40.0, window_aspect, 1, 1500);

  // TODO call gluLookAt such that mesh fits nicely in viewport.
  // mesh.bb() may be useful.
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(eye[0], eye[1], eye[2],
            0, 0, 0,
            0, 1, 0);

  // TODO set up lighting, material properties and render mesh.
  // Be sure to call glEnable(GL_RESCALE_NORMAL) so your normals
  // remain normalized throughout transformations.

  // You can leave the axis in if you like.
  glDisable(GL_LIGHTING);
  glLineWidth(4);
  DrawAxis();
  glEnable(GL_LIGHTING);

  glFlush();
  glutSwapBuffers();
}

void PrintMatrix(GLfloat* m) {
  cout.precision(2);
  int w = 6;
  for (int i = 0; i < 4; ++i) {
    cout << setprecision(2) << setw(w) << m[i] << " "
         << setprecision(2) << setw(w) << m[i+4] << " "
         << setprecision(2) << setw(w) << m[i+8] << " "
         << setprecision(2) << setw(w) << m[i+12] << " "
         << endl;
  }
  cout << endl;
}

void PrintMatrix(GLint matrix) {
  GLfloat m[16];
  glGetFloatv(matrix, m);
  PrintMatrix(m);
}

void PrintMatrix() {
  PrintMatrix(GL_MODELVIEW_MATRIX);
}

void LoadMatrix(GLfloat* m) {
  // transpose to column-major
  for (int i = 0; i < 4; ++i) {
    for (int j = i; j < 4; ++j) {
      swap(m[i*4+j], m[j*4+i]);
    }
  }
  glLoadMatrixf(m);
}

void MultMatrix(GLfloat* m) {
  // transpose to column-major
  for (int i = 0; i < 4; ++i) {
    for (int j = i; j < 4; ++j) {
      swap(m[i*4+j], m[j*4+i]);
    }
  }
  glMultMatrixf(m);
}

void Init() {
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  glDepthFunc(GL_LEQUAL);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

  // resize the window
  window_aspect = window_width/static_cast<float>(window_height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  gluPerspective(40.0, window_aspect, 1, 1500);
}

void DrawAxis() {
  const Vec3f c = {0, 0, 0};
  const float L = 1;
  const Vec3f X = {L, 0, 0}, Y = {0, L, 0}, Z = {0, 0, L};

  glBegin(GL_LINES);
  glColor3f(1, 0, 0);
  glVertex3fv(c.x);
  glVertex3fv((c+X).x);
  glColor3f(0, 1, 0);
  glVertex3fv(c.x);
  glVertex3fv((c+Y).x);
  glColor3f(0, 0, 1);
  glVertex3fv(c.x);
  glVertex3fv((c+Z).x);
  glEnd();
}

// Gets called every click of the mouse
void MouseButton(int button, int state, int x, int y) {
  // TODO implement arc ball and zoom
  // Right-click: button = 0
  // Left-click: button = 2
  // MouseDown: state = 0
  // MouseUp: state = 1
  cout << "Window width: " << window_height << endl;
  cout << "X: " << x << endl;
  cout << "Y: " << y << endl;

  x_norm = ((2.0 * x / (window_width - 1)) - 1);
  y_norm = -((2.0 * y / (window_height - 1)) - 1);
  cout << "X_norm: " << x_norm << endl;
  cout << "Y_norm: " << y_norm << endl;
  // cout << "Button: " << button << endl;

// ROTATION HANDLING
  if (button == 0) {
    rotating = (state == 0) ? true : false;
  }
// ZOOM HANDLING
  if (button == 2) {
    // cout << "ZOOM" << endl;
    zooming = (state == 0) ? true : false;
  }
  glutPostRedisplay();
}

void MouseMotion(int x, int y) {
  // TODO implement arc ball and zoom
  cout << "MouseMotion | Y: " << y << " | old_Y: " << old_y << endl;

  // HANDLES ROTATION
  // Here we will have to develop a rotation matrix
  // given old value of x and y normalized, these are saved as globals
  if (rotating) {
    // normalize the new values
    float end_x_norm =  ((2.0 * x / (window_width - 1)) - 1);
    float end_y_norm = -((2.0 * y / (window_height - 1)) - 1);
    // get the length of the vector
    // NOT SURE THIS IS NECESSARY
    // float length = (end_x_norm * end_x_norm) + (end_y_norm * end_y_norm);
    // cout << "Length : " << length << endl;
    // float norm = radius / sqrt(length);
    // normanlize vector, point on sphere
    // float sphere_norm_x = end_x_norm * norm;
    // float sphere_norm_y = end_y_norm * norm;
    // cout << "sphere_norm_x: " << sphere_norm_x << endl;
    // cout << "sphere_norm_y: " << sphere_norm_y << endl;
  }

  // HANDLES ZOOM
  Vec3f difference = center - eye;
  if (zooming && old_y < y) {
    eye[0] = eye[0] + difference[0]*0.02;
    eye[1] = eye[1] + difference[1]*0.02;
    eye[2] = eye[2] + difference[2]*0.02;
  }
  if (zooming && old_y > y) {
    eye[0] = eye[0] - difference[0]*0.02;
    eye[1] = eye[1] - difference[1]*0.02;
    eye[2] = eye[2] - difference[2]*0.02;
  }
  old_y = y;
  glutPostRedisplay();
}

void Keyboard(unsigned char key, int x, int y) {
  switch (key) {
    case 'q':
    case 27:  // esc
      exit(0);
      break;
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    cout << endl;
    cout << "Usage: ./viewer (filename.obj | -s) [-l]" << endl;
    cout << endl;
    cout << "To load data/test.obj: ./viewer data/test.obj" << endl;
    cout << "To load a custom scene: ./viewer -s" << endl;
    cout << endl;
    return 0;
  }

  // Initialize GLUT
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
  glutInitWindowSize(window_width, window_height);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("Object viewer");
  glutMouseFunc(MouseButton);
  glutMotionFunc(MouseMotion);
  glutKeyboardFunc(Keyboard);
  glutDisplayFunc(Display);

  Init();

  if (string(argv[1]) == "-s") {
    cout << "Create scene" << endl;
  } else {
    string filename(argv[1]);
    cout << filename << endl;

    // Detect whether to fix the light source(s) to the camera or the world
    scene_lighting = false;
    if (argc > 2 && string(argv[2]) == "-l") {
      scene_lighting = true;
    }

    // Parse the obj file, compute the normals, read the textures

    ParseObj(filename, mesh);
    mesh.compute_normals();

    texture_ids = new GLuint[mesh.num_materials()];
    glGenTextures(mesh.num_materials(), texture_ids);

    for (int i = 0; i < mesh.num_materials(); ++i) {
      Material& material = mesh.material(i);
      material.LoadTexture(texture_ids[i]);
    }
  }

  glutMainLoop();

  return 0;
}
