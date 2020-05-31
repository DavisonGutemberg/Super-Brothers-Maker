#ifndef CG_H
#define CG_H

#include <GL/glew.h>

typedef GLuint CgShader;
typedef GLuint CgTexture;

int  cg_shader_create(CgShader* shader, char* vs, char* fs);
void cg_shader_select(CgShader shader);
void cg_shader_set_uniform1f(CgShader shader, char* uniform, float f1);
void cg_shader_set_uniform2f(CgShader shader, char* uniform, float f1, float f2);
void cg_shader_set_uniform3f(CgShader shader, char* uniform, float f1, float f2, float f3);
void cg_shader_set_uniform4f(CgShader shader, char* uniform, float f1, float f2, float f3, float f4);
void cg_shader_set_uniform1i(CgShader shader, char* uniform, int i1);
void cg_shader_set_uniform2i(CgShader shader, char* uniform, int i1, int i2);
void cg_shader_set_uniform3i(CgShader shader, char* uniform, int i1, int i2, int i3);
void cg_shader_set_uniform4i(CgShader shader, char* uniform, int i1, int i2, int i3, int i4);
void cg_shader_set_uniform1u(CgShader shader, char* uniform, int u1);
void cg_shader_set_uniform2u(CgShader shader, char* uniform, unsigned int u1, unsigned int u2);
void cg_shader_set_uniform3u(CgShader shader, char* uniform, unsigned int u1, unsigned int u2, unsigned int u3);
void cg_shader_set_uniform4u(CgShader shader, char* uniform, unsigned int u1, unsigned int u2, unsigned int u3, unsigned int u4);
int  cg_shader_delete(CgShader* shader);

void cg_texture_create(CgTexture* texture, void* pixels, int w, int h);
void cg_texture_select(CgTexture texture, int slot);
void cg_texture_delete(CgTexture* texture);

#endif