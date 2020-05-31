#version 330 core

in vec2 f_texture_position;
uniform sampler2D diffusemap;
uniform sampler2D normalmap;
uniform vec3 light_position;
uniform vec2 resolution;

void main()
{
   vec4  diffuse = texture(diffusemap,vec2(f_texture_position.x,1.0 - f_texture_position.y));
   vec3  normal = texture(normalmap,vec2(f_texture_position.x,1.0 - f_texture_position.y)).rgb;
   vec3  light_direction = vec3(light_position.xy - (gl_FragCoord.xy/resolution.xy),light_position.z);
   float light_distance = length(light_direction);
   float diffuse_reflection = max(dot(normalize((normal * 2.0) - 1.0),normalize(light_direction)),0.0);
   float light_falloff = 1.0/pow(light_distance,2);
   vec4 ambient_light = diffuse * 0.1;
   gl_FragColor = vec4(ambient_light + (diffuse * diffuse_reflection * light_falloff));
}
