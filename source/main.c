#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <os.h>
#include <io.h>
#include "game.h"
#include <GL/glew.h>

unsigned char i1[] = {
    0xff,0x00,0x00,0xff,0x00,0x00,0xff,0xff,
    0xff,0x00,0x00,0xff,0x00,0x00,0xff,0xff
};

unsigned int i2[] = {
    0xff0000ff,0x00000000,
    0xff0000ff,0x0000ffff
};

int main(int argc, char* argv[]) /*Nintendo me dá coisas grátis!*/
{
    OsEvent event;
    OsWindow* game_main_window;
    GameRenderer* renderer;
    GameAtlas tile_atlas;
    GameTexture tile_brick_diffuse;
    GameTexture tile_brick_normal;
    
    
    int game_quit = 0;

    int i, j;
    void* pixels;
    int w, h, channels;
    int mx, my;
    int bx,by;
    float sx, sy;
    int angle = 0;
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
    
    
    pixels = io_image_load("icon.png",&w,&h,&channels);
    if(pixels == NULL){
        printf("could not set icon\n");
    }
    else
    {
        os_window_set_icon(game_main_window,pixels,w,h);
        io_image_unload(pixels);
    }
    
    
    bx = 0; 
    by = 0;
    sx = 1.0;
    sy = 1.0;
    
    pixels = io_image_load("tiles.png",&w,&h,&channels);
    if(pixels == NULL){
        printf("could not load texture\n");
        return 0;
    }
    game_renderer_create_atlas(renderer,&tile_atlas,pixels,w,h,channels);

    game_renderer_create_texture(renderer,&tile_brick_diffuse,&tile_atlas,0,0,128,128);
    game_renderer_create_texture(renderer,&tile_brick_normal,&tile_atlas,128,0,128,128);
    
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
                        if(event.keyboard.scancode == OS_KEYBOARD_UP){
                            if(sx < 1.0){
                            sx += 0.1;
                            sy += 0.1;
                            }
                        }
                        if(event.keyboard.scancode == OS_KEYBOARD_DOWN){
                            if(sx > 0.25){
                            sx -= 0.1;
                            sy -= 0.1;
                            }
                        }
                    }
                }
            }
        }
        
        t1 = os_timer_acquire();
        os_window_get_size(game_main_window,&w,&h);
        game_renderer_clear(renderer,w,h);
    
        game_renderer_draw_texture(renderer,&tile_brick_diffuse,0,0,1.0,1.0,angle);
        game_renderer_draw_texture(renderer,&tile_brick_normal,tile_brick_diffuse.w,0,1.0,1.0,angle);
        game_renderer_draw_entity(renderer,&tile_brick_diffuse,&tile_brick_normal,(w/2)-(tile_brick_diffuse.w * sx / 2),(h/2)-(tile_brick_diffuse.h * sy / 2),sx,sy,angle);
        
        game_renderer_present(renderer);
        os_opengl_update(game_main_window);
        t2 = os_timer_acquire();
        printf("%i ms\n",t2 - t1);
        printf("block size w: %i h: %i\n",(int)(tile_brick_diffuse.w * sx),(int)(tile_brick_diffuse.h * sy));
        
    }

    return 0;
}