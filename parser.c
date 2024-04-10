#ifndef PARSER_C
#define PARSER_C

#include "slow_c.h"

const char* op_enum_to_char(Binop bp) {
    switch (bp) {
        case OP_INVALID:
            return "Invalid";
        case OP_PLUS:
            return "Plus (+)";
        case OP_MINUS:
            return "Minus (-)";
        case OP_TIMES:
            return "Times (*)";
        case OP_DIV:
            return "Divide (/)";
        case OP_EQ:
            return "Equal (==)";
        case OP_NE:
            return "Not Equal (!=)";
        case OP_MT:
            return "Greater Than (>)";
        case OP_LT:
            return "Less Than (<)";
        case OP_ME:
            return "Greater Than or Equal (>=)";
        case OP_LE:
            return "Less Than or Equal (<=)";
        default:
            return "Unknown";
    }
} 

void print_indent(int level, const char* end = "") {
    if (level < 0) return;
    for (int i = 0; i < level; ++i) {
        printf("  ");  // Assuming 2 spaces per indentation level
    }
    printf("%s", end);
}

void print_node(Node *node, int indent) {
    if (node == NULL) {
        // printf("NULL Node\n");
        return;
    }
    
    NodeVar var = node->var;
    
    if (var == VAL) {
        printf("#%d\n", node->val.val);
    } else if (var == BIN_EXPR) {
        printf("{\n");
        print_indent(indent);
        print_node(node->val.bin_expr->l, indent + 1);
        print_indent(indent);
        printf("%s\n", op_enum_to_char(node->val.bin_expr->op));
        print_indent(indent);
        print_node(node->val.bin_expr->r, indent + 1);
        print_indent(indent - 1);
        printf("}\n");
    } else if (var == BLOCK) {
        printf("Program:\n");
        print_indent(indent);
        NodeList ndlist = node->val.block;
        int i; Node* nd;
        vec_foreach(&ndlist, nd, i) {
            print_indent(indent + 1);
            print_node(nd, indent + 1);
        }
    } else if (var == VARIABLE_ASSIGNMENT) {
        VariableAssignment* va = node->val.variable_assignment;
        printf("%d%s = ", va->type, va->name);
        print_node(va->val, indent + 1);
    } else if (var == FUNCTION_CALL) {
        FunctionCall fc = node->val.function_call;
        printf("Function Call: %s\n", fc.name);
        NodeList ndlist = fc.args;
        int i; Node* nd;
        vec_foreach(&ndlist, nd, i) {
            print_node(nd, indent + 1);
        }
    } else if (var == VARIABLE_IDENT) {
        printf("$%s\n", node->val.variable_ident);
    } else {
        printf("print_node not implemented for node type: %d\n", node->var);
    }
}

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
    } else if (var == BLOCK) {
        NodeList list = nd->val.block; 
        int i; Node* nd;
        vec_foreach(&list, nd, i) {
            free_node_children(nd);
        }
        free(list.data);
    } else if (var == VARIABLE_ASSIGNMENT) {
        VariableAssignment* va = nd->val.variable_assignment;
        free(va->name); // this is char*
        free_node_children(va->val);
    } else if (var == VAL || var == VARIABLE_IDENT) {
        // pas
    } else {
        printf("Not all node types are freed, node type: %d\n", var);
        exit(-1);
    }
}

// function that parses a BINOP to a InstructionType
InstructionType binop_to_instructiontype(Binop op){
    switch (op){
        case OP_PLUS:
            return ADD;
        case OP_MINUS:
            return SUB;
        case OP_TIMES:
            return MUL;
        case OP_DIV:
            return DIV;
        case OP_EQ:
            return SETE;
        case OP_NE:
            return SETNE;
        case OP_MT:
            return SETG;
        case OP_LT:
            return SETL;
        case OP_ME:
            return SETGE;
        case OP_LE:
            return SETLE;
        default:
            printf("Invalid Binop\n");
            exit(-1);
    }
}

void add_variable(Parser* p, Token var, Type type){
    int idx;
    vec_find(&p->variables.names, var.data.ident, idx);
    if (idx != -1) {
        printf("Variable '%s' already exists\n", var.data.ident);
        print_error_tok(&var, p->absolute_start);
        exit(-1);
    }

    vec_push(&p->variables.names, var.data.ident);
    vec_push(&p->variables.types, type);
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
void print_vars(Parser* p) {
    int i; char* name;
    vec_foreach(&p->variables.names, name, i) {
        printf("%d $%s\n", p->variables.types.data[i], name);
    }
}

void append_node(NodeList* list, Node* nd) {
    vec_push(list, nd);
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
        eat_token(tk, TK_ASSIGN);

        Node expr = parse_expr(p, tk);

        eat_token(tk, TK_SEMICOLON);

        NodeVal nv;
        nv.variable_assignment = (VariableAssignment*) malloc(sizeof(VariableAssignment));
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
    // Variable redeclaration
    } else if (next_token(tk).type == TK_IDENT) {
        Token var = eat_token(tk, TK_IDENT);
        if (next_token(tk).type == TK_ASSIGN) {
            eat_token(tk, TK_ASSIGN);
            Node expr = parse_expr(p, tk);

            eat_token(tk, TK_SEMICOLON);

            NodeVal nv;
            nv.variable_assignment = (VariableAssignment*) malloc(sizeof(VariableAssignment));
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
    } else {
        printf("Expected statement\n");
        exit(-1);
    }
}

Node* parse_next_statement(Parser* p, Token** tk) {
    Node* result = NULL;
    Token next = next_token(tk);
    if (next.type == TK_TYPE_KEYWORD || next.type == TK_IDENT) {
        result = (Node*) malloc(sizeof(Node));
        if (result == NULL) exit(69);

        *result = parse_statement(p, tk);
    }
    return result;
}

Node parse_function(Parser* p, Token** tk) {
    Type return_type = eat_token(tk, TK_TYPE_KEYWORD).data.type;
    char* fn_name = eat_token(tk, TK_IDENT).data.ident;
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
    nv.block = list;

    Node* next = parse_next_statement(p, tk);
    while (next != NULL) {
        append_node(&nv.block, next);
        next = parse_next_statement(p, tk);
    }
    Node result = {
        BLOCK,
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
    print_node(&result);

    return result;
}

#endif
