#include "slow_c.h"


const char* op_enum_to_char(Binop bp) {
    switch (bp) {
        case OP_INVALID:
            return "Invalid";
        case OP_PLUS:
            return "+";
        case OP_MINUS:
            return "-";
        case OP_TIMES:
            return "*";
        case OP_DIV:
            return "/";
        case OP_EQ:
            return "==";
        case OP_NE:
            return "!=";
        case OP_MT:
            return ">";
        case OP_LT:
            return "<";
        case OP_ME:
            return ">=";
        case OP_LE:
            return "<=";
        default:
            return "Unknown";
    }
}

void print_indent(int level, const char* end) {
    if (level < 0) return;
    for (int i = 0; i < level; ++i) {
        printf("  ");  // Assuming 2 spaces per indentation level
    }
    printf("%s", end);
}

void print_program(NodeList block, int indent) {
    int i; Node* nd;
    vec_foreach(&block, nd, i) {
        print_node(nd, indent);
    }
}

void print_block(NodeList block, int indent) {
    printf("{\n");
    print_program(block, indent + 1);
    printf("}\n");
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
        print_indent(indent, "");
        print_node(node->val.bin_expr->l, indent + 1);
        print_indent(indent, "");
        printf("%s\n", op_enum_to_char(node->val.bin_expr->op));
        print_indent(indent, "");
        print_node(node->val.bin_expr->r, indent + 1);
        print_indent(indent - 1, "");
        printf("}\n");
    } else if (var == PROGRAM) {
        print_indent(indent, "");
        NodeList ndlist = node->val.block;
        print_program(ndlist, indent);
    } else if (var == BLOCK) {
        NodeList ndlist = node->val.block;
        print_block(ndlist, indent + 1);
    } else if (var == VARIABLE_ASSIGNMENT) {
        VariableAssignment* va = node->val.variable_assignment;
        printf("%s %s = ", type_to_string(va->type), va->name);
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
    } else if (var == RETURN) {
        printf("return ");
        print_node(node->val.return_, indent + 1);
    } else if (var == FUNCTION_DEFINITION) {
        FunctionDefinition fd = node->val.function_definition;
        printf("%s %s ", type_to_string(fd.signature.type), fd.signature.name);
        NodeList ndlist = fd.body;
        print_block(ndlist, indent + 1);
    } else {
        printf("print_node not implemented for node type: %d\n", node->var);
    }
}

void print_vars(Parser* p) {
    int i; char* name;
    vec_foreach(&p->variables.names, name, i) {
        printf("%d $%s\n", p->variables.types.data[i], name);
    }
}

void print_token(Token* t){
    switch (t->type) {
        case TK_PLUS:
            printf("Token: plus\n");
            break;
        case TK_MINUS:
            printf("Token: minus\n");
            break;
        case TK_TIMES:
            printf("Token: times\n");
            break;
        case TK_DIV:
            printf("Token: div\n");
            break;
        case TK_NUM:
            printf("Token: number");
            printf(" %d\n", t->data.num);
            break;
        case TK_EOF:
            printf("Token: EOF\n");
            break;
        case TK_LCURLY:
            printf("Token: left curly paren\n");
            break;
        case TK_RCURLY:
            printf("Token: right curly paren\n");
            break;
        case TK_LPAREN:
            printf("Token: left paren\n");
            break;
        case TK_RPAREN:
            printf("Token: right paren\n");
            break;
        case TK_IDENT:
            printf("Token: identifier");
            printf(" '%s'\n", t->data.ident);
            break;
        case TK_NE:
            printf("Token: not equal\n");
            break;
        case TK_LT:
            printf("Token: less than\n");
            break;
        case TK_MT:
            printf("Token: more than\n");
            break;
        case TK_ME:
            printf("Token: more than or equal to\n");
            break;
        case TK_LE:
            printf("Token: less than or equal to\n");
            break;
        case TK_EQ:
            printf("Token: equal\n");
            break;
        case TK_SEMICOLON:
            printf("Token: semicolon\n");
            break;
        case TK_TYPE_KEYWORD:
            printf("Token: Type Keywoard %d\n", t->data.type);
            break;
        case TK_ASSIGN:
            printf("Token: assign\n");
            break;
        case TK_RETURN:
            printf("Token: return\n");
            break;
        case TK_COMMA:
            printf("Token: comma\n");
            break;
        default:
            printf("Unknown token type %d\n", t->type);
            break;
    }
}

void printTokens(Token* t) {
    while (t->type != TK_EOF){
        print_token(t);
        t++;
    }
}

const char* type_to_string(Type type) {
    switch (type) {
        case INT:
            return "integer";
        case VOID:
            return "void";
        default:
            return "Unknown";
    }
}
