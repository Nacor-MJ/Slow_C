#include "../include/slow_c.h"

int temporary_count = 1;
int loop_label_count = 1;

Address new_temporary()
{
    return (Address){
        .temporary = temporary_count++,
        ADDR_TEMPORARY};
};

const ConstVal ZERO = {.integer = 0, CONST_INT};
Address EMPTY_ADDRESS = {.constant = ZERO, ADDR_NONE};

// TAC of a label with Var
TAC label_to_tac(Variable *var)
{
    Address label_a = {.variable = NULL, ADDR_VARIABLE};
    label_a.variable = var;
    TAC t = {EMPTY_ADDRESS, EMPTY_ADDRESS, label_a, TAC_LABEL};
    return t;
}

int get_last_tac(IR *destination)
{
    return (
        arrlen(*destination) - 1);
}

// Appends the new loop label and returns its adress
Address new_loop_label(IR *destination)
{
    IR dest = *destination;
    Address label = {
        .temporary = -(loop_label_count++),
        ADDR_TEMPORARY};
    TAC tac = {
        EMPTY_ADDRESS,
        EMPTY_ADDRESS,
        label,
        TAC_LABEL};
    arrput(dest, tac);

    Address a = {
        .label_index = get_last_tac(&dest),
        ADDR_LABEL};

    *destination = dest;

    return a;
}

TAC_OP binop_to_tacop(Binop b)
{
    TAC_OP t;
    switch (b)
    {
    case OP_PLUS:
        t = TAC_ADD;
        break;
    case OP_MINUS:
        t = TAC_SUB;
        break;
    case OP_TIMES:
        t = TAC_MUL;
        break;
    case OP_DIV:
        t = TAC_DIV;
        break;
    case OP_EQ:
        t = TAC_EQ;
        break;
    default:
        printf("invalid binop_to_tacop: %d\n", b);
        my_exit(69);
        /*
        case OP_EQ:
                t = ;
            break;
        case OP_NE:
                t = ;
            break;
        case OP_MT:
                t = ;
            break;
        case OP_LT:
                t = ;
            break;
        case OP_ME:
                t = ;
            break;
        case OP_LE:
                t = ;
            break;
        */
    }
    return t;
}

Address bin_expr_to_ir(IR *destination, BinExpr *b)
{
    IR dest = *destination;
    Address l_a = expr_to_ir(&dest, b->l);
    Address r_a = expr_to_ir(&dest, b->r);

    Address temporary = new_temporary();

    TAC_OP op = binop_to_tacop(b->op);

    TAC tac = {l_a, r_a, temporary, op};
    arrput(dest, tac);

    *destination = dest;

    return temporary;
}

Address expr_to_ir(IR *destination, Expr *e)
{
    IR dest = *destination;
    Address result = EMPTY_ADDRESS;

    switch (e->var)
    {
    case EXPR_CONSTANT:
        result.constant = e->val.constant;
        result.kind = ADDR_CONSTANT;
        break;
    case VARIABLE_IDENT:
        result.variable = e->val.variable_ident;
        result.kind = ADDR_VARIABLE;
        break;
    case BIN_EXPR:
        BinExpr *b = e->val.bin_expr;
        result = bin_expr_to_ir(&dest, b);
        break;
    case EMPTY_EXPR:
        printf("EMTPY_EXPR shouldn't get here\n");
        my_exit(69);
        break;
    default:
        printf("expr_to_ir not implemented for %d\n", e->var);
        my_exit(69);
    }

    *destination = dest;
    return result;
}

void throwaway_to_ir(IR *destination, Expr e)
{
    IR dest = *destination;
    expr_to_ir(&dest, &e);
    *destination = dest;
}

void return_to_ir(IR *destination, Expr e)
{
    IR dest = *destination;
    TAC return_ir = (TAC){
        expr_to_ir(&dest, &e),
        EMPTY_ADDRESS,
        EMPTY_ADDRESS,
        TAC_RETURN};
    arrput(dest, return_ir);
    *destination = dest;
}

void block_to_ir(IR *destination, StmtList block)
{
    IR dest = *destination;
    for (int i = 0; i < arrlen(block.data); i++)
    {
        statement_to_ir(&dest, &block.data[i]);
    }
    *destination = dest;
}

