#ifndef OS_H
#define OS_H


typedef void OsWindow;
typedef void OsThread;
typedef void OsMutex;
typedef void OsFile;
typedef void OsFolder;
typedef void OsLibrary;


typedef enum OsWindowFlags
{
    OS_WINDOW_HIDDEN     = (1 << 0),
    OS_WINDOW_FULLSCREEN = (1 << 1),
    OS_WINDOW_MAXIMIZED  = (1 << 3),
    OS_WINDOW_MINIMIZED  = (1 << 4),
    OS_WINDOW_BORDERLESS = (1 << 5)
}OsWindowFlags;

typedef enum OsEventType
{
    OS_EVENT_SHOW,
    OS_EVENT_HIDE,
    OS_EVENT_MOVE,
    OS_EVENT_RESIZE,
    OS_EVENT_MAXIMIZE,
    OS_EVENT_MINIMIZE,
    OS_EVENT_RESTORE,
    OS_EVENT_CLOSE,
    OS_EVENT_MOUSE,
    OS_EVENT_KEYBOARD,
    OS_EVENT_COMPOSE,
    OS_EVENT_TEXT,
    OS_EVENT_DROP,
    OS_EVENT_QUIT
}OsEventType;

typedef enum OsActionType
{
    OS_ACTION_MOVE,
    OS_ACTION_PRESS,
    OS_ACTION_REPEAT,
    OS_ACTION_RELEASE
}OsActionType;

typedef enum OsMouseButton
{
    OS_MOUSE_NONE   = (0 << 0),
    OS_MOUSE_LEFT   = (1 << 0),
    OS_MOUSE_MIDDLE = (1 << 1),
    OS_MOUSE_RIGHT  = (1 << 2)
}OsMouseButton;

typedef enum OsKeyboardCode
{
    OS_KEYBOARD_UNKNOWN,
    
    OS_KEYBOARD_A, OS_KEYBOARD_B,
    OS_KEYBOARD_C, OS_KEYBOARD_D,
    OS_KEYBOARD_E, OS_KEYBOARD_F,
    OS_KEYBOARD_G, OS_KEYBOARD_H,
    OS_KEYBOARD_I, OS_KEYBOARD_J,
    OS_KEYBOARD_K, OS_KEYBOARD_L,
    OS_KEYBOARD_M, OS_KEYBOARD_N,
    OS_KEYBOARD_O, OS_KEYBOARD_P,
    OS_KEYBOARD_Q, OS_KEYBOARD_R,
    OS_KEYBOARD_S, OS_KEYBOARD_T,
    OS_KEYBOARD_U, OS_KEYBOARD_V,
    OS_KEYBOARD_W, OS_KEYBOARD_X,
    OS_KEYBOARD_Y, OS_KEYBOARD_Z,

    OS_KEYBOARD_0, OS_KEYBOARD_1,
    OS_KEYBOARD_2, OS_KEYBOARD_3,
    OS_KEYBOARD_4, OS_KEYBOARD_5,
    OS_KEYBOARD_6, OS_KEYBOARD_7,
    OS_KEYBOARD_8, OS_KEYBOARD_9,

    OS_KEYBOARD_F1,  OS_KEYBOARD_F2,  OS_KEYBOARD_F3,
    OS_KEYBOARD_F4,  OS_KEYBOARD_F5,  OS_KEYBOARD_F6,
    OS_KEYBOARD_F7,  OS_KEYBOARD_F8,  OS_KEYBOARD_F9,
    OS_KEYBOARD_F10, OS_KEYBOARD_F11, OS_KEYBOARD_F12,

    OS_KEYBOARD_UP,   OS_KEYBOARD_DOWN,
    OS_KEYBOARD_LEFT, OS_KEYBOARD_RIGHT,

    OS_KEYBOARD_TAB, OS_KEYBOARD_SPACE, OS_KEYBOARD_BACKSPACE,
    OS_KEYBOARD_CAPSLOCK,
    OS_KEYBOARD_ENTER,
    OS_KEYBOARD_ESCAPE,
    OS_KEYBOARD_SNAPSHOT,
    OS_KEYBOARD_PAUSE,
    OS_KEYBOARD_INSERT,
    OS_KEYBOARD_DELETE,
    OS_KEYBOARD_HOME, OS_KEYBOARD_END,
    OS_KEYBOARD_PAGEUP, OS_KEYBOARD_PAGEDOWN,
    
    OS_KEYBOARD_ALT,
    OS_KEYBOARD_CTRL,
    OS_KEYBOARD_NUMLOCK,
    OS_KEYBOARD_SCRLOCK,
    OS_KEYBOARD_SHIFT,
    OS_KEYBOARD_OS,
        
    OS_KEYBOARD_QUOTE,
    OS_KEYBOARD_COMMA, OS_KEYBOARD_SEMICOLLON,
    OS_KEYBOARD_PERIOD,
    OS_KEYBOARD_SLASH, OS_KEYBOARD_BACKSLASH,
    OS_KEYBOARD_TILDE,
    OS_KEYBOARD_BRACKETL, OS_KEYBOARD_BRACKETR,
    OS_KEYBOARD_MINUS, OS_KEYBOARD_EQUALS
}OsKeyboardCode;

