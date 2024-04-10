#ifndef TOKENS_C
#define TOKENS_C

#include "slow_c.h"

char* global_start_of_file;

Token consume_token(Token** tk){
    Token a = **tk;
    (*tk)++;
    return a;
}

Token eat_token(Token** tk, TokenType check){
    Token next = consume_token(tk);
    if (next.type != check){
        printf("\033[91mInvalid Token, expected: \n\t");
        Token check_tk = {check, {0}, NULL};
        print_token(&check_tk);
        printf("Got:\n\t");
        print_error_tok(&next, global_start_of_file);
        printf("\033[0m");
        exit(-1);
    }
    return next;
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
        case TK_COMMA:
            printf("Token: comma\n");
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

void tokenize_num(Token* tk, char** src){
    int num = strtol(*src, src, 0);
    
    tk->type = TK_NUM;
    tk->data.num = num;
}
void add_token(Token** tks, Token tk) {
    **tks = tk;
    (*tks)++;
}

// Compares two tokens and their data
bool compare_tokens(Token a, Token b) {
    if (a.type == b.type){
        switch (a.type){
            case TK_NUM:
                return a.data.num == b.data.num;
            case TK_IDENT:
                return strcmp(a.data.ident, b.data.ident) == 0;
            default:
                return true;
        }
    } 
    return false;
}

void tokenize_comp_operator(Token* tk, char** src){
    if (strncmp(*src, "==", 2) == 0){
        tk->type = TK_EQ;
    } else if (strncmp(*src, "!=", 2) == 0){
        tk->type = TK_NE;
    } else if (strncmp(*src, "<=", 2) == 0){
        tk->type = TK_LE;
    } else if (strncmp(*src, ">=", 2) == 0){
        tk->type = TK_ME;
    } else if ('<' == **src){
        *src -=1;
        tk->type = TK_LT;
    } else if ('>' == **src){
        *src -=1;
        tk->type = TK_MT;
    } else if ('=' == **src) {
        *src -=1;
        tk->type = TK_ASSIGN;
    } else {
        printf("Not a Valid Comp Operator: %c", **src);
        exit(-1);
    }
    (*src) += 2;
}

// Prints The Error message and the line above and below the token
void print_error_tok(Token* tk, char* absolute_start) {
    print_token(tk);
    printf("\n");
    
    int line_num = 1;
    while (absolute_start != tk->start_of_token){
        if (*absolute_start == '\n'){
            line_num++;
        }
        absolute_start++;
    }
    printf("\tLine num: %d\n", line_num);

    if (line_num != 1) {
        line_num--;
    }
    
    for (int i = 0; i < 3; i++ ) {
        printf("%d | ", line_num + i);
        while (*absolute_start != '\n' && *absolute_start != '\0') {
            printf("%c", *absolute_start++);
        }
        absolute_start++;
        printf("\n");
    }
}

void tokenize(Token* tk, char* src) {
    char* absolute_start = src;
    global_start_of_file = src;
    char* start = src;
    skip_whitespace(&src);
    while (*src){
        start = src;
        Token tmp = {
            TK_EOF,
            {0},
            start,
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
            case '{':
                src++;
                tmp.type = TK_LCURLY;
                break;
            case '}':
                src++;
                tmp.type = TK_RCURLY;
                break;
            case ';':
                src++;
                tmp.type = TK_SEMICOLON;
                break;
            case ',':
                src++;
                tmp.type = TK_COMMA;
                break;
            case '=':
            case '!':
            case '<':
            case '>':
                tokenize_comp_operator(&tmp, &src);
                break;
            default:
                if (isdigit((int) *src)){
                    tokenize_num(&tmp, &src);
                } else if (strncmp(src, "int", 3) == 0) {
                    src += 3;
                    tmp.type = TK_TYPE_KEYWORD;
                    tmp.data.type = INT;
                } else {
                    char* buff;
                    int i = 0;
                    if (isalpha((int) *src) == 0 || *src == '_'){
                        printf(
                            "Not a valid identifier: '%c', hex: '%x' \n", 
                            *src, 
                            *src && 0xff
                        );
                        print_error_tok(&tmp, absolute_start);
                        exit(-1);
                    }
                    while (isalpha((int) *src)){
                        i++;
                        src++;
                    }
                    buff = (char*) malloc(i + 1);
                    strncpy(buff, (src - i), i);
                    buff[i] = '\0';
                    tmp.type = TK_IDENT;
                    tmp.data.ident = buff;
                }
        }
        add_token(&tk, tmp);
        skip_whitespace(&src);
    }
}

#endif
