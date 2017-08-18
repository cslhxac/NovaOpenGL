set( ENABLE_POINTCLOUD_PLUGIN OFF CACHE BOOL "Enable PointCloud Renderable plugin." )

if( ENABLE_POINTCLOUD_PLUGIN )
    
  add_library( plugin_PointCloud SHARED
    PointCloudRenderable.cpp
    ../../Config.cpp
    ../../ApplicationFactory.cpp
    ../../ShaderManager.cpp
    ../../Scene.cpp
    ../../PluginManager.cpp
    ../../Plugin.cpp
    ../../RenderableManager.cpp
    ../../World.cpp
    ../../Shader.cpp
    ../../IOEvent.cpp
    ../../IOService.cpp
    ../../TextRenderingService.cpp
    ../../CommandDispatch.cpp
    ../../Camera.cpp
    ../../OrbitCameraControl.cpp
    ../../TrackballCameraControl.cpp
    ../../TextureManager.cpp
    ../../ViewportManager.cpp
    )

  if(WIN32)
    target_compile_definitions( plugin_PointCloud PUBLIC OPENGL_3D_WIN32=1)
  endif()
  
  if(UNIX)
    target_compile_definitions( plugin_PointCloud PUBLIC OPENGL_3D_LINUX=1)
  endif()
  
  
  target_link_libraries( plugin_PointCloud
    PRIVATE
    )
  
  target_compile_features( plugin_PointCloud
    PRIVATE
    cxx_auto_type
    )
endif()

