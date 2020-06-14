#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <math.h>

#define GAME_RENDERER_MAX_PRIMITIVES 50000
#define GAME_RENDERING_NOTHING    0
#define GAME_RENDERING_PRIMITIVES 1
#define GAME_RENDERING_TEXTURES   2
#define GAME_RENDERING_ENTITYS    3

#define PI 3.14159265359

const char* game_renderer_primitive_vs = "    \n"
    "#version 330 core                        \n"
    "layout (location = 0) in vec2 coord;     \n"
    "layout (location = 1) in vec4 color;     \n"
    "out vec4 f_color;                        \n"
    "uniform vec2 view;                       \n"
    "void main()                              \n"
    "{                                        \n"
    "   gl_Position = vec4((coord.x - view.x) / view.x,(view.y - coord.y) / view.y,0.0,1.0);    \n"
    "   f_color = color;                      \n"
    "}                                        \n";

const char* game_renderer_primitive_fs = " \n"
    "#version 330 core                     \n"
    "in vec4 f_color;                      \n"
    "void main()                           \n"
    "{                                     \n"
    "   gl_FragColor = f_color;            \n" 
    "}                                     \n";


const char* game_renderer_texture_vs = ""
    "#version 330 core\n"
    "layout (location = 0) in vec2 position;\n"
    "layout (location = 1) in vec2 uv;\n"
    "layout (location = 2) in float id;\n"
    "out vec2 f_uv;\n"
    "out float f_id;\n"
    "uniform vec2 view;\n"
    "void main()\n"
    "{\n"
    "   vec2 coord = vec2((position.x - view.x) / view.x,(view.y - position.y) / view.y);\n"
    "   gl_Position = vec4(coord,0.0,1.0);\n"
    "   f_uv = uv;\n"
    "   f_id = id;\n"
    "}\n";

const char* game_renderer_texture_fs = "   \n"
    "#version 330 core                     \n"
    "in vec2 f_uv;\n"
    "in float f_id;\n"
    "uniform sampler2D samplers[16];\n"
    "void main()                           \n"
    "{                                     \n"
    "   gl_FragColor = texture(samplers[int(f_id)],f_uv);  \n" 
    "}                                     \n";


const char* game_renderer_entity_vs[] = {
    "#version 330 core\n",
    "layout (location = 0) in vec2 position;\n",
    "layout (location = 1) in vec4 uvs;\n",
    "layout (location = 2) in float id;\n",
    "out vec4 f_uvs;\n",
    "out float f_id;\n",
    "uniform vec2 view;\n",
    "void main()\n",
    "{\n",
    "   vec2 coord = vec2((position.x - view.x) / view.x,(view.y - position.y) / view.y);\n",
    "   gl_Position = vec4(coord,0.0,1.0);\n",
    "   f_uvs = uvs;\n",
    "   f_id = id;\n",
    "}\n"
};

const char* game_renderer_entity_fs[] = {
    "#version 330 core                     \n",
    "in vec4 f_uvs;\n",
    "in float f_id;\n",
    "uniform sampler2D samplers[16];\n",
    "void main()                           \n",
    "{                                     \n",
    "   vec4 diffuse = texture(samplers[int(f_id)],f_uvs.xy);\n",
    "   vec3 normal = texture(samplers[int(f_id)],f_uvs.zw).rgb;\n"
    "   vec4 ambient_light = diffuse * 0.5;\n",
    "   vec3 light_direction = vec3(0.0,1.0,0.5);\n",
    "   float diffuse_reflection = max(dot(normalize((normal * 2.0) - 1.0),normalize(light_direction)),0.0);\n",
    "   gl_FragColor = ambient_light + (diffuse * diffuse_reflection);\n", 
    "}                                     \n"
};

struct GameInternalRenderer
{
    int w, h;
    unsigned int rendering;
    unsigned short texture_slots_used;
    unsigned short texture_slots_available;
    unsigned short int texture_ids[16];
    struct
    {
        GLuint vao;
        GLuint vbo;
        GLuint ebo;
        GLuint shader;
        struct
        {
            float position[2];
            unsigned char color[4];
        }*vertices;
        int vertex;
        unsigned int* indices;
        int index;
    }primitive;
    struct
    {
        GLuint vao;
        GLuint vbo;
        GLuint ebo;
        GLuint shader;
        struct
        {
            float position[2];
            float uv[2];
            float id;
        }*vertices;
        int vertex;
        unsigned int* indices;
        int index;
    }texture;
    struct
    {
        GLuint vao;
        GLuint vbo;
        GLuint ebo;
        GLuint shader;
        struct
        {
            float position[2];
            float uv[4];
            float id;
        }*vertices;
        int vertex;
        unsigned int* indices;
        int index;
    }entity;
};

