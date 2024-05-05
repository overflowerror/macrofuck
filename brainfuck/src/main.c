#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BAND_LENGTH (30000)

char band[BAND_LENGTH];
long long position = 0; // signed

void init_band(void) {
    memset(band, 0, BAND_LENGTH);
}

int max(int a, int b) {
    return a > b ? a : b;
}

int error(const char* msg, const char* input, size_t offset) {
    fprintf(stderr, "%s\n", msg);
    if (input) {
        const char* display_segment = input;
        int display_offset = offset;
        if (offset > 5) {
            display_segment += offset - 5;
            display_offset = 5;
        }
        fprintf(stderr, "input position: %zu\n", offset);
        fprintf(stderr, "         %.*s%c\n", display_offset, "           ", input[offset]);
        fprintf(stderr, "context: %.10s\n", display_segment);
        fprintf(stderr, "         %.*s^\n", display_offset,  "          ");
        fprintf(stderr, "position: %lld\n", position);
    }
    return 1;
}

size_t search_loop_end(const char* input, size_t length, size_t start) {
    size_t nested_loops = 0;
    for (size_t i = start + 1; i < length; i++) {
        if (input[i] == '[') {
            nested_loops++;
        } else if (input[i] == ']') {
            if (nested_loops == 0) {
                return i;
            } else {
                nested_loops--;
            }
        }
    }
    return start;
}

size_t search_loop_begin(const char* input, size_t start) {
    size_t nested_loops = 0;
    for (long long i = start - 1; i >= 0; i--) {
        if (input[i] == ']') {
            nested_loops++;
        } else if (input[i] == '[') {
            if (nested_loops == 0) {
                return i;
            } else {
                nested_loops--;
            }
        }
    }
    return start;
}

int run(const char* input) {
    size_t program_counter = 0;
    size_t input_length = strlen(input);

    while(program_counter < input_length) {
        switch (input[program_counter]) {
            case '+': band[position]++; break;
            case '-': band[position]--; break;
            case '<':
                if (--position < 0) return error("band underflow", input, program_counter);
                break;
            case '>':
                if (++position >= BAND_LENGTH) return error("band overflow", input, program_counter);
                break;
            case '.': putchar(band[position]); break;
            case ',': band[position] = getchar(); break;
            case '[':
                if (!band[position]) {
                    size_t target = search_loop_end(input, input_length, program_counter);
                    if (target == program_counter) {
                        return error("no matching ] found", input, program_counter);
                    }
                    program_counter = target;
                }
                break;
            case ']':
                if (band[position]) {
                    size_t target = search_loop_begin(input, program_counter);
                    if (target == program_counter) {
                        return error("no matching [ found", input, program_counter);
                    }
                    program_counter = target;
                }
                break;
        }

        program_counter++;
    }

    return 0;
}

char* read_to_string(FILE* input) {
    size_t buffer_used = 0;
    size_t buffer_size = 1024;
    char* buffer = malloc(buffer_size);
    if (!buffer) {
        return NULL;
    }

    int c;
    while ((c = getc(input)) != EOF) {
        switch(c) {
            case '+':
            case '-':
            case '>':
            case '<':
            case '[':
            case ']':
            case '.':
            case ',':
                buffer[buffer_used++] = c;
                if (buffer_used >= buffer_size) {
                    buffer_size *= 2;
                    buffer = realloc(buffer, buffer_size);
                    if (!buffer) {
                        return NULL;
                    }
                }
                break;
        }
    }

    buffer[buffer_used] = '\0';
    return buffer;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        return 1;
    }

    FILE* input = stdin;

    if (strcmp(argv[1], "-") != 0) {
        input = fopen(argv[1], "r");
    }

    setvbuf(stdout, NULL, _IONBF, 0);

    init_band();

    char* program = read_to_string(input);
    if (!program) {
        return error("error reading input", NULL, 0);
    }

    int result = run(program);

    free(program);

    return result;
}
