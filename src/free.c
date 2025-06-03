#include "../include/slow_c.h"

void free_token_list_and_data(TokenList *list)
{
    for (int i = 0; arrlen(list->data) > i; i++)
    {
        Token tk = list->data[i];
        if (tk.type == TK_IDENT)
        {
            free(tk.data.ident);
        };
    }
    arrfree(list->data);
}

void free_expr_list(ExprList);
void free_stmt_list(StmtList);
void free_expr_children(Expr *);
void free_statement_children(Statement *);

void free_expr_list(ExprList list)
{
    for (int i = 0; arrlen(list) > i; i++)
    {
        free_expr_children(&list[i]);
    }
    arrfree(list);
}

void deinit_scope(Scope *s)
{
    if (s->variables != NULL)
        shfree(s->variables);
    if (s != NULL)
        free(s);
}

void free_stmt_list_not_scope(StmtList list)
{
    for (int i = 0; arrlen(list.data) > i; i++)
    {
        free_statement_children(&list.data[i]);
    }
    arrfree(list.data);
}
void free_stmt_list(StmtList list)
{
    free_stmt_list_not_scope(list);

    deinit_scope(list.scope);
}

void free_statement_children(Statement *st)
{
    if (st == NULL)
        return;
    if (
        st->var == STMT_THROW_AWAY ||
        st->var == STMT_RETURN)
    {
        free_expr_children(&st->throw_away);
    }
    else if (st->var == STMT_VARIABLE_ASSIGNMENT)
    {
        VariableAssignment *va = &st->variable_assignment;
        free_expr_children(&va->val);
    }
    else if (st->var == STMT_PROGRAM || st->var == STMT_BLOCK)
    {
        free_stmt_list(st->block);
    }
    else if (st->var == STMT_FUNCTION_DEFINITION)
    {
        FunctionDefinition *fd = &st->function_definition;
        free_stmt_list(fd->args);
        free_stmt_list_not_scope(fd->body);
    }
    else if (st->var == STMT_CONDITIONAL_JUMP)
    {
        ConditionalJump *cj = &st->conditional_jump;
        free_expr_children(cj->condition);
        free_statement_children(cj->then_block);
        free_statement_children(cj->else_block);
    }
    else if (st->var == STMT_LOOP)
    {
        Loop *l = &st->loop;
        free_expr_children(l->condition);
        free(l->condition);
        free_statement_children(l->body);
        free(l->body);
        free_statement_children(l->increment);
        free_statement_children(l->init);
    }
    else
    {
        printf("Not all statement types are freed %d\n", st->var);
        my_exit(-1);
    }
}

void free_expr_children(Expr *nd)
{
    if (nd == NULL)
    {
        printf("Tried to free a null expr\n");
        my_exit(-1);
    }

    ExprVar var = nd->var;
    if (var == BIN_EXPR)
    {
        BinExpr *be = nd->val.bin_expr;
        if (be->l != NULL)
        {
            free_expr_children(be->l);
            free(be->l);
        }
        if (be->r != NULL)
        {
            free_expr_children(be->r);
            free(be->r);
        }
        free(be);
    }
    else if (var == VARIABLE_IDENT)
    {
        // pass
    }
    else if (var == EXPR_CONSTANT)
    {
        // pas
    }
    else if (var == FUNCTION_CALL)
    {
        FunctionCall fc = nd->val.function_call;
        free_expr_list(fc.args);
    }
    else if (var == EMPTY_EXPR)
    {
        // pass
    }
    else
    {
        printf("Not all expr types are freed %d\n", var);
        my_exit(-1);
    }
}

void free_program(Program p)
{
    free_stmt_list((StmtList)p);
};
