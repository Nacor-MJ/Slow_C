#ifndef PARSER_C
#define PARSER_C

#include "slow_c.h"

void free_node_children(Node* nd){
    NodeVar var = nd->var;
    if (var == BIN_EXPR) {
        BinExpr* be = nd->val.bin_expr;
        if (be->l != NULL) {
            free_node_children(be->l);
        }
        if (be->r != NULL) {
            free_node_children(be->r);
        }
        free(be); // This is good <3
    } else if (var == BLOCK || var == PROGRAM) {
        NodeList list = nd->val.block; 
        int i; Node* nod;
        vec_foreach(&list, nod, i) {
            free_node_children(nod);
        }
        free(list.data);
    } else if (var == VARIABLE_ASSIGNMENT) {
        VariableAssignment* va = nd->val.variable_assignment;
        free(va->name); // this is char*
        free_node_children(va->val);
        free(va);
    } else if (var == VARIABLE_IDENT) {
        free(nd->val.variable_ident);
    } else if (var == VAL ) {
        // pas
    } else if (var == FUNCTION_CALL) {
        FunctionCall fc = nd->val.function_call;
        NodeList list = fc.args; 
        int i; Node* nd;
        vec_foreach(&list, nd, i) {
            free_node_children(nd);
        }
        free(list.data);
        free(fc.name);
    } else if (var == FUNCTION_DEFINITION) {
        FunctionDefinition fd = nd->val.function_definition;
        NodeList list = fd.body; 
        int i; Node* nd;
        vec_foreach(&list, nd, i) {
            free_node_children(nd);
        }
        free(list.data);
        FunctionCall fc = fd.signature;
        free(fc.name);
        NodeList list2 = fc.args; 
        vec_foreach(&list2, nd, i) {
            free_node_children(nd);
        }
        free(list2.data);
    } else if (var == RETURN) {
        free_node_children(nd->val.return_);
    } else {
        printf("Not all node types are freed, node type: %d\n", var);
        exit(-1);
    }
    free(nd);
}

void add_variable(Parser* p, Token var, Type type){
    int idx;
    vec_find(&p->variables.names, var.data.ident, idx);
    if (idx != -1) {
        printf("Variable '%s' already exists\n", var.data.ident);
        print_error_tok(&var, p->absolute_start);
        exit(-1);
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
        printf("Variable '%s' doesn't exist\n", var.data.ident);
        print_error_tok(&var, p->absolute_start);
        exit(-1);
    }
    return p->variables.types.data[idx];
}
void append_node(NodeList* list, Node* nd) {
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-value"
    vec_push(list, nd);
    #pragma GCC diagnostic pop
}

void parse_arg_list(Parser*p, Token** tk, NodeList* list) {
    while (next_token(tk).type != TK_RPAREN) {
        Node* nd = (Node*) malloc(sizeof(Node));
        if (nd == NULL) exit(69);
        *nd = parse_expr(p, tk);
        if (next_token(tk).type != TK_RPAREN) eat_token(tk, TK_COMMA);
        append_node(list, nd);
    }
}

