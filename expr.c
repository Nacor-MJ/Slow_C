#include "slow_c.h"

extern char* absolute_start;

Expr zero_expr(Token* start) {
    ExprVal ev;
    ev.val = 0;
    Expr e = {
        EMPTY_EXPR,
        ev,
        start
    };
    return e;
}

void assign_l_to_BinExpr(BinExpr* be, Expr nd){
    be->l = (Expr*) malloc(sizeof(Expr));
    if (be->l == NULL) my_exit(69);
    *(be->l) = nd;
}

void assign_r_to_BinExpr(BinExpr* be, Expr nd){
    be->r = (Expr*) malloc(sizeof(Expr));
    if (be->r == NULL) my_exit(69);
    *(be->r) = nd;
}

void parse_expr_list(Scope* p, TokenList* tk, ExprList* list) {
    while (next_token(tk)->type != TK_RPAREN) {
        Expr nd = parse_expr(p, tk);
        append_expr(list, nd);
        if (next_token(tk)->type == TK_COMMA) {
            consume_token(tk);
        }
    }
}

Expr parse_function_call(Scope* p, TokenList* tk) {
    Token* var = eat_token(tk, TK_IDENT);
    ExprVal nv;

    nv.function_call.name = var->data.ident;
    nv.function_call.type = get_var_type(p, *var);

    vec_init(&nv.function_call.args);

    eat_token(tk, TK_LPAREN);

    parse_expr_list(p, tk, &nv.function_call.args);

    eat_token(tk, TK_RPAREN);

    Expr nd = {
        FUNCTION_CALL,
        nv,
        var
    };
    return nd;
}

Expr parse_factor(Scope* p, TokenList* tk){
    Token* next = next_token(tk);
    Token* start = next;

    if (next->type == TK_MINUS){
        consume_token(tk);
        Token* negated = next_token(tk);
        negated->data.num *= -1;
        next = next_token(tk);
    } else if (next->type == TK_PLUS){
        consume_token(tk);
        next = next_token(tk);
    }

    if (next->type == TK_NUM){
        eat_token(tk, TK_NUM);
        ExprVal nv;
        nv.val = next->data.num;
        Expr nd = {
            VAL,
            nv,
            start
        };
        return nd;
    } else if (next->type == TK_LPAREN){
        eat_token(tk, TK_LPAREN);
        Expr result = parse_expr(p, tk);
        eat_token(tk, TK_RPAREN);
        return result;
    } else if (next->type == TK_IDENT) {
        if (next_token_with_offset(tk, 1)->type == TK_LPAREN) {
            return parse_function_call(p, tk);
        }

        Token* name = consume_token(tk);
        
        ExprVal ndata;
        ndata.variable_ident.name = name->data.ident;
        ndata.variable_ident.version = get_var_version(p, *name);
        Expr nd = {
            VARIABLE_IDENT,
            ndata,
            start
        };
        return nd;

    } else {
        printf("Expected factor, got ");
        print_error_tok(next, absolute_start);
        my_exit(-1);
    }
}

Expr parse_term(Scope* p, TokenList* tk){
    Expr factor = parse_factor(p, tk);

    if (next_token(tk)->type == TK_TIMES || next_token(tk)->type == TK_DIV){
        BinExpr result = {
            NULL,
            OP_INVALID,
            NULL,
        };
        assign_l_to_BinExpr(&result, factor);

        Token* token = next_token(tk);

        switch (token->type){
            case (TK_TIMES):
                result.op = OP_TIMES;
                break;
            case (TK_DIV):
                result.op = OP_DIV;
                break;
            default:
                printf("Expected term");
                my_exit(-1);
        }

        consume_token(tk);
        assign_r_to_BinExpr(&result, parse_factor(p, tk));

        ExprVal nv;
        nv.bin_expr = (BinExpr*) malloc(sizeof(BinExpr));
        *(nv.bin_expr) = result;

        Expr nd = {
            BIN_EXPR,
            nv,
            factor.start
        };

        return nd;
    } else {
        return factor; 
    }

}

