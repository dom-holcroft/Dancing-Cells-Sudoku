#include "graphics.h"
#include "tui.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

int checkForCursorMovement() { return 0; }

int update(TuiContext *tuiContext) {
    int count = 0;
    while (1) {
        refresh(tuiContext);
        count++;
        char aBuffer[50];
        sprintf(aBuffer, "%d", count);
        writeAtPos(3, 3, aBuffer, tuiContext);
        checkForResize(tuiContext);
        unsigned char buffer[1];
        ssize_t result = read(tuiContext->tty, buffer, sizeof(buffer));

        if (result > 0) {
            switch (buffer[0]) {
            case 'q':
                tcsetattr(tuiContext->tty, TCSAFLUSH,
                          &tuiContext->originalTerminal);
                return 0;
            case 'h':
                moveCursor(-1, 0, tuiContext);
                break;
            case 'j':
                moveCursor(0, 1, tuiContext);
                break;
            case 'k':
                moveCursor(0, -1, tuiContext);
                break;
            case 'l':
                moveCursor(1, 0, tuiContext);
                break;
            }
        }
    }
}

int main() {
    TuiContext tuiContext;
    setupTUI(&tuiContext, 38, 19);
    setCursor(2, 1, &tuiContext);
    enableAutoResize();
    drawGrid(&tuiContext);
    writeAtPos(0, 0, "Amy and Josh", &tuiContext);
    refresh(&tuiContext);
    update(&tuiContext);
}
