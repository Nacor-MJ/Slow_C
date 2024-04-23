#include "slow_c.h"

void free_token_list_and_data(TokenList* list) {
    int i; Token tk;
    vec_foreach(list, tk, i) {
        if (tk.type == TK_IDENT) free(tk.data.ident);
    }
    vec_deinit(list);
}

void free_expr_list(ExprList);
void free_stmt_list(StmtList);
void free_expr_children(Expr*);
void free_statement_children(Statement*);

void free_expr_list(ExprList list) {
    int i; Expr nd;
    vec_foreach(&list, nd, i) {
        free_expr_children(&nd);
    }
    vec_deinit(&list);
}

void free_stmt_list(StmtList list) {
    int i; Statement nd;
    vec_foreach(&list, nd, i) {
        free_statement_children(&nd);
    }
    vec_deinit(&list);
}

void free_statement_children(Statement* st) {
    if (
        st->var == STMT_THROWAWAY ||
        st->var == STMT_RETURN
    ) {
        free_expr_children(&st->val.throw_away);
    } else if (st->var == STMT_VARIABLE_ASSIGNMENT) {
        VariableAssignment* va = &st->val.variable_assignment;
        free_expr_children(&va->val);
    } else if (st->var == STMT_PROGRAM || st->var == STMT_BLOCK) {
        free_stmt_list(st->val.block);
    } else if (st->var == STMT_FUNCTION_DEFINITION) {
        FunctionDefinition* fd = &st->val.function_definition;
        free_stmt_list(fd->body);
        free_stmt_list(fd->args);
    } else if (st->var == STMT_CONDITIONAL_JUMP) {
        Conditional_jump* cj = &st->val.conditional_jump;
        free_expr_children(cj->condition);
        free_expr_children(cj->true_block);
        free_expr_children(cj->false_block);
    } else {
        printf("Not all statement types are freed %d\n", st->var);
        my_exit(-1);
    }
}

void free_expr_children(Expr* nd){
    if (nd == NULL) {
        printf("Tried to free a null expr\n");
        my_exit(-1);
    }

    ExprVar var = nd->var;
    if (var == BIN_EXPR) {
        BinExpr* be = nd->val.bin_expr;
        if (be->l != NULL) {
            free_expr_children(be->l);
            free(be->l);
        }
        if (be->r != NULL) {
            free_expr_children(be->r);
            free(be->r);
        }
        free(be); // This is good <3
    } else if (var == VARIABLE_IDENT) {
        // pass
    } else if (var == VAL ) {
        // pas
    } else if (var == FUNCTION_CALL) {
        FunctionCall fc = nd->val.function_call;
        free_expr_list(fc.args);
    } else if (var == EMPTY_EXPR) {
        // pass
    } else {
        printf("Not all expr types are freed %d\n", var);
        my_exit(-1);
    }
}

void free_program(Program p) {
    free_stmt_list((StmtList) p);
};
