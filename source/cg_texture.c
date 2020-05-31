#include "cg.h"

void cg_texture_create(CgTexture* texture, void* pixels, int w, int h)
{
    glGenTextures(1,(GLuint*)texture);
    glBindTexture(GL_TEXTURE_2D,(GLuint)*texture);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
}

void cg_texture_select(CgTexture texture, int slot)
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D,(GLuint)texture);
}

void cg_texture_delete(CgTexture* texture)
{
    glDeleteTextures(1,(GLuint*)texture);
    *texture = 0;
}