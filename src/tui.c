#include "tui.h"
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

/**
 * @brief - ANSI escape code setup
 *
 * Consists of: hiding the cursor, saves the current cursor position for later
 * restoration, enables the alternaive buffer, switches buffer, and clears the
 * screen
 */
const char TERMINAL_SETUP[27] = "\x1B[s\x1B[?47h\x1B[?1049h\x1B[2J\x1b[?25h";

volatile sig_atomic_t resized = 0;

void sigwinchHandler(int signum) { resized = 1; }

/**
 * Buffer used to write all changes to the terminal at once
 *
 */
typedef struct {
    char *text;
    size_t length;
    size_t position;
} WriteBuffer;

/**
 * @brief - Finds the smaller of two numbers
 *
 * @param a - first number to compare.
 * @param b - second number to compare.
 *
 * @return - The smaller of the two inputs
 */
int32_t MIN(int32_t a, int32_t b) { return ((a) < (b) ? a : b); }

int32_t MAX(int32_t a, int32_t b) { return ((a) > (b) ? a : b); }

enum TCharBitmask {
    DIRTY,
    BOLD,
    ITALICS,
    FAINT,
    BLINKING,
    REVERSE,
    STRIKETHROUGH,
    UNDERLINE
};

void setTCharOption(TChar *tChar, enum TCharBitmask tCharBitmask) {
    tChar->ansiOptions = tChar->ansiOptions | (1 << tCharBitmask);
}

void clearTCharOption(TChar *tChar, enum TCharBitmask tCharBitmask) {
    tChar->ansiOptions = tChar->ansiOptions & ~(1 << tCharBitmask);
}

/**
 * @brief - gets the number of digits in a denary number
 *
 * @param a - Number to get length of.
 *
 * @return - The digits in the number
 */
int getNumberLength(int x) {
    x++;
    unsigned int count = 0;
    while (x) {
        x /= 10;
        count++;
    }
    return count;
}

/**
 * @brief Gets number of bytes in a unicode symbol
 *
 * Using the first byte of a unicode symbol, its possible to figure out
 * how many bytes it consists of, a table of which is shown below
 *
 * | Number of Bytes | First Byte | Second Byte | Third Byte | Fourth Byte |
 * |        1        |  0xxxxxxx  |     N/A     |     N/A    |     N/A     |
 * |        2        |  110xxxxx  |   10xxxxxx  |     N/A    |     N/A     |
 * |        3        |  1100xxxx  |   10xxxxxx  |  10xxxxxx  |     N/A     |
 * |        4        |  11110xxx  |   10xxxxxx  |  10xxxxxx  |   10xxxxxx  |
 *
 * @param firstByte - Fist byte which makes up a unicode character.
 * @return How many bytes are in the unicode seqeunce.
 */
int getUTF8ByteLength(unsigned char firstByte) {
    if ((firstByte & 0x80) == 0x00) {
        return 1;
    } else if ((firstByte & 0xE0) == 0xC0) {
        return 2;
    } else if ((firstByte & 0xF0) == 0xE0) {
        return 3;
    } else if ((firstByte & 0xF8) == 0xF0) {
        return 4;
    } else {
        return -1;
    }
}

/**
 * @brief - Checks if each byte that makes up a unicode character is the same.
 *
 * @return - True if they are different, false if not
 */
bool symbolsChanged(TChar *currentTChar, TChar *newTChar) {
    for (int i = 0; i < newTChar->symbolByteLength; ++i) {
        if (currentTChar->symbol[i] != newTChar->symbol[i]) {
            return true;
        }
    }
    return false;
}

/**
 * @brief - Moves cursor to x, y on the screen
 *
 * Works out how many bytes the finished string will be and adds that to the
 * buffer as to keep track of how large it is. x and y offset by 1 as ANSI uses
 * 1,1 to represent top left of terminal.
 *
 */
void moveCursorToPos(size_t x, size_t y, WriteBuffer *writeBuffer) {
    size_t requiredSnPrintLen = 5 + getNumberLength(x) + getNumberLength(y);
    snprintf(writeBuffer->text + writeBuffer->position, requiredSnPrintLen,
             "\x1B[%zu;%zuH", y + 1, x + 1);
    writeBuffer->position += requiredSnPrintLen;
}

/**
 * @brief - Updates screen to match canvas
 *
 * Compares the canvas to the screen, updating the screen to match the
 * current canvis and marking the cell as changed by updating dirty field.
 */
