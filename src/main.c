#include "graphics.h"
#include "tui.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

int update(TuiContext *tuiContext) {

    while (1) {
        checkForResize(tuiContext);
        // printf("\033[1A");
        unsigned char buffer[1];
        ssize_t result = read(tuiContext->tty, buffer, sizeof(buffer));
        if (result == -1) {
            perror("Error reading from tty");
            return 1;
        }
        if (buffer[0] == 'q') {
            tcsetattr(tuiContext->tty, TCSAFLUSH, &tuiContext->originalTerminal);
            return 0;
        }
        if (buffer[0] == 'k') {
            printf("\033[1A");
            fflush(stdout);
            usleep(500000);
        }
        if (buffer[0] == 'j') {
            printf("\033[1B");
            fflush(stdout);
            usleep(500000);
        }
    }
}

int main() {
    TuiContext tuiContext;
    setupTUI(&tuiContext, 38, 19);
    cursorStartPosition(2, 1, &tuiContext);
    enableAutoResize();
    drawGrid(&tuiContext);
    writeAtPos(35, 10, "Amy and Josh", &tuiContext);
    refresh(&tuiContext);
    update(&tuiContext);
}
