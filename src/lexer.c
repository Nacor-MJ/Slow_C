#ifndef TOKENS_C
#define TOKENS_C

#include "../include/slow_c.h"
#include "math.h"

char *global_start_of_file;

extern char current_file[256];

// FIXME: this is a mess
char *print_line(int line_num, char *line, int col_num)
{
    char offset_string[16];
    snprintf(offset_string, 16, "%d | ", line_num);

    printf("%s", offset_string);

    while (*line != '\n' && *line != '\0')
    {
        printf("%c", *line++);
    }
    printf("\n");
    if (col_num > 0)
    {
        printf("%*s", (int)(col_num + strlen(offset_string) - 2 - 1), "");
        printf("\033[95m--^--\033[0m\n");
    }
    return (line + 1);
}

// Prints The Error message and the line above and below the token
// FIXME: this honestly sucks just improve it overall :D
void print_error_tok(Token *tk, char *absolute_start)
{
    print_token(tk);
    printf("\n\033[0m");

    int line_num = 1;
    char *local_absolute_start = absolute_start;
    char *line_above = local_absolute_start;
    char *error_line = local_absolute_start;

    while (local_absolute_start != tk->start_of_token)
    {
        if (*local_absolute_start == '\n')
        {
            line_above = error_line;
            error_line = (local_absolute_start + 1);
            line_num++;
        }
        local_absolute_start++;
    }
    int col_num = tk->start_of_token - error_line + 1;

    printf("%s:%d:%d\n", current_file, line_num, col_num);

    if (line_num != 1)
    {
        print_line(line_num - 1, line_above, 0);
    }
    line_above = print_line(line_num, error_line, col_num);

    print_line(line_num + 1, line_above, 0);
}

// consumes the next token in line
Token *eat_token(TokenList *tk)
{
    Token *a = &tk->data[tk->pars_ptr];
    tk->pars_ptr += 1;
    return a;
}

// consumes the next token in line and returns it
Token *eat_token_checked(TokenList *tk, TokenType check)
{
    Token *next = eat_token(tk);
    if (next->type != check)
    {
        printf("\033[91mERROR: expected ");
        Token check_tk = {check, {0}, NULL};
        print_token(&check_tk);
        printf(" got ");
        print_error_tok(next, global_start_of_file);
        printf("\033[0m");
        my_exit(-1);
    }
    return next;
}

// returns the next token in line without consuming it
Token *next_token(TokenList *tk)
{
    return &tk->data[tk->pars_ptr];
}

// returns the token with offset,
// offset 0 returns the same as Token* next_token(TokenList*)
Token *next_token_with_offset(TokenList *tk, int offset)
{
    return &tk->data[tk->pars_ptr + offset];
}

// consumes whitespaces
// ' ', '\n', '\t', '\r', '\v', '\f'
void skip_whitespace_comments(char **src)
{
    while (isspace((int)**src))
        *src += 1;

    // comments
    if (**src == '/' && *(*src + 1) == '/')
    {
        while (**src != '\n')
            *src += 1;
        *src += 1;
    }
    else if (**src == '/' && *(*src + 1) == '*')
    {
        *src += 2;
        while (**src != '*' || *(*src + 1) != '/')
            *src += 1;
        *src += 2;
    }

    while (isspace((int)**src))
        *src += 1;
}

// convert the numerical string to a token
//
// god knows what happens when someone tries to use non decimal
// base with floats
void tokenize_num(Token *tk, char **src)
{
    int num = strtol(*src, src, 0);

    if (**src == '.')
    {
        (*src) += 1;
        tk->type = TK_FLOAT;

        float tmp = num;

        float fraction = (float)strtol(*src, src, 0);
        tmp += fraction / (float)pow(10, (int)strlen(*src));

        tk->data.floating = tmp;
    }
    else
    {
        tk->type = TK_INT;
        tk->data.integer = num;
    }
}
void add_token(TokenList *tks, Token tk)
{
    arrput(tks->data, tk);
}

// Compares two tokens and their data
bool compare_tokens(Token a, Token b)
{
    if (a.type == b.type)
    {
        switch (a.type)
        {
        case TK_INT:
            return a.data.integer == b.data.integer;
        case TK_FLOAT:
            return a.data.floating == b.data.floating;
        case TK_TYPE_KEYWORD:
            return a.data.type == b.data.type;
        case TK_IDENT:
            return strcmp(a.data.ident, b.data.ident) == 0;
        default:
            return true;
        }
    }
    return false;
}

