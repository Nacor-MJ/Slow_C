#include "slow_c.h"

void assign_l_to_BinExpr(BinExpr* be, Node nd){
    be->l = (Node*) malloc(sizeof(Node));
    if (be->l == NULL) {
        printf("Failed to allocate memory\n");
        exit(-1);
    };
    *(be->l) = nd;
}

void assign_r_to_BinExpr(BinExpr* be, Node nd){
    be->r = (Node*) malloc(sizeof(Node));
    if (be->r == NULL) {
        printf("Failed to allocate memory\n");
        exit(-1);
    };
    *(be->r) = nd;
}

Node parse_factor(Parser* p, Token** tk){
    Token next = next_token(tk);

    if (next.type == TK_MINUS){
        eat_token(tk, TK_MINUS);
        (*tk)->data.num *= -1;
        next = next_token(tk);
    } else if (next.type == TK_PLUS){
        eat_token(tk, TK_PLUS);
        next = next_token(tk);
    }

    if (next.type == TK_NUM){
        eat_token(tk, TK_NUM);
        NodeVal nv;
        nv.val = next.data.num;
        Node nd = {
            VAL,
            nv
        };
        return nd;
    } else if (next.type == TK_LPAREN){
        eat_token(tk, TK_LPAREN);
        Node result = parse_expr(p, tk);
        eat_token(tk, TK_RPAREN);
        return result;
    } else if (next.type == TK_IDENT) {
        eat_token(tk, TK_IDENT);
        
        NodeVal ndata;
        ndata.variable_ident = next.data.ident;
        Node nd = {
            VARIABLE_IDENT,
            ndata
        };
        return nd;

    } else {
        printf("Expected factor, got: ");
        print_token(&next);
        exit(-1);
    }
}

Node parse_term(Parser* p, Token** tk){
    Node factor = parse_factor(p, tk);

    if (next_token(tk).type == TK_TIMES || next_token(tk).type == TK_DIV){
        BinExpr result = {
            NULL,
            OP_INVALID,
            NULL,
        };
        assign_l_to_BinExpr(&result, factor);

        Token token = next_token(tk);

        switch (token.type){
            case (TK_TIMES):
                result.op = OP_TIMES;
                break;
            case (TK_DIV):
                result.op = OP_DIV;
                break;
            default:
                printf("Expected term");
                exit(-1);
        }

        consume_token(tk);
        assign_r_to_BinExpr(&result, parse_factor(p, tk));

        NodeVal nv;
        nv.bin_expr = (BinExpr*) malloc(sizeof(BinExpr));
        *(nv.bin_expr) = result;

        Node nd = {
            BIN_EXPR,
            nv
        };

        return nd;
    } else {
        return factor; 
    }

}

Node parse_bin_expr(Parser* p, Token** tk){
    Node term = parse_term(p, tk);

    if (next_token(tk).type == TK_PLUS || next_token(tk).type == TK_MINUS){
        
        Token op = consume_token(tk);

        BinExpr result = {
            NULL,
            OP_INVALID,
            NULL,
        };
        assign_l_to_BinExpr(&result, term);

        switch (op.type){
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
                exit(-1);
        }
        NodeVal nv;
        nv.bin_expr = (BinExpr*) malloc(sizeof(BinExpr));
        *(nv.bin_expr) = result;

        Node nd = {
            BIN_EXPR,
            nv
        };

        return nd;
    } else {
        return term;
    }
}


// <, >, <=, >=
Node parse_relational_expr(Parser* p, Token** tk){
    Node bin_expr = parse_bin_expr(p, tk);

    if (
        next_token(tk).type == TK_MT ||
        next_token(tk).type == TK_LT ||
        next_token(tk).type == TK_ME ||
        next_token(tk).type == TK_LE
    ){
        
        Token op = consume_token(tk);

        BinExpr result = {
            NULL,
            OP_INVALID,
            NULL,
        };
        assign_l_to_BinExpr(&result, bin_expr);

        switch (op.type){
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
                exit(-1);
        }
        
        assign_r_to_BinExpr(&result, parse_term(p, tk));
        
        NodeVal nv;
        nv.bin_expr = (BinExpr*) malloc(sizeof(BinExpr));
        *(nv.bin_expr) = result;

        Node nd = {
            BIN_EXPR,
            nv
        };

        return nd;
    } else {
        return bin_expr;
    }
}

// == !=
Node parse_eq_ne(Parser* p, Token** tk){
    Node bin_expr = parse_relational_expr(p, tk);

    if (
        next_token(tk).type == TK_EQ ||
        next_token(tk).type == TK_NE
    ){
        
        Token op = consume_token(tk);

        BinExpr result = {
            NULL,
            OP_INVALID,
            NULL,
        };
        assign_l_to_BinExpr(&result, bin_expr);

        switch (op.type){
            case (TK_NE):
                result.op = OP_NE;
                break;
            case (TK_EQ):
                result.op = OP_EQ;
                break;
            default:
                printf("Expected a Comparison token");
                exit(-1);
        }
        
        assign_r_to_BinExpr(&result, parse_term(p, tk));
        
        NodeVal nv;
        nv.bin_expr = (BinExpr*) malloc(sizeof(BinExpr));
        *(nv.bin_expr) = result;

        Node nd = {
            BIN_EXPR,
            nv
        };

        return nd;
    } else {
        return bin_expr;
    }
}

Node parse_expr(Parser* p, Token** tk){
    return parse_eq_ne(p, tk);
}