/*TODO: join vertices and indices buffers in one malloc*/
int game_renderer_create(GameRenderer** renderer)
{
    GLenum error;
    GLint max;
    GLuint vs;
    GLuint fs;
    int success;
    char log[512];
    int i, index, vertex;
    struct GameInternalRenderer* irenderer = malloc(sizeof(*irenderer));
    if(irenderer == NULL){
        return 0;
    }
    irenderer->w = 0;
    irenderer->h = 0;
    irenderer->rendering = GAME_RENDERING_NOTHING;
    irenderer->texture_slots_used = 0;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS,(GLint*)&irenderer->texture_slots_available);
    glGetIntegerv(GL_MAX_TEXTURE_SIZE,&max);
    

    /*TEXTURES*/
    glGenVertexArrays(1,&irenderer->texture.vao);
    glBindVertexArray(irenderer->texture.vao);

    irenderer->texture.vertices = malloc(sizeof(*irenderer->texture.vertices) * 4 * irenderer->texture_slots_available);
    if(irenderer->texture.vertices == NULL){
        return 0;
    }
    irenderer->texture.vertex = 0;
    
    
    glGenBuffers(1,&irenderer->texture.vbo);
    glBindBuffer(GL_ARRAY_BUFFER,irenderer->texture.vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(*irenderer->texture.vertices) * 4 * irenderer->texture_slots_available,
                 NULL,GL_DYNAMIC_DRAW);
    
    irenderer->texture.indices = malloc(sizeof(*irenderer->texture.indices) * 6 * irenderer->texture_slots_available);
    if(irenderer->texture.indices == NULL){
        return 0;
    }
    irenderer->texture.index = 0;

    for(i = 0, index = 0, vertex = 0; i < irenderer->texture_slots_available; i++, vertex += 4)
    {
        irenderer->texture.indices[index++] = vertex + 0;
        irenderer->texture.indices[index++] = vertex + 1;
        irenderer->texture.indices[index++] = vertex + 2;
        irenderer->texture.indices[index++] = vertex + 0;
        irenderer->texture.indices[index++] = vertex + 2;
        irenderer->texture.indices[index++] = vertex + 3;
    }

    glGenBuffers(1,&irenderer->texture.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,irenderer->texture.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(*irenderer->texture.indices) * 6 * irenderer->texture_slots_available,
                 irenderer->texture.indices,GL_STATIC_DRAW);

    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,sizeof(*irenderer->texture.vertices),(void*)0);
    glEnableVertexAttribArray(0); /*position*/
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(*irenderer->texture.vertices),(void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1); /*uv*/
    glVertexAttribPointer(2,1,GL_FLOAT,GL_FALSE,sizeof(*irenderer->texture.vertices),(void*)(4 * sizeof(float)));
    glEnableVertexAttribArray(2); /*id*/


    vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs,1,(const GLchar**)&game_renderer_texture_vs,NULL);
    glCompileShader(vs);
    glGetShaderiv(vs,GL_COMPILE_STATUS,&success);
    if(!success)
    {
        glGetShaderInfoLog(vs,512,NULL,log);
        printf("texture vs compilation error: %s\n",log);
    }

    fs = glCreateShader(GL_FRAGMENT_SHADER);
    if(fs == 0){
        return 0;
    }
    glShaderSource(fs,1,(const GLchar**)&game_renderer_texture_fs,NULL);
    glCompileShader(fs);
    glGetShaderiv(fs,GL_COMPILE_STATUS,&success);
    if(!success)
    {
        glGetShaderInfoLog(vs,512,NULL,log);
        printf("texture fs compilation error: %s\n",log);
    }

    irenderer->texture.shader = glCreateProgram();

    glAttachShader(irenderer->texture.shader,vs);
    glAttachShader(irenderer->texture.shader,fs);
    glLinkProgram(irenderer->texture.shader);
    glGetProgramiv(irenderer->texture.shader,GL_LINK_STATUS,&success);
    if(!success)
    {
        glGetProgramInfoLog(irenderer->texture.shader,512,NULL,log);
        printf("texture shader linking error: %s\n",log);
    }

    glDetachShader(irenderer->texture.shader,vs);
    glDetachShader(irenderer->texture.shader,fs);
    glDeleteShader(vs);
    glDeleteShader(fs);
    
    
    /*PRIMITIVES*/
    glGenVertexArrays(1,&irenderer->primitive.vao);
    glBindVertexArray(irenderer->primitive.vao);

    irenderer->primitive.vertices = malloc(sizeof(*irenderer->primitive.vertices) * 4 * GAME_RENDERER_MAX_PRIMITIVES);
    if(irenderer->primitive.vertices == NULL){
        return 0;
    }
    irenderer->primitive.vertex = 0;
    
    
    glGenBuffers(1,&irenderer->primitive.vbo);
    glBindBuffer(GL_ARRAY_BUFFER,irenderer->primitive.vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(*irenderer->primitive.vertices) * 4 * GAME_RENDERER_MAX_PRIMITIVES,
                 NULL,GL_DYNAMIC_DRAW);
    
    irenderer->primitive.indices = malloc(sizeof(*irenderer->primitive.indices) * 6 * GAME_RENDERER_MAX_PRIMITIVES);
    if(irenderer->primitive.indices == NULL){
        return 0;
    }
    irenderer->primitive.index = 0;

    for(i = 0, index = 0, vertex = 0; i < GAME_RENDERER_MAX_PRIMITIVES; i++, vertex += 4)
    {
        irenderer->primitive.indices[index++] = vertex + 0;
        irenderer->primitive.indices[index++] = vertex + 1;
        irenderer->primitive.indices[index++] = vertex + 2;
        irenderer->primitive.indices[index++] = vertex + 0;
        irenderer->primitive.indices[index++] = vertex + 2;
        irenderer->primitive.indices[index++] = vertex + 3;
    }

    glGenBuffers(1,&irenderer->primitive.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,irenderer->primitive.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(*irenderer->primitive.indices) * 6 * GAME_RENDERER_MAX_PRIMITIVES,
                 irenderer->primitive.indices,GL_STATIC_DRAW);

    glVertexAttribPointer(0,2,
                          GL_FLOAT,GL_FALSE,sizeof(*irenderer->primitive.vertices),(void*)0);
    glEnableVertexAttribArray(0); /*xy*/

    glVertexAttribPointer(1,4,GL_UNSIGNED_BYTE,GL_TRUE,
                          sizeof(*irenderer->primitive.vertices),(void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1); /*color*/

    vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs,1,(const GLchar**)&game_renderer_primitive_vs,NULL);
    glCompileShader(vs);
    glGetShaderiv(vs,GL_COMPILE_STATUS,&success);
    if(!success)
    {
        glGetShaderInfoLog(vs,512,NULL,log);
        printf("primitive vs compilation error: %s\n",log);
    }

    fs = glCreateShader(GL_FRAGMENT_SHADER);
    if(fs == 0){
        return 0;
    }
    glShaderSource(fs,1,(const GLchar**)&game_renderer_primitive_fs,NULL);
    glCompileShader(fs);
    glGetShaderiv(fs,GL_COMPILE_STATUS,&success);
    if(!success)
    {
        glGetShaderInfoLog(vs,512,NULL,log);
        printf("primitive fs compilation error: %s\n",log);
    }

    irenderer->primitive.shader = glCreateProgram();

    glAttachShader(irenderer->primitive.shader,vs);
    glAttachShader(irenderer->primitive.shader,fs);
    glLinkProgram(irenderer->primitive.shader);
    glGetProgramiv(irenderer->primitive.shader,GL_LINK_STATUS,&success);
    if(!success)
    {
        glGetProgramInfoLog(irenderer->primitive.shader,512,NULL,log);
        printf("primitive shader linking error: %s\n",log);
    }

    glDetachShader(irenderer->primitive.shader,vs);
    glDetachShader(irenderer->primitive.shader,fs);
    glDeleteShader(vs);
    glDeleteShader(fs);
    
    /*ENTITY*/
    glGenVertexArrays(1,&irenderer->entity.vao);
    glBindVertexArray(irenderer->entity.vao);

    irenderer->entity.vertices = malloc(sizeof(*irenderer->entity.vertices) * 4 * irenderer->texture_slots_available);
    if(irenderer->entity.vertices == NULL){
        return 0;
    }
    irenderer->entity.vertex = 0;
    
    
    glGenBuffers(1,&irenderer->entity.vbo);
    glBindBuffer(GL_ARRAY_BUFFER,irenderer->entity.vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(*irenderer->entity.vertices) * 4 * irenderer->texture_slots_available,
                 NULL,GL_DYNAMIC_DRAW);
    
    irenderer->entity.indices = malloc(sizeof(*irenderer->entity.indices) * 6 * irenderer->texture_slots_available);
    if(irenderer->entity.indices == NULL){
        return 0;
    }
    irenderer->entity.index = 0;

    for(i = 0, index = 0, vertex = 0; i < irenderer->texture_slots_available; i++, vertex += 4)
    {
        irenderer->entity.indices[index++] = vertex + 0;
        irenderer->entity.indices[index++] = vertex + 1;
        irenderer->entity.indices[index++] = vertex + 2;
        irenderer->entity.indices[index++] = vertex + 0;
        irenderer->entity.indices[index++] = vertex + 2;
        irenderer->entity.indices[index++] = vertex + 3;
    }

    glGenBuffers(1,&irenderer->entity.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,irenderer->entity.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(*irenderer->entity.indices) * 6 * irenderer->texture_slots_available,
                 irenderer->entity.indices,GL_STATIC_DRAW);

    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,sizeof(*irenderer->entity.vertices),(void*)0);
    glEnableVertexAttribArray(0); /*position*/
    glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,sizeof(*irenderer->entity.vertices),(void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1); /*uv*/
    glVertexAttribPointer(2,1,GL_FLOAT,GL_FALSE,sizeof(*irenderer->entity.vertices),(void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2); /*id*/


    vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs,sizeof(game_renderer_entity_vs)/sizeof(char*),(const GLchar**)game_renderer_entity_vs,NULL);
    glCompileShader(vs);
    glGetShaderiv(vs,GL_COMPILE_STATUS,&success);
    if(!success)
    {
        glGetShaderInfoLog(vs,512,NULL,log);
        printf("entity vs compilation error: %s\n",log);
    }

    fs = glCreateShader(GL_FRAGMENT_SHADER);
    if(fs == 0){
        return 0;
    }
    glShaderSource(fs,sizeof(game_renderer_entity_fs)/sizeof(char*),(const GLchar**)game_renderer_entity_fs,NULL);
    glCompileShader(fs);
    glGetShaderiv(fs,GL_COMPILE_STATUS,&success);
    if(!success)
    {
        glGetShaderInfoLog(vs,512,NULL,log);
        printf("entity fs compilation error: %s\n",log);
    }

    irenderer->entity.shader = glCreateProgram();

    glAttachShader(irenderer->entity.shader,vs);
    glAttachShader(irenderer->entity.shader,fs);
    glLinkProgram(irenderer->entity.shader);
    glGetProgramiv(irenderer->entity.shader,GL_LINK_STATUS,&success);
    if(!success)
    {
        glGetProgramInfoLog(irenderer->entity.shader,512,NULL,log);
        printf("texture shader linking error: %s\n",log);
    }

    glDetachShader(irenderer->entity.shader,vs);
    glDetachShader(irenderer->entity.shader,fs);
    glDeleteShader(vs);
    glDeleteShader(fs);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    *renderer = (GameRenderer*)irenderer;
    return 1;
}


