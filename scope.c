#include "slow_c.h"

extern char* absolute_start;

int idfk (const char *p1, const char *p2)
{
  const unsigned char *s1 = (const unsigned char *) p1;
  const unsigned char *s2 = (const unsigned char *) p2;
  unsigned char c1, c2;
  do
    {
      c1 = (unsigned char) *s1++;
      c2 = (unsigned char) *s2++;
      if (c1 == '\0') return c1 - c2;
    }
  while (c1 == c2);
  return c1 - c2;
}

void add_variable(Scope* p, Token var, Type type, int version) {
    int idx;
    vec_find_custom_comp_func(&p->variables.names, var.data.ident, idx, idfk);
    if (idx != -1) {
        if ( strcmp(var.data.ident, "$throw" ) == 0 ||
             strcmp(var.data.ident, "$return") == 0) {
            return;
        }
        printf("Variable '%s' already exists\n", var.data.ident);
        print_error_tok(&var, absolute_start);
        my_exit(-1);
    }

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-value"

    char* tmp = malloc(strlen(var.data.ident) + 1);
    if (tmp == NULL) my_exit(69);
    strcpy(tmp, var.data.ident);

    vec_push(&p->variables.names, tmp);
    vec_push(&p->variables.types, type);
    vec_push(&p->variables.versions, version);
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
    char* name = var.data.ident;
    if (strcmp(name, "$throw") == 0 ||
        strcmp(name, "$return") == 0) {
        return 0;
    }
    int idx;
    vec_find_custom_comp_func(&p->variables.names, name, idx, idfk);
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
    int idx; char* name;
    vec_foreach(&s->variables.names, name, idx) {
        free(name);
    }
    vec_deinit(&s->variables.names);
    vec_deinit(&s->variables.types);
    vec_deinit(&s->variables.versions);
    free(s);
}
