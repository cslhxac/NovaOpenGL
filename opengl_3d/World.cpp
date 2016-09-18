//#####################################################################
// Copyright (c) 2016, Mridul Aanjaneya.
//#####################################################################
#include "World.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "ApplicationFactory.h"

#include "StaticCameraControl.h"
#include "TrackballCameraControl.h"
#include "OrbitCameraControl.h"
#include "IOEvent.h"

using namespace Nova;
//#####################################################################
// Constructor
//#####################################################################
World::
World(ApplicationFactory& app)
    :_app(app),window(nullptr),control(nullptr)
{
    render_camera.Set_Mode(FREE);
    render_camera.Set_Position(glm::vec3(0,0,-10));
    render_camera.Set_Look_At(glm::vec3(0,0,0));
    render_camera.Set_Clipping(.1,1000);
    render_camera.Set_FOV(45);
    control = new OrbitCameraControl( render_camera );
    _app.GetIOService().On( IOService::MOUSE_DOWN, [&](IOEvent& event){control->MouseDown(event);} );
    _app.GetIOService().On( IOService::MOUSE_UP, [&](IOEvent& event){control->MouseUp(event);} );
    _app.GetIOService().On( IOService::MOUSE_MOVE, [&](IOEvent& event){control->MouseMove(event);} );
    _app.GetIOService().On( IOService::SCROLL, [&](IOEvent& event){control->Scroll(event);} );
    _app.GetIOService().On( IOService::KEY_DOWN, [&](IOEvent& event){control->KeyDown(event);} );
    _app.GetIOService().On( IOService::KEY_UP, [&](IOEvent& event){control->KeyUp(event);} );
    _app.GetIOService().On( IOService::KEY_HOLD, [&](IOEvent& event){control->KeyHold(event);} );
    _app.GetIOService().On( IOService::REDRAW, [&](IOEvent& event){control->Redraw(event);} );
    _app.GetIOService().On( IOService::TIME, [&](IOEvent& event){control->Update(event);} );


}
//#####################################################################
// Destructor
//#####################################################################
World::
~World()
{
    glfwTerminate();
    if( control ){
        delete control;
        control = nullptr;
    }
}
//#####################################################################
// Camera Matrix Operators
//#####################################################################
    glm::mat4 
    World::Get_ViewMatrix() const {
return render_camera.Get_ViewMatrix();
}

        glm::mat4
        World::Get_ModelMatrix() const {
return render_camera.Get_ModelMatrix();

}

            glm::mat4
            World::Get_ProjectionMatrix() const {
return render_camera.Get_ProjectionMatrix();

}

//#####################################################################
// Initialize
//#####################################################################
void World::
Initialize(int size_x,int size_y)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);

    window=glfwCreateWindow(size_x,size_y,"OpenGL Visualization",nullptr,nullptr);
    if(window==nullptr)
    {
        std::cout<<"Failed to create GLFW window!"<<std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);

    // disable vsync
    glfwSwapInterval(0);

    glewExperimental=GL_TRUE;
    if(glewInit()!=GLEW_OK)
    {
        std::cout<<"Failed to initialize GLEW!"<<std::endl;
        return;
    }

    glfwSetWindowCloseCallback(window,Close_Callback);
    glfwSetFramebufferSizeCallback(window,Reshape_Callback);
    glfwSetKeyCallback(window,Keyboard_Callback);
    glfwSetMouseButtonCallback(window,Mouse_Button_Callback);
    glfwSetCursorPosCallback(window,Mouse_Position_Callback);
    glfwSetScrollCallback(window,Scroll_Callback);

    glfwGetFramebufferSize(window,&size_x,&size_y);
    if(size_y>0)
    {
        window_size=glm::ivec2(size_x,size_y);
        window_aspect=float(size_x)/float(size_y);

        GLFWmonitor* primary=glfwGetPrimaryMonitor();
        glfwGetMonitorPhysicalSize(primary,&window_physical_size.x,&window_physical_size.y);
        glfwGetMonitorPos(primary,&window_position.x,&window_position.y);
        const GLFWvidmode* mode=glfwGetVideoMode(primary);
        dpi=mode->width/(window_physical_size.x/25.4);
    }

    glfwSetWindowUserPointer(window,this);
    glfwSetInputMode(window,GLFW_CURSOR_DISABLED,GL_FALSE);
    Initialize_Camera_Controls();
}
//#####################################################################
// Initialize_Camera_Controls
//#####################################################################
void World::
Initialize_Camera_Controls()
{
  int width, height;
  glfwGetWindowSize(window, &width, &height);

  // Fire a redraw event to initialize the control to the window size,
  // just in case it needs it. 
  IOEvent event;
  event.type = IOEvent::DRAW;
  event.draw_data.width = width;
  event.draw_data.height = height;
  event.currentTime = glfwGetTime();
  control->Redraw( event );

  // Call reset to clear any state
  control->Reset();
}
//#####################################################################
// Handle_Input
//#####################################################################
void World::
Handle_Input(int key)
{
}
//#####################################################################
// Main_Loop
//#####################################################################
void World::
Main_Loop()
{
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glEnable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_CULL_FACE);

        glClearColor(18.0f/255.0f,26.0f/255.0f,32.0f/255.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        render_camera.aspect=window_aspect;
        render_camera.window_width=window_size.x;
        render_camera.window_height=window_size.y;
        
        IOEvent event;
        event.type = IOEvent::TIME;
        event.currentTime = glfwGetTime();
        _app.GetIOService().Trigger( event );
        render_camera.Update();

        glm::mat4 projection,view,model;
        render_camera.Get_Matrices(projection,view,model);

        glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

        _app.GetScene().Draw();

        // Test the TextRender Service     
        _app.GetTextRenderingService().Render( "OpenGL_3D", 1.0f, 0.0f, 0.0f); 

        
        glfwSwapBuffers(window);
    }
}
//#####################################################################

