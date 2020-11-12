/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <GLES2/gl2.h>

#ifdef __GNUC__
# if __GNUC__ >= 4
#  ifndef API
#   define API __attribute__ ((visibility("default")))
#  endif
# endif
#endif

#include <dali-toolkit/dali-toolkit.h>
#include <dali/devel-api/adaptor-framework/gl-window.h>
#include <dali/devel-api/adaptor-framework/window-devel.h>
#include <dali/public-api/adaptor-framework/window-enum.h>
#include <dali/public-api/signals/callback.h>
#include <dali/integration-api/debug.h>


typedef struct {
    float x, y;
} FloatPoint;

/* Application data */
typedef struct GLDATA {
    float model[16];
    float view[16];
    float mvp[16];

    FloatPoint anglePoint;
    FloatPoint curPoint;
    FloatPoint prevPoint;

    /*A program object is an object to which shader objects can be attached*/
    unsigned int program;

    /* Generate Vertex Buffer */
    unsigned int vbo;

    int width;
    int height;

    bool mouse_down;

    int windowAngle;

    bool initialized;
} GLData;

using namespace Dali;
using Dali::Toolkit::TextLabel;

#define SCREEN_WIDTH            1920
#define SCREEN_HEIGHT           1080

const Vector4 WINDOW_COLOR(0.5f, 0.0f, 0.0f, 0.5f );

