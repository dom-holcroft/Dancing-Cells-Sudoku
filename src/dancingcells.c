#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int SOLUTION_SIZE = 81;
int TRAIL_SIZE = 20000;

int flag;
int active;
int oactive;

int xArray[81] = {0};
int yArray[81] = {0};

typedef struct {
    int item;
    int size;
} TrailEntry;

typedef struct {
    TrailEntry *stack;
    int top;
    int capacity;
} TrailStack;

typedef struct {
    int16_t itm;
    int16_t loc;
} Node;

typedef struct {
    uint16_t pos;
    uint16_t size;
    uint16_t options[9];
} Item;

typedef struct {
    uint16_t *itemArray;
    Item *setArray;
    Node *nodeArray;
    TrailStack *trailStack;
} DancingCellArrays;

void setupDancingCellArrays(DancingCellArrays *dancingCellArrays,
                            uint16_t itemArraySize, uint16_t nodeArraySize,
                            uint16_t setArraySize) {
    dancingCellArrays->itemArray = malloc(itemArraySize * sizeof(uint16_t));
    dancingCellArrays->nodeArray = malloc(nodeArraySize * sizeof(Node));
    dancingCellArrays->setArray = malloc(itemArraySize * sizeof(Item));
    dancingCellArrays->trailStack = malloc(sizeof(TrailStack));
    dancingCellArrays->trailStack->stack =
        malloc(sizeof(TrailEntry) * TRAIL_SIZE);
    dancingCellArrays->trailStack->top = -1;
    dancingCellArrays->trailStack->capacity = SOLUTION_SIZE;
}

int filled(DancingCellArrays *dancingCellArrays, int *grid, int row, int col) {
    int digit = grid[row * 9 + col];
    return digit;
}

int addPossibility(DancingCellArrays *dancingCellArrays, int row, int col,
                   int digit) {
    int rowIndex = (col * 81 + row * 9 + digit) * 5 + 1;
    int blockIndex = (col / 3) + ((row / 3) * 3);
    int colIndexRow = 3 * 9 * digit + row;
    int colIndexCol = 3 * 9 * digit + 9 + col;
    int colIndexBlock = 3 * 9 * digit + 2 * 9 + blockIndex;
    int colIndexSimple = 3 * 9 * 9 + (col + 9 * row);

    for (int i = 0; i < 4; i++) {
        int currentRowIndex = rowIndex + i;
        int colIndex;

        switch (i) {
        case 0:
            colIndex = colIndexSimple;
            break;
        case 1:
            colIndex = colIndexCol;
            break;
        case 2:
            colIndex = colIndexRow;
            break;
        case 3:
            colIndex = colIndexBlock;
            break;
        default:
            colIndex = -1;
            break;
        }

        dancingCellArrays->nodeArray[currentRowIndex].itm =
            dancingCellArrays->itemArray[colIndex];

        dancingCellArrays->nodeArray[currentRowIndex].loc =
            dancingCellArrays->setArray[dancingCellArrays->itemArray[colIndex]]
                .size;
        dancingCellArrays->setArray[dancingCellArrays->itemArray[colIndex]]
            .options[dancingCellArrays
                         ->setArray[dancingCellArrays->itemArray[colIndex]]
                         .size] = currentRowIndex;

        dancingCellArrays->setArray[dancingCellArrays->itemArray[colIndex]]
            .size += 1;
    }

    dancingCellArrays->nodeArray[rowIndex + 4].loc = 4;
    dancingCellArrays->nodeArray[rowIndex + 4].itm = -4;

    return 0;
}

void initDancingCellArrays(DancingCellArrays *dancingCellArrays, int *grid) {
    setupDancingCellArrays(dancingCellArrays, 324, 2916 + 730 + 324, 13);
    for (int i = 0; i < 324; ++i) {
        dancingCellArrays->itemArray[i] = i;
        dancingCellArrays->setArray[i].pos = i;
    }
    dancingCellArrays->nodeArray[0].itm = 0;
    dancingCellArrays->nodeArray[0].loc = 4;
    for (int col = 0; col < 9; ++col) {
        for (int row = 0; row < 9; ++row) {
            int digitOnGrid = filled(dancingCellArrays, grid, row, col);
            if (!digitOnGrid) {
                for (int digit = 0; digit < 9; ++digit) {
                    addPossibility(dancingCellArrays, row, col, digit);
                }
            } else {
                addPossibility(dancingCellArrays, row, col, digitOnGrid - 1);
            }
        }
    }
}

