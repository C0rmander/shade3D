// using GLSL version 4.5

#version 450

layout (location = 0) out vec3 outColour;

const vec3 position[3] = vec3[3](
                                 vec3(1.f,1.f,0.0f),
                                 vec3(-1.f,1.f, 0.0f),
                                 vec3(0.f, -1.f, 0.0f)
                                 );

const vec3 colours[3] = vec3[3](
                                vec3(1.0f,0.0f,0.0f), // R
                                vec3(0.0f,1.0f, 0.0f), //G
                                vec3(00.f, 0.0f, 1.0f) //B
                                );
void main()
{


    //output position of each vertex
    gl_Position = vec4(position[gl_VertexIndex], 1.0f);
    outColour = colours[gl_VertexIndex];
}
