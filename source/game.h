#ifndef GAME_H
#define GAME_H

typedef void GameRenderer;

typedef struct GameAtlas
{
    unsigned int id;
    unsigned short w, h;
}GameAtlas;

typedef struct GameTexture
{
    unsigned int id;
    unsigned short w, h;
    float x1, y1;
    float x2, y2;
}GameTexture;

typedef enum GameUiAction
{
    GAME_UI_NONE,
    GAME_UI_OVER,
    GAME_UI_PRESS,
    GAME_UI_HOLD,
    GAME_UI_RELEASE
}GameUiAction;


int  game_renderer_create(GameRenderer** renderer);
int  game_renderer_create_atlas(GameRenderer* renderer, GameAtlas* atlas, void* pixels, int w, int h, int channels);
int  game_renderer_create_texture(GameRenderer* renderer, GameTexture* texture, GameAtlas* atlas, int x, int y, int w, int h);
void game_renderer_clear(GameRenderer* renderer, int w, int h);
void game_renderer_draw_line(GameRenderer* renderer, int x1, int y1, int x2, int y2, int r, int g, int b, int a);
void game_renderer_draw_circle(GameRenderer* renderer, int x, int y, int radius, int r, int g, int b, int a);
void game_renderer_draw_triangle(GameRenderer* renderer, int x1, int y1, int x2, int y2, int x3, int y3, int r, int g, int b, int a);
void game_renderer_draw_rectangle(GameRenderer* renderer,int x, int y, int w, int h, int r, int g, int b, int a);
void game_renderer_draw_texture(GameRenderer* renderer, GameTexture* texture, int x, int y, float sx, float sy, float angle);
void game_renderer_draw_entity(GameRenderer* renderer, GameTexture* diffuse, GameTexture* normal, int x, int y, float sx, float sy, float angle);
void game_renderer_present(GameRenderer* renderer);
int  game_renderer_delete_texture(GameTexture* texture);
int  game_renderer_delete(GameRenderer* renderer);


int game_ui_layer_do(void* id, int x, int y, int w, int h);
int game_ui_button_do(void* id, int x, int y, int w, int h);
int game_ui_slide_do(void* id, int x, int y, int w, int h, float* value, int bar_size);
int game_ui_wheel_do(void* id, int x, int y, int radius, float* value, float* distance);


#endif