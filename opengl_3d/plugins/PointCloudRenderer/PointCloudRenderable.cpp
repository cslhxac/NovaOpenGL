#include "../../RenderableManager.h"
#include "../../ApplicationFactory.h"
#include "../../Shader.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Nova {

    class PointCloudRenderable : public Renderable {        
        bool selected;
        unsigned long n_nodes;
        float* points;
        unsigned int* point_types;
        float* point_colors;
        GLuint vbo_position;
        GLuint vbo_color;
        GLuint vao;
    public:
        PointCloudRenderable( ApplicationFactory& app ) : Renderable( app ), selected(false), points(NULL), point_types(NULL), point_colors(NULL)
        {
        }

        virtual ~PointCloudRenderable(){
            if(points) delete [] points;
            if(point_types) delete [] point_types;
            if(point_colors) delete [] point_colors;
        }
        
        virtual void load(std::string path){
            std::cout << "PointCloudRenderable::load" << std::endl;            
            std::ifstream input;
            input.open(path,std::ios::in|std::ios::binary);
            input.read((char*)&n_nodes,sizeof(unsigned long));
            std::cout << "# of nodes read: " <<n_nodes<< std::endl;
            if(points) delete [] points;
            points = new float[n_nodes*3];
            input.read((char*)points,sizeof(float)*n_nodes*3);            
            if(point_types) delete [] point_types;
            point_types = new unsigned int[n_nodes];
            input.read((char*)point_types,sizeof(unsigned int)*n_nodes);
            input.close();            

            if(point_colors) delete [] point_colors;
            point_colors = new float[n_nodes*3];
            for(int i = 0;i < n_nodes;++i)
                if(point_types[i]==1){
                    point_colors[i*3+0]=1.0;
                    point_colors[i*3+1]=1.0;
                    point_colors[i*3+2]=1.0;}
                else if(point_types[i]==2){
                    point_colors[i*3+0]=0.0;
                    point_colors[i*3+1]=1.0;
                    point_colors[i*3+2]=1.0;}
                else{
                    point_colors[i*3+0]=1.0;
                    point_colors[i*3+1]=0.0;
                    point_colors[i*3+2]=0.0;}
            //for(int i = 0;i < n_nodes;++i) if(point_types[i]==2)std::cout<<point_types[i];
            // float points[]={
            //     -0.5,-0.5,0,
            //     -0.5, 0.5,0,
            //      0.5,-0.5,0,
            //      0.5, 0.5,0,
            // };
            
            auto _shader = _app.GetShaderManager().GetShader( "BasicPointCloudShader" );
            glGenBuffers(1, &vbo_position);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float)*n_nodes*3, points, GL_STATIC_DRAW);
            //glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
            glGenVertexArrays(1,&vao); 
            glBindVertexArray(vao);
            GLint posAttrib = glGetAttribLocation(_shader->GetProgramId(), "position");
            glEnableVertexAttribArray(posAttrib);
            glVertexAttribPointer(posAttrib,3,GL_FLOAT,GL_FALSE,0,0);
            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glGenBuffers(1, &vbo_color);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float)*n_nodes*3, point_colors, GL_STATIC_DRAW);
            glBindVertexArray(vao);
            GLint colorAttrib = glGetAttribLocation(_shader->GetProgramId(), "node_color");
            glEnableVertexAttribArray(colorAttrib);
            glVertexAttribPointer(colorAttrib,3,GL_FLOAT,GL_FALSE,0,0);
            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }


        virtual void draw(){
            glm::mat4 projection,view,model;
            view = _app.GetWorld().Get_ViewMatrix();
            model = _app.GetWorld().Get_ModelMatrix();
            projection = _app.GetWorld().Get_ProjectionMatrix();
            auto _shader = _app.GetShaderManager().GetShader( "BasicPointCloudShader" );
            // const float* matrix_ptr = (const float*)glm::value_ptr(projection);
            // std::cout<<"Projection: "<<std::endl;
            // for(int i = 0;i < 4;++i){
            //     for(int j = 0;j < 4;++j){
            //         std::cout<<" "<<matrix_ptr[j*4+i];}
            //     std::cout<<std::endl;
            // }
            _shader->SetMatrix4("projection",projection);
            _shader->SetMatrix4("view",view);
            _shader->SetMatrix4("model",model);
            _shader->Use();
            glBindVertexArray(vao);
            GLsizei n_points=(GLsizei)n_nodes;
            glDrawArrays(GL_POINTS,0,n_points);
            //glDrawArrays(GL_POINTS,0,4);
            glBindVertexArray(0);
        }

        virtual bool selectable() { return false; };

        virtual float hit_test( glm::vec3 start_point, glm::vec3 end_point )
        {
        }

        virtual glm::vec4 bounding_sphere()
        {
        }

        virtual void assign_selection( glm::vec3 start_point, glm::vec3 end_point, glm::vec3 intersection )
        {
            selected = true;
        }

        virtual void unassign_selection()
        {
            selected = false;
        }
    };

  
  class PointCloudRenderableFactory : public RenderableFactory {
  public:
    PointCloudRenderableFactory() : RenderableFactory()
    {}
    
    virtual ~PointCloudRenderableFactory() {}

    virtual std::unique_ptr<Renderable> Create( ApplicationFactory& app, std::string path ){
      PointCloudRenderable* renderable = new PointCloudRenderable(app);
      renderable->load( path );
      return std::unique_ptr<Renderable>( renderable );
    }
    
    virtual bool AcceptExtension(std::string ext) const {
        return ext=="ptc";
        //return (importer.GetImporter( ext.c_str() ) != NULL);
    };

  };
  

}

extern "C" void registerPlugin(Nova::ApplicationFactory& app) {
  app.GetRenderableManager().AddFactory( std::move( std::unique_ptr<Nova::RenderableFactory>( new Nova::PointCloudRenderableFactory() )));
}

extern "C" int getEngineVersion() {
  return Nova::API_VERSION;
}