int game_renderer_create_atlas(GameRenderer* renderer, GameAtlas* atlas, void* pixels, int w, int h, int channels)
{
    struct GameInternalRenderer* irenderer = (struct GameInternalRenderer*)renderer;

    glGenTextures(1,&atlas->id);
    glBindTexture(GL_TEXTURE_2D,atlas->id);
    if(channels == 4){
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,pixels);
    }
    else{
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,w,h,0,GL_RGB,GL_UNSIGNED_BYTE,pixels);
    }
    glGenerateMipmap(GL_TEXTURE_2D); /*Should we do it?*/
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); /*change to nearest*/
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    atlas->w = w;
    atlas->h = h;

    return 1;
}


int game_renderer_create_texture(GameRenderer* renderer, GameTexture* texture, GameAtlas* atlas, int x, int y, int w, int h)
{
    struct GameInternalRenderer* irenderer = (struct GameInternalRenderer*)renderer;
    
    texture->id = atlas->id;
    texture->x1 = x / (float)atlas->w;
    texture->y1 = y / (float)atlas->h;
    texture->x2 = (x + w) / (float)atlas->w;
    texture->y2 = (y + h) / (float)atlas->h;
    texture->w = w;
    texture->h = h;
    
    return 1; 
}


void game_renderer_clear(GameRenderer* renderer, int w, int h)
{
    struct GameInternalRenderer* irenderer = (struct GameInternalRenderer*)renderer;
    glViewport(0,0,w,h);
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT);
    
    irenderer->w = w / 2;
    irenderer->h = h / 2;
}


