add_library( opengl_3d_kernel
  Config.cpp
  ApplicationFactory.cpp
  ShaderManager.cpp
  Scene.cpp
  PluginManager.cpp
  Plugin.cpp
  RenderableManager.cpp
  World.cpp
  Shader.cpp
  IOEvent.cpp
  IOService.cpp
  TextRenderingService.cpp
  CommandDispatch.cpp
  Camera.cpp
  OrbitCameraControl.cpp
  TrackballCameraControl.cpp
  TextureManager.cpp
  ViewportManager.cpp
  )

#pkg_check_modules (glfw3 REQUIRED glfw3)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLM REQUIRED)

set( USE_C11_REGEX OFF CACHE BOOL "Use regular expressions from the c++11 standard library, otherwise use Boost." )

if( USE_C11_REGEX )
  target_compile_definitions( opengl_3d_kernel PRIVATE USE_C11_REGEX )
  find_package(Boost 1.54 REQUIRED COMPONENTS filesystem program_options)
else()
  find_package(Boost 1.54 REQUIRED COMPONENTS filesystem program_options regex)
endif()

if(WIN32)
  target_compile_definitions(opengl_3d_kernel PUBLIC OPENGL_3D_WIN32=1)
endif()

if(UNIX)
  target_compile_definitions(opengl_3d_kernel PUBLIC OPENGL_3D_LINUX=1)
endif()

target_include_directories( opengl_3d_kernel PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include> )

target_link_libraries( opengl_3d_kernel
  ${Boost_LIBRARIES}
  ${CMAKE_DL_LIBS}
  ${OPENGL_LIBRARIES}
  glfw
  GLEW
  )

target_compile_features( opengl_3d_kernel
  PUBLIC
  cxx_auto_type
)

#
#  Now the main program
#

add_executable(opengl_3d_new 
  main.cpp
  )

target_link_libraries( opengl_3d_new
  opengl_3d_kernel )

#
#  Now all the plugins...
#

add_subdirectory( plugins/Assimp )
add_subdirectory( plugins/FreeTypeTextRenderer )
add_subdirectory( plugins/SOILTextureProvider )
add_subdirectory( plugins/OIIOTextureProvider )
add_subdirectory( plugins/PointCloudRenderer )
