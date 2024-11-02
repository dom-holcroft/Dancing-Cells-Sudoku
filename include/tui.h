#ifndef TUI_H
#define TUI_H

#include <stdint.h>
#include <stdio.h>
#include <termios.h>
#include <stdbool.h>

#define UNICODE_MAX_LEN 4

typedef struct {
    char symbol[UNICODE_MAX_LEN];
    size_t symbolByteLength;
    /**
     * Ansi Options per bit
     * dirty
     * bold
     * italics
     * faint
     * blinking
     * reverse
     * strikethough
     * underline
     */
    uint8_t ansiOptions;
    int colour;
} TChar;

typedef struct {
    unsigned int width, height;
    TChar *screenDisplayArray;
} Display;

typedef struct {
    Display display;
    unsigned int viewportX, viewportY;
} Canvas;

typedef struct {
    int cursorX, cursorY;
    bool cursorShown;
} Cursor;

typedef struct {
    struct termios originalTerminal, raw;
    int tty;
    Cursor cursor;
    Display screen;
    Canvas canvas;
    char *finalWriteBuffer;
} TuiContext;

int writeAtPos(unsigned int x, unsigned int y, const char *text,
               TuiContext *tuiContext);

void setCursor(unsigned int x, unsigned int y, TuiContext *tuiContext);

void moveCursor(unsigned int x, unsigned int y, TuiContext *tuiContext);

int setupTUI(TuiContext *tuiContext, unsigned int canvasWidth,
             unsigned int canvasHeight);

int enableVirtualCursorMovement(TuiContext *tuiContext);

void enableAutoResize();

int checkForResize(TuiContext *tuiContext);

int refresh(TuiContext *tuiContext);

#endif