typedef enum OsFileOrigin
{
    OS_FILE_BEGIN,
    OS_FILE_CURRENT,
    OS_FILE_END
}OsFileOrigin;

typedef enum OsFileType
{
    OS_FILE_NORMAL  = 1,
    OS_FILE_FOLDER  = 2,
    OS_FILE_UNKNOWN = 3
}OsFileType;


typedef struct OsEvent
{
    OsEventType type;
    unsigned int time;
    OsWindow* window;
    struct
    {
        unsigned char action;
        unsigned char button;
        unsigned char state;
        short x, y;
        short h, v;
    }mouse;
    struct
    {
        unsigned char action;
        unsigned char keycode;
        unsigned char scancode;
        unsigned char modifier;
    }keyboard;
    struct
    {
        char* buffer;
        struct
        {
            unsigned short amount;
            unsigned short current;
            char* buffer;
        }candidate;
    }compose;
    struct
    {
        char* buffer;
    }text;
    struct
    {
        unsigned int amount;
        char** paths;
    }drop;
}OsEvent;


int os_init();

int os_event_poll(OsEvent* event);

int  os_window_open(OsWindow** window, char* title, int x, int y, int w, int h, OsWindowFlags flags);
int  os_window_get_framebuffer(OsWindow* window, void** pixels, int* w, int* h);
int  os_window_get_clipboard(OsWindow* window, unsigned char** text);
int  os_window_set_clipboard(OsWindow* window, unsigned char* text);
void os_window_get_opacity(OsWindow* window, float* opacity);
void os_window_set_opacity(OsWindow* window, float opacity);
void os_window_get_position(OsWindow* window, int* x, int* y);
void os_window_set_position(OsWindow* window, int x, int y);
void os_window_get_size(OsWindow* window, int* w, int* h);
void os_window_set_size(OsWindow* window, int w, int h);
int  os_window_set_icon(OsWindow* window, unsigned char* pixels, int w, int h);
void os_window_get_title(OsWindow* window, char** title);
void os_window_set_title(OsWindow* window, char* title);
void os_window_raise(OsWindow* window);
void os_window_hide(OsWindow* window);
void os_window_show(OsWindow* window);
void os_window_maximize(OsWindow* window);
void os_window_minimize(OsWindow* window);
void os_window_restore(OsWindow* window);
void os_window_update(OsWindow* window);
int  os_window_close(OsWindow** window);

int os_display_get_amount(void);
int os_display_get_bounds(int display, int* x, int* y, int* w, int* h);
int os_display_get_dpi(int display, float* horizontal, float* vertical);
int os_display_get_mode_amount(int display, int* amount);
int os_display_get_mode(int display, int mode, int* w, int* h, int* rate);
int os_display_set_mode(int display, int mode);
int os_display_get_name(int display, char** name);


int os_path_create(char* path, int what);
int os_path_differ(char* path);
int os_path_change(char* path, char* new);
int os_path_delete(char* path);

int os_file_open(OsFile** file, char* path, char* mode);
unsigned long os_file_seek(OsFile* file, long position, int origin);
unsigned long os_file_read(OsFile* file, void* buffer, unsigned long size);
unsigned long os_file_write(OsFile* file, void* buffer, unsigned long size);
int os_file_close(OsFile** file);

int os_folder_open(OsFolder** folder, char* path);
int os_folder_list(OsFolder* folder, char** name, unsigned long* size, unsigned long* time);
int os_folder_close(OsFolder** folder);

int os_library_open(OsLibrary** library, char* path);
int os_library_get_function(OsLibrary* library, char* name, void (**function)());
int os_library_close(OsLibrary** library);


int os_thread_fork(OsThread** thread, int detach, int(*function)(void*), void* data);
int os_thread_join(OsThread** thread, unsigned long miliseconds, int* result);

int  os_mutex_create(OsMutex** mutex);
void os_mutex_enter(OsMutex* mutex);
void os_mutex_leave(OsMutex* mutex);
int  os_mutex_delete(OsMutex** mutex);


unsigned long os_timer_acquire();
void os_timer_delay(unsigned long miliseconds);


int os_opengl_create(OsWindow* window);
int os_opengl_update(OsWindow* window);
int os_opengl_get_function(char* name, void (**function)());
int os_opengl_delete(OsWindow* window);

#endif