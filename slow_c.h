#ifndef SLOW_C_H
#define SLOW_C_H

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "stb_ds.h"


// this is the best function I have here
static void inline __attribute__((noreturn)) fuck(int i, const char* a, int b) {
    printf("\033[31mError %d in %s:%d\033[0m\n", i, a, b); \
    *(int*)0 = 0;\
    exit(0);
};
#define my_exit(i) fuck(i, __FILE__, __LINE__);
#define NOT_IMPLEMENTED \
    printf("\033[31mIn function %s not implemented\033[0m\n", __FUNCTION__);\
    fuck(69, __FILE__, __LINE__)

typedef struct Expr Expr;
typedef struct Statement Statement;
typedef struct Type Type;

typedef enum TypeKind {
    TY_NONE,

    TY_INT,
    TY_FLOAT,
    TY_CHAR,
    TY_BOOL,

    TY_VOID,

    TY_FUNC,

    TY_STRUCT,
    TY_UNION,
    TY_ENUM,
    TY_ARRAY,

    TY_PTR,
} TypeKind;
typedef Type* Types;

typedef struct Member {
    char* name;
    Type* type;
    int offset;
} Member;
typedef Member* Members;
typedef struct FuncData {
    Types return_type;
    Types param_type;
} FuncData;
typedef struct PointerData {
    Type* type;
    int array_len;
} PointerData;
typedef union TypeData {
    Members members; // structs
    PointerData base; // pointers and arrays
    FuncData func;
} TypeData;

typedef struct Type {
    TypeKind kind;
    int size;
    int allign;
    // int _is_signed;
    TypeData data;
} Type;

extern Type* ty_none;
extern Type *ty_void;
extern Type *ty_bool;
extern Type *ty_char;
extern Type *ty_int;
extern Type *ty_float;

//
// tokens.c
//

typedef enum {
    TK_EOF = 0,
    TK_PLUS,
    TK_MINUS,
    TK_TIMES,
    TK_DIV,
    TK_INT,
    TK_FLOAT,
    TK_IDENT,
    TK_LPAREN,
    TK_RPAREN,
    TK_LCURLY, // {
    TK_RCURLY, // }
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
    TK_COMMENT,
    TK_IF,
    TK_ELSE,
    TK_WHILE,
    TK_FOR,
} TokenType;
typedef union {
    // If you add anything here implement compare_tokens for it
    int integer;
    float floating;
    char* ident;
    Type* type;
} TokenData;
typedef struct {
    TokenType type;
    TokenData data;
    char* start_of_token;
} Token;

typedef struct {
    Token* data;
    int pars_ptr;
} TokenList;

void print_error_tok(Token*, char*);
Token* eat_token_checked(TokenList* tk, TokenType check);
Token* eat_token(TokenList* tk);
Token* next_token(TokenList* tk);
Token* next_token_with_offset(TokenList* tk, int offset);
void printTokens(TokenList* t);
bool compare_tokens(Token a, Token b);
void tokenize(TokenList* tk, char* src);

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

typedef struct {
    char* key;
    Type* value;
} Variable;

typedef struct Scope Scope;
typedef struct Scope {
    Variable* variables;
    Scope* parent;
    int depth;
} Scope;

typedef Expr* ExprList;
typedef struct StmtList {
    Statement* data;
    Scope* scope;
} StmtList;

StmtList new_stmt_list(Scope* );

typedef StmtList Program;

typedef struct {
    Expr* l;
    Binop op;
    Expr* r;
} BinExpr;

typedef struct {
    Type* type;
    char* name;
    ExprList args;
} FunctionCall;
typedef struct {
    union {
        int integer;
        float floating;
        char character;
        char* string;
    };
    enum {
        CONST_INT,
        CONST_FLOAT,
        CONST_CHAR,
        CONST_STRING
    } kind;
} ConstVal;

typedef union {
    ConstVal constant;
    Variable* variable_ident;
    FunctionCall function_call;
    BinExpr* bin_expr;
} ExprVal;
typedef enum {
    EMPTY_EXPR,
    EXPR_CONSTANT,
    VARIABLE_IDENT,
    FUNCTION_CALL,
    BIN_EXPR
} ExprVar;
// TODO: get rid of ExprVal with anonymous union
typedef struct Expr {
    ExprVar var;
    ExprVal val;
    Token* start;
    Type* type;
} Expr;

