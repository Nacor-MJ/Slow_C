#ifndef SLOW_C_H
#define SLOW_C_H

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "vec.c"

typedef struct Node Node;

typedef enum Type {
    INT,
    VOID
} Type;

//
// tokens.c
//

typedef enum {
    TK_EOF = 0,
    TK_PLUS,
    TK_MINUS,
    TK_TIMES,
    TK_DIV,
    TK_NUM,
    TK_IDENT,
    TK_LPAREN,
    TK_RPAREN,
    TK_LCURLY,
    TK_RCURLY,
    TK_EQ, // ==
    TK_NE, // !=
    TK_MT, // >
    TK_LT, // <
    TK_ME, // >=
    TK_LE, // <=
    TK_SEMICOLON,
    TK_ASSIGN,  // =
    TK_COMMA,
    TK_TYPE_KEYWORD
} TokenType;
typedef union {
    // If you add anything here implement compare_tokens for it
    int num;
    char* ident;
    Type type; 
} TokenData;
typedef struct {
    TokenType type;
    TokenData data;
    char* start_of_token;
} Token;

void print_token(Token*);
void printTokens(Token* t);
bool compare_tokens(Token a, Token b);
void tokenize(Token* tk, char* src);

#include "tokens.c"

//
// assembly.c
//

typedef enum Reg{
    RAX,
    RCX,
    RDX,
    R8,
    R9,
    R10,
    R11,
    INVALID, // this needs to be right after the 64bit general purpose registers because of the increase_reg function
    AL,
    R15,
    NA
} Reg;

typedef enum InstructionType {
    // data transfer
    MOV,
    MOVZX,

    // arithmetic
    ADD,
    MUL,
    DIV,
    SUB,

    // binops
    SETE,
    SETNE,
    SETG,
    SETL,
    SETGE,
    SETLE,

    // cmp
    CMP,
    TEST,

    // stack manipulation
    PUSH,
    POP,

    // functions
    CALL,
    RETURN,

    // NOP
    NOP
} InstructionType;

typedef union InstructionVal{
    Reg reg;
    int constant;
    char* function_name;
} InstructionVal;

typedef enum {
    REG_CONST,
    REG_REG,
    REG,
    CONST,
    FUNCTION_NAME,
    NONE
} InstructionArgs;

typedef struct Instruction{
    InstructionType type;
    InstructionVal first_arg;
    InstructionVal second_arg;
    InstructionArgs args;
    Instruction* next;
} Instruction;

#include "assembly.c"

//
// parser.c
//

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
    OP_LE,
    OP_MOV // Not really a binop but meh
} Binop;

typedef enum {
    VARIABLE_ASSIGNMENT,
    VARIABLE_IDENT,
    FUNCTION_CALL,
    FUNCTION_DEFINITION,
    BLOCK,
    VAL,
    CONDITIONAL_JUMP,
    BIN_EXPR
} NodeVar;

typedef struct {
    Node* l;
    Binop op;
    Node* r;
} BinExpr;

typedef vec_t(Node*) NodeList;


typedef struct {
    Type type;
    char* name;
    NodeList args;
} FunctionCall;
typedef struct {
    FunctionCall signature;
    NodeList body;
} FunctionDefinition;
typedef struct {
    Type type;
    char* name;
    Node* val;
} VariableAssignment;

typedef struct Conditional_jump {
    Node* condition;
    Node* true_block;
    Node* false_block;
    Node* next;
} Conditional_jump;

typedef union {
    VariableAssignment* variable_assignment;
    char* variable_ident;
    FunctionCall function_call;
    FunctionDefinition function_definition;
    NodeList block;
    int val;
    Conditional_jump* conditional_jump;
    BinExpr* bin_expr;
} NodeVal;

typedef struct Node {
    NodeVar var;
    NodeVal val;
} Node;

void print_node(Node *node, int indent = 0);
void free_node_children(Node* nd);
Node parse(Token* src);
InstructionType binop_to_instructiontype(Binop op);

typedef vec_t(char*) VariablesNames;
typedef vec_t(Type) VariablesTypes;
// Oh yes a hashmap
typedef struct {
    VariablesNames names;
    VariablesTypes types;
} Variables;
typedef struct Parser {
    Variables variables;
    char* absolute_start;
} Parser;

Node parse_program(Parser*, Token**);
Node parse_statement(Parser*, Token**);
Node parse_expr(Parser*, Token**);
Node parse_eq_ne(Parser*, Token**);
Node parse_comp_expr(Parser*, Token**);
Node parse_bin_expr(Parser*, Token**);
Node parse_term(Parser*, Token**);
Node parse_factor(Parser*, Token**);

#include "parser.c"

//
// semantic_check.c
//

#include "semantic_check.c"
/*

//
// ir.c
//
typedef struct IR_val IR_val;
typedef union IR_val_data {
    int i;
    IR_val* p;
} IR_val_data;
typedef enum IR_val_type {
    IR_VAL_INT,
    IR_VAL_PTR
} IR_val_type;
typedef struct IR_val {
    IR_val_type type;
    IR_val_data data;
} IR_val;

typedef struct IR {
    int index;
    IR_val l;
    IR_val r;
    InstructionType op;
} IR;

typedef vec_t(IR*) IRList;

void print_ir(IR* ir);

#include "ir.c"

#include "instructions.c"
*/
#endif
