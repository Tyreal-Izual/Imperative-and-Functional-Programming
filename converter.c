#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdbool.h>
#include <assert.h>

#define W 200
#define H 200

// Operations
enum { DX = 0, DY = 1, TOOL = 2, DATA = 3 };

// Tool Types
enum {
    NONE = 0,
    LINE = 1,
    BLOCK = 2,
    COLOUR = 3,
    TARGETX = 4,
    TARGETY = 5,
    SHOW = 6,
    PAUSE = 7,
    NEXTFRAME = 8
};

typedef unsigned char byte;

typedef struct {
    int x, y, tx, ty;
    unsigned char tool;
    unsigned int data;
    bool end;

    unsigned char colour;

    byte *ops;
    int nops;
    int ops_capacity;
} sk_state;

// Read pgm pixels from file.
bool read_pgm(FILE *fp, unsigned char image[H][W]) {
    int c;
    while ((c = fgetc(fp)) != '\n') {
        if (c == EOF) {
            break;
        }
    }
    return fread(image, 1, W * H, fp) == W * H;
}

// Write pgm pixels to the file.
void write_pgm(FILE *fp, unsigned char image[H][W]) {
    fprintf(fp, "P5 %d %d 255\n", W, H);
    fwrite(image, 1, W * H, fp);
}

void tool(sk_state *s, int operand) {
    if (operand == NONE || operand == LINE || operand == BLOCK) {
        s->tool = operand;
    } else if (operand == COLOUR) {
        // colour(d, s->data);
    } else if (operand == TARGETX) {
        s->tx = s->data;
    } else if (operand == TARGETY) {
        s->ty = s->data;
    } else if (operand == SHOW) {
        // show(d);
    } else if (operand == PAUSE) {
        // pause(d, s->data);
    } else if (operand == NEXTFRAME) {
        s->end = true;
    }
    // reset data after tool command
    s->data = 0;
}

// Add one skech operation.
void add_sk_op(sk_state *s, int opcode, int operand) {
    byte op = (opcode << 6) | (operand & 0x3f);
    if (s->nops == s->ops_capacity) {
        s->ops_capacity += s->ops_capacity;
        s->ops = (byte *)realloc(s->ops, sizeof(byte) * s->ops_capacity);
    }
    s->ops[s->nops++] = op;

    if (opcode == TOOL) {
        tool(s, operand);
    } else if (opcode == DX) {
        s->tx += operand;
    } else if (opcode == DY) {
        s->ty += operand;
        // draw(d, s);
        s->x = s->tx;
        s->y = s->ty;
    } else /* if (opcode==DATA) */ {
        s->data <<= 6;
        s->data |= (operand & 0x3f);
    }
}

// Set current data.
void set_data(sk_state *s, unsigned int data) {
    if (s->data == data) {
        return;
    }

    // find the highest 1 bit
    int high_bit = 0;
    for (int i = 31; i >= 0; --i) {
        if (data & ((unsigned int)1 << i)) {
            high_bit = i + 1;
            break;
        }
    }

    // DATA operation can transfer 6 bits each time
    int ndata = (high_bit + 5) / 6;
    for (int i = ndata - 1; i >= 0; --i) {
        add_sk_op(s, DATA, (data >> (6 * i)) & 0x3f);
    }
}

// Add one DY 0 operation to force drawing picture.
void force_draw(sk_state *s) {
    add_sk_op(s, DY, 0);
}

// Move tx.
void move_tx(sk_state *s, int tx) {
    int dx = tx - s->tx;
    if (dx == 0) {
        // do nothing
    } else if (dx >= -32 && dx <= 31) {
        // use 1 DX
        add_sk_op(s, DX, dx);
    } else if (dx >= -32 * 2 && dx <= 31 * 2) {
        // use 2 DXs
        int first = dx < 0 ? -32 : 31;
        add_sk_op(s, DX, first);
        int second = dx - first;
        add_sk_op(s, DX, second);
    } else {
        // use 1 or 2 DATAs and 1 TARGETX
        set_data(s, tx);
        add_sk_op(s, TOOL, TARGETX);
    }
}

// Move ty.
// Note that this function will not use DY operation.
void move_ty(sk_state *s, int ty) {
    int dy = ty - s->ty;
    if (dy == 0) {
        // do nothing
    } else {
        // use 1 or 2 DATAs and 1 TARGETY
        set_data(s, ty);
        add_sk_op(s, TOOL, TARGETY);
    }
}

// Move ty.
// Note that this function may use DY operation.
bool move_ty_draw(sk_state *s, int ty) {
    int dy = ty - s->ty;
    if (dy == 0) {
        // do nothing
    } else if (dy >= -32 && dy <= 31) {
        // use 1 DY
        add_sk_op(s, DY, dy);
        return true;
    } else if (dy >= -32 * 2 && dy <= 31 * 2) {
        // use 2 DYs
        int first = dy < 0 ? -32 : 31;
        add_sk_op(s, DY, first);
        int second = dy - first;
        add_sk_op(s, DY, second);
        return true;
    } else {
        // use 1 or 2 DATAs and 1 TARGETY
        set_data(s, ty);
        add_sk_op(s, TOOL, TARGETY);
    }
    return false;
}

