//#####################################################################
// Copyright (c) 2016, Mridul Aanjaneya.
//#####################################################################
#include "AssimpRenderable_Model.h"
#include "../../ApplicationFactory.h"
//#include <nova/Rendering/OpenGL/Utilities/OpenGL_Utilities.h>
#include <assimp/postprocess.h>
#include <iostream>
#include "miniball/Seb.h"
#include <limits>
#include <glm/gtx/intersect.hpp>

using namespace Nova;

#include <SOIL/SOIL.h>
GLint Texture_From_File(const char* path,const std::string& directory)
{
    std::string filename(path);
    filename=directory+'/'+filename;
    GLuint texture_id;
    glGenTextures(1,&texture_id);
    int width,height;
    unsigned char* image=SOIL_load_image(filename.c_str(),&width,&height,0,SOIL_LOAD_RGB);
    // assign textures to id
    glBindTexture(GL_TEXTURE_2D,texture_id);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,width,height,0,GL_RGB,GL_UNSIGNED_BYTE,image);
    glGenerateMipmap(GL_TEXTURE_2D);

    // parameters
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D,0);
    SOIL_free_image_data(image);
    return texture_id;
}

//#####################################################################
// Constructor
//#####################################################################
AssimpRenderable_Model::
AssimpRenderable_Model( ApplicationFactory& app) : _app(app)
{
}
//#####################################################################
// Draw
//#####################################################################
void AssimpRenderable_Model::
Draw(const Shader& shader)
{
    shader.Use();
    for(GLuint i=0;i<meshes.size();++i) meshes[i].Draw(shader);
}
//#####################################################################
// Load_Model
//#####################################################################
void AssimpRenderable_Model::
Load_Model(const std::string& path)
{
    std::cout << "AssimpRenderable_Model::Load_Model" << std::endl;
            
    Assimp::Importer importer;
    const aiScene* scene=importer.ReadFile(path,aiProcessPreset_TargetRealtime_MaxQuality);

    if(!scene || scene->mFlags==AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout<<"ERROR::ASSIMP:: "<<importer.GetErrorString()<<std::endl;
        return;
    }

    std::cout << "Model contains: " << scene->mRootNode->mNumMeshes << " meshes;" << std::endl;

    directory=path.substr(0,path.find_last_of('/'));

    // process ASSIMP's root node recursively
    Process_Node(scene->mRootNode,scene);
    Build_Bounding_Sphere();
}
//#####################################################################
// Process_Node
//#####################################################################
void AssimpRenderable_Model::
Process_Node(aiNode* node,const aiScene* scene)
{
    // process each mesh located at the current node
    for(GLuint i=0;i<node->mNumMeshes;++i)
    {        
        aiMesh *mesh=scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(Process_Mesh(mesh,scene));
    }

    // recursively process each of the children nodes
    for(GLuint i=0;i<node->mNumChildren;++i)
        Process_Node(node->mChildren[i],scene);
}
//#####################################################################
// Process_Mesh
//#####################################################################
AssimpRenderable_Mesh AssimpRenderable_Model::
Process_Mesh(aiMesh *mesh,const aiScene *scene)
{
    std::cout << "Processing mesh" << std::endl;
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<Texture> textures;
    
    for(GLuint i=0;i<mesh->mNumVertices;++i)
    {
        Vertex vertex;
        glm::vec3 vector;

        // positions
        vector.x=mesh->mVertices[i].x;
        vector.y=mesh->mVertices[i].y;
        vector.z=mesh->mVertices[i].z;
        vertex.position=vector;

        // normals
        vector.x=mesh->mNormals[i].x;
        vector.y=mesh->mNormals[i].y;
        vector.z=mesh->mNormals[i].z;
        vertex.normal=vector;

        // texture coordinates
        if(mesh->mTextureCoords[0])
        {
            glm::vec2 vec;
            vec.x=mesh->mTextureCoords[0][i].x;
            vec.y=mesh->mTextureCoords[0][i].y;
            vertex.tex_coords=vec;
        }
        else vertex.tex_coords=glm::vec2(0.0f,0.0f);
        
        vertices.push_back(vertex);
    }

    // walk through all the faces
    for(GLuint i=0;i<mesh->mNumFaces;++i)
    {
        aiFace face=mesh->mFaces[i];
        for(GLuint j=0;j<face.mNumIndices;++j) indices.push_back(face.mIndices[j]);
    }

    // process materials
    if(mesh->mMaterialIndex>=0)
    {
        aiMaterial *material=scene->mMaterials[mesh->mMaterialIndex];

        // diffuse maps
        std::vector<Texture> diffuse_maps;
        Load_Material_Textures(material,aiTextureType_DIFFUSE,"texture_diffuse",diffuse_maps);
        textures.insert(textures.end(),diffuse_maps.begin(),diffuse_maps.end());
        // specular maps
        std::vector<Texture> specular_maps;
        Load_Material_Textures(material,aiTextureType_SPECULAR,"texture_specular",specular_maps);
        textures.insert(textures.end(),specular_maps.begin(),specular_maps.end());
    }

    return AssimpRenderable_Mesh(vertices,indices,textures);
}
//#####################################################################
// Build_Bounding_Sphere
//#####################################################################
void
AssimpRenderable_Model::Build_Bounding_Sphere()
{

    std::vector< glm::vec3 > all_vertices;
    for( auto mesh : meshes ){
        for( auto vertex : mesh.vertices ){
            all_vertices.push_back( vertex.position );
        }
    }
    if( all_vertices.size() == 0 ){
        _boundingSphere = glm::vec4(0,0,0,0);
        return;
    }

   // For bounding sphere construction
    SEB_NAMESPACE::Smallest_enclosing_ball<float,glm::vec3> miniball(3, all_vertices);
    
    glm::vec4 sphere;
    sphere.x = *(miniball.center_begin()+0);
    sphere.y = *(miniball.center_begin()+1);
    sphere.z = *(miniball.center_begin()+2);
    sphere[3] = miniball.radius(); 

    std::cout << "Bounding sphere computed to at " << sphere.x << " " << sphere.y << " " << sphere.z <<
        ", with a radius of  " << sphere[3] << std::endl;

    _boundingSphere = sphere;
}
//#####################################################################
// Load_Material_Textures
//#####################################################################
void AssimpRenderable_Model::
Load_Material_Textures(aiMaterial *material,aiTextureType type,const std::string& type_name,std::vector<Texture>& textures)
{
    for(GLuint i=0;i<material->GetTextureCount(type);++i)
    {
        aiString str;
        material->GetTexture(type,i,&str);
        // skip texture if it was loaded before
        GLboolean skip=false;
        for(GLuint j=0;j<textures_loaded.size();++j)
        {
            if(textures_loaded[j].path==str)
            {
                textures.push_back(textures_loaded[j]);
                skip=true;break;
            }
        }

        if(!skip)
        {
            Texture texture;
            texture.id=_app.GetTextureManager().GetOpenGLTexture( (std::string(directory)+'/')+str.C_Str() );
            texture.type=type_name;
            texture.path=str;
            textures.push_back(texture);
            textures_loaded.push_back(texture);
        }
    }
}
//#####################################################################
// TestIntersection
//#####################################################################
float 
AssimpRenderable_Model::TestIntersection( glm::vec3 start_point, glm::vec3 end_point ){
    
    glm::vec3 origin = start_point;
    glm::vec3 direction = glm::normalize(end_point-start_point);    
    glm::vec3 position;
    bool intersection_found = false;
    float distance = std::numeric_limits<float>::infinity();

    for( auto mesh : meshes ){
        for( int triangle = 0; triangle < mesh.indices.size()/3; triangle++ ){
            
            bool intersecting = glm::intersectLineTriangle( origin, direction, 
                                                            mesh.vertices[ mesh.indices[triangle*3+0] ].position,
                                                            mesh.vertices[ mesh.indices[triangle*3+1] ].position,
                                                            mesh.vertices[ mesh.indices[triangle*3+2] ].position,
                                                            position );
            if( intersecting ){
                intersection_found = true;
                if( distance > glm::length( position - origin ) )
                    distance = glm::length( position - origin );
            }   
        }
    }
    
    return distance;
}
//#####################################################################
// BoundingSphere
//#####################################################################
glm::vec4 
AssimpRenderable_Model::BoundingSphere()
{
    return _boundingSphere;
}
