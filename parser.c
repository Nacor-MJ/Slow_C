#ifndef PARSER_C
#define PARSER_C

#include "slow_c.h"

char* absolute_start;

void check_types(Type t1, Type t2, Token* tk) {
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
                0
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
    Type type = type_tk->data.type;
    Token* name = consume_token(tk);
    VariableAssignment va = {
        type,
        zero_expr(name),
        {
            name->data.ident,
            0
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
        va.vi.version = -1;
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
    Type type = get_var_type(p, *name);

    eat_token(tk, TK_ASSIGN);
    Expr expr = parse_expr(p, tk);
    check_types(type, expr.type, next_token(tk));
    eat_token(tk, TK_SEMICOLON);
    VariableAssignment va = {
        type,
        expr,
        {
            name->data.ident,
            increase_var_version(p, *name)
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

    Scope* subscope = new_scope(p);

    eat_token(tk, TK_LPAREN);
    
    StmtList args;
    vec_init(&args);
    parse_arg_list(subscope, tk, &args);

    eat_token(tk, TK_RPAREN);

    StmtList body = { 0 };

    if (next_token(tk)->type == TK_LCURLY) {
        consume_token(tk);
        body = parse_block(subscope, tk);
        eat_token(tk, TK_RCURLY);
    }

    deinit_scope(subscope);

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
    return fn;
}

Statement temporary_variable_assignment(Scope* p, Expr e, char* name) {
    Type tp = e.type;
    /*
    if (e.var == VARIABLE_IDENT || e.var == FUNCTION_CALL) {
        tp = get_var_type(p, *(e.start));
    } else {
        print_expr(&e, 0);
        tp = get_expr_type(&e);
    }
    */

    VariableAssignment va = {
        tp,
        e,
        {
            name,
            0
        }
    };

    StmtVal sv = { va };
    Statement st = {
        STMT_VARIABLE_ASSIGNMENT,
        sv,
        e.start
    };
    return st;
}

void push_stmt(StmtList* list, Statement st) {
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-value"
    vec_push(list, st);
    #pragma GCC diagnostic pop
}

// TODO fix this unholy mess of a function
// honestly this is horrible but it seems to work for now :D
VariableIdent flatten_bin_expr(StmtList* list, Statement* st, Scope* p, Token* target_var_tk) {
    bool increase = false; // should the variable version be increased
    VariableAssignment var = st->val.variable_assignment;
    BinExpr* e = var.val.val.bin_expr;

    // TODO create a function export_BinExpr(BinExpr*) -> VariableIdent

    if (e->l->var == BIN_EXPR) {
        StmtVal sv;
        sv.variable_assignment = (VariableAssignment) {
            get_expr_type(e->l),
            *(e->l),
            {
                var.vi.name,
                get_var_version(p, *target_var_tk)
            }
        };
        Statement l = {
            STMT_VARIABLE_ASSIGNMENT,
            sv,
            e->l->start
        };

        VariableIdent l_vi = flatten_bin_expr(list, &l, p, target_var_tk);
        ExprVal ev;
        ev.variable_ident = l_vi;
        Expr l_e = {
            VARIABLE_IDENT,
            ev,
            l.start,
            e->l->type
        };

        *(st->val.variable_assignment.val.val.bin_expr->l) = l_e;
        increase = true;
    }
    if (e->r->var == BIN_EXPR) {

        StmtVal sv;
        int version;
        if (increase) version = increase_var_version(p, *target_var_tk);
        else version = get_var_version(p, *target_var_tk);
        sv.variable_assignment = (VariableAssignment) {
            get_expr_type(e->r),
            *(e->r),
            {
                var.vi.name,
                version
            }
        };
        Statement r = {
            STMT_VARIABLE_ASSIGNMENT,
            sv,
            e->r->start
        };

        VariableIdent r_vi = flatten_bin_expr(list, &r, p, target_var_tk);
        ExprVal ev;
        ev.variable_ident = r_vi;
        Expr r_e = {
            VARIABLE_IDENT,
            ev,
            r.start,
            e->r->type
        };

        *(st->val.variable_assignment.val.val.bin_expr->r) = r_e;
        increase = true;
    }


    if (increase) st->val.variable_assignment.vi.version = increase_var_version(p, *target_var_tk);
    push_stmt(list, *st);

    // check l
    check_types(var.type, e->l->type, e->l->start);
    // check r
    check_types(var.type, e->r->type, e->r->start);

    return var.vi;
}

void flatten_var_assignment(StmtList* list, Statement stmt, Scope* p) {
    VariableAssignment va = stmt.val.variable_assignment;
    StmtVal sv;
    sv.variable_assignment = va;
    Statement st = {
        STMT_VARIABLE_ASSIGNMENT,
        sv,
        stmt.start
    };

    Token tk = *(stmt.start);
    tk.data.ident = va.vi.name;

    if (va.val.var == BIN_EXPR) {
        flatten_bin_expr(list, &st, p, &tk);
    } else {
        push_stmt(list, st);
    }
}

void flatten_block(StmtList* list, Statement stmt, Scope* p) {
    StmtList b = stmt.val.block;
    for (int i = 0; i < b.length; i++) {
        flatten_statement(list, b.data[i], p);
    }
}

void flatten_statement(StmtList* list, Statement stmt, Scope* p) {
    if (stmt.var == STMT_BLOCK || stmt.var == STMT_PROGRAM) {
        flatten_block(list, stmt, p);
    } else if (stmt.var == STMT_VARIABLE_ASSIGNMENT) {
        flatten_var_assignment(list, stmt, p);
    } else if (stmt.var == STMT_FUNCTION_DEFINITION) {
        flatten_block(list, stmt, p);
    } else if (stmt.var == STMT_RETURN) {
        Statement rt = temporary_variable_assignment(p, stmt.val.return_, "return");
        flatten_var_assignment(list, rt, p);
    } else if (stmt.var == STMT_THROWAWAY) {
        Statement th = temporary_variable_assignment(p, stmt.val.throw_away, "throw");
        flatten_var_assignment(list, th, p);
    } else {
        printf("flatten statement not implemented for %d\n", stmt.var);
        my_exit(-1);
    }
}

StmtList parse_block(Scope* p, TokenList* tk) {
    StmtList result;
    vec_init(&result);

    Statement next;
    while (next_token(tk)->type != TK_RCURLY) {
        next = parse_statement(p, tk);


        StmtList tmp;
        vec_init(&tmp);
        flatten_statement(&tmp, next, p);

        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wunused-value"
        vec_extend(&result, &tmp);
        #pragma GCC diagnostic pop

        vec_deinit(&tmp);
    }
    return result;
}

Program parse_program(Parser* p, TokenList* tk) {
    Program result;
    vec_init(&result);

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
    add_variable(p.global_scope, print_tk, VOID, 0);

    src.pars_ptr = 0;

    absolute_start = next_token(&src)->start_of_token;

    Program result = parse_program(&p, &src);

    deinit_scope(p.global_scope);
    return result;
}

#endif