typedef struct {
    Variable* type;
    StmtList args;
    StmtList body;
} FunctionDefinition;
typedef struct {
    Type* type;
    Expr val;
    Variable* ident;
} VariableAssignment;
typedef struct Conditional_jump {
    Expr* condition;
    Statement* then_block;
    Statement* else_block;
} ConditionalJump;
typedef struct Loop {
    Expr* condition;
    Statement* init;
    Statement* increment;
    Statement* body;
    enum {
        DO_WHILE,
        WHILE,
        FOR
    } kind;
} Loop;

typedef enum StmtVar {
    STMT_VARIABLE_ASSIGNMENT,
    STMT_FUNCTION_DEFINITION,
    STMT_CONDITIONAL_JUMP,
    STMT_LOOP,
    STMT_BLOCK,
    STMT_PROGRAM,
    STMT_RETURN,
    STMT_THROW_AWAY
} StmtVar;
typedef struct Statement {
    StmtVar var;
    union StmtVal {
        VariableAssignment variable_assignment;
        FunctionDefinition function_definition;
        ConditionalJump conditional_jump;
        Loop loop;
        StmtList block;
        Program program;
        Expr return_;
        Expr throw_away;
    };
    Token* start;
} Statement;

void free_stmt_list(StmtList);
Program parse(TokenList src);

typedef struct Parser {
    Scope* global_scope;
    char* absolute_start;
} Parser;

Program parse_program(Parser*, TokenList*);
StmtList parse_block(Scope* p, TokenList* tk);
Statement parse_statement(Scope* p, TokenList* tk);

Statement parse_function_definition(Scope* p, TokenList* tk);

void parse_arg_list(Scope*p, TokenList* tk, StmtList* list);
void append_statement(StmtList* list, Statement nd);
void append_expr(ExprList* list, Expr nd);

void check_types(Type* t1, Type* t2, Token* tk);

//
// scope.c
//

Variable* add_variable(Scope* p, Token var, Type* type);
Type* get_var_type(Scope* p, Token var);
Variable* get_variable(Scope* p, char* ident);

Scope* new_scope(Scope* parent);
void deinit_scope(Scope* s);

//
// expr.c
//

Expr zero_expr(Token* );
Expr parse_function_call(Scope* p, TokenList* tk);
Expr parse_expr(Scope*, TokenList*);
Type* get_expr_type(Expr*);


//
// post_processing.c
//

void post_processing(Program* root);


// 
// free.c
//

void free_program(Program);
void free_token_list_and_data(TokenList* list);

// 
// assembly.c
//

void generate_asm(FILE* f, Program* program);

// 
// ir.c
//

typedef struct TAC TAC;

// named labels are stored as variable 
// and compiler generated ones are in temporary
typedef struct Address {
    union {
        ConstVal constant;
        Variable* variable;
        int temporary;
        TAC* label;
    };
    enum {
        ADDR_CONSTANT,
        ADDR_VARIABLE,
        ADDR_TEMPORARY,
        ADDR_LABEL
    } kind;
} Address;
extern Address EMPTY_ADDRESS;

// Three address code type
//  binops, assign, label, jumps, call, params 
typedef enum {
    TAC_ADD,
    TAC_SUB,
    TAC_MUL,
    TAC_DIV,

    TAC_ASSIGN,

    TAC_LABEL,

    TAC_JMP,
    TAC_IF_JMP,
    TAC_IF_FALSE_JMP,

    TAC_LE,
    TAC_LT,
    TAC_ME,
    TAC_MT,
    TAC_NE,
    TAC_EQ,

    TAC_CALL, 
    TAC_PARAM,
    TAC_RETURN,
} TAC_OP;

// Represents the Three address instruction
//
// result = arg1 op arg2
// if arg1 op arg2 goto result
// if arg1 goto result
// ifFalse arg1 goto result
// result: 
// jmp result
// result = arg1
// param arg1
// result = arg1() ; arg2 = number of params
typedef struct TAC {
    Address arg1; // param; assign, num of params for call
    Address arg2;
    Address result; // target of jumps; label
    TAC_OP op;
} TAC;

typedef TAC* IR;
typedef IR* IRList;

IRList ast_to_tac(Program* program);

void statement_to_ir(IR* destination, Statement* st);
Address expr_to_ir(IR* destination, Expr* e);

//
// print.c
//

const char* type_to_string(Type*);
void print_vars(Scope* p);
void print_token(Token*);
void print_type_keyword(Type*);
void print_program(Program*, int);
void print_statement(Statement*, int);
void print_expr(Expr*, int);
void print_address(Address*);
void print_ir_list(IRList);
void print_ir(IR ir);
void print_tac(TAC* tac);
#endif
