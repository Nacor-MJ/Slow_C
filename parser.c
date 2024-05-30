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

// TODO: check if this screws up the list thingie, idk how the modifying of lengths works
void append_expr(ExprList* list, Expr nd) {
    arrput(*list, nd);
}
void append_statement(StmtList* list, Statement nd) {
    arrput(list->data, nd);
}

void parse_arg_list(Scope* current_scope, TokenList* tk, StmtList* list) {
    while (next_token(tk)->type != TK_RPAREN) {
        Token* type_tk = eat_token_checked(tk, TK_TYPE_KEYWORD);
        Token* name = eat_token_checked(tk, TK_IDENT);
        Variable* var = add_variable(current_scope, *name, type_tk->data.type);

        VariableAssignment va = {
            type_tk->data.type,
            zero_expr(name),
            var
        };
        Statement nd = (Statement) {
            STMT_VARIABLE_ASSIGNMENT,
            .variable_assignment = va
        };

        append_statement(list, nd);

        if (next_token(tk)->type != TK_RPAREN) eat_token_checked(tk, TK_COMMA);
    }
}


Statement parse_var_declaration(Scope* p, TokenList* tk) {
    Token* type_tk = eat_token_checked(tk, TK_TYPE_KEYWORD);
    Type* type = type_tk->data.type;
    Token* name = eat_token(tk);
    VariableAssignment va = {
        type,
        zero_expr(name),
        NULL
    };
    if (next_token(tk)->type == TK_ASSIGN) {
        eat_token_checked(tk, TK_ASSIGN);
        Expr expr = parse_expr(p, tk);
        check_types(type, expr.type, next_token(tk));
        eat_token_checked(tk, TK_SEMICOLON);
        va.val = expr;
    } else {
        eat_token_checked(tk, TK_SEMICOLON);
    }
    va.ident = add_variable(p, *name, type_tk->data.type);

    return (Statement) {
        STMT_VARIABLE_ASSIGNMENT,
        .variable_assignment = va,
        name
    };
}

Statement parse_var_redeclaration(Scope* p, TokenList* tk) {
    Token* name = eat_token(tk);
    Type* type = get_var_type(p, *name);

    eat_token_checked(tk, TK_ASSIGN);
    Expr expr = parse_expr(p, tk);
    check_types(type, expr.type, next_token(tk));
    eat_token_checked(tk, TK_SEMICOLON);
    VariableAssignment va = {
        type,
        expr,
        get_variable(p, name->data.ident)
    };

    return (Statement) {
        STMT_VARIABLE_ASSIGNMENT,
        .variable_assignment = va,
        name
    };
}

// Parses if statement in these forms:
// if (Expr) Statement
// if (Expr) Statement else Statement
Statement parse_if_statement(Scope * p, TokenList* tk) {
    eat_token_checked(tk, TK_IF);
    eat_token_checked(tk, TK_LPAREN);

    Expr* cond = (Expr*) malloc(sizeof(Expr));
    if (cond == NULL) my_exit(69);
    *cond = parse_expr(p, tk);

    eat_token_checked(tk, TK_RPAREN);

    Statement* if_body = (Statement*) malloc(sizeof(Statement)); 
    if (if_body == NULL) my_exit(69);
    *if_body = parse_statement(p, tk);

    Statement* else_body = NULL;

    if (next_token(tk)->type == TK_ELSE) {
        eat_token_checked(tk, TK_ELSE);
        else_body = (Statement*) malloc(sizeof(Statement));
        if (else_body == NULL) my_exit(69);
        *else_body = parse_statement(p, tk);
    }

    if (next_token(tk)->type == TK_SEMICOLON) eat_token(tk);

    ConditionalJump  l = { cond, if_body, else_body };
    return (Statement) { STMT_CONDITIONAL_JUMP, .conditional_jump = l};
}

Statement parse_statement(Scope* p, TokenList* tk) {
    while (
        next_token(tk)->type == TK_COMMENT ||
        next_token(tk)->type == TK_SEMICOLON
    ) {
        eat_token(tk);
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
            Expr expr = parse_expr(p, tk);
            Statement rst = {
                STMT_THROW_AWAY,
                .throw_away = expr,
                expr.start
            };
            eat_token_checked(tk, TK_SEMICOLON);
            return rst;
        }
    } else if (next->type == TK_RETURN) {
        Token* start = eat_token(tk);

        Statement rst = {
            STMT_RETURN,
            .return_ = parse_expr(p, tk),
            start
        };
        eat_token_checked(tk, TK_SEMICOLON);
        return rst;
    } else if (next->type == TK_IF) {
        return parse_if_statement(p, tk);
    } else if (next->type == TK_LCURLY) {
        StmtList block = parse_block(p, tk);
        return (Statement) {
            STMT_BLOCK,
            .block = block,
            next
        };
    } else {
        printf("Expected statement got ");
        print_error_tok(next, absolute_start);
        my_exit(-1);
    }
}

Statement parse_function_definition(Scope* parent, TokenList* tk) {
    StmtList args = {
        NULL,
        new_scope(parent)
    };

    Token* type_tk = eat_token_checked(tk, TK_TYPE_KEYWORD);
    Token* name = eat_token(tk);

    Variable* var = add_variable(parent, *name, type_tk->data.type);

    eat_token_checked(tk, TK_LPAREN);

    parse_arg_list(args.scope, tk, &args);

    eat_token_checked(tk, TK_RPAREN);

    StmtList body = { 0 };

    if (next_token(tk)->type == TK_LCURLY) {
        body = parse_block(args.scope, tk);
    }
    if (next_token(tk)->type == TK_SEMICOLON) {
        eat_token_checked(tk, TK_SEMICOLON);
    }

    FunctionDefinition fd = {
        var,
        args,
        body
    };

    Statement fn = {
        STMT_FUNCTION_DEFINITION,
        .function_definition = fd,
        type_tk
    };
    return fn;
}

void push_stmt(StmtList* list, Statement st) {
    arrput(list->data, st);
}

// Parse the block in format:
// {
//      StmtList
// }
//
// the caller is responsible for creating the new scope
StmtList parse_block(Scope* current_scope, TokenList* tk) {
    eat_token_checked(tk, TK_LCURLY);

    StmtList result = {
        NULL,
        current_scope
    };

    Statement next;
    while (next_token(tk)->type != TK_RCURLY) {
        next = parse_statement(current_scope, tk);

        arrput(result.data, next);
    }

    eat_token_checked(tk, TK_RCURLY);
    return result;
}

// Parses the global scope
Program parse_program(Parser* parser, TokenList* tk) {
    Program result = {NULL, parser->global_scope};

    Statement next;
    while (next_token(tk)->type != TK_EOF) {
        if ( next_token(tk)->type != TK_COMMENT &&
            next_token(tk)->type != TK_SEMICOLON &&
            next_token(tk)->type != TK_TYPE_KEYWORD) {
            printf("Only assignments allowed in the global scope\n");
            print_error_tok(next_token(tk), absolute_start);
            my_exit(-1);
        }
        next = parse_statement(parser->global_scope, tk);

        arrput(result.data, next);
    }
    return result;
}

// Entry point of parser.c
Program parse(TokenList src){
    Parser p;
    p.global_scope = new_scope(NULL);

    src.pars_ptr = 0;
    absolute_start = next_token(&src)->start_of_token;

    Program result = parse_program(&p, &src);

    return result;
}

#endif
