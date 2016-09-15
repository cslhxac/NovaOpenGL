//#####################################################################
// Copyright (c) 2016, Mridul Aanjaneya.
//#####################################################################
#include "Shader.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>

using namespace Nova;
//#####################################################################
// Constructor
//#####################################################################
Shader::Shader() : program(0) {


}

Shader::~Shader() {
    if( program )
        glDeleteProgram( this->program );
}

GLuint 
Shader::GetProgramId() const {
    return program;
}


void
Shader::LoadFromFiles(std::string vertex_path,std::string fragment_path)
{
    // retrieve the vertex/fragment source code
    std::string vertex_code,fragment_code;
    std::ifstream vertex_shader_file,fragment_shader_file;

    // ensures ifstream objects can throw exceptions
    vertex_shader_file.exceptions(std::ifstream::badbit);
    fragment_shader_file.exceptions(std::ifstream::badbit);

    try{
        // open files
        vertex_shader_file.open(vertex_path.c_str());
        fragment_shader_file.open(fragment_path.c_str());
        std::stringstream vertex_shader_stream,fragment_shader_stream;

        // read file's buffer contents into streams
        vertex_shader_stream<<vertex_shader_file.rdbuf();
        fragment_shader_stream<<fragment_shader_file.rdbuf();

        // close file handlers
        vertex_shader_file.close();
        fragment_shader_file.close();

        // convert stream into string
        vertex_code=vertex_shader_stream.str();
        fragment_code=fragment_shader_stream.str();
    }
    catch(std::ifstream::failure& e){
        throw std::runtime_error(std::string("Error::Shader::File Not Successfully Read!"));
    }

    LoadFromString( vertex_code.c_str(), fragment_code.c_str() );
    
}

void
Shader::LoadFromString(const GLchar* vertex_shader_code,const GLchar* fragment_shader_code)
{

    // compile shaders
    GLuint vertex,fragment;
    GLint success;
    GLchar info_log[512];

    // vertex shader
    vertex=glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex,1,&vertex_shader_code,NULL);
    glCompileShader(vertex);
    // print compile errors if any
    glGetShaderiv(vertex,GL_COMPILE_STATUS,&success);
    if(!success)
    {
        glGetShaderInfoLog(vertex,512,NULL,info_log);
        std::cout<<"Error::Shader::Vertex::Compilation Failed!"<<std::endl<<info_log<<std::endl;
    }

    // fragment shader
    fragment=glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment,1,&fragment_shader_code,NULL);
    glCompileShader(fragment);
    // print compile errors if any
    glGetShaderiv(fragment,GL_COMPILE_STATUS,&success);
    if(!success)
    {
        glGetShaderInfoLog(fragment,512,NULL,info_log);
        std::cout<<"Error::Shader::Fragment::Compilation Failed!"<<std::endl<<info_log<<std::endl;
    }

    // shader program
    this->program=glCreateProgram();
    glAttachShader(this->program,vertex);
    glAttachShader(this->program,fragment);
    glLinkProgram(this->program);
    // print linking errors if any
    glGetProgramiv(this->program,GL_LINK_STATUS,&success);
    if(!success)
    {
        glGetProgramInfoLog(this->program,512,NULL,info_log);
        std::cout<<"Error::Shader::Program::Linking Failed!"<<std::endl<<info_log<<std::endl;
    }

    // delete the shaders as they are now linked into our program and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}
//#####################################################################
// Use
//#####################################################################
void Shader::
Use() const 
{
    glUseProgram(this->program);
}
//#####################################################################