Expr parse_bin_expr(Scope* p, TokenList* tk){
    Expr term = parse_term(p, tk);

    if (next_token(tk)->type == TK_PLUS || next_token(tk)->type == TK_MINUS){
        
        Token* op = consume_token(tk);

        BinExpr result = {
            NULL,
            OP_INVALID,
            NULL,
        };
        assign_l_to_BinExpr(&result, term);

        switch (op->type){
            case (TK_MINUS):
                result.op = OP_MINUS;
                assign_r_to_BinExpr(&result, parse_term(p, tk));
                break;
            case (TK_PLUS):
                result.op = OP_PLUS;
                assign_r_to_BinExpr(&result, parse_term(p, tk));
                break;
            default:
                printf("Expected Plus or Minus");
                my_exit(-1);
        }
        ExprVal nv;
        nv.bin_expr = (BinExpr*) malloc(sizeof(BinExpr));
        *(nv.bin_expr) = result;

        Expr nd = {
            BIN_EXPR,
            nv,
            term.start
        };

        return nd;
    } else {
        return term;
    }
}

// <, >, <=, >=
Expr parse_relational_expr(Scope* p, TokenList* tk){
    Expr bin_expr = parse_bin_expr(p, tk);

    if (
        next_token(tk)->type == TK_MT ||
        next_token(tk)->type == TK_LT ||
        next_token(tk)->type == TK_ME ||
        next_token(tk)->type == TK_LE
    ){
        
        Token* op = consume_token(tk);

        BinExpr result = {
            NULL,
            OP_INVALID,
            NULL,
        };
        assign_l_to_BinExpr(&result, bin_expr);

        switch (op->type){
            case (TK_MT):
                result.op = OP_MT;
                break;
            case (TK_LT):
                result.op = OP_LT;
                break;
            case (TK_ME):
                result.op = OP_ME;
                break;
            case (TK_LE):
                result.op = OP_LE;
                break;
            default:
                printf("Expected a Comparison token");
                my_exit(-1);
        }
        
        assign_r_to_BinExpr(&result, parse_term(p, tk));
        
        ExprVal nv;
        nv.bin_expr = (BinExpr*) malloc(sizeof(BinExpr));
        *(nv.bin_expr) = result;

        Expr nd = {
            BIN_EXPR,
            nv,
            bin_expr.start
        };

        return nd;
    } else {
        return bin_expr;
    }
}

// == !=
Expr parse_eq_ne(Scope* p, TokenList* tk){
    Expr bin_expr = parse_relational_expr(p, tk);

    if (
        next_token(tk)->type == TK_EQ ||
        next_token(tk)->type == TK_NE
    ){
        
        Token* op = consume_token(tk);

        BinExpr result = {
            NULL,
            OP_INVALID,
            NULL,
        };
        assign_l_to_BinExpr(&result, bin_expr);

        switch (op->type){
            case (TK_NE):
                result.op = OP_NE;
                break;
            case (TK_EQ):
                result.op = OP_EQ;
                break;
            default:
                printf("Expected a Comparison token");
                my_exit(-1);
        }
        
        assign_r_to_BinExpr(&result, parse_term(p, tk));
        
        ExprVal nv;
        nv.bin_expr = (BinExpr*) malloc(sizeof(BinExpr));
        *(nv.bin_expr) = result;

        Expr nd = {
            BIN_EXPR,
            nv,
            bin_expr.start
        };

        return nd;
    } else {
        return bin_expr;
    }
}

Expr parse_expr(Scope* p, TokenList* tk){
    Expr e = parse_eq_ne(p, tk);
    return e;
}

Type get_expr_type(Expr* e){
    Type tp;

    switch (e->var) {
        case FUNCTION_CALL:
            tp = e->val.function_call.type;
            break;
        case BIN_EXPR:
            tp = get_expr_type(e->val.bin_expr->l);
            break;
        case VAL:
            tp = INT;
            break;
        default:
            printf("get_expr_type %d not implemented\n", e->var);
            my_exit(-1);
    }

    return tp;
} 