void game_renderer_draw_line(GameRenderer* renderer, int x1, int y1, int x2, int y2, int r, int g, int b, int a)
{
    /*Not working yet.*/
}


void game_renderer_draw_circle(GameRenderer* renderer, int x, int y, int radius, int r, int b, int g, int a)
{
    int i;
    struct GameInternalRenderer* irenderer = (struct GameInternalRenderer*)renderer;
    int amount = 6 * (radius / 6.0);
    float step = (PI * 2) / amount;
    int cv = irenderer->primitive.vertex;

    if(irenderer->rendering != GAME_RENDERING_NOTHING && irenderer->rendering != GAME_RENDERING_PRIMITIVES){
        game_renderer_present(renderer);
    }
    irenderer->rendering = GAME_RENDERING_PRIMITIVES;

    irenderer->primitive.vertices[irenderer->primitive.vertex].position[0] = (float)x;
    irenderer->primitive.vertices[irenderer->primitive.vertex].position[1] = (float)y;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[0] = 255;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[1] = g;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[2] = b;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[3] = a;
    irenderer->primitive.vertex++;
    irenderer->primitive.vertices[irenderer->primitive.vertex].position[0] = (float)x + radius;
    irenderer->primitive.vertices[irenderer->primitive.vertex].position[1] = (float)y;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[0] = r;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[1] = g;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[2] = b;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[3] = a;
    irenderer->primitive.vertex++;

    for(i = 0; i < amount; i += 2)
    {
        /*Its broken because the center is a vertex common to the others.*/
        /*
        irenderer->primitive.indices[irenderer->primitive.index++] = cv + 0;
        irenderer->primitive.indices[irenderer->primitive.index++] = cv + i + 1;
        irenderer->primitive.indices[irenderer->primitive.index++] = cv + i + 2;
        irenderer->primitive.indices[irenderer->primitive.index++] = cv + 0;
        irenderer->primitive.indices[irenderer->primitive.index++] = cv + i + 2;
        irenderer->primitive.indices[irenderer->primitive.index++] = cv + i + 3;*/
        irenderer->primitive.index += 6;
        
        irenderer->primitive.vertices[irenderer->primitive.vertex].position[0] = (float)x + (radius * cos((i + 1) * step));
        irenderer->primitive.vertices[irenderer->primitive.vertex].position[1] = (float)y - (radius * sin((i + 1) * step));
        irenderer->primitive.vertices[irenderer->primitive.vertex].color[0] = r;
        irenderer->primitive.vertices[irenderer->primitive.vertex].color[1] = g;
        irenderer->primitive.vertices[irenderer->primitive.vertex].color[2] = b;
        irenderer->primitive.vertices[irenderer->primitive.vertex].color[3] = a;
        irenderer->primitive.vertex++;

        irenderer->primitive.vertices[irenderer->primitive.vertex].position[0] = (float)x + (radius * cos((i + 2) * step));
        irenderer->primitive.vertices[irenderer->primitive.vertex].position[1] = (float)y - (radius * sin((i + 2) * step));
        irenderer->primitive.vertices[irenderer->primitive.vertex].color[0] = r;
        irenderer->primitive.vertices[irenderer->primitive.vertex].color[1] = g;
        irenderer->primitive.vertices[irenderer->primitive.vertex].color[2] = b;
        irenderer->primitive.vertices[irenderer->primitive.vertex].color[3] = a;
        irenderer->primitive.vertex++;
    }
}