#define LOG_TAG "DALI_NATIVEGL_LIBRARY"





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
  unsigned int vtx_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vtx_shader, 1, &p, NULL);
  glCompileShader(vtx_shader);

  p = fragment_shader;
  unsigned int fgmt_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fgmt_shader, 1, &p, NULL);
  glCompileShader(fgmt_shader);

  glData->program = glCreateProgram();
  glAttachShader(glData->program, vtx_shader);
  glAttachShader(glData->program, fgmt_shader);
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
void initialize_gl()
{
     fprintf(stderr,"%s\n",__FUNCTION__);
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
int renderFrame_gl()
{
  if( mGLData.initialized == false )
  {
    //initialize_gl();
    mGLData.initialized = true;
  }

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
void terminate_gl()
{
 fprintf(stderr,"%s\n",__FUNCTION__);
}

void update_touch_event_state( bool down )
{
     fprintf(stderr,"%s\n",__FUNCTION__);
  mGLData.mouse_down = down;
}

void update_touch_position(int x, int y)
{
     fprintf(stderr,"%s\n",__FUNCTION__);
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

void update_window_size(int w, int h)
{
  mGLData.width = w;
  mGLData.height = h;
}

void update_window_rotation_angle(int angle)
{
  mGLData.windowAngle = angle;
}


// This example shows how to create and display Hello World! using a simple TextActor
//
class HelloWorldController : public ConnectionTracker
{
public:

  HelloWorldController( Application& application )
  : mApplication( application )
  {
    // Connect to the Application's Init signal
    mApplication.InitSignal().Connect( this, &HelloWorldController::Create );
  }

  ~HelloWorldController()
  {
    // Nothing to do here;
  }

  // The Init signal is received once (only) during the Application lifetime
  void Create( Application& application )
  {
    #if 0
    // UI control window
    Window window = application.GetWindow();
    mUIWindow = window;
    window.SetBackgroundColor( WINDOW_COLOR );
    window.SetTransparency( true );

    window.SetInputRegion( Rect< int >( 0, SCREEN_HEIGHT - 200, SCREEN_WIDTH, 200 ));
    mTextLabel = TextLabel::New( "Test NativeGL" );

    mTextLabel.SetProperty( Actor::Property::PARENT_ORIGIN, ParentOrigin::BOTTOM_LEFT );
    mTextLabel.SetProperty( Actor::Property::ANCHOR_POINT, AnchorPoint::BOTTOM_LEFT );
    mTextLabel.SetProperty( Actor::Property::SIZE, Vector2(SCREEN_WIDTH, 200) );
    mTextLabel.SetBackgroundColor( Vector4(0.6f,0.2f,0.2f,1.0f) );
    window.Add( mTextLabel );

    // Respond to a click anywhere on the stage
    window.GetRootLayer().TouchSignal().Connect( this, &HelloWorldController::OnTouch );

    // Respond to key events
    window.KeyEventSignal().Connect( this, &HelloWorldController::OnKeyEvent );

    Dali::Vector<Dali::Window::WindowOrientation> orientations;
    orientations.PushBack( Dali::Window::WindowOrientation::PORTRAIT );
    orientations.PushBack( Dali::Window::WindowOrientation::LANDSCAPE  );
    orientations.PushBack( Dali::Window::WindowOrientation::PORTRAIT_INVERSE  );
    orientations.PushBack( Dali::Window::WindowOrientation::LANDSCAPE_INVERSE  );
    DevelWindow::SetAvailableOrientations( window, orientations );

    window.ResizeSignal().Connect( this, &HelloWorldController::OnWindowResized );
    window.Show();
#endif
////////////////////////////////////////////////////////////////////////////////////////////
    fprintf(stderr,"%s\n",__FUNCTION__);
    mGLData.initialized = true;
    mGLWindow = Dali::GlWindow::New( PositionSize(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), "GLWindow", "", false);
    mGLWindow.SetEglConfig( true, true, 0, Dali::GlWindow::GlesVersion::VERSION_3_0 );
    mGLWindow.RegisterGlCallback( Dali::MakeCallback( initialize_gl ), Dali::MakeCallback( renderFrame_gl ), Dali::MakeCallback( terminate_gl ) );

    mGLData.width = SCREEN_WIDTH;
    mGLData.height = SCREEN_HEIGHT;

    currentWindowOrientation = Dali::WindowOrientation::NO_ORIENTATION_PREFERENCE;

    mGLWindow.Show();

    mGLWindow.ResizeSignal().Connect( this,  &HelloWorldController::OnGLWindowResized );

    Dali::Vector<Dali::WindowOrientation> glWindowOrientations;
    glWindowOrientations.PushBack( Dali::WindowOrientation::PORTRAIT );
    glWindowOrientations.PushBack( Dali::WindowOrientation::LANDSCAPE );
    glWindowOrientations.PushBack( Dali::WindowOrientation::PORTRAIT_INVERSE );
    glWindowOrientations.PushBack( Dali::WindowOrientation::LANDSCAPE_INVERSE );
    mGLWindow.SetAvailableOrientations( glWindowOrientations );
    //mGLWindow.TouchedSignal().Connect( this, &HelloWorldController::OnGLWindowTouch );
    //mGLWindow.KeyEventSignal().Connect( this, &HelloWorldController::OnGLWindowKeyEvent );
  }

  void OnWindowResized( Dali::Window winHandle, Dali::Window::WindowSize size )
  {
    fprintf(stderr,"%s\n",__FUNCTION__);
    int width = size.GetWidth();
    int height = size.GetHeight();
    mTextLabel.SetProperty( Actor::Property::SIZE, Vector2(width, 200) );
    DALI_LOG_ERROR("OnWindowResized, current orientation:%d, width:%d, height:%d\n", DevelWindow::GetCurrentOrientation(winHandle), width, height );
  }

  bool OnTouch( Actor actor, const TouchEvent& touch )
  {
       fprintf(stderr,"%s\n",__FUNCTION__);
    // quit the application
    mUIWindow.Lower();
    return true;
  }

  void OnKeyEvent( const KeyEvent& event )
  {
       fprintf(stderr,"%s\n",__FUNCTION__);
    if( event.GetState() == KeyEvent::DOWN )
    {
      if ( IsKey( event, Dali::DALI_KEY_ESCAPE ) || IsKey( event, Dali::DALI_KEY_BACK ) )
      {
        //mUIWindow.Raise();
        mApplication.Quit();
      }
    }
  }

  void OnGLWindowResized( Dali::GlWindow::WindowSize size )
  {
    int windowAngle = 0;
    currentWindowOrientation = mGLWindow.GetCurrentOrientation();
    fprintf(stderr, "GetCurrentOrientation(): %d\n", static_cast< int >( currentWindowOrientation ) );
    {
        if ( currentWindowOrientation == Dali::WindowOrientation::LANDSCAPE )
           windowAngle = 0;
        else if ( currentWindowOrientation == Dali::WindowOrientation::PORTRAIT )
           windowAngle = 90;
        else if ( currentWindowOrientation == Dali::WindowOrientation::LANDSCAPE_INVERSE )
           windowAngle = 180;
        else if ( currentWindowOrientation == Dali::WindowOrientation::PORTRAIT_INVERSE )
           windowAngle = 270;
    }

    mGLData.windowAngle = windowAngle;

    mGLData.width = size.GetWidth();
    mGLData.height = size.GetHeight();
    fprintf(stderr, "current rotation angle: %d, width: %d, height: %d\n", mGLData.windowAngle, mGLData.width, mGLData.height );
  }

  void OnGLWindowTouch( const TouchEvent& touch )
  {
       fprintf(stderr,"%s\n",__FUNCTION__);
     DALI_LOG_ERROR("touch.GetState(), %d\n",touch.GetState( 0 ));
    if( touch.GetState( 0 ) == 0 )
    {
      mGLData.mouse_down = true;
      DALI_LOG_ERROR("DOWN, x:%f, y:%f\n",touch.GetScreenPosition( 0 ).x, touch.GetScreenPosition( 0 ).y);
    }
    else if( touch.GetState( 0 ) == 1 )
    {
      mGLData.mouse_down = false;
      DALI_LOG_ERROR("UP, x:%f, y:%f\n",touch.GetScreenPosition( 0 ).x, touch.GetScreenPosition( 0 ).y);
    }
    else if( touch.GetState( 0 ) == 2 )
    {
      float dx = 0;
      float dy = 0;
      mGLData.curPoint.x = touch.GetScreenPosition( 0 ).x;
      mGLData.curPoint.y = touch.GetScreenPosition( 0 ).y;
      DALI_LOG_ERROR("MOTION x:%f, y:%f\n",mGLData.curPoint.x, mGLData.curPoint.y);
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
    return;
  }

  void OnGLWindowKeyEvent( const KeyEvent& event )
  {
       fprintf(stderr,"%s\n",__FUNCTION__);
    if(event.GetState() == KeyEvent::DOWN)
    {
      if ( IsKey( event, Dali::DALI_KEY_ESCAPE ) || IsKey( event, Dali::DALI_KEY_BACK ) )
      {
        mApplication.Quit();
        //mUIWindow.Raise();
      }
    }
  }

private:
  Application&    mApplication;
  Dali::Window    mUIWindow;
  Dali::GlWindow  mGLWindow;
  TextLabel       mTextLabel;

  Dali::WindowOrientation currentWindowOrientation;
};

int DALI_EXPORT_API main( int argc, char **argv )
{
  Application application = Application::New( &argc, &argv );
  HelloWorldController test( application );
  application.MainLoop();
  return 0;
}
