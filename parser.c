#ifndef PARSER_C
#define PARSER_C

#include "slow_c.h"

char* absolute_start;

void check_types(Type* t1, Type* t2, Token* tk) {
    if (t1 != t2) {
        printf("\033[31mType mismatch %s!=%s\033[0m", type_to_string(t1), type_to_string(t2));

        TokenData tv;
        tv.ident = "";
        Token dbg_tk = {
            TK_IDENT,
            tv,
            tk->start_of_token
        };

        print_error_tok(&dbg_tk, absolute_start);
    }
}

// TODO check if this screws up the list thingie, idk how the modifying of lengths works
void append_expr(ExprList* list, Expr nd) {
    arrput(*list, nd);
}
void append_statement(StmtList* list, Statement nd) {
    arrput(list->data, nd);
}

void parse_arg_list(Scope*p, TokenList* tk, StmtList* list) {
    while (next_token(tk)->type != TK_RPAREN) {
        Token* type_tk = eat_token(tk, TK_TYPE_KEYWORD);
        Token* name = eat_token(tk, TK_IDENT);
        add_variable(p, *name, type_tk->data.type, 0);

        StmtVal sv;
        VariableAssignment va = {
            type_tk->data.type,
            zero_expr(name),
            {
                name->data.ident,
            }
        };
        sv.variable_assignment = va; 
        Statement nd = (Statement) {
            STMT_VARIABLE_ASSIGNMENT,
            sv
        };

        append_statement(list, nd);

        if (next_token(tk)->type != TK_RPAREN) eat_token(tk, TK_COMMA);
    }
}


Statement parse_var_declaration(Scope* p, TokenList* tk) {
    Token* type_tk = eat_token(tk, TK_TYPE_KEYWORD);
    Type* type = type_tk->data.type;
    Token* name = consume_token(tk);
    VariableAssignment va = {
        type,
        zero_expr(name),
        {
            name->data.ident,
        }
    };
    if (next_token(tk)->type == TK_ASSIGN) {
        eat_token(tk, TK_ASSIGN);
        Expr expr = parse_expr(p, tk);
        check_types(type, expr.type, next_token(tk));
        eat_token(tk, TK_SEMICOLON);
        va.val = expr;
        add_variable(p, *name, type_tk->data.type, 0);
    } else {
        add_variable(p, *name, type_tk->data.type, -1);
        eat_token(tk, TK_SEMICOLON);
    }

    StmtVal sv = { va };
    return (Statement) {
        STMT_VARIABLE_ASSIGNMENT,
        sv,
        name
    };
}

Statement parse_var_redeclaration(Scope* p, TokenList* tk) {
    Token* name = consume_token(tk);
    Type* type = get_var_type(p, *name);

    eat_token(tk, TK_ASSIGN);
    Expr expr = parse_expr(p, tk);
    check_types(type, expr.type, next_token(tk));
    eat_token(tk, TK_SEMICOLON);
    VariableAssignment va = {
        type,
        expr,
        {
            name->data.ident,
        }
    };

    StmtVal sv = { va };
    return (Statement) {
        STMT_VARIABLE_ASSIGNMENT,
        sv,
        name
    };
}


Statement parse_statement(Scope* p, TokenList* tk) {
    while (
        next_token(tk)->type == TK_COMMENT ||
        next_token(tk)->type == TK_SEMICOLON
    ) {
        consume_token(tk);
    }

    Token* next = next_token(tk);
    // Variable definiton
    if (next->type == TK_TYPE_KEYWORD) {
        Token* over_next = next_token_with_offset(tk, 2);
        // Variable
        if (over_next->type == TK_ASSIGN || over_next->type == TK_SEMICOLON) {
            return parse_var_declaration(p, tk);
        // Function
        } else if (over_next->type == TK_LPAREN) {
            return parse_function_definition(p, tk);
        } else {
            printf("Expected definiton got ");
            print_error_tok(next, absolute_start);
            my_exit(-1);
        }
    // Variable redeclaration
    } else if (next->type == TK_IDENT) {
        if (next_token_with_offset(tk, 1)->type == TK_ASSIGN) {
            return parse_var_redeclaration(p, tk);
            // function_call
        } else {
            StmtVal sv;
            sv.throw_away = parse_expr(p, tk);
            Statement rst = {
                STMT_THROWAWAY,
                sv,
                sv.throw_away.start
            };
            eat_token(tk, TK_SEMICOLON);
            return rst;
        }
    } else if (next->type == TK_RETURN) {
        Token* start = consume_token(tk);
        StmtVal sv;

        sv.return_ = parse_expr(p, tk);

        Statement rst = {
            STMT_RETURN,
            sv,
            start
        };
        eat_token(tk, TK_SEMICOLON);
        return rst;
    } else {
        printf("Expected statement got ");
        print_error_tok(next, absolute_start);
        my_exit(-1);
    }
}

Statement parse_function_definition(Scope* p, TokenList* tk) {
    Token* type_tk = eat_token(tk, TK_TYPE_KEYWORD);
    Token* name = consume_token(tk);
    add_variable(p, *name, type_tk->data.type, 0);

    StmtList args = new_stmt_list(p);

    eat_token(tk, TK_LPAREN);
    
    parse_arg_list(args.scope, tk, &args);

    eat_token(tk, TK_RPAREN);

    StmtList body = { 0 };

    if (next_token(tk)->type == TK_LCURLY) {
        consume_token(tk);
        body = parse_block(args.scope, tk);
        eat_token(tk, TK_RCURLY);
    }

    FunctionDefinition fd = {
        type_tk->data.type,
        name->data.ident,
        args,
        body
    };

    if (next_token(tk)->type == TK_SEMICOLON) eat_token(tk, TK_SEMICOLON);

    StmtVal sv;
    sv.function_definition = fd;
    Statement fn = {
        STMT_FUNCTION_DEFINITION,
        sv,
        type_tk
    };
    printf("%p\n", args.scope);
    return fn;
}

void push_stmt(StmtList* list, Statement st) {
    arrput(list->data, st);
}

StmtList parse_block(Scope* p, TokenList* tk) {
    StmtList result = new_stmt_list(p);

    Statement next;
    while (next_token(tk)->type != TK_RCURLY) {
        next = parse_statement(p, tk);

        arrput(result.data, next);
    }
    return result;
}

Program parse_program(Parser* p, TokenList* tk) {
    Program result = new_stmt_list(p->global_scope);

    Statement next;
    while (next_token(tk)->type != TK_EOF) {
        if ( next_token(tk)->type != TK_COMMENT &&
            next_token(tk)->type != TK_SEMICOLON &&
            next_token(tk)->type != TK_TYPE_KEYWORD) {
            printf("Only assignments allowed in the global scope\n");
            print_error_tok(next_token(tk), absolute_start);
        }
        next = parse_statement(p->global_scope, tk);

        // TODO check that there are no function calls or redeclarations in the global scope

        arrput(result.data, next);
    }
    return result;
}

Program parse(TokenList src){
    Parser p;
    p.global_scope = new_scope(NULL);

    src.pars_ptr = 0;

    absolute_start = next_token(&src)->start_of_token;

    Program result = parse_program(&p, &src);
    return result;
}

#endif
