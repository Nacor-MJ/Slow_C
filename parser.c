#ifndef PARSER_C
#define PARSER_C

#include "slow_c.h"

void add_variable(Parser* p, Token var, Type type){
    int idx;
    vec_find(&p->variables.names, var.data.ident, idx);
    if (idx != -1) {
        printf("Variable '%s' already exists\n", var.data.ident);
        print_error_tok(&var, p->absolute_start);
        my_exit(-1);
    }

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-value"
    vec_push(&p->variables.names, var.data.ident);
    vec_push(&p->variables.types, type);
    #pragma GCC diagnostic pop
}
Type get_var_type(Parser* p, Token var){
    int idx;
    vec_find_custom_comp_func(&p->variables.names, var.data.ident, idx, strcmp);
    if (idx == -1) {
        printf("Variables: \n");
        print_vars(p);
        printf("Variable '%s' doesn't exist\n", var.data.ident);
        print_error_tok(&var, p->absolute_start);
        my_exit(-1);
    }
    return p->variables.types.data[idx];
}
void append_expr(ExprList* list, Expr nd) {
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-value"
    vec_push(list, nd);
    #pragma GCC diagnostic pop
}

void parse_arg_list(Parser*p, TokenList* tk, ExprList* list) {
    while (next_token(tk).type != TK_RPAREN) {
        Expr nd = parse_expr(p, tk);
        if (next_token(tk).type != TK_RPAREN) eat_token(tk, TK_COMMA);
        append_expr(list, nd);
    }
}

Statement parse_var_declaration(Parser* p, TokenList* tk) {
    Token type_tk = eat_token(tk, TK_TYPE_KEYWORD);
    Token name = consume_token(tk);
    VariableAssignment va = {
        type_tk.data.type,
        name.data.ident,
        zero_expr(),
        0
    };
    if (next_token(tk).type == TK_ASSIGN) {
        eat_token(tk, TK_ASSIGN);
        Expr expr = parse_expr(p, tk);
        eat_token(tk, TK_SEMICOLON);
        va.val = expr;
    } else {
        eat_token(tk, TK_SEMICOLON);
    }
    StmtVal sv = { va };
    return (Statement) {
        STMT_VARIABLE_ASSIGNMENT,
        sv
    };
}

Statement parse_var_redeclaration(Parser* p, TokenList* tk) {
    Token name = consume_token(tk);

    eat_token(tk, TK_ASSIGN);
    Expr expr = parse_expr(p, tk);
    eat_token(tk, TK_SEMICOLON);
    VariableAssignment va = {
        get_var_type(p, name),
        name.data.ident,
        expr,
        1 // TODO get the actual version
    };

    StmtVal sv = { va };
    return (Statement) {
        STMT_VARIABLE_ASSIGNMENT,
        sv
    };
}

Statement parse_statement(Parser* p, TokenList* tk) {
    while (
        next_token(tk).type == TK_COMMENT ||
        next_token(tk).type == TK_SEMICOLON
    ) {
        consume_token(tk);
    }
    Token next = next_token(tk);
    // Variable definiton
    if (next.type == TK_TYPE_KEYWORD) {
        // Variable
        if (next_token_with_offset(tk, 2).type == TK_ASSIGN) {
            return parse_var_declaration(p, tk);
        // Function
        } else if (next_token_with_offset(tk, 2).type == TK_LPAREN) {
            return parse_function_definition(p, tk);
        } else {
            printf("Expected definiton got ");
            print_error_tok(&next, p->absolute_start);
            my_exit(-1);
        }
    // Variable redeclaration
    } else if (next.type == TK_IDENT) {
        if (next_token_with_offset(tk, 1).type == TK_ASSIGN) {
            return parse_var_redeclaration(p, tk);
            // function_call
        } else if (next_token_with_offset(tk, 1).type == TK_LPAREN) {
            StmtVal sv;
            sv.throw_away = parse_function_call(p, tk);
            Statement rst = {
                STMT_THROWAWAY,
                sv
            };
            eat_token(tk, TK_SEMICOLON);
            return rst;
        } else {
            printf("Expected redeclaration or function call got ");
            print_error_tok(&next, p->absolute_start);
            my_exit(-1);
        }
    } else if (next.type == TK_RETURN) {
        consume_token(tk);
        StmtVal sv;

        sv.return_ = parse_expr(p, tk);

        Statement rst = {
            STMT_THROWAWAY,
            sv
        };
        eat_token(tk, TK_SEMICOLON);
        return rst;
    } else {
        printf("Expected statement got ");
        print_error_tok(&next, p->absolute_start);
        my_exit(-1);
    }
}

Statement parse_function_definition(Parser* p, TokenList* tk) {
    Token type_tk = eat_token(tk, TK_TYPE_KEYWORD);
    Token name = consume_token(tk);
    add_variable(p, name, type_tk.data.type);
    eat_token(tk, TK_LPAREN);
    
    ExprList args;
    vec_init(&args);

    parse_arg_list(p, tk, &args);

    eat_token(tk, TK_RPAREN);
    eat_token(tk, TK_LCURLY);

    StmtList body = parse_block(p, tk);

    eat_token(tk, TK_RCURLY);

    if (next_token(tk).type == TK_SEMICOLON) eat_token(tk, TK_SEMICOLON);

    FunctionCall fc = {
        type_tk.data.type,
        name.data.ident,
        args
    };
    FunctionDefinition fd = {
        fc,
        body
    };
    StmtVal sv;
    sv.function_definition = fd;
    Statement fn = {
        STMT_FUNCTION_DEFINITION,
        sv
    };
    return fn;
}

StmtList parse_block(Parser* p, TokenList* tk) {
    StmtList result;
    vec_init(&result);

    Statement next;
    while (next_token(tk).type != TK_RCURLY) {
        next = parse_statement(p, tk);
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wunused-value"
        vec_push(&result, next);
        #pragma GCC diagnostic pop
    }
    return result;
}

Program parse_program(Parser* p, TokenList* tk) {
    Program result;
    vec_init(&result);

    Statement next;
    while (next_token(tk).type != TK_EOF) {
        next = parse_statement(p, tk);

        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wunused-value"
        vec_push(&result, next);
        #pragma GCC diagnostic pop
    }
    return result;
}

Program parse_from_tok(Parser* p, TokenList tk){
    Program program = parse_program(p, &tk);

    return program;
}

Program parse(TokenList src){
    Parser p;
    vec_init(&p.variables.names);
    vec_init(&p.variables.types);

    TokenData td;
    td.ident = "print";
    Token print_tk = {
        TK_IDENT,
        td,
        NULL,
    };
    add_variable(&p, print_tk, VOID);

    src.pars_ptr = 0;

    p.absolute_start = next_token_ptr(&src)->start_of_token;

    Program result = parse_from_tok(&p, src);

    vec_deinit(&p.variables.types);
    vec_deinit(&p.variables.names);

    return result;
}

#endif