int updateScreen(TuiContext *tuiContext) {
    Display *screenDisplay = &tuiContext->screen;
    Canvas *canvas = &tuiContext->canvas;
    Display *canvasDisplay = &canvas->display;
    int count = 0;
    for (int row = 0; row < screenDisplay->height; ++row) {
        for (int col = 0; col < screenDisplay->width; ++col) {
            count++;
            TChar *screenTChar =
                &screenDisplay
                     ->screenDisplayArray[row * screenDisplay->width + col];
            TChar *canvasTChar =
                &canvasDisplay->screenDisplayArray[(row + canvas->viewportY) *
                                                       canvasDisplay->width +
                                                   col + canvas->viewportX];
            if (symbolsChanged(screenTChar, canvasTChar)) {
                setTCharOption(screenTChar, DIRTY);
                memcpy(screenTChar->symbol, canvasTChar->symbol,
                       UNICODE_MAX_LEN);
                screenTChar->symbolByteLength = canvasTChar->symbolByteLength;
            }
        }
    }
    return 0;
}

/**
 * @brief - Collates all writes into one buffer and prints to the screen
 *
 * Goes through each character, keeping track of state as create the most
 * concise write possible. Frees the buffer at the end.
 */
int writeFromScreen(TuiContext *tuiContext) {
    Display *screenDisplay = &tuiContext->screen;
    size_t bufferLength = screenDisplay->width * screenDisplay->height * 4 + 50;
    WriteBuffer writeBuffer = {
        .text = malloc(bufferLength), .length = bufferLength, .position = 0};

    bool currentlyWriting = false;
    bool currentlyBold = false;
    bool currentlyItalics = false;
    for (int row = 0; row < screenDisplay->height; ++row) {
        for (int col = 0; col < screenDisplay->width; ++col) {
            if (writeBuffer.position > writeBuffer.length) {
                printf("ohno\n");
                return -1;
            }
            TChar *currentTChar =
                &screenDisplay
                     ->screenDisplayArray[row * screenDisplay->width + col];
            if (currentTChar->ansiOptions & (1 << DIRTY)) {
                if (!currentlyWriting) {
                    moveCursorToPos(col, row, &writeBuffer);
                    currentlyWriting = true;
                }
                clearTCharOption(currentTChar, DIRTY);
                memcpy(writeBuffer.text + writeBuffer.position,
                       currentTChar->symbol, currentTChar->symbolByteLength);
                writeBuffer.position += currentTChar->symbolByteLength;
            } else {
                currentlyWriting = false;
            }
        }
        currentlyWriting = false;
    }
    if (tuiContext->cursor.cursorX >= tuiContext->canvas.viewportX &&
        tuiContext->cursor.cursorY >= tuiContext->canvas.viewportY &&
        tuiContext->cursor.cursorX <
            tuiContext->canvas.viewportX + tuiContext->screen.width &&
        tuiContext->cursor.cursorY <
            tuiContext->canvas.viewportY + tuiContext->screen.height) {

        memcpy(writeBuffer.text + writeBuffer.position, "\x1b[?25h", 6);
        writeBuffer.position += 6;

        moveCursorToPos(
            tuiContext->cursor.cursorX - tuiContext->canvas.viewportX,
            tuiContext->cursor.cursorY - tuiContext->canvas.viewportY,
            &writeBuffer);
    } else {
        memcpy(writeBuffer.text + writeBuffer.position, "\x1b[?25l", 6);
        writeBuffer.position += 6;
    }
    write(tuiContext->tty, writeBuffer.text, writeBuffer.position);
    free(writeBuffer.text);
    return 0;
}

int refresh(TuiContext *tuiContext) {
    updateScreen(tuiContext);
    writeFromScreen(tuiContext);
    return 0;
}

int writeAtPos(unsigned int x, unsigned int y, const char *text,
               TuiContext *tuiContext) {
    if (text == NULL) {
        return -1;
    }
    Display *canvasDisplay = &tuiContext->canvas.display;
    TChar *canvasArray = canvasDisplay->screenDisplayArray;
    size_t textLength = strlen(text);
    size_t UTF8ByteLength;
    for (size_t i = 0; i < textLength;) {
        if (x >= canvasDisplay->width - 1) {
            y += 1;
            x = 0;
        }
        if (y > canvasDisplay->height) {
            return -1;
        }
        UTF8ByteLength = getUTF8ByteLength(text[i]);
        canvasArray[y * canvasDisplay->width + x].symbolByteLength =
            UTF8ByteLength;

        memcpy(canvasArray[y * canvasDisplay->width + x].symbol, &text[i],
               UTF8ByteLength);

        i += UTF8ByteLength;
        x += 1;
    }
    return 0;
}

/**
 * @brief - Creates a display
 *
 * Allocates memory for a display of a given size and sets all TCHARs to the
 * null character.
 *
 * @param - display - a struct consisting of width, height, and an array of
 * TCHARs of size width * height
 * @return - 0 if works, -1 if failed
 *
 */
int allocateDisplay(Display *display) {
    size_t arrayTotalSize = display->height * display->width;
    display->screenDisplayArray = malloc(arrayTotalSize * sizeof(TChar));

    for (int i = 0; i < arrayTotalSize; i++) {
        strcpy(display->screenDisplayArray[i].symbol, "");
        display->screenDisplayArray[i].symbolByteLength = 1;
        display->screenDisplayArray->ansiOptions = 0;
    }

    return 0;
}