void testNodeArray(DancingCellArrays *dancingCellArrays) {
    char items[2916 + 730] = {0};
    for (int i = 0; i < 2916 + 730; ++i) {
        if (i % 5 != 0) {
            items[dancingCellArrays->itemArray
                      [dancingCellArrays
                           ->setArray[dancingCellArrays->nodeArray[i].itm]
                           .pos]] += 1;

            int k = dancingCellArrays->itemArray
                        [dancingCellArrays
                             ->setArray[dancingCellArrays->nodeArray[i].itm]
                             .pos] > 324;

            printf("%d ", i);
        }
    }
    printf("\n");
    for (int i = 0; i < 324; ++i) {
        printf("%d", items[i]);
    }
    for (int i = 0; i < 324; ++i) {
        for (int j = 0; j < 9; ++j) {
            printf("%d ", dancingCellArrays->setArray[i].options[j]);
        }
        printf("\n");
    }
    for (int i = 0; i < 2916 + 730; ++i) {
        if (i % 5 != 0) {
            printf(
                "node index is %d \nitm is %d \n loc is %d\n the option points "
                "to node index %d\n",
                i, dancingCellArrays->nodeArray[i].itm,
                dancingCellArrays->nodeArray->loc,
                dancingCellArrays->setArray[dancingCellArrays->nodeArray[i].itm]
                    .options[dancingCellArrays->nodeArray[i].loc]);
            if (dancingCellArrays->setArray[dancingCellArrays->nodeArray[i].itm]
                    .options[dancingCellArrays->nodeArray[i].loc] != i) {
            }
        }
    }
}

void hide(DancingCellArrays dancingCellArrays, int i) {
    for (int j = 0; j < dancingCellArrays.setArray[i].size; ++j) {
        int x = dancingCellArrays.setArray[i].options[j];
        int offset = 1;

        while (offset != 0) {
            Node *currentSibling = &dancingCellArrays.nodeArray[x + offset];
            if (currentSibling->itm < 0) {
                offset += (currentSibling->itm);
                continue;
            }

            int siblingItem = currentSibling->itm;
            if (dancingCellArrays.setArray[siblingItem].pos < oactive) {
                int newSize = dancingCellArrays.setArray[siblingItem].size - 1;
                if (newSize == 0 && flag == 0 &&
                    dancingCellArrays.setArray[siblingItem].pos < active) {
                    flag = 1;
                    return;
                } else {
                    int oldSibling = dancingCellArrays.setArray[siblingItem]
                                         .options[newSize];

                    dancingCellArrays.setArray[siblingItem].size = newSize;

                    dancingCellArrays.setArray[siblingItem].options[newSize] =
                        x + offset;

                    int i__ = currentSibling->loc;
                    dancingCellArrays.nodeArray[x + offset].loc = newSize;

                    dancingCellArrays.setArray[currentSibling->itm]
                        .options[i__] = oldSibling;
                    dancingCellArrays.nodeArray[oldSibling].loc = i__;
                }
            }
            offset += 1;
        }
    }
}

bool algorithmC(DancingCellArrays dancingCellArrays) {
    active = 324;
    int l = 0, t = 0;
    int k = 0;
    int i = -1;
    int minSize = 0;
    bool running = true;
    while (running) {
        minSize = __INT_MAX__;
        for (int k = 0; k < active; ++k) {
            int item = dancingCellArrays.itemArray[k];
            if (dancingCellArrays.setArray[item].size < minSize) {
                minSize = dancingCellArrays.setArray[item].size;
                i = item;
            }
        }
        if (minSize == __INT_MAX__) {
            return true;
        }

        int k0 = active - 1;
        active = k0;
        int i0 = dancingCellArrays.itemArray[k0];
        k = dancingCellArrays.setArray[i].pos;

        dancingCellArrays.itemArray[k0] = i;
        dancingCellArrays.itemArray[k] = i0;
        dancingCellArrays.setArray[i0].pos = k;
        dancingCellArrays.setArray[i].pos = k0;

        oactive = active;
        flag = -1;

        hide(dancingCellArrays, i);

        int j = 0;
        if (t + active > TRAIL_SIZE) {
            printf("error\n");
            return 0;
        } else {
            for (int k = 0; k < active; ++k) {
                dancingCellArrays.trailStack->stack[t + k].item =
                    dancingCellArrays.itemArray[k];

                dancingCellArrays.trailStack->stack[t + k].size =
                    dancingCellArrays.setArray[dancingCellArrays.itemArray[k]]
                        .size;

                dancingCellArrays.trailStack->top += 1;
            }
            t = t + active;
            yArray[l + 1] = t;
        }
        // C6
        bool processLevel = true;
        while (processLevel) {

            xArray[l] = dancingCellArrays.setArray[i].options[j];
            oactive = active;
            k = oactive;

            int offset = 1;
            while (offset != 0) {
                Node *currentSibling =
                    &dancingCellArrays.nodeArray[xArray[l] + offset];
                if (currentSibling->itm < 0) {
                    offset += (currentSibling->itm);
                    continue;
                }

                int siblingItem = currentSibling->itm;
                int k0 = dancingCellArrays.setArray[siblingItem].pos;
                if (k0 < k) {
                    k -= 1;
                    int otherItem = dancingCellArrays.itemArray[k];
                    dancingCellArrays.itemArray[k] = siblingItem;
                    dancingCellArrays.itemArray[k0] = otherItem;
                    dancingCellArrays.setArray[otherItem].pos = k0;
                    dancingCellArrays.setArray[siblingItem].pos = k;
                }

                offset += 1;
            }
            active = k;
            flag = 0;
            offset = 1;

            while (offset != 0) {
                Node *currentSibling =
                    &dancingCellArrays.nodeArray[xArray[l] + offset];
                if (currentSibling->itm < 0) {
                    offset += (currentSibling->itm);
                    continue;
                }
                int i_ = currentSibling->itm;
                if (dancingCellArrays.setArray[i_].pos < oactive) {
                    hide(dancingCellArrays, i_);
                    if (flag == 1) {
                        processLevel = false;
                    }
                }
                offset += 1;
            }

            if (processLevel) {
                l += 1;
                processLevel = false;
            } else {
                if (j + 1 >= dancingCellArrays.setArray[i].size) {
                    if (l == 0) {
                        return false;
                    } else {
                        l -= 1;
                        i = dancingCellArrays.nodeArray[xArray[l]].itm;
                        j = dancingCellArrays.nodeArray[xArray[l]].loc;
                    }
                } else {
                    for (int k = yArray[l]; k < yArray[l + 1]; ++k) {
                        int s0 = dancingCellArrays.trailStack->stack[k].size;
                        int i0 = dancingCellArrays.trailStack->stack[k].item;
                        dancingCellArrays.setArray[i0].size = s0;
                    }
                    t = yArray[l + 1];
                    active = t - yArray[l];
                    j += 1;
                }
            }
        }
    }
    return false;
}

