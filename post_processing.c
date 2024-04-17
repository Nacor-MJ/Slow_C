#include "slow_c.h"

// void increase_var_version(StmtList);

void post_processing(Program* root) {
    for (int i = 0; i < root->length; i++) {
        Statement st = root->data[i];
        if (st.var == STMT_FUNCTION_DEFINITION) {
            StmtList body = st.val.function_definition.body;
        }
        print_statement(&st, 0);
        printf("\n------\n");

    }
    return ;
}