void World::Close_Callback(GLFWwindow* window)
{glfwSetWindowShouldClose(window,GL_TRUE);}

void World::Scroll_Callback( GLFWwindow* window,double xoffset,double yoffset)
{
    World *world=static_cast<World*>(glfwGetWindowUserPointer(window));
    
    IOEvent event;
    event.type = IOEvent::SCROLL;
    event.scroll_data.x = xoffset;
    event.scroll_data.y = yoffset;
    event.currentTime = glfwGetTime();
    world->_app.GetIOService().Trigger( event );
    
}


void World::Reshape_Callback(GLFWwindow* window,int w,int h)
{
    World *world=static_cast<World*>(glfwGetWindowUserPointer(window));
    if(h>0)
        {
            world->window_size=glm::ivec2(w,h);
            world->window_aspect=float(w)/float(h);
        }
    
    GLFWmonitor *primary=glfwGetPrimaryMonitor();
    glfwGetMonitorPhysicalSize(primary,&world->window_physical_size.x,&world->window_physical_size.y);
    glfwGetMonitorPos(primary,&world->window_position.x,&world->window_position.y);
    const GLFWvidmode *mode=glfwGetVideoMode(primary);
    world->dpi=mode->width/(world->window_physical_size.x/25.4);
    
    IOEvent event;
    event.type = IOEvent::DRAW;
    event.draw_data.width = w;
    event.draw_data.height = h;
    event.currentTime = glfwGetTime();
    world->_app.GetIOService().Trigger( event );
}

void World::Keyboard_Callback(GLFWwindow* window,int key,int scancode,int action,int mode)
{
    World *world=static_cast<World*>(glfwGetWindowUserPointer(window));
    if(key==GLFW_KEY_ESCAPE && action==GLFW_PRESS)
        glfwSetWindowShouldClose(window,GL_TRUE);           
    
    IOEvent event;
    event.type = IOEvent::KEYBOARD;
    event.key_data.key = IOEvent::KEY_CODE(key);
    event.key_data.scancode = scancode;
    event.key_data.mods =
        ( (mode & GLFW_MOD_SUPER != 0 ) ?  IOEvent::mSHIFT : 0 )  | 
        ( (mode & GLFW_MOD_ALT != 0 ) ?  IOEvent::mALT : 0 )  | 
        ( (mode & GLFW_MOD_CONTROL != 0 ) ?  IOEvent::mCONTROL : 0 )  | 
        ( (mode & GLFW_MOD_SUPER != 0 ) ?  IOEvent::mSUPER : 0  );
    event.currentTime = glfwGetTime();
    switch( action ){
    case GLFW_PRESS:
        event.key_data.action = IOEvent::K_DOWN;
        break;
    case GLFW_RELEASE:
        event.key_data.action = IOEvent::K_UP;
        break;
    case GLFW_REPEAT:
        event.key_data.action = IOEvent::K_HOLD;
        break;
    }

    world->_app.GetIOService().Trigger( event );
}

void World::Mouse_Button_Callback(GLFWwindow* window,int button,int state,int mods)
{
    World *world=static_cast<World*>(glfwGetWindowUserPointer(window));
    double x,y;
    glfwGetCursorPos(window,&x,&y);
    
    IOEvent event;
    event.type = IOEvent::MOUSEBUTTON;
    event.currentTime = glfwGetTime();
    switch( button ){
    case GLFW_MOUSE_BUTTON_LEFT:
        event.mousebutton_data.button = IOEvent::M_LEFT;
        break;
    case GLFW_MOUSE_BUTTON_RIGHT:
        event.mousebutton_data.button = IOEvent::M_RIGHT;
        break;
    case GLFW_MOUSE_BUTTON_MIDDLE:
        event.mousebutton_data.button = IOEvent::M_MIDDLE;
        break;
    default:
        event.mousebutton_data.button = IOEvent::M_OTHER;
        break;
    }
    event.mousebutton_data.button_raw = button;
    event.mousebutton_data.x = x;
    event.mousebutton_data.y = y;
    event.mousebutton_data.mods =
        ( (mods & GLFW_MOD_SUPER != 0 ) ?  IOEvent::mSHIFT : 0 )  | 
        ( (mods & GLFW_MOD_ALT != 0 ) ?  IOEvent::mALT : 0 )  | 
        ( (mods & GLFW_MOD_CONTROL != 0 ) ?  IOEvent::mCONTROL : 0 )  | 
        ( (mods & GLFW_MOD_SUPER != 0 ) ?  IOEvent::mSUPER : 0  );
    switch( state ){
    case GLFW_PRESS:
        event.mousebutton_data.action = IOEvent::M_DOWN;
        break;
    case GLFW_RELEASE:
        event.mousebutton_data.action = IOEvent::M_UP;
        break;
    }
    world->_app.GetIOService().Trigger( event );
}

void World::Mouse_Position_Callback(GLFWwindow* window,double x,double y)
{
    World *world=static_cast<World*>(glfwGetWindowUserPointer(window));
    IOEvent event;
    event.type = IOEvent::MOUSEMOVE;
    event.currentTime = glfwGetTime();
    event.mousemotion_data.x = x;
    event.mousemotion_data.y = y;
    world->_app.GetIOService().Trigger( event );
}