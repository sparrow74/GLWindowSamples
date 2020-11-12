/*
 * Copyright (c) 2011-2017 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "DALI_NATIVEGL_LIBRARY"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <GLES2/gl2.h>


#include <dlog.h>
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#include <dali-nativegl-library.h>

#ifndef EXPORT_API
#define EXPORT_API __attribute__ ((visibility("default")))
#endif

/* Define the cube's vertices
   Each vertex consist of x, y, z, r, g, b */
static const float cube_vertices[] = {
    /* front surface is blue */
    0.5, 0.5, 0.5, 0.0, 0.0, 1.0,
    -0.5, -0.5, 0.5, 0.0, 0.0, 1.0,
    0.5, -0.5, 0.5, 0.0, 0.0, 1.0,
    0.5, 0.5, 0.5, 0.0, 0.0, 1.0,
    -0.5, 0.5, 0.5, 0.0, 0.0, 1.0,
    -0.5, -0.5, 0.5, 0.0, 0.0, 1.0,
    /* left surface is green */
    -0.5, 0.5, 0.5, 0.0, 1.0, 0.0,
    -0.5, -0.5, -0.5, 0.0, 1.0, 0.0,
    -0.5, -0.5, 0.5, 0.0, 1.0, 0.0,
    -0.5, 0.5, 0.5, 0.0, 1.0, 0.0,
    -0.5, 0.5, -0.5, 0.0, 1.0, 0.0,
    -0.5, -0.5, -0.5, 0.0, 1.0, 0.0,
    /* top surface is red */
    -0.5, 0.5, 0.5, 1.0, 0.0, 0.0,
    0.5, 0.5, -0.5, 1.0, 0.0, 0.0,
    -0.5, 0.5, -0.5, 1.0, 0.0, 0.0,
    -0.5, 0.5, 0.5, 1.0, 0.0, 0.0,
    0.5, 0.5, 0.5, 1.0, 0.0, 0.0,
    0.5, 0.5, -0.5, 1.0, 0.0, 0.0,
    /* right surface is yellow */
    0.5, 0.5, -0.5, 1.0, 1.0, 0.0,
    0.5, -0.5, 0.5, 1.0, 1.0, 0.0,
    0.5, -0.5, -0.5, 1.0, 1.0, 0.0,
    0.5, 0.5, -0.5, 1.0, 1.0, 0.0,
    0.5, 0.5, 0.5, 1.0, 1.0, 0.0,
    0.5, -0.5, 0.5, 1.0, 1.0, 0.0,
    /* back surface is cyan */
    -0.5, 0.5, -0.5, 0.0, 1.0, 1.0,
    0.5, -0.5, -0.5, 0.0, 1.0, 1.0,
    -0.5, -0.5, -0.5, 0.0, 1.0, 1.0,
    -0.5, 0.5, -0.5, 0.0, 1.0, 1.0,
    0.5, 0.5, -0.5, 0.0, 1.0, 1.0,
    0.5, -0.5, -0.5, 0.0, 1.0, 1.0,
    /* bottom surface is magenta */
    -0.5, -0.5, -0.5, 1.0, 0.0, 1.0,
    0.5, -0.5, 0.5, 1.0, 0.0, 1.0,
    -0.5, -0.5, 0.5, 1.0, 0.0, 1.0,
    -0.5, -0.5, -0.5, 1.0, 0.0, 1.0,
    0.5, -0.5, -0.5, 1.0, 0.0, 1.0,
    0.5, -0.5, 0.5, 1.0, 0.0, 1.0
};

/* Vertex Shader Source */
static const char vertex_shader[] =
    "attribute vec4 vPosition;\n"
    "attribute vec3 inColor;\n"
    "uniform mat4 mvpMatrix;"
    "varying vec3 outColor;\n"
    "void main()\n"
    "{\n"
    "   outColor = inColor;\n"
    "   gl_Position = mvpMatrix * vPosition;\n"
    "}\n";

/* Fragment Shader Source */
static const char fragment_shader[] =
    "#ifdef GL_ES\n"
    "precision mediump float;\n"
    "#endif\n"
    "varying vec3 outColor;\n"
    "void main()\n"
    "{\n"
    "   gl_FragColor = vec4 ( outColor, 1.0 );\n"
    "}\n";

static GLData mGLData;

static void generateAndBindBuffer(unsigned int *vbo);
static void init_matrix(float matrix[16]);
static void init_shaders(GLData* glData);
static void multiply_matrix(float matrix[16], const float matrix0[16], const float matrix1[16]);
static void rotate_xyz(float matrix[16], const float anglex, const float angley, const float anglez);
static int view_set_ortho(float result[16], const float left, const float right, const float bottom, const float top, const float near, const float far);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Internal functions
/*
 * brief Generate and bind vertex buffer.
 */
