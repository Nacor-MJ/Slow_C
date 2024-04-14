#ifndef SLOW_C_H
#define SLOW_C_H

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "vec.h"

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
    TK_TYPE_KEYWORD,
    TK_RETURN
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
void print_error_tok(Token* tk, char* absolute_start);
Token eat_token(Token** tk, TokenType check);
Token consume_token(Token** tk);
Token next_token(Token** tk);
void printTokens(Token* t);
bool compare_tokens(Token a, Token b);
void tokenize(Token* tk, char* src);

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
    RETURN_INSTR,

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
typedef struct Instruction Instruction;
typedef struct Instruction{
    InstructionType type;
    InstructionVal first_arg;
    InstructionVal second_arg;
    InstructionArgs args;
    Instruction* next;
} Instruction;

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
const char* op_enum_to_char(Binop op);

typedef enum {
    VARIABLE_ASSIGNMENT,
    VARIABLE_IDENT,
    FUNCTION_CALL,
    FUNCTION_DEFINITION,
    BLOCK,
    VAL,
    CONDITIONAL_JUMP,
    BIN_EXPR,
    PROGRAM,
    RETURN
} NodeVar;

typedef struct {
    Node* l;
    Binop op;
    Node* r;
} BinExpr;

// TODO make this vec_t(Node) 
// i.e. remove the pointer, it's a mess
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
    bool is_declaration;
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
    NodeList program;
    int val;
    Conditional_jump* conditional_jump;
    BinExpr* bin_expr;
    Node* return_;
} NodeVal;

typedef struct Node {
    NodeVar var;
    NodeVal val;
} Node;

void print_node(Node*, int);
void free_node_children(Node* nd);
Node parse(Token* src);

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

Node parse_function_definition(Parser* p, Token** tk, Type return_type, Token fn_name);

//
// semantic_check.c
//

void semantic_check(Node* root);

//
// fortran.c
//

void generate_ir(FILE* file, Node* root);
void node_to_ir(FILE* file, Node *nd);
void bin_expr_to_ir(FILE* file, BinExpr* expr);
void conditional_jump_to_ir(FILE* file, Conditional_jump* jump);
void fprintf_val(FILE* file, int val);
void block_to_ir(FILE* file, NodeList block, int indent);
void function_definition_to_ir(FILE* file, FunctionDefinition definition);
void function_call_to_ir(FILE* file, FunctionCall call);
void variable_assignment_to_ir(FILE* file, VariableAssignment *assignment);
void fprint_ident(FILE* file, char* ident);
void fprint_type(FILE* file, Type type);

//
// print.c
//

const char* type_to_string(Type);
void print_vars(Parser* p);

#endif
