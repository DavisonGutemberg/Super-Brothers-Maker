#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <io.h>
#include "cg.h"


float vertices[] = {
     0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
     0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
    -0.5f,  0.5f, 0.0f, 0.0f, 1.0f
};

unsigned int indices[] = {
    0, 1, 3,
    1, 2, 3 
};  


int main(int argc, char* argv[]) /*Nintendo me dá coisas grátis!*/
{
    GLuint vbo;
    GLuint vao;
    GLuint ebo;
    CgTexture texture_diffuse;
    CgTexture texture_normal;
    CgShader shader;
    int uniform_color;
    int game_main_quit = 0;
    void* pixels;
    int mx, my;
    int w, h, channels;
    FILE* file;
    int size;
    char* fsb;
    char* vsb;
    SDL_Window* game_main_window;
    SDL_Event game_main_event;
    SDL_GLContext* game_main_context;
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,8);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,8);

    game_main_window = SDL_CreateWindow("Renderer",50,50,640,480,SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    
    game_main_context = SDL_GL_CreateContext(game_main_window);
    glewInit();

    glViewport(0,0,640,480);

    file = fopen("source/shader.fs","rb");
    if(file == NULL){
        printf("file null\n");
    }
    fseek(file,0,SEEK_END);
    size = ftell(file);
    fseek(file,0,SEEK_SET);
    fsb = malloc(size + 1);
    if(fsb == NULL){
        printf("malloc failed\n");
    }
    fread(fsb,size,1,file);
    fsb[size] = '\0';
    printf("%s\n",fsb);

    file = fopen("source/shader.vs","rb");
    if(file == NULL){
        printf("file null\n");
    }
    fseek(file,0,SEEK_END);
    size = ftell(file);
    fseek(file,0,SEEK_SET);
    vsb = malloc(size + 1);
    if(vsb == NULL){
        printf("malloc failed\n");
    }
    fread(vsb,size,1,file);
    vsb[size] = '\0';
    printf("%s\n",vsb);


    cg_shader_create(&shader,vsb,fsb);

    glGenVertexArrays(1,&vao);
    glGenBuffers(1,&vbo);
    glGenBuffers(1,&ebo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);

    glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(indices),indices,GL_STATIC_DRAW);

    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,5 * sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,5 * sizeof(float),(void*)(3 *sizeof(float)));
    glEnableVertexAttribArray(1);
    
    pixels = io_image_load("brick_diffuse_map.png",&w,&h,&channels);
    if(pixels == NULL){
        printf("failed to load image\n");
    }
    printf("image: %i %i %i\n",w,h,channels);
    cg_texture_create(&texture_diffuse,pixels,w,h);
    io_image_unload(pixels);
    
    pixels = io_image_load("brick_normal_map.png",&w,&h,&channels);
    if(pixels == NULL){
        printf("failed to load image\n");
    }
    printf("image: %i %i %i\n",w,h,channels);
    cg_texture_create(&texture_normal,pixels,w,h);
    io_image_unload(pixels);

    while(!game_main_quit)
    {
        SDL_GetMouseState(&mx,&my);
        while(SDL_PollEvent(&game_main_event))
        {
            if(game_main_event.type == SDL_QUIT){
                game_main_quit = 1;
            }
        }
        glClearColor(0,0,0,1);
        glClear(GL_COLOR_BUFFER_BIT);

        cg_texture_select(texture_diffuse,0);
        cg_texture_select(texture_normal,1);
        cg_shader_select(shader);
        cg_shader_set_uniform1i(shader,"diffusemap",0);
        cg_shader_set_uniform1i(shader,"normalmap",1);
        cg_shader_set_uniform2f(shader,"resolution",640.0,480.0);
        cg_shader_set_uniform3f(shader,"light_position",(float)mx/640.0,1.0 -(float)my/480.0,0.8);
        
        
        glBindVertexArray(vao);
        /*glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);*/
        glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);

        SDL_GL_SwapWindow(game_main_window);
    }

    return 0;
}