void game_renderer_draw_triangle(GameRenderer* renderer, int x1, int y1, int x2, int y2, int x3, int y3, int r, int g, int b, int a)
{
    struct GameInternalRenderer* irenderer = (struct GameInternalRenderer*)renderer;
    if(irenderer->rendering != GAME_RENDERING_NOTHING && irenderer->rendering != GAME_RENDERING_PRIMITIVES){
        game_renderer_present(renderer);
    }
    irenderer->rendering = GAME_RENDERING_PRIMITIVES;

    irenderer->primitive.index += 6;
    
    
    irenderer->primitive.vertices[irenderer->primitive.vertex].position[0] = (float)x1;
    irenderer->primitive.vertices[irenderer->primitive.vertex].position[1] = (float)y1;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[0] = r;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[1] = g;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[2] = b;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[3] = a;
    irenderer->primitive.vertex++;

    irenderer->primitive.vertices[irenderer->primitive.vertex].position[0] = (float)x1;
    irenderer->primitive.vertices[irenderer->primitive.vertex].position[1] = (float)y1;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[0] = r;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[1] = g;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[2] = b;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[3] = a;
    irenderer->primitive.vertex++;

    irenderer->primitive.vertices[irenderer->primitive.vertex].position[0] = (float)x2;
    irenderer->primitive.vertices[irenderer->primitive.vertex].position[1] = (float)y2;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[0] = r;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[1] = g;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[2] = b;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[3] = a;
    irenderer->primitive.vertex++;

    irenderer->primitive.vertices[irenderer->primitive.vertex].position[0] = (float)x3;
    irenderer->primitive.vertices[irenderer->primitive.vertex].position[1] = (float)y3;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[0] = r;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[1] = g;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[2] = b;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[3] = a;
    irenderer->primitive.vertex++;
}


