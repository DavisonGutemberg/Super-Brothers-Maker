#include <stdio.h>
#include <string.h>
#include <io.h>
#include <os.h>
#include <GL/glew.h>
#include <json.h>
#include "../../game.h"

typedef enum ToolUiActionType 
{
    TOOL_UI_ACTION_NONE = 1,
    TOOL_UI_ACTION_ROLL,
    TOOL_UI_ACTION_OVER,
    TOOL_UI_ACTION_PRESS,
    TOOL_UI_ACTION_REPEAT,
    TOOL_UI_ACTION_RELEASE
}ToolUiActionType;


int tool_ui_button(int mx, int my, int action, int x, int y, int w, int h)
{
    if(mx < x || mx > (x + w) || my < y || my > (y + h)){
        return TOOL_UI_ACTION_NONE;
    }
    return action == TOOL_UI_ACTION_PRESS ? TOOL_UI_ACTION_PRESS : TOOL_UI_ACTION_OVER; 
}


int tool_ui_slider(int mx, int my, int action, int x, int y, int w, int h, float* value)
{
    if(mx < x || mx > (x + w) || my < y || my > (y + h)){
        return TOOL_UI_ACTION_NONE;
    }

    if(action == TOOL_UI_ACTION_PRESS || action == TOOL_UI_ACTION_REPEAT){
        *value = (w > h) ? (mx - x) / (float)w : (my - y) / (float)h;
    }
    
    return action == TOOL_UI_ACTION_PRESS ? TOOL_UI_ACTION_PRESS : TOOL_UI_ACTION_OVER;
}


int tool_renderer_draw_text(GameRenderer* renderer, GameTexture letters[], int x, int y, char* text)
{
    int i;
    unsigned int length = strlen(text);

    for(i = 0; i < length; i++)
    {
        game_renderer_draw_texture(renderer,&letters[text[i]],x + (i * 8),y + (i / 32) * 8,1.0,1.0,0.0);
    }
}

