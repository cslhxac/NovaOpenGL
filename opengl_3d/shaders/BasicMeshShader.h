namespace NovaBuiltinShaders{

    struct BasicMeshShader {
        static constexpr char const * fragment_shader = R"lang::GLSL(
#version 330 core
in vec2 TexCoord;

out vec4 color;

uniform sampler2D texture_diffuse1;

void main()
{    
    color=vec4(texture(texture_diffuse1,TexCoord));
}
)lang::GLSL";

        static constexpr char const * vertex_shader = R"lang::GLSL(
#version 330 core
layout (location=0) in vec3 position;
layout (location=1) in vec3 normal;
layout (location=2) in vec2 tex_coord;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position=projection*view*model*vec4(position,1.0f);
    TexCoord=tex_coord;
}
)lang::GLSL";
        static constexpr char const * geometry_shader = nullptr;
            };
};
