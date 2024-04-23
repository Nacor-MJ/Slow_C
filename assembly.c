#include "slow_c.h"

void generate_asm(FILE* f, Program* program) {
    for (int i = 0; i < program->length; i++) {
        Statement st = program->data[i];
        print_statement(&st, 0);
        printf("\n");
    }
}
