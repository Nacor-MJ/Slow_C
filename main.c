#include <stdio.h>
#include <stdlib.h>

#include "slow_c.h"

char current_file[256];

// test my executable and compare it to the gcc executable
void test_idk() {
    printf("\033[94mTesting IDK\033[0m\n");
    int s_return = system("idk.exe");
    system("gcc idk.c -o idk.exe");
    int g_return = system("idk.exe");

    printf("\033[93ms_return:\033[0m %d\n", s_return);
    printf("\033[93mg_return:\033[0m %d\n", g_return);

    if (s_return == g_return) {
        printf("\033[92mPassed\033[0m\n");
    } else {
        printf("\033[91mFailed\033[0m\n");
    }
}

// Load the file and return the malloced buffer
char* load_file(char const* path) {
    char* buffer = 0;
    long length;
    #ifndef _WIN64
    FILE * f = fopen(path, "rb");
    strcpy(current_file, path);
    #else
    FILE * f = NULL;
    fopen_s(&f, path, "rb"); // I have no clue why i need the b in there but without it it just breaks :)
    strcpy_s(current_file, strlen(path), path);
    #endif


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
        my_exit(-1);
    }

    buffer[length] = '\0';

    return buffer;
}

// This is so fun <3
int main(int argc, char *argv[]) {
    if (argc < 2){
        printf("Failed to provide program name\n");
        my_exit(-1);
    }

    char* file_path_with_extenstion = argv[1];

    printf("\033[94mCompiling %s\033[0m\n", file_path_with_extenstion);
    char* buff = load_file(file_path_with_extenstion);


    char file_path[strlen(file_path_with_extenstion) - 2];
    #ifndef _WIN64
    strcpy(file_path, file_path_with_extenstion);
    #else
    strcpy_s(
        file_path,
        strlen(file_path_with_extenstion) - 2, 
        file_path_with_extenstion
    );
    #endif
    file_path[strlen(file_path_with_extenstion) - 2] = '\0';

    // ------------ Tokenize --------------
    TokenList tokens = { NULL, 0 };
    tokenize(&tokens, buff);


    // printf("\033[94mParsed Tokens:\033[0m\n");
    // printTokens(tokens);

    // ------------ Parse --------------
    Program program = parse(tokens);
    printf("\033[94mParsed Program:\033[0m\n");
    print_program(&program, 0);

    // ----------- Generate IR ------------
    printf("\033[94mGenerating IR:\033[0m\n");
    IRList ir = ast_to_tac(&program);
    // print_ir_list(ir);

    // ---------- Convert to ASM ----------
    printf("\033[94mGenerating ASM:\033[0m\n");
    /*
    FILE* f = fopen("tmp.s", "w");
    generate_asm(f, &program);
    fclose(f);
    */

    // ---------- Convert to ASM ----------
    printf("\033[94mAssembling:\033[0m\n");

    char* asm_file_command = (char*) malloc(sizeof(char) * 256);

    sprintf(asm_file_command, "gcc tmp.s -o %s.exe", file_path);
    system(asm_file_command);

    printf("%s\n", asm_file_command);
    free(asm_file_command);

    // ---------- Cleanup ----------
    free_program(program);
    free(buff);

    for (int i = 0; arrlen(ir) > i; i++) {
        IR tac = ir[i];
        arrfree(tac);
    }
    arrfree(ir);
    free_token_list_and_data(&tokens);

    // test_idk();

    printf("\033[92mDone\033[0m\n");
    return 0;
}
