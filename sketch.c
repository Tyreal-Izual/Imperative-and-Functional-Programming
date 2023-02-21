// Basic program skeleton for a Sketch File (.sk) Viewer
#include "displayfull.h"
#include "sketch.h"
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <stdbool.h>

// Allocate memory for a drawing state and initialise it
state *newState() {
    state *s = (state *)malloc(sizeof(state));
    memset(s, 0, sizeof(state));
    s->tool = LINE;
    return s;
}

// Release all memory associated with the drawing state
void freeState(state *s) {
    free(s);
}

// Reset state's data fields except 'start'.
void resetState(state *s) {
    unsigned int start = s->start;
    memset(s, 0, sizeof(state));
    s->start = start;
    s->tool = LINE;
}

// Extract an opcode from a byte (two most significant bits).
int getOpcode(byte b) {
    int opcode = (b >> 6) & 3;
    return opcode;
}

// Extract an operand (-32..31) from the rightmost 6 bits of a byte.
int getOperand(byte b) {
    int operand = b & 0x3f;
    if (operand >= 32) {
        operand -= 64;
    }
    return operand;
}

// Handle tool command.
void tool(display *d, state *s, int operand) {
    if (operand == NONE || operand == LINE || operand == BLOCK) {
        s->tool = operand;
    } else if (operand == COLOUR) {
        colour(d, s->data);
    } else if (operand == TARGETX) {
        s->tx = s->data;
    } else if (operand == TARGETY) {
        s->ty = s->data;
    } else if (operand == SHOW) {
        show(d);
    } else if (operand == PAUSE) {
        pause(d, s->data);
    } else if (operand == NEXTFRAME) {
        s->end = true;
    }
    // reset data after tool command
    s->data = 0;
}

// Call draw line or block function based on current tool type. 
void draw(display *d, state *s) {
    if (s->tool == LINE) {
        line(d, s->x, s->y, s->tx, s->ty);
    } else if (s->tool == BLOCK) {
        int w = s->tx - s->x;
        int h = s->ty - s->y;
        block(d, s->x, s->y, w, h);
    }
}

// Execute the next byte of the command sequence.
void obey(display *d, state *s, byte op) {
    int opcode = getOpcode(op);
    int operand = getOperand(op);
    if (opcode == TOOL) {
        tool(d, s, operand);
    } else if (opcode == DX) {
        s->tx += operand;
    } else if (opcode == DY) {
        s->ty += operand;
        draw(d, s);
        s->x = s->tx;
        s->y = s->ty;
    } else /* if (opcode==DATA) */ {
        s->data <<= 6;
        s->data |= (operand & 0x3f);
    }
}

// Draw a frame of the sketch file. For basic and intermediate sketch files
// this means drawing the full sketch whenever this function is called.
// For advanced sketch files this means drawing the current frame whenever
// this function is called.
bool processSketch(display *d, void *data, const char pressedKey) {

    // TO DO: OPEN, PROCESS/DRAW A SKETCH FILE BYTE BY BYTE, THEN CLOSE IT
    // NOTE: CHECK DATA HAS BEEN INITIALISED... if (data == NULL) return
    // (pressedKey == 27); NOTE: TO GET ACCESS TO THE DRAWING STATE USE... state
    // *s = (state*) data; NOTE: TO GET THE FILENAME... char *filename =
    // getName(d); NOTE: DO NOT FORGET TO CALL show(d); AND TO RESET THE DRAWING
    // STATE APART FROM
    //      THE 'START' FIELD AFTER CLOSING THE FILE

    if (data == NULL) {
        return (pressedKey == 27);
    }

    state *s = (state *)data;
    char *filename = getName(d);

    FILE *fp = fopen(filename, "r");
    if (fp) {
        byte b;

        fseek(fp, s->start, SEEK_SET);
        s->start = 0;

        while (fread(&b, 1, 1, fp) == 1) {
            obey(d, s, b);
            if (s->end) {
                s->start = ftell(fp);
                break;
            }
        }

        fclose(fp);
    }

    show(d);
    resetState(s);

    return (pressedKey == 27);
}

// View a sketch file in a 200x200 pixel window given the filename
void view(char *filename) {
    display *d = newDisplay(filename, 200, 200);
    state *s = newState();
    run(d, s, processSketch);
    freeState(s);
    freeDisplay(d);
}

// Include a main function only if we are not testing (make sketch),
// otherwise use the main function of the test.c file (make test).
#ifndef TESTING
int main(int n, char *args[n]) {
    if (n != 2) { // return usage hint if not exactly one argument
        printf("Use ./sketch file\n");
        exit(1);
    } else
        view(args[1]); // otherwise view sketch file in argument
    return 0;
}
#endif