int solveSudoku(int* grid) {
    DancingCellArrays dancingCellArrays;
    initDancingCellArrays(&dancingCellArrays, grid);
    //testNodeArray(&dancingCellArrays);
    bool possible = algorithmC(dancingCellArrays);
    if (!possible) {
        return 0;
    }

    for (int l = 0; l < 81; ++l) {
        int nodeIndex = xArray[l];
        int itm = dancingCellArrays.nodeArray[nodeIndex].itm;

        int offset = 1;
        while (offset != 0) {
            Node *currentSibling =
                &dancingCellArrays.nodeArray[nodeIndex + offset];
            if (currentSibling->itm < 0) {
                offset += (currentSibling->itm);
                break;
            }
            offset += 1;
        }
        int baseIndex =
            (nodeIndex + offset - 1) / 5; // Remove the offset and scale factor
        int col = baseIndex / 81;         // Extract column
        int remainder = baseIndex % 81;   // Remaining part after column
        int row = remainder / 9;          // Extract row
        int digit = remainder % 9;        // Extract digit
        grid[row * 9 + col] = digit + 1;
    }
    return 1;
}

/* int main() { */
/*     int grid[81] = { */
/*         3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, */
/*         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, */
/*         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, */
/*         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, */
/*     }; */
/*     DancingCellArrays dancingCellArrays; */
/**/
/*     initDancingCellArrays(&dancingCellArrays, grid); */
/*     testNodeArray(&dancingCellArrays); */
/*     algorithmC(dancingCellArrays); */
/**/
/*     for (int l = 0; l < 81; ++l) { */
/*         int nodeIndex = xArray[l]; */
/*         int itm = dancingCellArrays.nodeArray[nodeIndex].itm; */
/**/
/*         int offset = 1; */
/*         while (offset != 0) { */
/*             Node *currentSibling = */
/*                 &dancingCellArrays.nodeArray[nodeIndex + offset]; */
/*             if (currentSibling->itm < 0) { */
/*                 offset += (currentSibling->itm); */
/*                 break; */
/*             } */
/*             offset += 1; */
/*         } */
/*         int baseIndex = */
/*             (nodeIndex + offset - 1) / 5; // Remove the offset and scale factor */
/*         int col = baseIndex / 81;         // Extract column */
/*         int remainder = baseIndex % 81;   // Remaining part after column */
/*         int row = remainder / 9;          // Extract row */
/*         int digit = remainder % 9;        // Extract digit */
/*         grid[row * 9 + col] = digit + 1; */
/*     } */
/*     printf("Solved Sudoku:\n"); */
/*     for (int row = 0; row < 9; ++row) { */
/*         for (int col = 0; col < 9; ++col) { */
/*             printf("%d ", grid[row * 9 + col]); */
/*             if ((col + 1) % 3 == 0 && col != 8) { */
/*                 printf("| "); */
/*             } */
/*         } */
/*         printf("\n"); */
/*         if ((row + 1) % 3 == 0 && row != 8) { */
/*             printf("---------------------\n"); // Row separator for 3x3 grids */
/*         } */
/*     } */
/**/
/*     return 0; */
/* } */