// Set color.
void set_color(sk_state *s, unsigned char colour) {
    if (s->colour != colour) {
        unsigned int data = ((unsigned int)colour << 24) |
                            ((unsigned int)colour << 16) |
                            ((unsigned int)colour << 8) | 255;

        set_data(s, data);
        add_sk_op(s, TOOL, COLOUR);

        s->colour = colour;
    }
}

void pgm_to_sk(const char *pgm_path, const char *sk_path) {
    FILE *pgmfp = fopen(pgm_path, "r");
    if (pgmfp == NULL) {
        fprintf(stderr, "Can't read pgm file!\n");
        exit(1);
    }

    unsigned char image[H][W];
    if (!read_pgm(pgmfp, image)) {
        fprintf(stderr, "Wrong format pgm file!\n");
        exit(1);
    }

    fclose(pgmfp);

    //////////////////////////////////////////////////

    sk_state _s, *s = &_s;
    memset(s, 0, sizeof(sk_state));
    s->ops_capacity = W * H * 5;
    s->ops = (byte *)malloc(sizeof(byte) * s->ops_capacity);
    s->tool = LINE;
    s->colour = 255; // black
    for (int y = 0; y < H; ++y) {
        //! The following codes do not work well yet

        /*
        move_ty_draw(s, y);

        assert(s->y == y && s->ty == y);
        assert(s->x == 0);

        int x = 0;
        bool done = false;
        while (!done) {
            unsigned char colour = image[y][x];

            // printf("%03d,%03d ", x, (int)colour);

            ++x;
            while (x < W && image[y][x] == colour) {
                ++x;
            }
            if (x == W) {
                x = W - 1;
                done = true;
            }

            move_tx(s, x);
            set_color(s, colour);
            force_draw(s);
        }
        // printf("\n");

        // -------------------------------------------------------

        ++y;

        move_ty_draw(s, y);

        assert(s->y == y && s->ty == y);
        assert(s->x == W - 1);

        x = W - 1;
        done = false;
        while (!done) {
            unsigned char colour = image[y][x];

            // printf("%03d,%03d ", x, (int)colour);

            --x;
            while (x >= 0 && image[y][x] == colour) {
                --x;
            }
            if (x < 0) {
                x = 0;
                done = true;
            }

            move_tx(s, x);
            set_color(s, colour);
            force_draw(s);
        }
        // printf("\n");
        */

        move_ty_draw(s, y);

        for (int x = 0; x < W; ++x) {
            move_tx(s, x);
            set_color(s, image[y][x]);
            force_draw(s);
        }

        ++y;
        move_ty_draw(s, y);

        for (int x = W - 1; x >= 0; --x) {
            move_tx(s, x);
            set_color(s, image[y][x]);
            force_draw(s);
        }
    }

    //////////////////////////////////////////////////

    FILE *skfp = fopen(sk_path, "w");
    if (skfp == NULL) {
        fprintf(stderr, "Can't write sk file!\n");
        exit(1);
    }

    fwrite(s->ops, sizeof(byte), s->nops, skfp);

    fclose(skfp);

    free(s->ops);
}

void sk_to_pgm(const char *pgm_path, const char *sk_path) {
    fprintf(stderr, "Not implemented yet\n");
    exit(1);
}

void usage(const char *prog) {
    fprintf(stderr, "Usage: %s pgm-file\n", prog);
    exit(1);
}

void error_file_type() {
    fprintf(stderr, "File type not supported!\n");
    exit(1);
}

void test_pic() {
    unsigned char image[H][W];
    for (int x = 0; x < W; ++x) {
        for (int y = 0; y < H; ++y) {
            image[y][x] = x % 2 == 0 ? 0 : 255;
        }
    }
    FILE *fp = fopen("test.pgm", "w");
    write_pgm(fp, image);
    fclose(fp);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Wrong number of arguments!\n");
        usage(argv[0]);
    }

    // test_pic();

    const char *dotp = strrchr(argv[1], '.');
    if (dotp == NULL) {
        error_file_type();
    }

    size_t len = strlen(argv[1]);

    char *path = (char *)malloc(sizeof(char) * (len + 3));
    strncpy(path, argv[1], dotp - argv[1]);
    path[dotp - argv[1]] = '\0';
    if (strcmp(dotp, ".pgm") == 0) {
        strcat(path, ".sk");
        pgm_to_sk(argv[1], path);
    } else if (strcmp(dotp, ".sk") == 0) {
        strcat(path, ".pgm");
        sk_to_pgm(path, argv[1]);
    } else {
        error_file_type();
    }
    free(path);

    return 0;
}
