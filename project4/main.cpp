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

// Our variables
int old_y;
float x_norm, y_norm;
bool rotating;
bool zooming = false;
bool normals = false;
bool disco = false;
bool axis = false;
bool scene = false;
float zoomFactor = 1.0f;
Vec3f eye = {0, 0, 6};
BoundingBox bbox = {{-1, -1, -1}, {1, 1, 1}};

// For arcBall implementation
Vec3f startArc;
Vec3f currentArc;
Vec3f arcNormal;
float arcTheta;
GLfloat arcMatrix[16];
GLfloat savedArcMatrix[16];

// For object / light translations
GLfloat objectTranslate[16];
GLfloat objectRotate[16];
GLfloat yOffset[16];
GLfloat translateStep;

// For lighting
GLfloat lightPos[4];
GLfloat lightColor[4];
GLfloat lightAtten;

void Display() {
  if (scene) {
    DisplayScene();
  } else {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(40.0, window_aspect, 1, 1500);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    Vec3f zoomedEye = {eye[0] * zoomFactor,
                       eye[1] * zoomFactor,
                       eye[2] * zoomFactor};
    gluLookAt(zoomedEye[0], zoomedEye[1], zoomedEye[2],
              0, 0, 0,
              0, 1, 0);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_RESCALE_NORMAL);

    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightColor);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.0);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, lightAtten);

    // Rotate our view to our current arcball state
    glPushMatrix();
    MultMatrix(objectTranslate);
    MultMatrix(objectRotate);
    MultMatrix(arcMatrix);
    MultMatrix(yOffset);

    if (scene_lighting) {
      glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    }

    // Draw our mesh
    mesh.draw_mesh(texture_ids, normals);

    glPopMatrix();
    MultMatrix(arcMatrix);

    if (axis) {
      glDisable(GL_LIGHTING);
      glLineWidth(4);
      DrawAxis();
      glEnable(GL_LIGHTING);
    }

    glFlush();
    glutSwapBuffers();

    if (disco) {
      usleep(90000);
      discoLight();
      glutPostRedisplay();
    }
  }
}

void DisplayScene() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(40.0, window_aspect, 1, 1500);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  Vec3f zoomedEye = {eye[0] * zoomFactor,
                     eye[1] * zoomFactor,
                     eye[2] * zoomFactor};
  gluLookAt(zoomedEye[0], zoomedEye[1], zoomedEye[2],
            0, 0, 0,
            0, 1, 0);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_RESCALE_NORMAL);

  glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
  glLightfv(GL_LIGHT0, GL_SPECULAR, lightColor);
  glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
  glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.0);
  glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, lightAtten);

  for (int i = 0; i < 7; i++) {
    // Draw a bunch of logos
    glPushMatrix();
    objectTranslate[12] = (rand() % 80 - 50) * 1.4;
    objectTranslate[13] = (rand() % 80 - 50) * 1.4;
    objectTranslate[14] = rand() % 100 - 150;
    MultMatrix(objectTranslate);
    glRotatef(rand() % 360, (rand() % 100) / 100.0, (rand() % 100) / 100.0,
              (rand() % 100) / 100.0);
    MultMatrix(arcMatrix);
    MultMatrix(yOffset);
    if (scene_lighting) {
      glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    }

    // Draw our logo
    mesh.draw_mesh(texture_ids, normals);

    glPopMatrix();
  }

  MultMatrix(arcMatrix);

  if (disco) {
    usleep(90000);
    discoLight();
    glutPostRedisplay();
  }

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

  // Set up our matrices
  glLoadIdentity();
  glGetFloatv(GL_MODELVIEW_MATRIX, arcMatrix);
  glGetFloatv(GL_MODELVIEW_MATRIX, savedArcMatrix);
  glGetFloatv(GL_MODELVIEW_MATRIX, objectTranslate);
  glGetFloatv(GL_MODELVIEW_MATRIX, objectRotate);
  glGetFloatv(GL_MODELVIEW_MATRIX, yOffset);

  // Change light properties
  lightColor[0] = 0.75;
  lightColor[1] = 0.75;
  lightColor[2] = 0.75;
  lightColor[3] = 1.0;
}

