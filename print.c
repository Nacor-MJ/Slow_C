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
        printf("  ");
    }
    printf("%s", end);
}

void print_program(Program* block, int indent) {
    for (int i = 0; arrlen(block->data) > i; i++) {
        print_indent(indent, "");
        print_statement(&block->data[i], indent);
    }
}

void print_block(StmtList block, int indent) {
    printf("{\n");
    print_program(&block, indent + 1);
    printf("}\n");
}

void print_expr_list(ExprList list, int indent, const char* separator) {
    for (int i = 0; arrlen(list) > i; i++) {
        print_expr(&list[i], indent);
        printf("%s", separator);
    }
}
void print_stmt_list(StmtList list, int indent, const char* separator) {
    for (int i = 0; arrlen(list.data) > i; i++) {
        print_statement(&list.data[i], indent);
        printf("%s", separator);
    }
}

void print_statement(Statement* stmt, int indent) {
    StmtVar var = stmt->var;
    if (var == STMT_VARIABLE_ASSIGNMENT) {
        VariableAssignment va = stmt->val.variable_assignment;
        print_type_keyword(va.type);
        printf(" %s", va.vi.name);
        if (va.val.var != EMPTY_EXPR) {
            printf(" = ");
            print_expr(&va.val, indent);
        }
        printf(";\n");
    } else if (var == STMT_FUNCTION_DEFINITION) {
        FunctionDefinition fd = stmt->val.function_definition;
        print_type_keyword(fd.type);
        printf(" %s(", fd.name);
        if (arrlen(fd.args.data) > 0) {
            printf("\n");
            print_indent(indent + 1, "");
            print_stmt_list(fd.args, indent + 1, "");
        }
        print_indent(indent, ")");
        if (arrlen(fd.body.data) > 0) {
            printf("{\n");
            print_program(&fd.body, indent + 1);
            print_indent(indent, "}\n");
        } else {
            printf(";\n");
        }
    } else if (var == STMT_BLOCK) {
        print_block(stmt->val.block, indent);
    } else if (var == STMT_PROGRAM) {
        print_program(&stmt->val.program, indent);
    } else if (var == STMT_RETURN) {
        printf("return ");
        print_expr(&stmt->val.return_, indent);
        printf(";\n");
    } else if (var == STMT_THROWAWAY) {
        print_expr(&stmt->val.throw_away, indent);
        printf(";\n");
    } else {
        printf("print_statement not implemented for: %d\n", stmt->var);
        my_exit(-1);
    }
}

void print_expr(Expr *node, int indent) {
    if (node == NULL) {
        // printf("NULL Expr\n");
        return;
    }
    
    ExprVar var = node->var;
    
    if (var == VAL) {
        if (node->type->kind == TY_INT) {
            printf("%d", node->val.integer);
        } else if (node->type->kind == TY_FLOAT) {
            printf("%f", node->val.floating);
        }
    } else if (var == BIN_EXPR) {
        print_expr(node->val.bin_expr->l, indent + 1);
        printf(" %s ", op_enum_to_char(node->val.bin_expr->op));
        print_expr(node->val.bin_expr->r, indent + 1);
    } else if (var == FUNCTION_CALL) {
        FunctionCall fc = node->val.function_call;
        printf("%s(", fc.name);
        if (arrlen(fc.args) > 0) {
            print_expr_list(fc.args, 0, ", ");
        }
        printf(")");
    } else if (var == VARIABLE_IDENT) {
        VariableIdent vi = node->val.variable_ident;
        printf("%s", vi.name);
    } else {
        print_indent(indent, "");
        printf("\nprint_expr not implemented for: %d\n", node->var);
    }
}

void print_type_keyword(Type* type) {
    if (type == TY_NONE) {
        printf("TYPE");
    } else {
        printf("%s", type_to_string(type));
    }

}

void print_vars(Scope* p) {
    Variable* vars = p->variables;
    for (int i = 0; shlen(vars) > i; i++) {
        print_type_keyword(vars[i].value);
        printf(" $%s\n", vars[i].key);
    }
}

void print_token(Token* t){
    switch (t->type) {
        case TK_PLUS:
            printf("+");
            break;
        case TK_MINUS:
            printf("-");
            break;
        case TK_TIMES:
            printf("*");
            break;
        case TK_DIV:
            printf("/");
            break;
        case TK_INT:
            printf("$%d", t->data.integer);
            break;
        case TK_FLOAT:
            printf("$%f", t->data.floating);
            break;
        case TK_EOF:
            printf("EOF");
            break;
        case TK_LCURLY:
            printf("{");
            break;
        case TK_RCURLY:
            printf("}");
            break;
        case TK_LPAREN:
            printf("(");
            break;
        case TK_RPAREN:
            printf(")");
            break;
        case TK_IDENT:
            printf(" '%s'", t->data.ident);
            break;
        case TK_NE:
            printf("!=");
            break;
        case TK_LT:
            printf("<");
            break;
        case TK_MT:
            printf(">");
            break;
        case TK_ME:
            printf(">=");
            break;
        case TK_LE:
            printf("<=");
            break;
        case TK_EQ:
            printf("==");
            break;
        case TK_SEMICOLON:
            printf(";");
            break;
        case TK_TYPE_KEYWORD:
            print_type_keyword(t->data.type);
            break;
        case TK_ASSIGN:
            printf("=");
            break;
        case TK_RETURN:
            printf("return");
            break;
        case TK_COMMA:
            printf(",");
            break;
        default:
            printf("Tk.type %d", t->type);
            break;
    }
}

void printTokens(TokenList* t) {
    printf("\n");
    for (int i = 0; i < arrlen(t); i++) {
        print_token(&t->data[i]);
        TokenType tp = t->data[i].type;
        if (tp == TK_SEMICOLON || tp == TK_RCURLY || tp == TK_LCURLY) printf("\n");
    }
    printf("\n");
}

const char* type_to_string(Type* type) {
    switch (type->kind) {
        case TY_INT:
            return "int";
        case TY_FLOAT:
            return "float";
        case TY_VOID:
            return "void";
        default:
            return "Unknown";
    }
}