int main(int argc, char* argv[])
{
    int i, j;
    int game_quit = 0;
    void* pixels;
    int w, h;
    int channels;
    int action;
    float tile_scale = 1.0;
    char buffer[8192];
    int size;
    struct
    {
        int x, y, action;
        struct
        {
            int x,y;
            int amount;
            int current;
            float scroll;
            int viewable;
            GameAtlas atlas;
            GameTexture diffuses[32];
            GameTexture normals[32];
        }tile_list;
        GameAtlas font;
        GameTexture letters[128];
    }ui;
    OsFile* file;
    JsonValue json;
    JsonValue value;
    OsEvent event;
    OsWindow* tool_main_window;
    int tool_main_window_w, tool_main_window_h;
    GameRenderer* renderer;
    GameAtlas tile_atlas;

    ui.action = 0;
    ui.tile_list.x = 0;
    ui.tile_list.y = 0;
    ui.tile_list.viewable = 0;
    ui.tile_list.scroll = 0;
    ui.tile_list.amount = 0;
    ui.tile_list.current = 0;
    ui.tile_list.scroll = 0.0;
    os_init();

    if(!os_window_open(&tool_main_window,"Mesh",50,50,640,512,OS_WINDOW_MAXIMIZED)){
        printf("Error: Could not create main window\n");
    }
    os_window_update(tool_main_window);

    pixels = io_image_load("icon.png",&w,&h,&channels);
    if(pixels == NULL){
        printf("Warning: Could not load icon.\n");
    }
    else
    {
        os_window_set_icon(tool_main_window,pixels,w,h);
        io_image_unload(pixels);
    }

    os_opengl_create(tool_main_window);
    if(glewInit() != GLEW_OK) /*Do it ourselves latter.*/
    {
        printf("Error: Glew failed to init.\n");
        return -1;
    }
    
    if(!game_renderer_create(&renderer)){
        printf("Error: Could not create renderer.\n");
    }
    
    /*Load mesh.json.*/
    if(!os_file_open(&file,"mesh.json","r"))
    {
        printf("Error: could not load mesh.json\n");
        return -1;
    }
    if((size = os_file_seek(file,0,OS_FILE_END)) > 8192)
    {
        printf("Error: mesh.json is too big\n");
        return -1;
    }
    os_file_seek(file,0,OS_FILE_BEGIN);
    os_file_read(file,buffer,size);
    buffer[size] = '\0';
    os_file_close(&file);

    if(!json_validate(buffer))
    {
        printf("Error in mesh.json: invalid json file.\n");
        return -1;
    }
    if(!json_deserialize(buffer,&json))
    {
        printf("Error: could not deserialize json file.\n");
        return -1;
    }

    if(json.type != JSON_OBJECT)
    {
        printf("Error in mesh.json: expecting a json object.\n");
        return -1;
    }
    
    if(!json_object_get(json.data.object,"path",&value))
    {
        printf("Error in mesh.json: expecting member \"path\".\n");
        return -1;
    }
    if(value.type != JSON_STRING)
    {
        printf("Error in mesh.json: \"path\" must be a string.\n");
        return -1;
    }

    pixels = io_image_load(value.data.string,&w,&h,&channels);
    if(pixels == NULL)
    {
        printf("Error: Could not load %s.\n",value.data.string);
        return -1;
    }
    
    game_renderer_create_atlas(renderer,&ui.tile_list.atlas,
                               pixels,w,h,channels);
    io_image_unload(pixels);

    if(!json_object_get(json.data.object,"tiles",&value))
    {
        printf("Error in mesh.json: missing member \"tiles\".\n");
        return -1;
    }
    if(value.type != JSON_ARRAY)
    {
        printf("Error in mesh.json: \"tiles\" must be an array.\n");
        return -1;
    }

    for(i = 0; i < value.data.array->length; i+=2)
    {
        JsonValue diffuse;
        JsonValue normal;
        JsonValue x, y, w, h;

        json_array_get(value.data.array,i,&diffuse);
        if(diffuse.type != JSON_OBJECT)
        {
            printf("Error in mesh.json: \"tiles[%i]\" must be an object.\n",i);
            return -1;
        }
        if(!json_object_get(diffuse.data.object,"x",&x))
        {
            printf("Error in mesh.json: missing member \"x\" in \"tiles[%i]\".\n",i);
            return -1;
        }
        if(x.type != JSON_NUMBER)
        {
            printf("Error in mesh.json: member \"x\" in \"tiles[%i]\" must be a number.\n",i);
            return -1;
        }
        if(!json_object_get(diffuse.data.object,"y",&y))
        {
            printf("Error in mesh.json: missing member \"y\" in \"tiles[%i]\".\n",i);
            return -1;
        }
        if(y.type != JSON_NUMBER)
        {
            printf("Error in mesh.json: member \"y\" in \"tiles[%i]\" must be a number.\n",i);
            return -1;
        }
        if(!json_object_get(diffuse.data.object,"w",&w))
        {
            printf("Error in mesh.json: missing member \"w\" in \"tiles[%i]\".\n",i);
            return -1;
        }
        if(w.type != JSON_NUMBER)
        {
            printf("Error in mesh.json: member \"w\" in \"tiles[%i]\" must be a number.\n",i);
            return -1;
        }
        if(!json_object_get(diffuse.data.object,"h",&h))
        {
            printf("Error in mesh.json: missing member \"h\" in \"tiles[%i]\".\n",i);
            return -1;
        }
        if(h.type != JSON_NUMBER)
        {
            printf("Error in mesh.json: member \"h\" in \"tiles[%i]\" must be a number.\n",i);
            return -1;
        }
        game_renderer_create_texture(renderer,&ui.tile_list.diffuses[ui.tile_list.amount],
                                    &ui.tile_list.atlas,x.data.number,y.data.number,w.data.number,h.data.number);
        
        json_array_get(value.data.array,i + 1,&normal);
        if(normal.type != JSON_OBJECT)
        {
            printf("Error in mesh.json: \"tiles[%i]\" must be an object.\n",i + 1);
            return -1;
        }
        if(!json_object_get(normal.data.object,"x",&x))
        {
            printf("Error in mesh.json: missing member \"x\" in \"tiles[%i]\".\n",i + 1);
            return -1;
        }
        if(x.type != JSON_NUMBER)
        {
            printf("Error in mesh.json: member \"x\" in \"tiles[%i]\" must be a number.\n",i + 1);
            return -1;
        }
        if(!json_object_get(normal.data.object,"y",&y))
        {
            printf("Error in mesh.json: missing member \"y\" in \"tiles[%i]\".\n",i + 1);
            return -1;
        }
        if(y.type != JSON_NUMBER)
        {
            printf("Error in mesh.json: member \"y\" in \"tiles[%i]\" must be a number.\n",i + 1);
            return -1;
        }
        if(!json_object_get(normal.data.object,"w",&w))
        {
            printf("Error in mesh.json: missing member \"w\" in \"tiles[%i]\".\n",i + 1);
            return -1;
        }
        if(w.type != JSON_NUMBER)
        {
            printf("Error in mesh.json: member \"w\" in \"tiles[%i]\" must be a number.\n",i + 1);
            return -1;
        }
        if(!json_object_get(normal.data.object,"h",&h))
        {
            printf("Error in mesh.json: missing member \"h\" in \"tiles[%i]\".\n",i + 1);
            return -1;
        }
        if(h.type != JSON_NUMBER)
        {
            printf("Error in mesh.json: member \"h\" in \"tiles[%i]\" must be a number.\n",i + 1);
            return -1;
        }
        game_renderer_create_texture(renderer,&ui.tile_list.normals[ui.tile_list.amount],
                                    &ui.tile_list.atlas,x.data.number,y.data.number,w.data.number,h.data.number);
        
        ui.tile_list.amount++;
    }

    pixels = io_image_load("Afont12.png",&w,&h,&channels);
    if(pixels == NULL)
    {
        printf("Error: Could not load font.\n");
        return -1;
    }

    game_renderer_create_atlas(renderer,&ui.font,pixels,w,h,channels);
    io_image_unload(pixels);
    
    for(i = 0; i < 4; i++)
    {
        for(j = 0; j < 32; j++)
        {
            game_renderer_create_texture(renderer,&ui.letters[(i * 32) + j],&ui.font,j * 16,i * 16,16,16);
        }
    }

    while(!game_quit)  
    {
        while(os_event_poll(&event))
        {
            switch(event.type)
            {
                case OS_EVENT_CLOSE:
                {
                    if(event.window == tool_main_window){
                        game_quit = 1;
                    }
                }break;
                case OS_EVENT_MOUSE:
                {
                    ui.x = event.mouse.x;
                    ui.y = event.mouse.y;
                    if(event.mouse.action != OS_ACTION_PRESS && event.mouse.state & OS_MOUSE_LEFT){
                        ui.action = TOOL_UI_ACTION_REPEAT;
                    }
                    else if(event.mouse.action == OS_ACTION_PRESS && event.mouse.button == OS_MOUSE_LEFT){
                        ui.action = TOOL_UI_ACTION_PRESS;
                    }
                    else{
                        ui.action = 0;
                    }

                    if(event.mouse.v > 0)
                    {
                        if(event.mouse.x < 50)
                        {
                            if(ui.tile_list.scroll > 0){
                                ui.tile_list.scroll--;
                            }
                        }
                        else
                        {
                            if(tile_scale < 2.0){
                                tile_scale += 0.1;
                            }
                        }
                    }
                    else if(event.mouse.v < 0)
                    {
                        if(event.mouse.x < 50)
                        {
                            if(ui.tile_list.scroll < ui.tile_list.amount - 1 && ui.tile_list.viewable < (ui.tile_list.amount - ui.tile_list.scroll)){
                                ui.tile_list.scroll++;
                            }
                        }
                        else
                        {
                            if(tile_scale > 0.5){
                                tile_scale -= 0.1;
                            }
                        }
                    }
                }break;
                case OS_EVENT_KEYBOARD:
                {
                    if(event.keyboard.action == OS_ACTION_PRESS)
                    {
                        if(event.keyboard.scancode == OS_KEYBOARD_UP){
                            if(tile_scale < 2.0){
                                tile_scale += 0.1;
                            }
                        }
                        if(event.keyboard.scancode == OS_KEYBOARD_DOWN){
                            if(tile_scale > 0.5){
                                tile_scale -= 0.1;
                            }
                        }
                    }
                }
            }
        }

        os_window_get_size(tool_main_window,&tool_main_window_w,&tool_main_window_h);
        game_renderer_clear(renderer,tool_main_window_w,tool_main_window_h);
        
        ui.tile_list.viewable = (tool_main_window_h / 40) - 2;
        ui.tile_list.y = (tool_main_window_h / 2) - ((ui.tile_list.viewable * 40) / 2);
        

        game_renderer_draw_rectangle(renderer,0,0,tool_main_window_w,tool_main_window_h,42,173,247,255); /*Show background.*/
        game_renderer_draw_rectangle(renderer,70,4,256,128,200,200,200,128); /*Show info window.*/

        game_renderer_draw_rectangle(renderer,4,ui.tile_list.y,40,ui.tile_list.viewable * 40,200,200,200,100); /*Show tile list background.*/
        if(ui.tile_list.current >= ui.tile_list.scroll && ui.tile_list.current < ui.tile_list.scroll + ui.tile_list.viewable){
            game_renderer_draw_rectangle(renderer,4,ui.tile_list.y + 4 + ((ui.tile_list.current - ui.tile_list.scroll) * 40),40,40,200,200,200,200);        
        }    
        
        if(action = tool_ui_button(ui.x,ui.y,ui.action,8,4,32,32))
        {
            if(action == TOOL_UI_ACTION_OVER)
            {
                game_renderer_draw_rectangle(renderer,8,4,32,32,200,200,200,200);
                game_renderer_draw_triangle(renderer,24,8,12,28,36,28,30,30,200,200);
            }
            else if(action == TOOL_UI_ACTION_PRESS)
            {
                if(ui.tile_list.scroll > 0){
                    ui.tile_list.scroll--;
                }
                game_renderer_draw_rectangle(renderer,8,4,32,32,200,200,200,200);
                game_renderer_draw_triangle(renderer,24,8,12,28,36,28,30,200,30,200);
            }
            else
            {
                game_renderer_draw_rectangle(renderer,8,4,32,32,200,200,200,100);
                game_renderer_draw_triangle(renderer,24,8,12,28,36,28,30,30,200,128);
            }
        }

        if(action = tool_ui_button(ui.x,ui.y,ui.action,8,tool_main_window_h - 36,32,32))
        {
            if(action == TOOL_UI_ACTION_OVER)
            {
                game_renderer_draw_rectangle(renderer,8,tool_main_window_h - 36,32,32,200,200,200,200);
                game_renderer_draw_triangle(renderer,24,tool_main_window_h - 8,12,tool_main_window_h - 28,36,tool_main_window_h - 28,30,30,200,200);
            }
            else if(action == TOOL_UI_ACTION_PRESS)
            {
                if(ui.tile_list.scroll < ui.tile_list.amount - 1 && ui.tile_list.viewable < (ui.tile_list.amount - ui.tile_list.scroll)){
                    ui.tile_list.scroll++;
                }
                game_renderer_draw_rectangle(renderer,8,tool_main_window_h - 36,32,32,200,200,200,200);
                game_renderer_draw_triangle(renderer,24,tool_main_window_h - 8,12,tool_main_window_h - 28,36,tool_main_window_h - 28,30,200,30,200);
            }
            else
            {
                game_renderer_draw_rectangle(renderer,8,tool_main_window_h - 36,32,32,200,200,200,100);
                game_renderer_draw_triangle(renderer,24,tool_main_window_h - 8,12,tool_main_window_h - 28,36,tool_main_window_h - 28,30,30,200,128);
            }
        }

        for(i = ui.tile_list.scroll; i < ui.tile_list.amount && i < ui.tile_list.viewable + ui.tile_list.scroll; i++) /*Show tile list.*/
        {
            if(action = tool_ui_button(ui.x,ui.y,ui.action,8,ui.tile_list.y + 8 + ((i - ui.tile_list.scroll) * 40),32,32))
            {
                if(action == TOOL_UI_ACTION_OVER)
                {
                    if(i != ui.tile_list.current)
                    {
                        game_renderer_draw_entity(renderer,&ui.tile_list.diffuses[i],
                                                  &ui.tile_list.normals[i],4,ui.tile_list.y + 4 + ((i - ui.tile_list.scroll) * 40),
                                                  32 / (float)ui.tile_list.diffuses[i].w,
                                                  32 / (float)ui.tile_list.diffuses[i].h,0.0);
                    }
                    else
                    {
                        game_renderer_draw_entity(renderer,&ui.tile_list.diffuses[i],
                                                  &ui.tile_list.normals[i],8,ui.tile_list.y + 8 + ((i - ui.tile_list.scroll) * 40),
                                                  32 / (float)ui.tile_list.diffuses[i].w,
                                                  32 / (float)ui.tile_list.diffuses[i].h,0.0);    
                    }
                }
                else if(action == TOOL_UI_ACTION_PRESS)
                {
                    game_renderer_draw_entity(renderer,&ui.tile_list.diffuses[i],
                                              &ui.tile_list.normals[i],8,ui.tile_list.y + 8 + ((i - ui.tile_list.scroll) * 40),
                                              32 / (float)ui.tile_list.diffuses[i].w,
                                              32 / (float)ui.tile_list.diffuses[i].h,0.0);
                    ui.tile_list.current = i;
                }
                else
                {
                    game_renderer_draw_entity(renderer,&ui.tile_list.diffuses[i],
                                              &ui.tile_list.normals[i],8,ui.tile_list.y + 8 + ((i - ui.tile_list.scroll) * 40),
                                              32 / (float)ui.tile_list.diffuses[i].w,
                                              32 / (float)ui.tile_list.diffuses[i].h,0.0);
                }
            }
        }
        
        /*Show current tile.*/
        game_renderer_draw_entity(renderer,&ui.tile_list.diffuses[ui.tile_list.current],
                                  &ui.tile_list.normals[ui.tile_list.current],
                                  ((tool_main_window_w + 70) / 2) - (ui.tile_list.diffuses[ui.tile_list.current].w / 2 * tile_scale),
                                  (tool_main_window_h / 2) - (ui.tile_list.diffuses[ui.tile_list.current].h / 2 * tile_scale),
                                  tile_scale,tile_scale,0.0);
        
        /*Show info.*/
        sprintf(buffer,"Scale: %.1f",tile_scale);
        tool_renderer_draw_text(renderer,ui.letters,70,4,buffer);
        sprintf(buffer,"Width: %.1f Height: %.1f",ui.tile_list.diffuses[ui.tile_list.current].w * tile_scale,ui.tile_list.diffuses[ui.tile_list.current].h * tile_scale);
        tool_renderer_draw_text(renderer,ui.letters,70,20,buffer);
        
        game_renderer_present(renderer);
        os_opengl_update(tool_main_window);
    }
    return 0;  
}