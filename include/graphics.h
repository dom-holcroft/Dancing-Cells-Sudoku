#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <termios.h>
#include "tui.h"

void drawGrid(TuiContext* tuiContext);
void drawNumbers(TuiContext* tuiContext, int numbers[9][9]);

#endif
