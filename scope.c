#include "slow_c.h"

extern char* absolute_start;

void add_variable(Scope* p, Token var, Type type){
    int idx;
    vec_find_custom_comp_func(&p->variables.names, var.data.ident, idx, strcmp);
    if (idx != -1) {
        printf("Variable '%s' already exists\n", var.data.ident);
        print_error_tok(&var, absolute_start);
        my_exit(-1);
    }

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-value"
    vec_push(&p->variables.names, var.data.ident);
    vec_push(&p->variables.types, type);
    vec_push(&p->variables.versions, 0);
    #pragma GCC diagnostic pop
}
// returns the new var version
int increase_var_version(Scope* p, Token var){
    int idx;
    vec_find_custom_comp_func(&p->variables.names, var.data.ident, idx, strcmp);
    if (idx == -1) {
        if (p->parent != NULL) {
            return increase_var_version(p->parent, var);
        } else {
            printf("Var '%s' doesn't exist\n", var.data.ident);
            print_error_tok(&var, absolute_start);
            my_exit(-1);
        }
    }
    p->variables.versions.data[idx]++;
    return p->variables.versions.data[idx];
}
int get_var_version(Scope* p, Token var){
    int idx;
    vec_find_custom_comp_func(&p->variables.names, var.data.ident, idx, strcmp);
    if (idx == -1) {
        if (p->parent != NULL) {
            return get_var_version(p->parent, var);
        } else {
            printf("Var '%s' doesn't exist\n", var.data.ident);
            print_error_tok(&var, absolute_start);
            my_exit(-1);
        }
    }
    return p->variables.versions.data[idx];
    
}
Type get_var_type(Scope* p, Token var){
    int idx;
    vec_find_custom_comp_func(&p->variables.names, var.data.ident, idx, strcmp);
    if (idx == -1) {
        if (p->parent != NULL) {
            return get_var_type(p->parent, var);
        } else {
            printf("Var '%s' doesn't exist\n", var.data.ident);
            print_error_tok(&var, absolute_start);
            my_exit(-1);
        }
    }
    return p->variables.types.data[idx];
}

Scope* new_scope(Scope* parent) {
    Scope* result = (Scope*) malloc(sizeof(Scope));
    if (result == NULL) my_exit(69);

    Variables vars;
    vec_init(&vars.names);
    vec_init(&vars.types);
    vec_init(&vars.versions);

    *result = (Scope) {
        vars,
        parent
    };
    return result;
}
void deinit_scope(Scope* s) {
    vec_deinit(&s->variables.names);
    vec_deinit(&s->variables.types);
    vec_deinit(&s->variables.versions);
    free(s);
}
