namespace NovaBuiltinShaders{

    struct BasicPointCloudShader {
        static constexpr char const * name = "BasicPointCloudShader";
        static constexpr char const * fragment_shader = R"lang::GLSL(
#version 330 core
in vec4 colorV;
out vec4 color;

void main()
{    
    color=colorV;
}
)lang::GLSL";

        static constexpr char const * vertex_shader = R"lang::GLSL(
#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 node_color;
out vec4 colorV;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position=projection*view*model*vec4(position,1.0f);
    colorV=vec4(node_color,1.0f);
}
)lang::GLSL";
        static constexpr char const * geometry_shader = nullptr;
    };
};
