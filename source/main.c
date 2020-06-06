#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <os.h>
#include <io.h>
#include "game.h"
#include <GL/glew.h>

int main(int argc, char* argv[]) /*Nintendo me dá coisas grátis!*/
{
    OsEvent event;
    OsWindow* game_main_window;
    GameRenderer* renderer;
    GameAtlas atlas1;
    GameAtlas atlas2;
    GameTexture tex1;
    GameTexture tex2;
    GameTexture tex3;
    
    int game_quit = 0;

    int i, j;
    void* pixels;
    int w, h, channels;
    int mx, my;
    int bx,by;
    unsigned long t1, t2;
    os_init();

    if(!os_window_open(&game_main_window,"スーパブラザーズメーカ",50,50,640,480,0)){
        printf("game main window null\n");
    }
    os_opengl_create(game_main_window);
    if(glewInit() != GLEW_OK) /*Do it ourselves latter.*/
    {
        printf("glew error\n");
    }
    if(!game_renderer_create(&renderer)){
        printf("could not create renderer\n");
    }
    
    /*
    pixels = io_image_load("icon.png",&w,&h,&channels);
    if(pixels == NULL){
        printf("could not set icon\n");
    }
    else
    {
        os_window_set_icon(game_main_window,pixels,w,h);
        io_image_unload(pixels);
    }
    */
    
   
    bx = 0; 
    by = 0;
    pixels = io_image_load("icon.png",&w,&h,&channels);
    if(pixels == NULL){
        printf("could not load texture\n");
        return 0;
    }
    game_renderer_create_atlas(renderer,&atlas1,pixels,w,h,channels);
    
    pixels = io_image_load("brick_normal_map.png",&w,&h,&channels);
    if(pixels == NULL){
        printf("could not load texture\n");
        return 0;
    }
    game_renderer_create_atlas(renderer,&atlas2,pixels,w,h,channels);

    game_renderer_create_texture(renderer,&tex1,&atlas1,0,0,16,16);
    game_renderer_create_texture(renderer,&tex2,&atlas1,16,16,16,16);
    game_renderer_create_texture(renderer,&tex3,&atlas2,0,0,w,h);
    
    while(!game_quit)  
    {
        while(os_event_poll(&event))
        {
            switch(event.type)
            {
                case OS_EVENT_CLOSE:
                {
                    if(event.window == game_main_window){
                        game_quit = 1;
                    }
                }break;
                case OS_EVENT_MOUSE:
                {
                    mx = event.mouse.x;
                    my = event.mouse.y;
                }break;
                case OS_EVENT_KEYBOARD:
                {
                    if(event.keyboard.action == OS_ACTION_PRESS)
                    {
                        if(event.keyboard.scancode == OS_KEYBOARD_D){
                            bx-=5;
                        }
                        if(event.keyboard.scancode == OS_KEYBOARD_A){
                            bx+=5;
                        }
                        if(event.keyboard.scancode == OS_KEYBOARD_S){
                            by-=5;
                        }
                        if(event.keyboard.scancode == OS_KEYBOARD_W){
                            by+=5;
                        }
                    }
                }
            }
        }
        
        t1 = os_timer_acquire();
        os_window_get_size(game_main_window,&w,&h);
        game_renderer_clear(renderer,w,h);

        
        game_renderer_draw_texture(renderer,&tex3,0,0,32,32,0,255);
        for(i = 0; i < 100; i++)
        {
            for(j = 0;j < 100; j++)
            {
                game_renderer_draw_rectangle(renderer,bx + (j * 5), by + (i * 5),3,3,0,0,255,200);
            }
        }
        
        game_renderer_draw_triangle(renderer,320-100,240,320+100,240,320,240-100,0,255,0,128);
        game_renderer_draw_texture(renderer,&tex1,50,50,0,0,0,255);
        game_renderer_draw_texture(renderer,&tex2,100,50,0,0,0,255);
        game_renderer_present(renderer);
        os_opengl_update(game_main_window);
        t2 = os_timer_acquire();
        printf("%i ms\n",t2 - t1);
            
    }

    return 0;
}