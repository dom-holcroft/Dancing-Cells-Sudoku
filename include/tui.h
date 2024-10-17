#ifndef TUI_H
#define TUI_H

#include <stdio.h>
#include <termios.h>
#include <stdbool.h>

#define UNICODE_MAX_LEN 4

typedef struct {
    char symbol[UNICODE_MAX_LEN];
    size_t symbolByteLength;
    bool dirty;
    bool bold;
    bool italics;
    bool faint;
    bool blinking;
    bool reverse;
    bool strikethough;
    bool underline;
    int colour;
} TChar;

typedef struct {
    unsigned int width, height;
    TChar *screenDisplayArray;
} Display;

typedef struct {
    Display* display;
    unsigned int viewportX, viewportY;
} Canvas;

typedef struct {
    struct termios originalTerminal, raw;
    int tty;
    unsigned int cursorX, cursorY;
    Display *screen;
    Canvas *canvas;
    char *finalWriteBuffer;
} TuiContext;

int writeAtPos(unsigned int x, unsigned int y, const char *text,
               TuiContext *tuiContext);

void cursorStartPosition(unsigned int x, unsigned int y, TuiContext *tuiContext);

int setupTUI(TuiContext *tuiContext, unsigned int canvasWidth,
             unsigned int canvasHeight);

void enableAutoResize();

int checkForResize(TuiContext *tuiContext);

int refresh(TuiContext *tuiContext);

#endif