Node parse_statement(Parser* p, Token** tk) {
    // Variable definiton
    if (next_token(tk).type == TK_TYPE_KEYWORD) {
        Token type_tk = eat_token(tk, TK_TYPE_KEYWORD);
        Token name = consume_token(tk);
        // Variable
        if (next_token(tk).type == TK_ASSIGN) {
            eat_token(tk, TK_ASSIGN);

            Node expr = parse_expr(p, tk);

            eat_token(tk, TK_SEMICOLON);

            NodeVal nv;
            nv.variable_assignment = (VariableAssignment*) malloc(sizeof(VariableAssignment));
            nv.variable_assignment->is_declaration = true;
            if (nv.variable_assignment == NULL) exit(69);
            nv.variable_assignment->name = name.data.ident;
            nv.variable_assignment->val = (Node*) malloc(sizeof(Node));
            *(nv.variable_assignment->val) = expr;
            nv.variable_assignment->type = type_tk.data.type;

            add_variable(p, name, type_tk.data.type);

            Node nd = {
                VARIABLE_ASSIGNMENT,
                nv
            };

            return nd;
        // Function
        } else {
            return parse_function_definition(p, tk, type_tk.data.type, name.data.ident);
        }
    // Variable redeclaration
    } else if (next_token(tk).type == TK_IDENT) {
        Token var = eat_token(tk, TK_IDENT);
        if (next_token(tk).type == TK_ASSIGN) {
            eat_token(tk, TK_ASSIGN);
            Node expr = parse_expr(p, tk);

            eat_token(tk, TK_SEMICOLON);

            NodeVal nv;
            nv.variable_assignment = (VariableAssignment*) malloc(sizeof(VariableAssignment));
            nv.variable_assignment->is_declaration = false;
            if (nv.variable_assignment == NULL) exit(69);
            nv.variable_assignment->name = var.data.ident;
            nv.variable_assignment->val = (Node*) malloc(sizeof(Node));
            *(nv.variable_assignment->val) = expr;
            nv.variable_assignment->type = get_var_type(p, var);

            Node nd = {
                VARIABLE_ASSIGNMENT,
                nv
            };

            return nd;
            // function_call
        } else {
            NodeVal nv;

            vec_init(&nv.function_call.args);

            eat_token(tk, TK_LPAREN);
            
            parse_arg_list(p, tk, &nv.function_call.args);

            eat_token(tk, TK_RPAREN);

            Node nd = {
                FUNCTION_CALL,
                nv
            };

            eat_token(tk, TK_SEMICOLON);

            return nd;
        }
    } else if (next_token(tk).type == TK_RETURN) {
        eat_token(tk, TK_RETURN);
        NodeVal nv;

        nv.return_ = (Node*) malloc(sizeof(Node));
        if (nv.return_ == NULL) exit(69);

        *(nv.return_) = parse_expr(p, tk);
    
        eat_token(tk, TK_SEMICOLON);
            
        Node nd = {
            RETURN,
            nv
        };
        return nd;

    } else {
        printf("Expected statement\n");
        exit(-1);
    }
}

Node* parse_next_statement(Parser* p, Token** tk) {
    Node* result = NULL;
    Token next = next_token(tk);
    if (next.type == TK_TYPE_KEYWORD || next.type == TK_IDENT || next.type == TK_RETURN) {
        result = (Node*) malloc(sizeof(Node));
        if (result == NULL) exit(69);

        *result = parse_statement(p, tk);
    }
    return result;
}

Node parse_function_definition(Parser* p, Token** tk, Type return_type, char* fn_name) {
    eat_token(tk, TK_LPAREN);
    
    NodeList args;
    vec_init(&args);

    parse_arg_list(p, tk, &args);

    eat_token(tk, TK_RPAREN);
    eat_token(tk, TK_LCURLY);

    Node body = parse_program(p, tk);

    eat_token(tk, TK_RCURLY);

    NodeVal fn_val;
    FunctionCall fc = {
        return_type,
        fn_name,
        args
    };
    FunctionDefinition fd = {
        fc,
        body.val.block
    };
    fn_val.function_definition = fd;
    Node fn = {
        FUNCTION_DEFINITION,
        fn_val
    };
    return fn;
}
Node parse_program(Parser* p, Token** tk) {
    NodeList list;
    vec_init(&list);

    NodeVal nv;
    nv.program = list;

    Node* next = parse_next_statement(p, tk);
    while (next != NULL) {
        append_node(&nv.block, next);
        next = parse_next_statement(p, tk);
    }
    Node result = {
        PROGRAM,
        nv
    };
    return result;
}

Node parse_from_tok(Parser* p, Token* tk){
    Node program = parse_program(p, &tk);

    return program;
}

Node parse(Token* src){
    Parser p;
    vec_init(&p.variables.names);
    vec_init(&p.variables.types);

    p.absolute_start = src->start_of_token;

    Node result = parse_from_tok(&p, src);

    vec_deinit(&p.variables.types);
    vec_deinit(&p.variables.names);

    printf("\033[94mParsed Node:\033[0m\n");
    print_node(&result, 0);
    return result;
}

#endif