void game_renderer_draw_rectangle(GameRenderer* renderer,int x, int y, int w, int h, int r, int g, int b, int a)
{
    struct GameInternalRenderer* irenderer = (struct GameInternalRenderer*)renderer;
    if(irenderer->rendering != GAME_RENDERING_NOTHING && irenderer->rendering != GAME_RENDERING_PRIMITIVES){
        game_renderer_present(renderer);
    }
    irenderer->rendering = GAME_RENDERING_PRIMITIVES;
    w += x;
    h += y;

    irenderer->primitive.index += 6;
    
    irenderer->primitive.vertices[irenderer->primitive.vertex].position[0] = (float)x;
    irenderer->primitive.vertices[irenderer->primitive.vertex].position[1] = (float)y;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[0] = r;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[1] = g;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[2] = b;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[3] = a;
    irenderer->primitive.vertex++;

    irenderer->primitive.vertices[irenderer->primitive.vertex].position[0] = (float)x;
    irenderer->primitive.vertices[irenderer->primitive.vertex].position[1] = (float)h;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[0] = r;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[1] = g;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[2] = b;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[3] = a;
    irenderer->primitive.vertex++;

    irenderer->primitive.vertices[irenderer->primitive.vertex].position[0] = (float)w;
    irenderer->primitive.vertices[irenderer->primitive.vertex].position[1] = (float)h;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[0] = r;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[1] = g;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[2] = b;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[3] = a;
    irenderer->primitive.vertex++;

    irenderer->primitive.vertices[irenderer->primitive.vertex].position[0] = (float)w;
    irenderer->primitive.vertices[irenderer->primitive.vertex].position[1] = (float)y;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[0] = r;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[1] = g;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[2] = b;
    irenderer->primitive.vertices[irenderer->primitive.vertex].color[3] = a;
    irenderer->primitive.vertex++;
}


void game_renderer_draw_texture(GameRenderer* renderer, GameTexture* texture, int x, int y, float sx, float sy, float angle)
{
    int i = 0;
    double c, s;
    float tx, ty;
    float cx, cy;
    float w, h;
    
    int id;
    struct GameInternalRenderer* irenderer = (struct GameInternalRenderer*)renderer;
    if(irenderer->rendering != GAME_RENDERING_NOTHING && irenderer->rendering != GAME_RENDERING_TEXTURES){
        game_renderer_present(renderer);
    }
    irenderer->rendering = GAME_RENDERING_TEXTURES;

    while(i < irenderer->texture_slots_used)
    {
        if(texture->id == irenderer->texture_ids[i])
        {
            id = i;
            break;
        }
        i++;
    }
    if(i == irenderer->texture_slots_used)
    {
        id = irenderer->texture_slots_used;
        irenderer->texture_ids[irenderer->texture_slots_used] = texture->id;
        irenderer->texture_slots_used++;
    }
    if(irenderer->texture_slots_used == 16)
    {
        game_renderer_present(renderer);
        id = 0;
        irenderer->texture_ids[0] = texture->id;
        irenderer->texture_slots_used++;
    }

    c = cos(angle * (PI / 180.0));
    s = sin(angle * (PI / 180.0));
    w = texture->w * sx;
    h = texture->h * sy;
    tx = x + (w / 2);
    ty = y + (h / 2);
    x *= sx;
    y *= sy;
    cx = x + (w / 2);
    cy = y + (h / 2);
    
    irenderer->texture.index += 6;

    irenderer->texture.vertices[irenderer->texture.vertex].position[0] = ((x - cx) * c) - ((y - cy) * s) + tx;
    irenderer->texture.vertices[irenderer->texture.vertex].position[1] = ((x - cx) * s) + ((y - cy) * c) + ty;
    irenderer->texture.vertices[irenderer->texture.vertex].uv[0] = texture->x1;
    irenderer->texture.vertices[irenderer->texture.vertex].uv[1] = texture->y1;
    irenderer->texture.vertices[irenderer->texture.vertex].id = id;
    irenderer->texture.vertex++;

    irenderer->texture.vertices[irenderer->texture.vertex].position[0] = ((x - cx) * c) - ((y + h - cy) * s) + tx;
    irenderer->texture.vertices[irenderer->texture.vertex].position[1] = ((x - cx) * s) + ((y + h - cy) * c) + ty;
    irenderer->texture.vertices[irenderer->texture.vertex].uv[0] = texture->x1;
    irenderer->texture.vertices[irenderer->texture.vertex].uv[1] = texture->y2;
    irenderer->texture.vertices[irenderer->texture.vertex].id = id;
    irenderer->texture.vertex++;

    irenderer->texture.vertices[irenderer->texture.vertex].position[0] = ((x + w - cx) * c) - ((y + h - cy) * s) + tx;
    irenderer->texture.vertices[irenderer->texture.vertex].position[1] = ((x + w - cx) * s) + ((y + h - cy) * c) + ty;
    irenderer->texture.vertices[irenderer->texture.vertex].uv[0] = texture->x2;
    irenderer->texture.vertices[irenderer->texture.vertex].uv[1] = texture->y2;
    irenderer->texture.vertices[irenderer->texture.vertex].id = id;
    irenderer->texture.vertex++;

    irenderer->texture.vertices[irenderer->texture.vertex].position[0] = ((x + w - cx) * c) - ((y - cy) * s) + tx;
    irenderer->texture.vertices[irenderer->texture.vertex].position[1] = ((x + w - cx) * s) + ((y - cy) * c) + ty;
    irenderer->texture.vertices[irenderer->texture.vertex].uv[0] = texture->x2;
    irenderer->texture.vertices[irenderer->texture.vertex].uv[1] = texture->y1;
    irenderer->texture.vertices[irenderer->texture.vertex].id = id;
    irenderer->texture.vertex++;
}