void postBoundingBox() {
  // Need bounding box info for light position / translations
  lightPos[0] = 0.0;
  lightPos[1] = 0.0;
  lightPos[2] = ((mesh.bb().xdim() + mesh.bb().ydim() + mesh.bb().zdim()) / 6.0)
                * 7.5;
  lightPos[3] = 1.0;
  lightAtten = 0.1 / (mesh.bb().zdim() / 2.0);
  translateStep = (mesh.bb().xdim() + mesh.bb().ydim() + mesh.bb().zdim())
                  / 300.0;

  // Center the object
  yOffset[13] = -0.5 * mesh.bb().ydim();

  // Set up our zoom if not already
  eye = eye * max(mesh.bb().xdim()/4, max(mesh.bb().ydim()/4,
                  mesh.bb().zdim()/4));
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

void discoLight() {
  cout << "DISCO TIME!!!" << endl;
  const int lastColor = 0;
  int color;
  do {
    color = rand() % 8;
  } while (color == lastColor || color == 0);
  lightColor[0] = color & 1;
  lightColor[1] = (color & 2) > 1;
  lightColor[2] = (color & 4) > 2;

  glClearColor(!(color & 1), !((color & 2) > 1), !((color & 4) > 2), 1.0f);
}

Vec3f computeArcBall(int x, int y) {
  Vec3f arc;
  arc[0] = (x * 2.0) / (window_width * 1.0) - 1.0;
  arc[1] = -1.0 * ((y * 2.0) / (window_height * 1.0) - 1.0);
  // Remember a sphere is defined by R = sqrt(x^2 + y^2 + z^2)
  if ((arc[0] * arc[0] + arc[1] * arc[1]) < 1) {
    arc[2] = 1 - (arc[0] * arc[0] + arc[1] * arc[1]);
  } else {
    // Avoid divide by zero errors
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
    for (int i = 0; i < 16; i++) {
      savedArcMatrix[i] = arcMatrix[i];
    }
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
    glLoadIdentity();
    glRotatef(arcTheta, arcNormal[0], arcNormal[1], arcNormal[2]);
    MultMatrix(savedArcMatrix);
    glGetFloatv(GL_MODELVIEW_MATRIX, arcMatrix);

    glutPostRedisplay();
  } else {
    // HANDLES ZOOM
    if (zooming && old_y < y) {
      zoomFactor *= 0.98;
    }
    if (zooming && old_y > y) {
      zoomFactor *= 1.02;
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
    case 'y':
      objectTranslate[12] -= translateStep;
      break;
    case 'u':
      objectTranslate[12] += translateStep;
      break;
    case 'h':
      objectTranslate[13] += translateStep;
      break;
    case 'j':
      objectTranslate[13] -= translateStep;
      break;
    case 'n':
      objectTranslate[14] -= translateStep;
      break;
    case 'm':
      objectTranslate[14] += translateStep;
      break;
    case 'r':
      glLoadIdentity();
      glRotatef(2, 1.0, 0.0, 0.0);
      MultMatrix(objectRotate);
      glGetFloatv(GL_MODELVIEW_MATRIX, objectRotate);
      break;
    case 't':
      glLoadIdentity();
      glRotatef(-2, 1.0, 0.0, 0.0);
      MultMatrix(objectRotate);
      glGetFloatv(GL_MODELVIEW_MATRIX, objectRotate);
      break;
    case 'f':
      glLoadIdentity();
      glRotatef(2, 0.0, 1.0, 0.0);
      MultMatrix(objectRotate);
      glGetFloatv(GL_MODELVIEW_MATRIX, objectRotate);
      break;
    case 'g':
      glLoadIdentity();
      glRotatef(-2, 0.0, 1.0, 0.0);
      MultMatrix(objectRotate);
      glGetFloatv(GL_MODELVIEW_MATRIX, objectRotate);
      break;
    case 'v':
      glLoadIdentity();
      glRotatef(2, 0.0, 0.0, 1.0);
      MultMatrix(objectRotate);
      glGetFloatv(GL_MODELVIEW_MATRIX, objectRotate);
      break;
    case 'b':
      glLoadIdentity();
      glRotatef(-2, 0.0, 0.0, 1.0);
      MultMatrix(objectRotate);
      glGetFloatv(GL_MODELVIEW_MATRIX, objectRotate);
      break;
    case 's':
      normals = !normals;
      break;
    case 'a':
      axis = !axis;
      break;
    case 'd':
      disco = !disco;
      if (!disco) {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        lightColor[0] = 0.75;
        lightColor[1] = 0.75;
        lightColor[2] = 0.75;
      }
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
    scene = true;
    // Parse the obj file, compute the normals, read the textures
    ParseObj("./data/original.obj", mesh);
    mesh.compute_normals();

    texture_ids = new GLuint[mesh.num_materials()];
    glGenTextures(mesh.num_materials(), texture_ids);

    for (int i = 0; i < mesh.num_materials(); ++i) {
      Material& material = mesh.material(i);
      material.LoadTexture(texture_ids[i]);
    }
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

  // Update our vars that needed the bounding box
  postBoundingBox();

  glutMainLoop();

  return 0;
}
