#include "ShaderManager.h"

// Include all Builtin Shaders //
#include "shaders/BasicMeshShader.h"
#include "shaders/BasicLineShader.h"
#include "shaders/BasicTextShader.h"
#include "shaders/BasicPointCloudShader.h"
#include "shaders/ViewportChromeShader.h"
#include "shaders/ViewportAxisShader.h"


// Done                        //

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

Nova::ShaderManager::ShaderManager(ApplicationFactory& app) : _app(app)
{
    // Need to populate the Built-in Shader maps. 
    // These shaders are compiled in and never change

#define LOAD_BUILTIN_SHADER( NAME )  {\
        unsigned int id = LoadFromString(NovaBuiltinShaders::NAME::vertex_shader,\
                                         NovaBuiltinShaders::NAME::fragment_shader,\
                                         NovaBuiltinShaders::NAME::geometry_shader);\
        assert( id );\
        std::string name( NovaBuiltinShaders::NAME::name );\
        _shaderRepo.insert( std::make_pair( name , id ) );\
    }

    LOAD_BUILTIN_SHADER( BasicMeshShader );
    LOAD_BUILTIN_SHADER( BasicLineShader );
    LOAD_BUILTIN_SHADER( BasicTextShader );
    LOAD_BUILTIN_SHADER( BasicPointCloudShader );
    LOAD_BUILTIN_SHADER( ViewportChromeShader );
    LOAD_BUILTIN_SHADER( ViewportAxisShader );

#undef LOAD_BUILTIN_SHADER

    
}

Nova::ShaderManager::~ShaderManager() 
{
    for( auto shader : _shaderRepo ){
        glDeleteProgram( shader.second );
    }
}

std::unique_ptr<Nova::Shader>
Nova::ShaderManager::GetShader(const std::string& name)
{
    auto res = _shaderRepo.find( name );
    if( res == _shaderRepo.end() ){
        unsigned int program_id = LoadFromFiles( name );        
        return std::unique_ptr<Nova::Shader>( new Shader(program_id) );           
    }
    else{
        return std::unique_ptr<Nova::Shader>( new Shader(res->second) );           
    }
}


unsigned int 
Nova::ShaderManager::LoadFromFiles( std::string name )
{
    // We need to search shader searchpaths for all files with the patterns:
    //      path/name.vert
    //      path/name.frag
    //      path/name.geom
    // And load them into a program

    // Path construction
    std::string vertex_path, fragment_path, geometry_path;
    vertex_path = name + ".vert";
    fragment_path = name + ".frag";
    geometry_path = name + ".geom";

    // retrieve the vertex/fragment source code
    std::string vertex_code,fragment_code,geometry_code;
    std::ifstream vertex_shader_file,fragment_shader_file,geometry_shader_file;
    std::stringstream vertex_shader_stream, fragment_shader_stream, geometry_shader_stream;    

    // ensures ifstream objects can throw exceptions
    vertex_shader_file.exceptions(std::ifstream::badbit);
    fragment_shader_file.exceptions(std::ifstream::badbit);
    geometry_shader_file.exceptions(std::ifstream::badbit);

    try{
        // open files
        vertex_shader_file.open(vertex_path.c_str());       
        // read file's buffer contents into streams
        vertex_shader_stream<<vertex_shader_file.rdbuf();
        // close file handlers
        vertex_shader_file.close();
        // convert stream into string
        vertex_code=vertex_shader_stream.str();
    }
    catch(std::ifstream::failure& e){
        std::cout << "Shader: Vertex Program Not Successfully Read!" << std::endl;
        vertex_code = std::string();
    }

    try{
        // open files
        fragment_shader_file.open(fragment_path.c_str());       
        // read file's buffer contents into streams
        fragment_shader_stream<<fragment_shader_file.rdbuf();
        // close file handlers
        fragment_shader_file.close();
        // convert stream into string
        fragment_code=fragment_shader_stream.str();
    }
    catch(std::ifstream::failure& e){
        std::cout << "Shader: Fragment Program Not Successfully Read!" << std::endl;
        fragment_code = std::string();
    }

    try{
        // open files
        geometry_shader_file.open(geometry_path.c_str());
        // read file's buffer contents into streams
        geometry_shader_stream<<geometry_shader_file.rdbuf();
        // close file handlers
        geometry_shader_file.close();
        // convert stream into string
        geometry_code=geometry_shader_stream.str();
    }
    catch(std::ifstream::failure& e){
        std::cout << "Shader: Geometry Program Not Successfully Read!" << std::endl;
        geometry_code = std::string();
    }

    if( vertex_code == "" || fragment_code == "" )
        throw std::runtime_error( "Could not load shader. Both vertex and fragment code must be present." );


    unsigned int program_id = 0;
    if( geometry_code != "" )
        program_id = LoadFromString( vertex_code.c_str(), fragment_code.c_str(), geometry_code.c_str() );
    else
        program_id = LoadFromString( vertex_code.c_str(), fragment_code.c_str(), NULL );

    if( program_id == 0 )
        throw std::runtime_error( "Failed to compile and link shader." );
    
    _shaderRepo.insert( std::make_pair( name, program_id ) );

    return program_id;
}

unsigned int 
Nova::ShaderManager::LoadFromString( const char* vertex_shader,
                                     const char* fragment_shader,
                                     const char* geometry_shader )
{

    // compile shaders
    GLuint vertex,fragment,geometry;
    GLuint program;
    GLint success;
    GLchar info_log[512];

    // vertex shader
    vertex=glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex,1,&vertex_shader,NULL);
    glCompileShader(vertex);
    // print compile errors if any
    glGetShaderiv(vertex,GL_COMPILE_STATUS,&success);
    if(!success)
    {
        glGetShaderInfoLog(vertex,512,NULL,info_log);
        std::cout<<"Error::Shader::Vertex::Compilation Failed!"<<std::endl<<info_log<<std::endl;
        return 0;
    }

    // fragment shader
    fragment=glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment,1,&fragment_shader,NULL);
    glCompileShader(fragment);
    // print compile errors if any
    glGetShaderiv(fragment,GL_COMPILE_STATUS,&success);
    if(!success)
    {
        glGetShaderInfoLog(fragment,512,NULL,info_log);
        std::cout<<"Error::Shader::Fragment::Compilation Failed!"<<std::endl<<info_log<<std::endl;
        return 0;
    }

    if( geometry_shader ){
        // geometry shader
        geometry=glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometry,1,&geometry_shader,NULL);
        glCompileShader(geometry);
        // print compile errors if any
        glGetShaderiv(geometry,GL_COMPILE_STATUS,&success);
        if(!success)
            {
                glGetShaderInfoLog(geometry,512,NULL,info_log);
                std::cout<<"Error::Shader::Geometry::Compilation Failed!"<<std::endl<<info_log<<std::endl;
                return 0;
            }
    }

    // shader program
    program=glCreateProgram();
    glAttachShader(program,vertex);
    glAttachShader(program,fragment);
    if( geometry_shader )
        glAttachShader(program,geometry);
    glLinkProgram(program);
    // print linking errors if any
    glGetProgramiv(program,GL_LINK_STATUS,&success);
    if(!success)
    {
        glGetProgramInfoLog(program,512,NULL,info_log);
        std::cout<<"Error::Shader::Program::Linking Failed!"<<std::endl<<info_log<<std::endl;
        return 0;
    }

    // delete the shaders as they are now linked into our program and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    if( geometry_shader )
        glDeleteShader(geometry);

    return program;        
}
