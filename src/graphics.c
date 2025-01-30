#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include "tui.h"

#define minint(a, b) ((a) < (b) ? (a) : (b))

const char TOP_ROW[] = "┏━━━┯━━━┯━━━┳━━━┯━━━┯━━━┳━━━┯━━━┯━━━┓";
const char HOLE_ROW[] = "┃   │   │   ┃   │   │   ┃   │   │   ┃";
const char LINE_ROW[] = "┠───┼───┼───╂───┼───┼───╂───┼───┼───┨";
const char BOLD_LINE_ROW[] = "┣━━━┿━━━┿━━━╋━━━┿━━━┿━━━╋━━━┿━━━┿━━━┫";
const char BOTTOM_ROW[] = "┗━━━┷━━━┷━━━┻━━━┷━━━┷━━━┻━━━┷━━━┷━━━┛";
int SUDOKU_GRID_SIZE = 9;

void drawGrid(TuiContext* tuiContext) {
    int maxGridHeight = SUDOKU_GRID_SIZE * 2 + 1;
    int maxGridWidth = SUDOKU_GRID_SIZE * 4 + 2;
    int unicodeByteOffset;
    Display* canvasDisplay = &tuiContext->canvas.display;
    const char* currentRow;
    for (int row = 0; row < canvasDisplay->height; ++row) {
        unsigned int colByte = 0;
            if(row == 0) {
                currentRow = TOP_ROW;
            } else if (row == maxGridHeight - 1){
                currentRow = BOTTOM_ROW;
            } else if(row == 6 || row == 12) {
                currentRow = BOLD_LINE_ROW;
            } else if((row & 1) != 0) {
                currentRow = HOLE_ROW;
            } else {
                currentRow = LINE_ROW;
            }
        writeAtPos(0, row, currentRow, tuiContext);
    }
}