static void generateAndBindBuffer(unsigned int *vbo)
{
  /* Generate buffer object names */
  glGenBuffers(1, vbo);

  /* Bind a named buffer object */
  glBindBuffer(GL_ARRAY_BUFFER, *vbo);

  /* Creates and initializes a buffer object's data store */
  glBufferData(GL_ARRAY_BUFFER, 3 * 72 * 4, cube_vertices, GL_STATIC_DRAW);
}

/*
 * @ brief Initialize matrix
 * @ param[in]
 *     1 0 0 0
 *     0 1 0 0
 *     0 0 1 0
 *     0 0 0 1
 */
static  void init_matrix(float matrix[16])
{
  matrix[0] = 1.0f;
  matrix[1] = 0.0f;
  matrix[2] = 0.0f;
  matrix[3] = 0.0f;
  matrix[4] = 0.0f;
  matrix[5] = 1.0f;
  matrix[6] = 0.0f;
  matrix[7] = 0.0f;
  matrix[8] = 0.0f;
  matrix[9] = 0.0f;
  matrix[10] = 1.0f;
  matrix[11] = 0.0f;
  matrix[12] = 0.0f;
  matrix[13] = 0.0f;
  matrix[14] = 0.0f;
  matrix[15] = 1.0f;
}

/**
 * @ brief Initialize vertex shader and fragment shader.
 */
static  void init_shaders(GLData* glData)
{
  const char *p = vertex_shader;
  glData->vtx_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(glData->vtx_shader, 1, &p, NULL);
  glCompileShader(glData->vtx_shader);

  p = fragment_shader;
  glData->fgmt_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(glData->fgmt_shader, 1, &p, NULL);
  glCompileShader(glData->fgmt_shader);

  glData->program = glCreateProgram();
  glAttachShader(glData->program, glData->vtx_shader);
  glAttachShader(glData->program, glData->fgmt_shader);
  glBindAttribLocation(glData->program, 0, "vPosition");
  glBindAttribLocation(glData->program, 1, "inColor");

  glLinkProgram(glData->program);
  glUseProgram(glData->program);
}

/*
 * @ brief Multiply 4x4 matrix
 * @ param[in] matrix
 * @ param[in] matrix0
 * @ param[in] matrix1
 * @ matrix = matrix0 x matrix1
 */
static void multiply_matrix(float matrix[16], const float matrix0[16], const float matrix1[16])
{
  int i;
  int row;
  int column;
  float temp[16];

  for (column = 0; column < 4; column++)
  {
    for (row = 0; row < 4; row++)
    {
      temp[column * 4 + row] = 0.0f;
      for (i = 0; i < 4; i++)
      {
        temp[column * 4 + row] += matrix0[i * 4 + row] * matrix1[column * 4 + i];
      }
    }
  }

  for (i = 0; i < 16; i++)
  {
    matrix[i] = temp[i];
  }
}

/*
 * @ brief Rotate a matrix
 * @ param[in] matrix The matrix rotated angle.
 * @ param[in] anglex Rotate x-angle.
 * @ param[in] angley Rotate y-angle.
 * @ param[in] anglez Rotate z-angle.
 */
static void rotate_xyz(float matrix[16], const float anglex, const float angley, const float anglez)
{
  const float pi = 3.141592f;
  float temp[16];
  float rz = 2.0f * pi * anglez / 360.0f;
  float rx = 2.0f * pi * anglex / 360.0f;
  float ry = 2.0f * pi * angley / 360.0f;

  float sy = sinf(ry);
  float cy = cosf(ry);
  float sx = sinf(rx);
  float cx = cosf(rx);
  float sz = sinf(rz);
  float cz = cosf(rz);
  init_matrix(temp);

  temp[0] = cy * cz - sx * sy * sz;
  temp[1] = cz * sx * sy + cy * sz;
  temp[2] = -cx * sy;

  temp[4] = -cx * sz;
  temp[5] = cx * cz;
  temp[6] = sx;

  temp[8] = cz * sy + cy * sx * sz;
  temp[9] = -cy * cz * sx + sy * sz;
  temp[10] = cx * cy;

  multiply_matrix(matrix, matrix, temp);
}

/*
 * @ brief Creates a matrix for an orthographic parallel viewing volume.
 * @ param[in] result
 * @ param[in] left, right Specify the coordinates for the left and right vertical clipping planes.
 * @ param[in] bottom, top Specify the coordinates for the bottom and top horizontal clipping planes.
 * @ param[in] near, far   Specify the distances to the nearer and farther depth clipping planes.
 *			   These values are negative if the plane is the plane is to be behind the viewer.
 */
