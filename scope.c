#include "slow_c.h"

extern char* absolute_start;

// returns the pointer to the variable
Variable* add_variable(Scope* p, Token var, Type* type) {
    char* identifier = var.data.ident;
    int idx = shgeti(p->variables, identifier);
    if (idx != -1) {
        printf("Variable '%s' already exists\n", var.data.ident);
        print_error_tok(&var, absolute_start);
        my_exit(-1);
    }

    shput(p->variables, var.data.ident, type);

    Variable* result = shgetp_null(p->variables, var.data.ident);  

    return result;
}
Variable* get_variable(Scope* p, char* ident) {
    return shgetp_null(p->variables, ident);
}
Type* get_var_type(Scope* p, Token var) {
    int idx = shgeti(p->variables, var.data.ident);
    if (idx == -1) {
        if (p->parent != NULL) {
            return get_var_type(p->parent, var);
        } else {
            printf("Var '%s' doesn't exist\n", var.data.ident);
            print_error_tok(&var, absolute_start);
            my_exit(-1);
        }
    }
    return p->variables[idx].value;
}

Scope* new_scope(Scope* parent) {
    Scope* result = (Scope*) malloc(sizeof(Scope));
    if (result == NULL) my_exit(69);

    Variable* vars = NULL;

    int depth = 0;
    if (parent != NULL) {
        depth = parent->depth + 1;
    }

    *result = (Scope) {
        vars,
        parent,
        depth
    };
    return result;
}

// Creates an empty StmtList with p as its parent scope
StmtList new_stmt_list(Scope* p) {
    Scope* subscope = new_scope(p);
    StmtList result = {
        NULL,
        subscope
    };
    return result;
}
