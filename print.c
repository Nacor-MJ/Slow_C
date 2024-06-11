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
    print_indent(indent, "");
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
        VariableAssignment va = stmt->variable_assignment;
        print_type_keyword(va.type);
        printf(" %s", va.ident->key);
        if (va.val.var != EMPTY_EXPR) {
            printf(" = ");
            print_expr(&va.val, indent);
        }
        printf(";\n");
    } else if (var == STMT_FUNCTION_DEFINITION) {
        FunctionDefinition fd = stmt->function_definition;
        print_type_keyword(fd.type->value);
        printf(" %s(", fd.type->key);
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
        print_block(stmt->block, indent);
    } else if (var == STMT_PROGRAM) {
        print_program(&stmt->program, indent);
    } else if (var == STMT_RETURN) {
        printf("return ");
        print_expr(&stmt->return_, indent);
        printf(";\n");
    } else if (var == STMT_THROW_AWAY) {
        print_expr(&stmt->throw_away, indent);
        printf(";\n");
    } else if (var == STMT_CONDITIONAL_JUMP) {
        ConditionalJump j = stmt->conditional_jump;
        printf("if ( ");
        print_expr(j.condition, indent);
        printf(" ) ");
        print_statement(j.then_block, indent);
        if (j.else_block != NULL) {
            print_indent(indent, "else ");
            print_statement(j.else_block, indent);
        }
    } else if (var == STMT_LOOP) {
        Loop l = stmt->loop;
        switch (l.kind) {
            case WHILE:
                printf("while ( ");
                print_expr(l.condition, indent);
                printf(" ) ");
                break;
            case FOR:
                printf("for ( ");
                print_statement(l.init, indent); puts("; ");
                print_expr(l.condition, indent); puts("; ");
                print_statement(l.increment, indent); puts(" )");
                break;
            case DO_WHILE:
                puts("do ");
                print_statement(l.body, indent);
                puts(" while ( ");
                print_expr(l.condition, indent);
                puts(" );");
        }
        print_statement(l.body, indent);
    } else {
        printf("print_statement not implemented for: %d\n", stmt->var);
        my_exit(-1);
        do {
            puts("sjdfkj");
        } while (true);
    }
}

void print_const_val(ConstVal* c) {
    printf("\033[32m");
    switch (c->kind) {
        case CONST_INT:
            printf("%d", c->integer);
            break;
        case CONST_FLOAT:
            printf("%f", c->floating);
            break;
        case CONST_CHAR:
            printf("%c", c->character);
            break;
        case CONST_STRING:
            printf("%s", c->string);
            break;
    }
    printf("\033[0m");
}

void print_expr(Expr *expr, int indent) {
    if (expr == NULL) {
        // printf("NULL Expr\n");
        return;
    }
    
    ExprVar var = expr->var;
    
    if (var == EXPR_CONSTANT) {
        ConstVal c = expr->val.constant;
        print_const_val(&c);
    } else if (var == BIN_EXPR) {
        print_expr(expr->val.bin_expr->l, indent + 1);
        printf(" %s ", op_enum_to_char(expr->val.bin_expr->op));
        print_expr(expr->val.bin_expr->r, indent + 1);
    } else if (var == FUNCTION_CALL) {
        FunctionCall fc = expr->val.function_call;
        printf("%s(", fc.name);
        if (arrlen(fc.args) > 0) {
            print_expr_list(fc.args, 0, ", ");
        }
        printf(")");
    } else if (var == VARIABLE_IDENT) {
        Variable* vi = expr->val.variable_ident;
        printf("%s", vi->key);
    } else if (var == EMPTY_EXPR) {
        // pass
    } else {
        print_indent(indent, "");
        printf("\nprint_expr not implemented for: %d\n", expr->var);
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
        case TK_IF:
            printf("if");
            break;
        case TK_ELSE:
            printf("else");
            break;
        case TK_WHILE:
            printf("while");
            break;
        case TK_FOR:
            printf("for");
            break;
        default:
            printf("Tk.type %d", t->type);
            break;
    }
}

