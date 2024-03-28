#include <math.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

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
    TK_EQ, // ==
    TK_NE, // !=
    TK_MT, // >
    TK_LT, // <
    TK_ME, // >=
    TK_LE, // <=
    TK_INVALID
} TokenType;
typedef union {
    int num;
    char* ident;
} TokenData;
typedef struct {
    TokenType type;
    TokenData data;
} Token;

Token consume_token(Token** tk){
    Token a = **tk;
    (*tk)++;
    return a;
}

void eat_token(Token** tk, TokenType check){
    Token next = consume_token(tk);
    if (next.type != check){
        printf("Invalid Token, expected: %d , got %d", check, next.type);
        exit(-1);
    }
}

Token next_token(Token** tk) {
    return **tk;
}

void skip_whitespace(char** src) {
    while (isspace((int) **src)) {
        *src += 1;
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
        case TK_LPAREN:
            printf("Token: left paren\n");
            break;
        case TK_RPAREN:
            printf("Token: right paren\n");
            break;
        case TK_IDENT:
            printf("Token: identifier");
            printf(" %s\n", t->data.ident);
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
        default:
            printf("Unknown token type\n");
            break;
    }
}

void printTokens(Token* t) {
    while (t->type != TK_EOF){
        print_token(t);
        t++;
    }
}

Token tokenize_num(char** src){
    int num = strtol(*src, src, 0);

    Token tk = {
        TK_NUM,
        {num},
    };
    return tk;
}
void add_token(Token** tks, Token tk) {
    **tks = tk;
    (*tks)++;
}

Token tokenize_comp_operator(char** src){
    Token tk = {
        TK_INVALID,
        {0}
    };
    if (strncmp(*src, "==", 2) == 0){
        tk.type = TK_EQ;
    } else if (strncmp(*src, "!=", 2) == 0){
        tk.type = TK_NE;
    } else if (strncmp(*src, "<=", 2) == 0){
        tk.type = TK_LE;
    } else if (strncmp(*src, ">=", 2) == 0){
        tk.type = TK_ME;
    } else if ('<' == **src){
        *src -=1;
        tk.type = TK_LT;
    } else if ('>' == **src){
        *src -=1;
        tk.type = TK_MT;
    } else {
        printf("Not a Valid Comp Operator: %c", **src);
        exit(-1);
    }
    (*src) += 2;
    return tk;
}
void tokenize(Token* tk, char* src) {
        skip_whitespace(&src);
    while (*src){
        Token tmp = {
            TK_PLUS,
            {0}
        };
        switch (*src) {
            case '-':
                src++;
                tmp.type = TK_MINUS;
                break;
            case '+':
                src++;
                tmp.type = TK_PLUS;
                break;
            case '*':
                src++;
                tmp.type = TK_TIMES;
                break;
            case '/':
                src++;
                tmp.type = TK_DIV;
                break;
            case '(':
                src++;
                tmp.type = TK_LPAREN;
                break;
            case ')':
                src++;
                tmp.type = TK_RPAREN;
                break;
            case '=':
            case '!':
            case '<':
            case '>':
                tmp = tokenize_comp_operator(&src);
                break;
            default:
                if (isdigit((int) *src)){
                    tmp = tokenize_num(&src);
                } else {
                    printf("Invalid character: %c\n", *src);
                    exit(-1);
                }
        }
        add_token(&tk, tmp);
        skip_whitespace(&src);
    }
}