void game_renderer_draw_entity(GameRenderer* renderer, GameTexture* diffuse, GameTexture* normal, int x, int y, float sx, float sy, float angle)
{
    int i = 0;
    double c, s;
    float tx, ty;
    float cx, cy;
    float w, h;
    
    int id;
    struct GameInternalRenderer* irenderer = (struct GameInternalRenderer*)renderer;
    if(irenderer->rendering != GAME_RENDERING_NOTHING && irenderer->rendering != GAME_RENDERING_ENTITYS){
        game_renderer_present(renderer);
    }
    irenderer->rendering = GAME_RENDERING_ENTITYS;

    while(i < irenderer->texture_slots_used)
    {
        if(diffuse->id == irenderer->texture_ids[i])
        {
            id = i;
            break;
        }
        i++;
    }
    if(i == irenderer->texture_slots_used)
    {
        id = irenderer->texture_slots_used;
        irenderer->texture_ids[irenderer->texture_slots_used] = diffuse->id;
        irenderer->texture_slots_used++;
    }
    if(irenderer->texture_slots_used == 16)
    {
        game_renderer_present(renderer);
        id = 0;
        irenderer->texture_ids[0] = diffuse->id;
        irenderer->texture_slots_used++;
    }

    c = cos(angle * (PI / 180.0));
    s = sin(angle * (PI / 180.0));
    w = diffuse->w * sx;
    h = diffuse->h * sy;
    tx = x + (w / 2);
    ty = y + (h / 2);
    x *= sx;
    y *= sy;
    cx = x + (w / 2);
    cy = y + (h / 2);
    
    irenderer->entity.index += 6;

    irenderer->entity.vertices[irenderer->entity.vertex].position[0] = ((x - cx) * c) - ((y - cy) * s) + tx;
    irenderer->entity.vertices[irenderer->entity.vertex].position[1] = ((x - cx) * s) + ((y - cy) * c) + ty;
    irenderer->entity.vertices[irenderer->entity.vertex].uv[0] = diffuse->x1;
    irenderer->entity.vertices[irenderer->entity.vertex].uv[1] = diffuse->y1;
    irenderer->entity.vertices[irenderer->entity.vertex].uv[2] = normal->x1;
    irenderer->entity.vertices[irenderer->entity.vertex].uv[3] = normal->y1;
    irenderer->entity.vertices[irenderer->entity.vertex].id = id;
    irenderer->entity.vertex++;

    irenderer->entity.vertices[irenderer->entity.vertex].position[0] = ((x - cx) * c) - ((y + h - cy) * s) + tx;
    irenderer->entity.vertices[irenderer->entity.vertex].position[1] = ((x - cx) * s) + ((y + h - cy) * c) + ty;
    irenderer->entity.vertices[irenderer->entity.vertex].uv[0] = diffuse->x1;
    irenderer->entity.vertices[irenderer->entity.vertex].uv[1] = diffuse->y2;
    irenderer->entity.vertices[irenderer->entity.vertex].uv[2] = normal->x1;
    irenderer->entity.vertices[irenderer->entity.vertex].uv[3] = normal->y2;
    irenderer->entity.vertices[irenderer->entity.vertex].id = id;
    irenderer->entity.vertex++;

    irenderer->entity.vertices[irenderer->entity.vertex].position[0] = ((x + w - cx) * c) - ((y + h - cy) * s) + tx;
    irenderer->entity.vertices[irenderer->entity.vertex].position[1] = ((x + w - cx) * s) + ((y + h - cy) * c) + ty;
    irenderer->entity.vertices[irenderer->entity.vertex].uv[0] = diffuse->x2;
    irenderer->entity.vertices[irenderer->entity.vertex].uv[1] = diffuse->y2;
    irenderer->entity.vertices[irenderer->entity.vertex].uv[2] = normal->x2;
    irenderer->entity.vertices[irenderer->entity.vertex].uv[3] = normal->y2;
    irenderer->entity.vertices[irenderer->entity.vertex].id = id;
    irenderer->entity.vertex++;

    irenderer->entity.vertices[irenderer->entity.vertex].position[0] = ((x + w - cx) * c) - ((y - cy) * s) + tx;
    irenderer->entity.vertices[irenderer->entity.vertex].position[1] = ((x + w - cx) * s) + ((y - cy) * c) + ty;
    irenderer->entity.vertices[irenderer->entity.vertex].uv[0] = diffuse->x2;
    irenderer->entity.vertices[irenderer->entity.vertex].uv[1] = diffuse->y1;
    irenderer->entity.vertices[irenderer->entity.vertex].uv[2] = normal->x2;
    irenderer->entity.vertices[irenderer->entity.vertex].uv[3] = normal->y1;
    irenderer->entity.vertices[irenderer->entity.vertex].id = id;
    irenderer->entity.vertex++;
}