void print_tokens(TokenList* t) {
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
IR* current_ir = NULL;
void print_address(Address* a) {
    if (a == NULL) {
        printf("NULL\n");
        return;
    }
    switch (a->kind) {
        case ADDR_NONE:
            printf("NONE");
            break;
        case ADDR_CONSTANT:
            print_const_val(&a->constant);
            break;
        case ADDR_VARIABLE:
            if (a->variable == NULL) {
                printf("NULL");
                break;
            }
            printf("\033[34m%s\033[0m", a->variable->key);
            break;
        case ADDR_TEMPORARY:
            int temp = a->temporary;
            if (temp < 0) {
                printf("\033[33ml%d\033[0m", -temp);
            } else {
                printf("\033[36mt%d\033[0m", temp);
            }
            break;
        case ADDR_LABEL:
            if (current_ir == NULL) {
                printf("NULL");
                break;
            }
            print_address(&(*current_ir)[a->label_index].result);
            break;
        case ADDR_BASIC_BLOCK:
            printf("\033[36mB%d\033[0m", a->bb_index);
            break;
        default:
            printf("Unknown");
    }
}
void print_tac_op(TAC_OP tac_op) {
    char op[10] = { 0 };
    switch (tac_op) {
        case TAC_ADD:
            strcpy(op, "+");
            break;
        case TAC_SUB:
            strcpy(op, "-");
            break;
        case TAC_MUL:
            strcpy(op, "*");
            break;
        case TAC_DIV:
            strcpy(op, "/");
            break;
        case TAC_ASSIGN:
            strcpy(op, "=");
            break;
        case TAC_LABEL:
            strcpy(op, ":");
            break;
        case TAC_JMP:
            strcpy(op, "jmp");
            break;
        case TAC_IF_JMP:
            strcpy(op, "jmp if");
            break;
        case TAC_IF_FALSE_JMP:
            strcpy(op, "jmp if !");
            break;
        case TAC_LE:
            strcpy(op, "le");
            break;
        case TAC_LT:
            strcpy(op, "lt");
            break;
        case TAC_ME:
            strcpy(op, "me");
            break;
        case TAC_MT:
            strcpy(op, "mt");
            break;
        case TAC_CALL:
            strcpy(op, "call");
            break;
        case TAC_PARAM:
            strcpy(op, "param");
            break;
        case TAC_RETURN:
            strcpy(op, "return");
            break;
        case TAC_EQ:
            strcpy(op, "==");
            break;
        case TAC_NE:
            strcpy(op, "!=");
            break;
        default:
            sprintf(op, "%d", tac_op);
    }
    printf("%s", op);
}
void print_tac_liveness(Liveness l[3]) {
    for (int i = 0; i < 3; i++) {
        printf("%d ", l[i].is_live);
    }
}
void print_tac(TAC* tac) {
    // if this thing is a label for the function call
    if (tac->op != TAC_LABEL || tac->result.kind != ADDR_VARIABLE) {
        printf("    ");
    }
    switch (tac->op) {
        case TAC_LABEL:
            print_address(&tac->result);
            printf(":\n");
            break;
        case TAC_ASSIGN:
            print_address(&tac->result);
            printf(" = ");
            print_address(&tac->arg1);
            printf("\n");
            break;
        case TAC_JMP:
            printf("jmp ");
            print_address(&tac->result);
            printf("\n");
            break;
        case TAC_CALL:
            printf("call ");
            print_address(&tac->result);
            printf(", ");
            print_address(&tac->arg1);
            printf("\n");
            break;
        case TAC_PARAM:
            printf("param ");
            print_address(&tac->arg1);
            printf("\n");
            break;
        case TAC_RETURN:
            printf("ret ");
            print_address(&tac->arg1);
            printf("\n");
            break;
        case TAC_IF_JMP:
            printf("\033[35mjmp if true \033[0m");
            goto jmp;
        case TAC_IF_FALSE_JMP:
            printf("\033[35mjmp if false \033[0m");
            goto jmp;
        jmp:
            print_address(&tac->arg1);
            printf("\033[35m goto \033[0m");
            print_address(&tac->result);
            printf("\n");
            break;
        default:
            print_address(&tac->result);
            printf(" = ");
            print_address(&tac->arg1);
            printf(" ");
            print_tac_op(tac->op);
            printf(" ");
            print_address(&tac->arg2);
            printf("\n");
            break;
    }
}
void print_ir(IR ir) {
    current_ir = &ir;
    for (int i = 0; i < arrlen(ir); i++) {
        printf("%03d: ", i);
        TAC tac = ir[i];
        print_tac(&tac);
    }
    current_ir = NULL;
}
void print_ir_list(IRList ir) {
    for (int i = 0; i < arrlen(ir); i++) {
        IR ir_s = ir[i];
        print_ir(ir_s);
    }
}
void print_basic_block(BasicBlock* bb) {
    printf("\033[96mB%d:\033[0m\n", bb->index);

    printf("Variables: \n");
    for (int j = 0; j < hmlen(bb->variables); j++) {
        Variable* v = bb->variables[j].key;
        if (v==NULL) continue;
        printf("%s budies: %lld :", v->key, hmlen(v->buddies));
        for (int i = 0; hmlen(v->buddies) > i; i++) {
            Variable* v2 = v->buddies[i].key;
            if (v2==NULL) continue;
            printf("%s, ", v2->key);
        }
        puts("");
    } puts("");

    for (TAC* tac = bb->leader; tac <= bb->end; tac++) {
        print_tac(tac);
    }

}
void print_basic_block_list(FunctionBlocks* list) {
    for (int i = 0; i < arrlen(*list); i++) {
        BasicBlock bb = (*list)[i];
        print_basic_block(&bb);
    }
}
