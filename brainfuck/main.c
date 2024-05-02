#include <stdio.h>
#include <string.h>

#define BAND_LENGTH (30000)

char band[BAND_LENGTH];
size_t position = 0;

void init_band() {
    memset(band, 0, BAND_LENGTH);
}

int error(const char* msg) {
    fprintf(stderr, "%s\n", msg);
    return 1;
}

int run(FILE* input) {
    int c;
    while((c = getc(input)) != EOF) {
        switch (c) {
            case '+': band[position]++; break;
            case '-': band[position]--; break;
            case '<':
                if (--position < 0) return error("band underflow");
                break;
            case '>':
                if (++position >= BAND_LENGTH) return error("band overflow");
                break;
            case '.': putchar(band[position]); break;
            case ',': band[position] = getchar(); break;
        }
    }

    return 0;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        return 1;
    }

    FILE* input = stdin;

    if (strcmp(argv[1], "-") != 0) {
        input = fopen(argv[1], "r");
    }

    init_band();
    return run(input);
}