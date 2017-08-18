namespace NovaBuiltinShaders{

    struct BasicPointCloudShader {
        static constexpr char const * name = "BasicPointCloudShader";
        static constexpr char const * fragment_shader = R"lang::GLSL(
#version 330 core
out vec4 color;

void main()
{    
    color=vec4(1.0f, 1.0f, 1.0f, 1.0f);
}
)lang::GLSL";

        static constexpr char const * vertex_shader = R"lang::GLSL(
#version 330 core
//layout (location = 0) in vec3 position;
layout (location = 0) in vec2 aPos;
//uniform mat4 model;
//uniform mat4 view;
//uniform mat4 projection;

void main()
{
    //gl_Position=projection*view*model*vec4(position,1.0f);
    gl_Position = vec4(aPos.x,aPos.y,0.0,1.0);
}
)lang::GLSL";
        static constexpr char const * geometry_shader = R"lang::GLSL(
#version 330 core
layout (points) in;
layout (line_strip, max_vertices = 2) out;

void main()
{
    gl_Position = gl_in[0].gl_Position + vec4(-0.1, 0.0, 0.0, 0.0);
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4(0.1, 0.0, 0.0, 0.0);
    EmitVertex();

    EndPrimitive();
}
)lang::GLSL";
            };
};
