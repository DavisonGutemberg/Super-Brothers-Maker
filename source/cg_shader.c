#include <stdio.h>
#include "cg.h"


int cg_shader_create(CgShader* shader, char* vs, char* fs)
{
    GLint  success;
    GLuint final_shader;
    GLuint vertex_shader;
    GLuint fragment_shader;
    GLchar log[512];

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    if(vertex_shader == 0){
        return 0;
    }
    glShaderSource(vertex_shader,1,(const GLchar**)&vs,NULL);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader,GL_COMPILE_STATUS,&success);
    if(!success)
    {
        glGetShaderInfoLog(vertex_shader,512,NULL,log);
        printf("vs compilation error: %s\n",log);
    }

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    if(fragment_shader == 0){
        return 0;
    }
    glShaderSource(fragment_shader,1,(const GLchar**)&fs,NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader,GL_COMPILE_STATUS,&success);
    if(!success)
    {
        glGetShaderInfoLog(fragment_shader,512,NULL,log);
        printf("fs compilation error: %s\n",log);
    }

    final_shader = glCreateProgram();
    if(final_shader == 0){
        return 0;
    }
    glAttachShader(final_shader,vertex_shader);
    glAttachShader(final_shader,fragment_shader);
    glLinkProgram(final_shader);
    glGetProgramiv(final_shader,GL_LINK_STATUS,&success);
    if(!success)
    {
        glGetProgramInfoLog(final_shader,512,NULL,log);
        printf("shader linking error: %s\n",log);
    }

    glDetachShader(final_shader,vertex_shader);
    glDetachShader(final_shader,fragment_shader);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    
    *shader = final_shader;
    return 1;
}


void cg_shader_select(CgShader shader)
{
    glUseProgram((GLuint)shader);
}


void cg_shader_set_uniform1f(CgShader shader, char* uniform, float f1)
{
    glUniform1f(glGetUniformLocation((GLuint)shader,uniform),f1);
}


void cg_shader_set_uniform2f(CgShader shader, char* uniform, float f1, float f2)
{
    glUniform2f(glGetUniformLocation((GLuint)shader,uniform),f1,f2);
}


void cg_shader_set_uniform3f(CgShader shader, char* uniform, float f1, float f2, float f3)
{
    glUniform3f(glGetUniformLocation((GLuint)shader,uniform),f1,f2,f3);
}


void cg_shader_set_uniform4f(CgShader shader, char* uniform, float f1, float f2, float f3, float f4)
{
    glUniform4f(glGetUniformLocation((GLuint)shader,uniform),f1,f2,f3,f4);
}


void cg_shader_set_uniform1i(CgShader shader, char* uniform, int i1)
{
    glUniform1i(glGetUniformLocation((GLuint)shader,uniform),i1);
}


void cg_shader_set_uniform2i(CgShader shader, char* uniform, int i1, int i2)
{
    glUniform2i(glGetUniformLocation((GLuint)shader,uniform),i1,i2);
}


void cg_shader_set_uniform3i(CgShader shader, char* uniform, int i1, int i2, int i3)
{
    glUniform3i(glGetUniformLocation((GLuint)shader,uniform),i1,i2,i3);
}


void cg_shader_set_uniform4i(CgShader shader, char* uniform, int i1, int i2, int i3, int i4)
{
    glUniform4i(glGetUniformLocation((GLuint)shader,uniform),i1,i2,i3,i4);
}


void cg_shader_set_uniform1u(CgShader shader, char* uniform, int u1)
{
    glUniform1ui(glGetUniformLocation((GLuint)shader,uniform),u1);
}


void cg_shader_set_uniform2u(CgShader shader, char* uniform, unsigned int u1, unsigned int u2)
{
    glUniform2ui(glGetUniformLocation((GLuint)shader,uniform),u1,u2);
}


void cg_shader_set_uniform3u(CgShader shader, char* uniform, unsigned int u1, unsigned int u2, unsigned int u3)
{
    glUniform3ui(glGetUniformLocation((GLuint)shader,uniform),u1,u2,u3);
}


void cg_shader_set_uniform4u(CgShader shader, char* uniform, unsigned int u1, unsigned int u2, unsigned int u3, unsigned int u4)
{
    glUniform4ui(glGetUniformLocation((GLuint)shader,uniform),u1,u2,u3,u4);
}


int cg_shader_delete(CgShader* shader)
{
    glDeleteProgram((GLuint)*shader);
    *shader = 0;
    return 1;
}
