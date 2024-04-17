#ifndef SLOW_C_H
#define SLOW_C_H

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "vec.h"

// this is the best function I have here
static void inline __attribute__((noreturn)) fuck(int i, const char* a, int b) {
    printf("\033[31mError %d in %s:%d\033[0m\n", i, __FILE__, __LINE__); \
    *(int*)0 = 0;\
    exit(0);
};
#define my_exit(i) fuck(i, __FILE__, __LINE__);

typedef struct Expr Expr;
typedef struct Statement Statement;

typedef enum Type {
    NONE_TYPE,
    VOID,
    INT
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
    TK_RETURN,
    TK_COMMENT
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

// if you change vec_t this will get fucked up <3
typedef struct { 
    Token *data;
    int length;
    int capacity;
    int pars_ptr;
} TokenList;

void print_token(Token*);
void print_error_tok(Token*, char*);
Token eat_token(TokenList* tk, TokenType check);
Token consume_token(TokenList* tk);
Token next_token(TokenList* tk);
Token* next_token_ptr(TokenList* tk);
Token next_token_with_offset(TokenList* tk, int offset);
void printTokens(TokenList* t);
bool compare_tokens(Token a, Token b);
void tokenize(TokenList* tk, char* src);

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

typedef vec_t(Expr) ExprList;

typedef vec_t(Statement) StmtList;

typedef StmtList Program;

typedef struct {
    Expr* l;
    Binop op;
    Expr* r;
} BinExpr;

typedef struct {
    Type type;
    char* name;
    ExprList args;
} FunctionCall;

typedef union {
    char* variable_ident;
    FunctionCall function_call;
    int val;
    BinExpr* bin_expr;
} ExprVal;
typedef enum {
    VARIABLE_IDENT,
    FUNCTION_CALL,
    VAL,
    BIN_EXPR,
} ExprVar;
typedef struct Expr {
    ExprVar var;
    ExprVal val;
} Expr;


typedef struct {
    FunctionCall signature;
    StmtList body;
} FunctionDefinition;
typedef struct {
    Type type;
    char* name;
    Expr val;
    int version;
} VariableAssignment;
typedef struct Conditional_jump {
    Expr* condition;
    Expr* true_block;
    Expr* false_block;
    Expr* next;
} Conditional_jump;

typedef union StmtVal {
    VariableAssignment variable_assignment;
    FunctionDefinition function_definition;
    Conditional_jump conditional_jump;
    StmtList block;
    Program program;
    Expr return_;
    Expr throw_away;
} StmtVal;
typedef enum StmtVar {
    STMT_VARIABLE_ASSIGNMENT,
    STMT_FUNCTION_DEFINITION,
    STMT_CONDITIONAL_JUMP,
    STMT_BLOCK,
    STMT_PROGRAM,
    STMT_RETURN,
    STMT_THROWAWAY
} StmtVar;
typedef struct Statement {
    StmtVar var;
    StmtVal val;
} Statement;

void print_program(Program*, int);
void print_statement(Statement*, int);
void print_expr(Expr*, int);
void free_stmt_list(StmtList);
Program parse(TokenList src);

typedef vec_t(char*) VariablesNames;
typedef vec_t(Type) VariablesTypes;
typedef vec_t(int) VariablesVersions;
// Oh yes a hashmap
typedef struct {
    VariablesNames names;
    VariablesTypes types;
    VariablesVersions versions;
} Variables;

typedef struct Scope Scope;
typedef struct Scope {
    Variables variables;
    Scope* parent;
} Scope;
Scope* new_scope(Scope* parent);
void deinit_scope();

typedef struct Parser {
    Scope* global_scope;
    char* absolute_start;
} Parser;

Program parse_program(Parser*, TokenList*);
StmtList parse_block(Scope* p, TokenList* tk);
Statement parse_statement(Scope* p, TokenList* tk);

Statement parse_function_definition(Scope* p, TokenList* tk);

Type get_var_type(Scope* p, Token var);
void parse_arg_list(Scope*p, TokenList* tk, ExprList* list);

//
// expr.c
//

Expr zero_expr();
Expr parse_function_call(Scope* p, TokenList* tk);
Expr parse_expr(Scope*, TokenList*);


//
// semantic_check.c
//

void semantic_check(Program* root);

//
// print.c
//

const char* type_to_string(Type);
void print_vars(Scope* p);

// 
// free.c
//

void free_program(Program);

#endif
