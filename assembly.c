#include "slow_c.h"


void statement_to_asm(FILE* f, Statement* st) {
    switch (st->var) {
        case STMT_RETURN:
            fprintf(f, "\tret\n");
            break;
        default:
            printf("statement_to_asm not implemented for type %d\n", st->var);
    }
}

void prolog(FILE* f) {
    fprintf(f, "\t.text\n");
}

void generate_asm(FILE* f, Program* program) {
    prolog(f);
}
