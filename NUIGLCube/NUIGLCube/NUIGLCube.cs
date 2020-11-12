using System;
using Tizen.System;
using Tizen.NUI;
using Tizen.NUI.BaseComponents;
using System.Collections.Generic;

namespace GLApplication
{
    class Program : NUIApplication
    {
        const string lib = "libdali-nativegl-library.so.0.0.1";
        [global::System.Runtime.InteropServices.DllImport(lib, EntryPoint = "intializeGL")]
        public static extern void intializeGL();

        [global::System.Runtime.InteropServices.DllImport(lib, EntryPoint = "renderFrameGL")]
        public static extern int renderFrameGL();

        [global::System.Runtime.InteropServices.DllImport(lib, EntryPoint = "terminateGL")]
        public static extern void terminateGL();

        [global::System.Runtime.InteropServices.DllImport(lib, EntryPoint = "updateTouchEventState")]
        public static extern void updateTouchEventState(int down);

        [global::System.Runtime.InteropServices.DllImport(lib, EntryPoint = "updateTouchPosition")]
        public static extern void updateTouchPosition(int x, int y);

        [global::System.Runtime.InteropServices.DllImport(lib, EntryPoint = "rotationCube")]
        public static extern void rotationCube(int x, int y);

        [global::System.Runtime.InteropServices.DllImport(lib, EntryPoint = "updateWindowSize")]
        public static extern void updateWindowSize(int w, int h);

        [global::System.Runtime.InteropServices.DllImport(lib, EntryPoint = "updateWindowRotationAngle")]
        public static extern void updateWindowRotationAngle(int angle);

        public GLWindow mGLWindow;
        protected override void OnCreate()
        {
          base.OnCreate();
          Initialize();
        }

        void Initialize()
        {
          mGLWindow = new GLWindow();
          mGLWindow.SetEglConfig(true, true, 0, GLWindow.GLESVersion.Version_2_0);

          mGLWindow.RegisterGlCallback(intializeGL, renderFrameGL, terminateGL);

          //int width, height;
          Information.TryGetValue("http://tizen.org/feature/screen.width", out int width);
          Information.TryGetValue("http://tizen.org/feature/screen.height",out int height);

          updateWindowSize(width, height);
          mGLWindow.Resized += OnResizedEvent;
          mGLWindow.KeyEvent += OnKeyEvent;
          mGLWindow.TouchEvent += OnTouchEvent;

          mGLWindow.Show();

          // Add GLWindow Avaialble Orientations
          List<GLWindow.GLWindowOrientation> orientations = new List<GLWindow.GLWindowOrientation>();
          orientations.Add(GLWindow.GLWindowOrientation.Portrait);
          orientations.Add(GLWindow.GLWindowOrientation.Landscape);
          orientations.Add(GLWindow.GLWindowOrientation.PortraitInverse);
          orientations.Add(GLWindow.GLWindowOrientation.LandscapeInverse);
          mGLWindow.SetAvailableOrientations(orientations);
        }

        public void OnKeyEvent(object sender, GLWindow.KeyEventArgs e)
        {
          if (e.Key.State == Key.StateType.Down)
          {
            if (e.Key.KeyPressedName == "XF86Back" || e.Key.KeyPressedName == "Escape")
            {
              Exit();
            }
            else if (e.Key.KeyPressedName == "Up")
            {
              rotationCube(0, 1);
            }
            else if (e.Key.KeyPressedName == "Down")
            {
              rotationCube(0, -1);
            }
            else if (e.Key.KeyPressedName == "Left")
            {
              rotationCube(-1, 0);
            }
            else if (e.Key.KeyPressedName == "Right")
            {
              rotationCube(1, 0);
            }
          }
        }

        public void OnTouchEvent(object sender, GLWindow.TouchEventArgs e)
        {
          if (e.Touch.GetState(0) == PointStateType.Up)
          {
            updateTouchEventState(2);
          }
          else if (e.Touch.GetState(0) == PointStateType.Down)
          {
            updateTouchEventState(1);
          }
          else if (e.Touch.GetState(0) == PointStateType.Motion)
          {
            updateTouchPosition((int)(e.Touch.GetScreenPosition(0).X), (int)(e.Touch.GetScreenPosition(0).Y));
          }
        }

        public void OnResizedEvent(object sender, GLWindow.ResizedEventArgs e)
        {
          Tizen.Log.Error("NUI", "OnResizedEvent w:" + e.WindowSize.Width);
          Tizen.Log.Error("NUI", "OnResizedEvent w:" + e.WindowSize.Height);

          GLWindow.GLWindowOrientation currentOrientation = mGLWindow.GetCurrentOrientation();
          Tizen.Log.Error("NUI", "OnResizedEvent currentOrientation:" + currentOrientation);

          if( currentOrientation == GLWindow.GLWindowOrientation.Portrait )
          {
            updateWindowRotationAngle(0);
          }
          else if(  currentOrientation == GLWindow.GLWindowOrientation.LandscapeInverse )
          {
            updateWindowRotationAngle(270);
          }
          else if(  currentOrientation == GLWindow.GLWindowOrientation.PortraitInverse )
          {
            updateWindowRotationAngle(180);
          }
          else
          {
            updateWindowRotationAngle(90);
          }
          updateWindowSize(e.WindowSize.Width, e.WindowSize.Height);
        }

        static void Main(string[] args)
        {
            var app = new Program();
            app.Run(args);
        }
    }
}

