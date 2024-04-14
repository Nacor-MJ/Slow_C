#include <stdio.h>
#include <stdlib.h>

#include "slow_c.h"
// #define DBG 1

char* load_file(char const* path)
{
    char* buffer = 0;
    long length;
    FILE * f = fopen (path, "rb");

    if (f)
    {
      fseek (f, 0, SEEK_END);
      length = ftell (f);
      fseek (f, 0, SEEK_SET);
      buffer = (char*)malloc ((length+1)*sizeof(char));
      if (buffer)
      {
        fread (buffer, sizeof(char), length, f);
      }
      fclose (f);
    } else {
        printf("Failed to open %s\n", path);
        exit(-1);
    }

    buffer[length] = '\0';

    return buffer;
}

void compile_fortran(char* file_path) {
    char command[256];

    sprintf(command, "gfortran -o %s.exe %s.f90", file_path, file_path);

    system(command);
}

int main(int argc, char *argv[]) {
    if (argc < 2){
        printf("Failed to provide program name\n");
        exit(-1);
    }

    char* file_path_with_extenstion = argv[1];

    printf("\033[94mCompiling %s\033[0m\n", file_path_with_extenstion);
    char* buff = load_file(file_path_with_extenstion);

    char file_path[strlen(file_path_with_extenstion) - 4];
    strncpy(file_path, file_path_with_extenstion, strlen(file_path_with_extenstion) - 4);
    file_path[strlen(file_path_with_extenstion) - 4] = '\0';

    // ------------ Tokenize --------------
    Token *tokens = (Token*) calloc(40, sizeof(Token));
    if (tokens == NULL) exit(69);
    tokenize(tokens, buff);
    
    // printf("\033[94mParsed Tokens:\033[0m\n");
    // printTokens(tokens);

    // ------------ Parse --------------
    Node* nd = (Node*) malloc(sizeof(Node));

    *nd = parse(tokens);

    // ------------ Semantic Checks -------------

    printf("\033[94mSemantic Checks:\033[0m\n");
    semantic_check(nd);

    // ----------- Generate IR ------------

    printf("\033[94mGenerating IR:\033[0m\n");
    char file_name_buff[64] = "";
    strncpy(file_name_buff, file_path, 60);
    strcat(file_name_buff, ".f90");

    FILE* ir = fopen(file_name_buff, "w");
    generate_ir(ir, nd);
    fclose(ir);

    printf("\033[94mCompiling:\033[0m\n");
    compile_fortran(file_path);

    free(tokens);
    free_node_children(nd);
    free(buff);
	return 0;
}
