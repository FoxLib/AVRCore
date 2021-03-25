#include "main.h"

// ---------------------------------------------------------------------
// Работа с окном
// ---------------------------------------------------------------------

// Обработчик кадра
uint AVRDisplayTimer(uint interval, void *param) {

    SDL_Event     event;
    SDL_UserEvent userevent;

    /* Создать новый Event */
    userevent.type  = SDL_USEREVENT;
    userevent.code  = 0;
    userevent.data1 = NULL;
    userevent.data2 = NULL;

    event.type = SDL_USEREVENT;
    event.user = userevent;

    SDL_PushEvent(&event);
    return (interval);
}

// Создать окно
void APP::window(const char* caption) {

    int w = config_width;
    int h = config_height;

    width  = w;
    height = h;

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    SDL_EnableUNICODE(1);

    sdl_screen = SDL_SetVideoMode(w, h, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    SDL_WM_SetCaption(caption, 0);
    SDL_AddTimer(1000 / clock_video, AVRDisplayTimer, NULL);
    SDL_EnableKeyRepeat(500, 30);
}

// Обновление экрана
void APP::update_screen() { if (ds_debugger) ds_update(); else display_update(); }

// ---------------------------------------------------------------------
// Методы вывода графики
// ---------------------------------------------------------------------

// Печать на экране Char
void APP::print16char(int col, int row, unsigned char ch, uint cl) {

    col *= 8;
    row *= 16;

    for (int i = 0; i < 16; i++) {

        unsigned char hl = ansi16[ch][i];
        for (int j = 0; j < 8; j++) {
            if (hl & (1<<(7-j)))
                pset(j + col, i + row, cl);
        }
    }
}

// Печать строки
void APP::print(int col, int row, const char* s, uint cl) {

    int i = 0;
    while (s[i]) { print16char(col, row, s[i], cl); col++; i++; }
}

// Нарисовать точку
void APP::pset(int x, int y, uint color) {

    if (x >= 0 && y >= 0 && x < width && y < height) {
        ( (Uint32*)sdl_screen->pixels )[ x + width*y ] = color;
    }
}

// Очистить экран в цвет
void APP::cls(uint color) {

    for (int i = 0; i < height; i++)
    for (int j = 0; j < width; j++)
        pset(j, i, color);
}

// Обменять буфер
void APP::flip() {
    SDL_Flip(sdl_screen);
}
