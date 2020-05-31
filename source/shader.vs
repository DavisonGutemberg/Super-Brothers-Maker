#version 330 core

in vec3 position;
in vec2 texture_position;
out vec2 f_texture_position;

void main()
{
   gl_Position = vec4(position,1.0);
   f_texture_position = texture_position;
};
