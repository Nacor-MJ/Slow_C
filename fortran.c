#include "slow_c.h"

int current_indent = 0;

void fprint_current_indent(FILE* file) {
    for (int i = 0; i < current_indent; i++) {
        fprintf(file, "\t");
    }
}


void fprint_type(FILE* file, Type type) {
    fprintf(file, "%s", type_to_string(type));
}

void fprint_ident(FILE* file, char* ident) {
    fprintf(file, "%s", ident);
}

void variable_definition_to_ir(FILE* file, VariableAssignment* definition) {
    fprint_current_indent(file);
    fprint_type(file, definition->type);
    fprintf(file, " :: ");
    fprint_ident(file, definition->name);
    fprintf(file, "\n");
}

void variable_assignment_to_ir(FILE* file, VariableAssignment *assignment) {
    if (assignment->is_declaration) variable_definition_to_ir(file, assignment);
    fprint_current_indent(file);
    fprint_ident(file, assignment->name);
    fprintf(file, " = ");
    node_to_ir(file, assignment->val);
    fprintf(file, "\n");
}

void function_call_to_ir(FILE* file, FunctionCall call) {
    fprint_current_indent(file);
    fprint_ident(file, call.name);
    fprintf(file, "(");
    for (int i = 0; i < call.args.length ; i++) {
        if (i) fprintf(file, ", ");
        node_to_ir(file, call.args.data[i]);
    }
    fprintf(file, ")");
}

void function_definition_to_ir(FILE* file, FunctionDefinition definition) {
    FunctionCall signature = definition.signature;
    if (strcmp(signature.name, "main") != 0) {
        current_indent += 1;
        fprint_type(file, signature.type);
        fprint_ident(file, signature.name);
        fprintf(file, "(");
        for (int i = 0; i < signature.args.length; i++) {
            if (i) fprintf(file, ", ");
            node_to_ir(file, signature.args.data[i]);
        }
        fprintf(file, ") ");
        current_indent -= 1;
    }
    block_to_ir(file, definition.body, 1);
}
void program_to_ir(FILE* file, NodeList block) {
    fprintf(file, "program idk\n \timplicit none\n");
    current_indent = 1;
    block_to_ir(file, block, 0);
    fprintf(file, "end program idk\n");
    current_indent = 0;
}

void block_to_ir(FILE* file, NodeList block, int indent) {
    for (int i = 0; i < block.length; i++) {
        node_to_ir(file, block.data[i]);
    }
}

void fprintf_val(FILE* file, int val) {
    fprintf(file, "%d", val);
}

void conditional_jump_to_ir(FILE* file, Conditional_jump* jump) {
    fprintf(file, "if (");
    node_to_ir(file, jump->condition);
    fprintf(file, ") ");
    node_to_ir(file, jump->true_block);
    fprintf(file, " else ");
    node_to_ir(file, jump->false_block);
}

void bin_expr_to_ir(FILE* file, BinExpr* expr) {
    fprintf(file, "(");
    node_to_ir(file, expr->l);
    fprintf(file, " %s ", op_enum_to_char(expr->op));
    node_to_ir(file, expr->r);
    fprintf(file, ")");
}

void node_to_ir(FILE* file, Node *nd) {
    switch (nd->var) {
        case VARIABLE_ASSIGNMENT:
            variable_assignment_to_ir(file, nd->val.variable_assignment);
            break;
        case VARIABLE_IDENT:
            fprint_ident(file, nd->val.variable_ident);
            break;
        case FUNCTION_CALL:
            function_call_to_ir(file, nd->val.function_call);
            break;
        case FUNCTION_DEFINITION:
            function_definition_to_ir(file, nd->val.function_definition);
            break;
        case BLOCK:
            block_to_ir(file, nd->val.block, 1);
            break;
        case VAL:
            fprintf_val(file, nd->val.val);
            break;
        case CONDITIONAL_JUMP:
            conditional_jump_to_ir(file, nd->val.conditional_jump);
        case BIN_EXPR:
            bin_expr_to_ir(file, nd->val.bin_expr);
            break;
        case PROGRAM:
            program_to_ir(file, nd->val.block);
            break;
        case RETURN:
            fprint_current_indent(file);
            fprintf(file, "stop ");
            node_to_ir(file, nd->val.return_);
            fprintf(file, "\n");
            break;
        default:
            printf("Unknown node type %d\n", nd->var);
            exit(-1);
    }
}

void generate_ir(FILE* file, Node *nd) {
    node_to_ir(file, nd);
}
