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
float zoomFactor = 1.0f;
Vec3f startArc;
Vec3f currentArc;
Vec3f arcNormal;
float arcTheta;
GLfloat arcMatrix[16];
GLfloat savedArcMatrix[16];
int radius = 1;
// (2,2,5) are the values that were originally in the gluLookAt()
Vec3f eye = {2, 2, 5};
BoundingBox bbox = {{-1, -1, -1}, {1, 1, 1}};
bool start = true;
Vec3f center = (bbox.max+bbox.min)/2.0f;
Vec3f translation = {0.0, 0.0, 0.0};
Vec3f rotation = {0.0, 0.0, 0.0};

void Display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(40.0, window_aspect, 1, 1500);

  // glEnable(GL_BLEND);
  // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_POLYGON_SMOOTH);

  // TODO call gluLookAt such that mesh fits nicely in viewport.
  // mesh.bb() may be useful.
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  if (start) {
    float inis_zoom = max(mesh.bb().xdim()/4,
                      max(mesh.bb().ydim()/4, mesh.bb().zdim()/4));
    eye = eye * inis_zoom;
    start = false;
  }
  Vec3f zoomedEye =
    {eye[0] * zoomFactor,
    eye[1] * zoomFactor,
    eye[2] * zoomFactor};
  gluLookAt(zoomedEye[0], zoomedEye[1], zoomedEye[2],
            0, 0, 0,
            0, 1, 0);

  // Rotate our view to our current arcball state
  MultMatrix(arcMatrix);

  // TODO set up lighting, material properties and render mesh.
  // Be sure to call glEnable(GL_RESCALE_NORMAL) so your normals
  // remain normalized throughout transformations.

  // Draw our mesh
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  mesh.draw_mesh(texture_ids, translation, rotation);

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
  InvertMatrix(m);
  glLoadMatrixf(m);
}

void MultMatrix(GLfloat* m) {
  glMultMatrixf(m);
}

void InvertMatrix(GLfloat* m) {
  // transpose to column-major
  for (int i = 0; i < 4; ++i) {
    for (int j = i; j < 4; ++j) {
      swap(m[i*4+j], m[j*4+i]);
    }
  }
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

  glLoadIdentity();
  glGetFloatv(GL_MODELVIEW_MATRIX, arcMatrix);
  glGetFloatv(GL_MODELVIEW_MATRIX, savedArcMatrix);
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

Vec3f computeArcBall(int x, int y) {
  Vec3f arc;
  arc[0] = (x * 2.0) / (window_width * 1.0) - 1.0;
  arc[1] = -1.0 * ((y * 2.0) / (window_height * 1.0) - 1.0);
  // Remember a sphere is defined by R = sqrt(x^2 + y^2 + z^2)
  if ((arc[0] * arc[0] + arc[1] * arc[1]) < 1) {
    arc[2] = 1 - (arc[0] * arc[0] + arc[1] * arc[1]);
  } else {
    if (arc[0] == 0) {
      arc[0] = .00001;
    }
    if (arc[1] == 0) {
      arc[1] = .00001;
    }
    float theta = atan(abs(arc[1]) / abs(arc[0]));
    arc[0] = cos(theta) * arc[0] / abs(arc[0]);
    arc[1] = sin(theta) * arc[1] / abs(arc[1]);
    arc[2] = 0;
  }
  return arc;
}

// Gets called every click of the mouse
void MouseButton(int button, int state, int x, int y) {
  // ROTATION HANDLING
  if (button == 0 && state == 0) {
    rotating = true;
    startArc = computeArcBall(x, y);
  } else if (button == 0 && state == 1) {
    rotating = false;
    // Save our current arc ball matrix
    glLoadMatrixf(arcMatrix);
    glGetFloatv(GL_MODELVIEW_MATRIX, savedArcMatrix);
  }

  // ZOOM HANDLING
  if (button == 2) {
    zooming = (state == 0) ? true : false;
  }
  glutPostRedisplay();
}

void MouseMotion(int x, int y) {
  // HANDLES ROTATION
  if (rotating) {
    currentArc = computeArcBall(x, y);
    // We need the normal between Origin, start pt, and current pt
    // We don't need to subtract the origin to make two vectors as the origin
    // is just (0, 0, 0) so the 'vectors' are the same values as the 'points'
    arcNormal = currentArc ^ startArc;
    Vec3f a = arcNormal;

    // We also need the central angle between the origin and two vectors
    arcTheta = acos(startArc.unit() * currentArc.unit()) / M_PI * -180.0;

    // Update our matrix
    glLoadMatrixf(savedArcMatrix);
    glRotatef(arcTheta, arcNormal[0], arcNormal[1], arcNormal[2]);
    glGetFloatv(GL_MODELVIEW_MATRIX, arcMatrix);

    glutPostRedisplay();
  } else {
    // HANDLES ZOOM
    if (zooming && old_y < y) {
      zoomFactor *= 0.98;
      // cout << "zoom: " << zoomFactor << endl;
    }
    if (zooming && old_y > y) {
      zoomFactor *= 1.02;
      // cout << "zoom: " << zoomFactor << endl;
    }
    old_y = y;
    glutPostRedisplay();
  }
}

void Keyboard(unsigned char key, int x, int y) {
  switch (key) {
    case 'q':
    case 27:  // esc
      exit(0);
      break;
    case 'j':
      cout << "translate left" << endl;
      translation[0]++;
      break;
    case 'k':
      cout << "translate right" << endl;
      translation[0]--;
      break;
    case 'i':
      cout << "translate up" << endl;
      translation[1]++;
      break;
    case 'm':
      cout << "translate down" << endl;
      translation[1]--;
      break;
    case '5':
      cout << "rotate around " << endl;
      rotation[0] += 5;
      break;
    case '6':
      cout << "rotate around " << endl;
      rotation[0] += 5;
      break;
    case '7':
      cout << "rotate around " << endl;
      rotation[1] += 5;
      break;
    case '8':
      cout << "rotate around " << endl;
      rotation[1] += 5;
      break;
    case '9':
      cout << "rotate around " << endl;
      rotation[2] += 5;
      break;
    case '0':
      cout << "rotate around " << endl;
      rotation[2] += 5;
      break;
  }
  glutPostRedisplay();
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
