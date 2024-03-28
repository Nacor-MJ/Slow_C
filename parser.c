#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ir.c"

#include "tokens.c"

typedef enum {
    OP_INVALID,
    OP_PLUS,
    OP_MINUS,
    OP_TIMES,
    OP_DIV,
    OP_EQ,
    OP_NE,
    OP_MT,
    OP_LT,
    OP_ME,
    OP_LE
} Binop;

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
typedef enum {
    VAL,
    BIN_EXPR
} NodeVar;

typedef struct Node Node;

typedef struct {
    Node* l;
    Binop op;
    Node* r;
} BinExpr;


typedef union {
    int val;
    BinExpr* bin_expr;
} NodeVal;

typedef struct Node {
    NodeVar var;
    NodeVal val;
} Node;

void print_indent(int level) {
    for (int i = 0; i < level; ++i) {
        printf("  ");  // Assuming 2 spaces per indentation level
    }
}

void print_node(Node *node, int indent = 0) {
    if (node == NULL) {
        return;
    }
    
    print_indent(indent);
    
    if (node->var == VAL) {
        printf("Value: %d\n", node->val.val);
    } else if (node->var == BIN_EXPR) {
        printf("Binary Expression:\n");
        print_indent(indent + 1);
        printf("Left:\n");
        print_node(node->val.bin_expr->l, indent + 2);
        print_indent(indent + 1);
        printf("Operator: %s\n", op_enum_to_char(node->val.bin_expr->op));
        print_indent(indent + 1);
        printf("Right:\n");
        print_node(node->val.bin_expr->r, indent + 2);
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
    if (nd->var == BIN_EXPR){
        BinExpr* be = nd->val.bin_expr;
        if (be->l != NULL) {
            free_node_children(be->l);
        }
        if (be->r != NULL) {
            free_node_children(be->r);
        }
        free(be); // This is good <3
    } else if(nd->var == VAL){

    } else {
        printf("Not all node types are freed");
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

// void node_to_ir(Node* nd, char* dst, Reg target_reg){
// returns the last Instruction
void node_to_ir(Node* nd, Instruction* parent, Reg target_reg){
    NodeVar variant = nd->var;

    Instruction* instr = empty_instruction();

    if (variant == VAL) {
        instr->type = MOV;
        instr->args = REG_CONST;
        instr->first_arg.reg = target_reg;
        instr->second_arg.constant = nd->val.val;
        append_instruction(parent, instr);
        return;
    } else if (variant == BIN_EXPR){
        BinExpr* be = nd->val.bin_expr;

        node_to_ir(be->l, instr, target_reg);
        append_instruction(parent, instr);

        Instruction* r_instr = empty_instruction();
        node_to_ir(be->r, r_instr, increase_reg(target_reg));
        append_instruction(parent, r_instr);

        instr = empty_instruction();

        instr->type = binop_to_instructiontype(be->op);

        // TODO handle weird registers with
        // div and times
        InstructionVal val;
        switch (be->op){
            case OP_DIV:
            case OP_TIMES:
                instr->args = REG;

                val.reg = target_reg;
                instr->first_arg = val;
                break;
            case OP_PLUS:
            case OP_MINUS:
            case OP_EQ:
            case OP_NE:
            case OP_MT:
            case OP_LT:
            case OP_ME:
            case OP_LE:
                instr->args = REG_REG;

                val.reg = target_reg;
                instr->first_arg = val;

                val.reg = increase_reg(target_reg);
                instr->second_arg = val;
                break;
            default:
                printf("Invalid Binop\n");
                exit(-1);
        }
        append_instruction(parent, instr);
    }
}

Node parse_expr(Token** tk);
Node parse_eq_ne(Token** tk);
Node parse_comp_expr(Token** tk);
Node parse_bin_expr(Token** tk);
Node parse_term(Token** tk);
Node parse_factor(Token** tk);

Node parse_factor(Token** tk){
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
        Node nd = {
            VAL,
            {next.data.num}
        };
        return nd;
    } else if (next.type == TK_LPAREN){
        eat_token(tk, TK_LPAREN);
        Node result = parse_expr(tk);
        eat_token(tk, TK_RPAREN);
        return result;
    } else {
        printf("Expected factor, got: ");
        print_token(&next);
        exit(-1);
    }
}

Node parse_term(Token** tk){
    Node factor = parse_factor(tk);

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
        assign_r_to_BinExpr(&result, parse_factor(tk));

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

Node parse_bin_expr(Token** tk){
    Node term = parse_term(tk);

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
                assign_r_to_BinExpr(&result, parse_term(tk));
                break;
            case (TK_PLUS):
                result.op = OP_PLUS;
                assign_r_to_BinExpr(&result, parse_term(tk));
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
Node parse_relational_expr(Token** tk){
    Node bin_expr = parse_bin_expr(tk);

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
        
        assign_r_to_BinExpr(&result, parse_term(tk));
        
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
Node parse_eq_ne(Token** tk){
    Node bin_expr = parse_relational_expr(tk);

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
        
        assign_r_to_BinExpr(&result, parse_term(tk));
        
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

Node parse_expr(Token** tk){
    return parse_eq_ne(tk);
}

Node parse_from_tok(Token* tk){
    return parse_expr(&tk);
}

Node parse(char* src){
    Token *tokens = (Token*) malloc(sizeof(Token) * 20);
    if (tokens == NULL) {
        printf("Could not allocate memory for tokens");
        exit(1);
    }

    tokenize(tokens, src);

    printTokens(tokens);
    
    Node result = parse_from_tok(tokens);

    print_node(&result);

    free(tokens);

    return result;
}
