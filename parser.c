#ifndef PARSER_C
#define PARSER_C

#include "slow_c.h"

char* absolute_start;
void append_expr(ExprList* list, Expr nd) {
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-value"
    vec_push(list, nd);
    #pragma GCC diagnostic pop
}
void append_statement(StmtList* list, Statement nd) {
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-value"
    vec_push(list, nd);
    #pragma GCC diagnostic pop
}

void parse_arg_list(Scope*p, TokenList* tk, StmtList* list) {
    while (next_token(tk).type != TK_RPAREN) {
        Token type_tk = eat_token(tk, TK_TYPE_KEYWORD);
        Token name = eat_token(tk, TK_IDENT);
        add_variable(p, name, type_tk.data.type);

        StmtVal sv;
        VariableAssignment va = {
            type_tk.data.type,
            name.data.ident,
            zero_expr(),
            0
        };
        sv.variable_assignment = va; 
        Statement nd = (Statement) {
            STMT_VARIABLE_ASSIGNMENT,
            sv
        };

        append_statement(list, nd);

        if (next_token(tk).type != TK_RPAREN) eat_token(tk, TK_COMMA);
    }
}

Statement parse_var_declaration(Scope* p, TokenList* tk) {
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

    add_variable(p, name, type_tk.data.type);
    StmtVal sv = { va };
    return (Statement) {
        STMT_VARIABLE_ASSIGNMENT,
        sv
    };
}

Statement parse_var_redeclaration(Scope* p, TokenList* tk) {
    Token name = consume_token(tk);

    eat_token(tk, TK_ASSIGN);
    Expr expr = parse_expr(p, tk);
    eat_token(tk, TK_SEMICOLON);
    VariableAssignment va = {
        get_var_type(p, name),
        name.data.ident,
        expr,
        increase_var_version(p, name)
    };

    StmtVal sv = { va };
    return (Statement) {
        STMT_VARIABLE_ASSIGNMENT,
        sv
    };
}

Statement parse_statement(Scope* p, TokenList* tk) {
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
            print_error_tok(&next, absolute_start);
            my_exit(-1);
        }
    // Variable redeclaration
    } else if (next.type == TK_IDENT) {
        if (next_token_with_offset(tk, 1).type == TK_ASSIGN) {
            return parse_var_redeclaration(p, tk);
            // function_call
        } else {
            StmtVal sv;
            sv.throw_away = parse_expr(p, tk);
            Statement rst = {
                STMT_THROWAWAY,
                sv
            };
            eat_token(tk, TK_SEMICOLON);
            return rst;
        }
    } else if (next.type == TK_RETURN) {
        consume_token(tk);
        StmtVal sv;

        sv.return_ = parse_expr(p, tk);

        Statement rst = {
            STMT_RETURN,
            sv
        };
        eat_token(tk, TK_SEMICOLON);
        return rst;
    } else {
        printf("Expected statement got ");
        print_error_tok(&next, absolute_start);
        my_exit(-1);
    }
}

Statement parse_function_definition(Scope* p, TokenList* tk) {
    Token type_tk = eat_token(tk, TK_TYPE_KEYWORD);
    Token name = consume_token(tk);
    add_variable(p, name, type_tk.data.type);
    eat_token(tk, TK_LPAREN);
    
    StmtList args;
    vec_init(&args);

    parse_arg_list(p, tk, &args);

    eat_token(tk, TK_RPAREN);
    eat_token(tk, TK_LCURLY);

    Scope* subscope = new_scope(p);

    StmtList body = parse_block(subscope, tk);

    eat_token(tk, TK_RCURLY);

    if (next_token(tk).type == TK_SEMICOLON) eat_token(tk, TK_SEMICOLON);

    FunctionDefinition fd = {
        type_tk.data.type,
        name.data.ident,
        args,
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

StmtList parse_block(Scope* p, TokenList* tk) {
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
        if ( next_token(tk).type != TK_COMMENT &&
            next_token(tk).type != TK_SEMICOLON &&
            next_token(tk).type != TK_TYPE_KEYWORD) {
            printf("Only assignments allowed in the global scope\n");
            print_error_tok(next_token_ptr(tk), absolute_start);
        }
        next = parse_statement(p->global_scope, tk);

        // TODO check that there are no function calls or redeclarations in the global scope

        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wunused-value"
        vec_push(&result, next);
        #pragma GCC diagnostic pop
    }
    return result;
}

Program parse(TokenList src){
    Parser p;
    p.global_scope = new_scope(NULL);

    TokenData td;
    td.ident = "print";
    Token print_tk = {
        TK_IDENT,
        td,
        NULL,
    };
    add_variable(p.global_scope, print_tk, VOID);

    src.pars_ptr = 0;

    absolute_start = next_token_ptr(&src)->start_of_token;

    Program result = parse_program(&p, &src);

    deinit_scope(p.global_scope);
    return result;
}

#endif
