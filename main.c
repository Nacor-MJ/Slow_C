#include <stdio.h>
#include <stdlib.h>

#include "slow_c.h"
#include "tests.c"
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

void compile(char* file_path, char* ir) {
    // Compiling ig
    char out[64] = "";
    strncpy(out, file_path, 60);
    strcat(out, ".S");

    FILE* fptr = fopen(out, "w");
    fprintf(fptr, "\t.text \n\t.globl main\n\t.intel_syntax noprefix\n\t.def\tmain\nmain:\n" );
    fprintf(fptr, ir);
    fprintf(fptr, "\tret\n");
    fclose(fptr);
    
    char ass[128] = "";
    char link[128] = "";

    sprintf(ass, "gcc -c %s.S -o %s.o", file_path, file_path);
    sprintf(link, "gcc %s.o -o %s.exe", file_path, file_path);

    // assembly
    if (0 != system(ass)) {
        printf("Failed to assemble %s\n", file_path);
        exit(-1);
    }
    printf("Successfully assembled\n");

    // linking
    if (0 != system(link)){
        printf("Failed to link %s\n", file_path);
        exit(-1);
    }

    printf("Successfully compiled %s\n", file_path);

    char tmp[64] = "";
    sprintf(tmp,"%s.o", file_path);
    remove(tmp);
    // sprintf(tmp,"%s.S", file_path);
    // remove(tmp);
}

int main(int argc, char *argv[]) {
    if (argc < 2){
        printf("Failed to provide program name\n");
        exit(-1);
    }

    char* file_path = argv[1];

    if (strcmp(file_path, "--test") == 0){
        test();
        return 0;
    }

    printf("Compiling %s\n", file_path);
    char* buff = load_file(file_path);;

    // printf("Src: %s\n", buff);

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

    semantic_check(nd);
    
    /*
    // ------------ IR --------------
    IRList ir;
    vec_init(&ir);

    node_to_ir(nd, &ir);

    printf("\033[92mIR:\033[0m\n");
    int i; IR* ir2;
    vec_foreach(&ir, ir2, i) {
        print_ir(ir2);
    }
    exit(0);

    Instruction* start = empty_instruction();

    start->next = NULL;

    char ir[256] = {'\0'};
    inst_seq_to_char(start, ir);

    // ------------ Assembly --------------

    char out[64] = ""; // filename 
    strncpy(out, file_path, strlen(file_path) - 4);
    compile(out, ir);

    free_instr_and_children(start);
    vec_deinit(&ir);
    */

    free(tokens);
    free_node_children(nd);
    free(buff);
	return 0;
}