int resizeDisplay(Display *display, unsigned int newWidth,
                  unsigned int newHeight) {
    unsigned int ogHeight = display->height;
    unsigned int ogWidth = display->width;
    size_t newSize = newWidth * newHeight;

    TChar *newScreenDisplayArray = malloc(newSize * sizeof(TChar));
    if (!newScreenDisplayArray) {
        return -1;
    }

    TChar *ogScreenDisplayArray = display->screenDisplayArray;

    for (size_t i = 0; i < newHeight; ++i) {
        unsigned int newCurrentRow = i * newWidth;
        unsigned int ogCurrentRow =
            (i + MAX(0, ogHeight - newHeight)) * ogWidth;
        if (i < ogHeight) {
            memcpy(newScreenDisplayArray + newCurrentRow,
                   ogScreenDisplayArray + ogCurrentRow,
                   MIN(ogWidth, newWidth) * sizeof(TChar));
            if (newWidth > ogWidth) {
                for (size_t j = 0; j < newWidth - ogWidth; ++j) {
                    memset(newScreenDisplayArray + newCurrentRow + ogWidth, 0,
                           (newWidth - ogWidth) * sizeof(TChar));
                }
            }
        } else {
            for (size_t j = 0; j < newWidth; j++) {
                memset(newScreenDisplayArray + newCurrentRow, 0,
                       newWidth * sizeof(TChar));
            }
        }
    }
    free(ogScreenDisplayArray);
    display->screenDisplayArray = newScreenDisplayArray;
    display->height = newHeight;
    display->width = newWidth;
    return 0;
}

void enableAutoResize() {
    struct sigaction action;

    action.sa_handler = sigwinchHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;

    sigaction(SIGWINCH, &action, NULL);
}

int checkForResize(TuiContext *tuiContext) {
    if (!resized) {
        return -1;
    }

    Display *screenDisplay = &tuiContext->screen;
    Display *canvasDisplay = &tuiContext->canvas.display;
    resized = 0;
    struct winsize sz;
    if (ioctl(tuiContext->tty, TIOCGWINSZ, &sz) == -1) {
        return -1;
    }

    size_t width = MIN(sz.ws_col, canvasDisplay->width);
    size_t height = MIN(sz.ws_row, canvasDisplay->height);

    resizeDisplay(screenDisplay, width, height);
    refresh(tuiContext);
    return 0;
}

void setupScreen(Display *screen, Display *canvasDisplay) {
    struct winsize sz;
    ioctl(0, TIOCGWINSZ, &sz);
    screen->width = MIN(sz.ws_col, canvasDisplay->width);
    screen->height = MIN(sz.ws_row, canvasDisplay->height);
    allocateDisplay(screen);
}

void setupCanvas(Canvas *canvas, unsigned int canvasWidth,
                 unsigned int canvasHeight) {
    canvas->display.width = canvasWidth;
    canvas->display.height = canvasHeight;
    canvas->viewportX = 0;
    canvas->viewportY = 0;
    allocateDisplay(&canvas->display);
}

void restoreTerm(TuiContext *tuiContext) {
    tcsetattr(tuiContext->tty, TCSAFLUSH, &tuiContext->originalTerminal);
}

void rawTerminal(TuiContext *tuiContext) {
    tuiContext->raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    tuiContext->raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
    tuiContext->raw.c_oflag &= ~(OPOST);
    tuiContext->raw.c_cflag |= CS8;

    tuiContext->raw.c_cc[VTIME] = 0;
    tuiContext->raw.c_cc[VMIN] = 0;

    tcsetattr(tuiContext->tty, TCSAFLUSH, &tuiContext->raw);
}

void moveCursor(unsigned int x, unsigned int y, TuiContext *tuiContext) {
    tuiContext->cursor.cursorX += x;
    tuiContext->cursor.cursorY += y;
}

void setCursor(unsigned int x, unsigned int y, TuiContext *tuiContext) {
    tuiContext->cursor.cursorX = x;
    tuiContext->cursor.cursorY = y;
}

int setupTUI(TuiContext *tuiContext, unsigned int canvasWidth,
             unsigned int canvasHeight) {
    int tty = open("/dev/tty", O_RDWR);
    int flags = fcntl(tty, F_GETFL, 0);
    fcntl(tuiContext->tty, F_SETFL, flags | O_NONBLOCK);

    tuiContext->tty = tty;
    if (tty == -1) {
        perror("Error opening /dev/tty");
        return -1;
    }

    if (tcgetattr(tty, &tuiContext->originalTerminal) == -1) {
        perror("Error getting terminal attributes");
        close(tty);
        return -1;
    }

    tuiContext->raw = tuiContext->originalTerminal;
    rawTerminal(tuiContext);

    setupCanvas(&tuiContext->canvas, canvasWidth, canvasHeight);
    setupScreen(&tuiContext->screen, &tuiContext->canvas.display);
    write(tty, TERMINAL_SETUP, 27);
    return 0;
}