void while_loop_to_ir(IR *destination, Loop *l)
{
    IR dest = *destination;

    Address loop_back = new_loop_label(&dest);

    Address condition = expr_to_ir(&dest, l->condition);

    TAC tmp = {
        condition,
        EMPTY_ADDRESS,
        EMPTY_ADDRESS,
        TAC_IF_FALSE_JMP,
        {{0}}};
    arrput(dest, tmp);
    int jump_over_index = get_last_tac(&dest);

    statement_to_ir(&dest, l->body);

    TAC goto_jmp = (TAC){
        EMPTY_ADDRESS,
        EMPTY_ADDRESS,
        loop_back,
        TAC_JMP,
        {{0}}};
    arrput(dest, goto_jmp);

    dest[jump_over_index].result = new_loop_label(&dest);

    *destination = dest;
}

void loop_to_ir(IR *destination, Loop *l)
{
    IR dest = *destination;
    switch (l->kind)
    {
    case WHILE:
        while_loop_to_ir(&dest, l);
        break;
    case FOR:
        NOT_IMPLEMENTED;
        break;
    case DO_WHILE:
        NOT_IMPLEMENTED;
        break;
    }

    *destination = dest;
}

void conditional_jump_to_ir(IR *destination, ConditionalJump *c)
{
    IR dest = *destination;
    Address condition = expr_to_ir(&dest, c->condition);

    TAC tmp = {
        condition,
        EMPTY_ADDRESS,
        EMPTY_ADDRESS,
        TAC_IF_FALSE_JMP,
        {{0}}};
    arrput(dest, tmp);

    int jump_over_index = get_last_tac(&dest);

    if (c->else_block != NULL)
    {
        NOT_IMPLEMENTED;
    }

    statement_to_ir(&dest, c->then_block);

    Address jump_over_label = new_loop_label(&dest);
    dest[jump_over_index].result = jump_over_label;

    *destination = dest;
}

void variable_assignment_to_ir(IR *destination, VariableAssignment *va)
{
    IR dest = *destination;

    if (va->val.var == EMPTY_EXPR)
        return;
    Address a = expr_to_ir(&dest, &va->val);
    Address result = {
        .variable = va->ident,
        ADDR_VARIABLE};
    TAC instr = {
        a,
        EMPTY_ADDRESS,
        result,
        TAC_ASSIGN};
    arrput(dest, instr);

    *destination = dest;
}

void statement_to_ir(IR *destination, Statement *st)
{
    IR dest = *destination;
    switch (st->var)
    {
    case STMT_VARIABLE_ASSIGNMENT:
        variable_assignment_to_ir(
            &dest,
            &st->variable_assignment);
        break;
    case STMT_CONDITIONAL_JUMP:
        conditional_jump_to_ir(
            &dest,
            &st->conditional_jump);
        break;
    case STMT_LOOP:
        loop_to_ir(&dest, &st->loop);
        break;
    case STMT_BLOCK:
        block_to_ir(&dest, st->block);
        break;
    case STMT_RETURN:
        return_to_ir(&dest, st->return_);
        break;
    case STMT_THROW_AWAY:
        throwaway_to_ir(&dest, st->throw_away);
        break;
    default:
        printf("I think it's literally imposible to get here but yeah");
        my_exit(69);
        break;
    }

    *destination = dest;
}

// Function Definition to an Array of Three adress codes
//
// the callee is responsible for cleanup
IR function_definition_to_tac(FunctionDefinition *fd)
{
    if (fd->body.data == NULL)
        return NULL;
    IR function_ir = NULL;
    arrput(function_ir, label_to_tac(fd->type));

    for (int i = 0; i < arrlen(fd->body.data); i++)
    {
        Statement st = fd->body.data[i];
        statement_to_ir(&function_ir, &st);
    }

    return function_ir;
}

// Ast of a Program to an Array of Three adress codes.
// every function gets its own array.
//
// the callee is responsible for cleanup.
IRList ast_to_tac(Program *program)
{
    IRList tac = NULL;
    for (int i = 0; i < arrlen(program->data); i++)
    {
        Statement st = program->data[i];
        switch (st.var)
        {
        case STMT_VARIABLE_ASSIGNMENT:
            printf("\033[91mGlobal var assignment not implemented\n");
            my_exit(69); // global var assign
            break;
        case STMT_FUNCTION_DEFINITION:
            IR function_scope = function_definition_to_tac(&st.function_definition);
            arrpush(tac, function_scope);
            break;
        default:
            printf("Unexpected Statement Kind in global scope: %d\n", st.var);
            my_exit(69);
        }
    }
    return tac;
}
