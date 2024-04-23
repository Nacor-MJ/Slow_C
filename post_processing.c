#include "slow_c.h"


void post_processing(Program* root) {
    for (int i = 0; i < root->length; i++) {
        Statement st = root->data[i];
        print_statement(&st, 0);
        printf("\n------\n");
    }
    return ;
}