static int view_set_ortho(float result[16], const float left, const float right,
               const float bottom, const float top, const float near, const float far)
{
  if ((right - left) == 0.0f || (top - bottom) == 0.0f || (far - near) == 0.0f)
  {
    return 0;
  }

  result[0] = 2.0f / (right - left);
  result[1] = 0.0f;
  result[2] = 0.0f;
  result[3] = 0.0f;
  result[4] = 0.0f;
  result[5] = 2.0f / (top - bottom);
  result[6] = 0.0f;
  result[7] = 0.0f;
  result[8] = 0.0f;
  result[9] = 0.0f;
  result[10] = -2.0f / (far - near);
  result[11] = 0.0f;
  result[12] = -(right + left) / (right - left);
  result[13] = -(top + bottom) / (top - bottom);
  result[14] = -(far + near) / (far - near);
  result[15] = 1.0f;

  return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// pullic Callbacks
// intialize callback that gets called once for intialization
EXPORT_API void intializeGL()
{
  mGLData.anglePoint.x = 45.f;
  mGLData.anglePoint.y = 45.f;
  /* Initialize shaders */
  init_shaders(&mGLData);
  /* Initlalize Camera View */
  init_matrix(mGLData.view);
  /* Generate and bind Vertex buffer object */
  generateAndBindBuffer(&(mGLData.vbo));

  /* Calculate view aspect */
  float aspect = (mGLData.width> mGLData.height ? (float)mGLData.width/mGLData.height : (float)mGLData.height/mGLData.width);
  if (mGLData.width > mGLData.height)
  {
    view_set_ortho(mGLData.view, -1.0*aspect, 1.0*aspect, -1.0, 1.0, -1.0, 100.0);
  }
  else
  {
    view_set_ortho(mGLData.view, -1.0, 1.0, -1.0*aspect, 1.0*aspect, -1.0, 100.0);
  }

  glEnable(GL_DEPTH_TEST);
}

// draw callback is where all the main GL rendering happens
EXPORT_API int renderFrameGL()
{
  int w, h;
  w = mGLData.width;
  h = mGLData.height;

  if( mGLData.windowAngle == 90 || mGLData.windowAngle == 270)
  {
    glViewport(0, 0, h, w);
  }
  else
  {
    glViewport(0, 0, w, h);
  }
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  init_matrix(mGLData.model);
  rotate_xyz(mGLData.model, mGLData.anglePoint.x, mGLData.anglePoint.y, mGLData.windowAngle);

  multiply_matrix(mGLData.mvp, mGLData.view, mGLData.model);
  glUseProgram(mGLData.program);

  glBindBuffer(GL_ARRAY_BUFFER, mGLData.vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, 0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, mGLData.vbo);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(sizeof(float) * 3));
  glEnableVertexAttribArray(1);

  glUniformMatrix4fv(glGetUniformLocation(mGLData.program, "mvpMatrix"), 1, GL_FALSE, mGLData.mvp);

  /* Render primitives from array data*/
  glDrawArrays(GL_TRIANGLES, 0, 36);

  return 1;
}


// delete callback gets called when glview is deleted
EXPORT_API void terminateGL()
{
  glDeleteShader(mGLData.vtx_shader);
  glDeleteShader(mGLData.fgmt_shader);
  glDeleteProgram(mGLData.program);
  glDeleteBuffers(1, &mGLData.vbo);
}

EXPORT_API void updateTouchEventState( bool down )
{
  mGLData.mouse_down = down;
}

EXPORT_API void updateTouchPosition(int x, int y)
{
  float dx = 0;
  float dy = 0;
  mGLData.curPoint.x = (float)x;
  mGLData.curPoint.y = (float)y;

  if( mGLData.mouse_down == true )
  {
    dx = mGLData.curPoint.x - mGLData.prevPoint.x;
    dy = mGLData.curPoint.y - mGLData.prevPoint.y;
    mGLData.anglePoint.x += dy;
    mGLData.anglePoint.y += dx;
  }
  mGLData.prevPoint.x = mGLData.curPoint.x;
  mGLData.prevPoint.y = mGLData.curPoint.y;
}

EXPORT_API void rotationCube(int x, int y)
{
  float dx = x;
  float dy = y;
  mGLData.anglePoint.x += dy;
  mGLData.anglePoint.y += dx;
}

EXPORT_API void updateWindowSize(int w, int h)
{
  mGLData.width = w;
  mGLData.height = h;
}

EXPORT_API void updateWindowRotationAngle(int angle)
{
  mGLData.windowAngle = angle;
}
