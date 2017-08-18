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
        float* points;
        GLuint vbo;
        GLuint vao;
    public:
        PointCloudRenderable( ApplicationFactory& app ) : Renderable( app ), selected(false), points(NULL)
        {
            auto _shader = _app.GetShaderManager().GetShader( "BasicPointCloudShader" );
            glGenBuffers(1, &vbo);
            float points[]={
                -0.5f,  0.5f,
                 0.5f,  0.5f,
                 0.5f, -0.5f,
                -0.5f, -0.5f,
            };
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

            glGenVertexArrays(1,&vao);
            glBindVertexArray(vao);
            GLint posAttrib = glGetAttribLocation(_shader->GetProgramId(), "aPos");
            glEnableVertexAttribArray(posAttrib);
            glVertexAttribPointer(posAttrib,2,GL_FLOAT,GL_FALSE,0,0);
            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            
        }

        virtual ~PointCloudRenderable(){
            if(points) delete points;
        }
        
        virtual void load(std::string path){
            std::cout << "PointCloudRenderable::load" << std::endl;            
            std::ifstream input;
            input.open(path,std::ios::in|std::ios::binary);
            unsigned long n_nodes;
            input.read((char*)&n_nodes,sizeof(unsigned long));
            std::cout << "# of nodes read: " <<n_nodes<< std::endl;
            if(points) delete points;
            points = new float[n_nodes*3];
            input.read((char*)points,sizeof(float)*n_nodes*3);            
            input.close();            
        }


        virtual void draw(){
            glm::mat4 projection,view,model;
            view = _app.GetWorld().Get_ViewMatrix();
            model = _app.GetWorld().Get_ModelMatrix();
            projection = _app.GetWorld().Get_ProjectionMatrix();
            auto _shader = _app.GetShaderManager().GetShader( "BasicPointCloudShader" );
            //_shader->SetMatrix4("projection",projection);
            //_shader->SetMatrix4("view",view);
            //_shader->SetMatrix4("model",model);
            _shader->Use();
            glBindVertexArray(vao);            
            GLint posAttrib = glGetAttribLocation(_shader->GetProgramId(), "pos");
            glEnableVertexAttribArray(posAttrib);
            glVertexAttribPointer(posAttrib,2,GL_FLOAT,GL_FALSE,0,0);
            glDrawArrays(GL_POINTS,0,4);
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
