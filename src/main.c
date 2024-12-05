#include "graphics.h"
#include "tui.h"
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

int checkForCursorMovement() { return 0; }

int update(TuiContext *tuiContext, u_int8_t cursorGridPositionX,
           u_int8_t cursorGridPositionY) {
    int32_t offsetBelowScreen, offsetAboveScreen, offsetRightScreen,
        offsetLeftScreen;
    int numbers[81] = {0};
    int count = 0;
    char buffer[2];
    while (1) {
        refresh(tuiContext);
        checkForResize(tuiContext);
        ssize_t result = read(tuiContext->tty, buffer, 1);

        if (result > 0) {
            switch (buffer[0]) {
            case 'q':
                tcsetattr(tuiContext->tty, TCSAFLUSH,
                          &tuiContext->originalTerminal);
                return 0;
            case 'l':
                offsetRightScreen = tuiContext->cursor.cursorX -
                                    tuiContext->canvas.viewportX -
                                    (tuiContext->screen.width - 1);
                if (tuiContext->cursor.cursorX <
                    tuiContext->canvas.display.width - 4) {
                    if (offsetRightScreen + 4 >= 0) {
                        tuiContext->canvas.viewportX += offsetRightScreen + 3;
                    }
                    moveCursor(4, 0, tuiContext);
                }

                break;
            case 'j':
                offsetBelowScreen = tuiContext->cursor.cursorY -
                                    tuiContext->canvas.viewportY -
                                    (tuiContext->screen.height - 1);
                if (tuiContext->cursor.cursorY <
                    tuiContext->canvas.display.height - 2) {
                    if (offsetBelowScreen + 2 >= 0) {
                        tuiContext->canvas.viewportY += offsetBelowScreen + 3;
                    }
                    moveCursor(0, 2, tuiContext);
                }

                break;
            case 'k':
                offsetAboveScreen =
                    tuiContext->canvas.viewportY - tuiContext->cursor.cursorY;
                if (tuiContext->cursor.cursorY - 2 > 0) {
                    if (offsetAboveScreen + 2 >= 0) {
                        tuiContext->canvas.viewportY -= offsetAboveScreen + 3;
                    }
                    moveCursor(0, -2, tuiContext);
                }

                break;
            case 'h':
                offsetLeftScreen =
                    tuiContext->canvas.viewportX - tuiContext->cursor.cursorX;
                if (tuiContext->cursor.cursorX - 4 > 0) {
                    if (offsetLeftScreen + 4 >= 0) {
                        tuiContext->canvas.viewportX -= offsetLeftScreen + 3;
                    }
                    moveCursor(-4, 0, tuiContext);
                }
                break;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                buffer[1] = '\0';
                writeAtPos(tuiContext->cursor.cursorX,
                           tuiContext->cursor.cursorY, &buffer[0], tuiContext);
                break;
            }
        }
    }
}

int main() {
    u_int8_t cursorGridPositionX = 0;
    u_int8_t cursorGridPositionY = 0;

    TuiContext tuiContext;
    setupTUI(&tuiContext, 38, 19);
    setCursor(2, 1, &tuiContext);
    enableAutoResize();
    drawGrid(&tuiContext);
    refresh(&tuiContext);
    update(&tuiContext, cursorGridPositionX, cursorGridPositionY);
}