void game_renderer_present(GameRenderer* renderer)
{
    int i;
    GLenum error;
    struct GameInternalRenderer* irenderer = (struct GameInternalRenderer*)renderer;

    if(irenderer->rendering == GAME_RENDERING_PRIMITIVES)
    {
        glUseProgram(irenderer->primitive.shader);
        glUniform2f(glGetUniformLocation(irenderer->primitive.shader,"view"),(float)irenderer->w,(float)irenderer->h);
        glBindVertexArray(irenderer->primitive.vao);
        glBindBuffer(GL_ARRAY_BUFFER,irenderer->primitive.vbo);
        glBufferSubData(GL_ARRAY_BUFFER,0,
                        sizeof(*irenderer->primitive.vertices) * irenderer->primitive.vertex,
                        irenderer->primitive.vertices);
        
        /*glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);*/
        glDrawElements(GL_TRIANGLES,irenderer->primitive.index,GL_UNSIGNED_INT,(void*)0);
        
        irenderer->primitive.vertex = 0;
        irenderer->primitive.index = 0;
        irenderer->rendering = GAME_RENDERING_NOTHING;
    }
    else if(irenderer->rendering == GAME_RENDERING_TEXTURES)
    {
        int samplers[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
        int result;
        glUseProgram(irenderer->texture.shader);
        
        glUniform2f(glGetUniformLocation(irenderer->texture.shader,"view"),(float)irenderer->w,(float)irenderer->h);
           
        for(i = 0; i < irenderer->texture_slots_used; i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D,(GLuint)irenderer->texture_ids[i]);
        }
        glUniform1iv(glGetUniformLocation(irenderer->texture.shader,"samplers"),irenderer->texture_slots_used,samplers);

        glBindVertexArray(irenderer->texture.vao);
        glBindBuffer(GL_ARRAY_BUFFER,irenderer->texture.vbo);
        glBufferSubData(GL_ARRAY_BUFFER,0,
                        sizeof(*irenderer->texture.vertices) * irenderer->texture.vertex,
                        irenderer->texture.vertices);
        
        glDrawElements(GL_TRIANGLES,irenderer->texture.index,GL_UNSIGNED_INT,(void*)0);
        irenderer->texture.vertex = 0;
        irenderer->texture.index = 0;
        irenderer->texture_slots_used = 0;
        irenderer->rendering = GAME_RENDERING_NOTHING;
    }
    else if(irenderer->rendering == GAME_RENDERING_ENTITYS)
    {
        int samplers[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
        int result;
        glUseProgram(irenderer->entity.shader);
        
        glUniform2f(glGetUniformLocation(irenderer->entity.shader,"view"),(float)irenderer->w,(float)irenderer->h);
           
        for(i = 0; i < irenderer->texture_slots_used; i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D,(GLuint)irenderer->texture_ids[i]);
        }
        glUniform1iv(glGetUniformLocation(irenderer->entity.shader,"samplers"),irenderer->texture_slots_used,samplers);

        glBindVertexArray(irenderer->entity.vao);
        glBindBuffer(GL_ARRAY_BUFFER,irenderer->entity.vbo);
        glBufferSubData(GL_ARRAY_BUFFER,0,
                        sizeof(*irenderer->entity.vertices) * irenderer->entity.vertex,
                        irenderer->entity.vertices);
        
        glDrawElements(GL_TRIANGLES,irenderer->entity.index,GL_UNSIGNED_INT,(void*)0);
        irenderer->entity.vertex = 0;
        irenderer->entity.index = 0;
        irenderer->texture_slots_used = 0;
        irenderer->rendering = GAME_RENDERING_NOTHING;
    }
}


int game_renderer_delete(GameRenderer* renderer)
{

}