// Eat the comp operator
void tokenize_comp_operator(Token *tk, char **src)
{
    if (strncmp(*src, "==", 2) == 0)
    {
        tk->type = TK_EQ;
    }
    else if (strncmp(*src, "!=", 2) == 0)
    {
        tk->type = TK_NE;
    }
    else if (strncmp(*src, "<=", 2) == 0)
    {
        tk->type = TK_LE;
    }
    else if (strncmp(*src, ">=", 2) == 0)
    {
        tk->type = TK_ME;
    }
    else if ('<' == **src)
    {
        *src -= 1;
        tk->type = TK_LT;
    }
    else if ('>' == **src)
    {
        *src -= 1;
        tk->type = TK_MT;
    }
    else if ('=' == **src)
    {
        *src -= 1;
        tk->type = TK_ASSIGN;
    }
    else
    {
        printf("Not a Valid Comp Operator: %c", **src);
        my_exit(-1);
    }
    (*src) += 2;
}

void parse_include(char *path, Parser *parser)
{
    compile_file_to_scope(parser, path);
}

// tokenize and load into the buffer
//
// aborts if the input is not valid
void tokenize(TokenList *tk, char *src, Parser *parser)
{
    char *absolute_start = src;
    global_start_of_file = src;
    char *start = src;
    skip_whitespace_comments(&src);
    while (*src)
    {
        start = src;
        Token tmp = {
            TK_EOF,
            {0},
            start,
        };
        switch (*src)
        {
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
            // KEYWORDS
            if (isdigit((int)*src))
            {
                tokenize_num(&tmp, &src);
            }
            else if (strncmp(src, "return", 3) == 0)
            {
                src += 6;
                tmp.type = TK_RETURN;
                tmp.data.type = ty_void;
            }
            else if (strncmp(src, "void", 3) == 0)
            {
                src += 4;
                tmp.type = TK_TYPE_KEYWORD;
                tmp.data.type = ty_void;
            }
            else if (strncmp(src, "int", 3) == 0)
            {
                src += 3;
                tmp.type = TK_TYPE_KEYWORD;
                tmp.data.type = ty_int;
            }
            else if (strncmp(src, "float", 5) == 0)
            {
                src += 5;
                tmp.type = TK_TYPE_KEYWORD;
                tmp.data.type = ty_float;
            }
            else if (strncmp(src, "if", 2) == 0)
            {
                src += 2;
                tmp.type = TK_IF;
            }
            else if (strncmp(src, "else", 4) == 0)
            {
                src += 4;
                tmp.type = TK_ELSE;
            }
            else if (strncmp(src, "while", 5) == 0)
            {
                src += 5;
                tmp.type = TK_WHILE;
            }
            else if (strncmp(src, "for", 3) == 0)
            {
                src += 3;
                tmp.type = TK_FOR;
            }
            else if (strncmp(src, "#include", 8) == 0)
            {
                src += 8;
                while (*src == ' ')
                    src++;
                if (*src != '<' && *src != '"')
                {
                    printf("Not a valid include path\n");
                    print_error_tok(&tmp, absolute_start);
                    my_exit(-1);
                }
                src++;
                int start = 0;
                while (*(src + start) != '"' && *(src + start) != '>')
                    start++;
                char *path = (char *)malloc(start + 1);
                strncpy(path, src, start);
                path[start] = '\0';
                src += start + 1;
                parse_include(path, parser);
                free(path);
                goto skip_put;
            }
            else
            {
                char *buff;
                int i = 0;
                if (isalpha((int)*src) == 0 || *src == '_')
                {
                    printf(
                        "Not a valid identifier: '%c', hex: '%x' \n",
                        *src,
                        *src && 0xff);
                    print_error_tok(&tmp, absolute_start);
                    my_exit(-1);
                }
                while (isalpha((int)*src) || isdigit((int)*src) || *src == '_')
                {
                    i++;
                    src++;
                }
                buff = (char *)malloc(i + 1);
                strncpy(buff, (src - i), i);
                buff[i] = '\0';
                tmp.type = TK_IDENT;
                tmp.data.ident = buff;
            }
        }
        arrput(tk->data, tmp);
    skip_put:
        skip_whitespace_comments(&src);
    }
    Token eof = {
        TK_EOF,
        {0},
        NULL,
    };
    arrput(tk->data, eof);
}

#endif
