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
    FILE * f = fopen(path, "rb");
    strcpy(current_file, path);

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

void compile_file_to_scope(Parser* parser, char const* path) {
    printf("\033[94mCompiling %s\033[0m\n", path);

    char save_current_file[256];
    strncpy(save_current_file, current_file, 256);

    char* buff = load_file(path);

    char file_path[strlen(path) - 2];
    strncpy(
        file_path,
        path,
        strlen(path) - 2
    );
    file_path[strlen(path) - 2] = '\0';

    // ------------ Tokenize --------------
    TokenList tokens = { NULL, 0 };
    tokenize(&tokens, buff, parser);

    // ------------ Parse --------------

    parse(parser, tokens);
    printf("\033[94mParsed Program:\033[0m\n");
    // print_program(&parser->program, 0);

    free(buff);
    free_token_list_and_data(&tokens);

    strncpy(current_file, save_current_file, 256);
}

// This is so fun <3
int main(int argc, char *argv[]) {
    if (argc < 2){
        printf("Failed to provide program name\n");
        my_exit(-1);
    }

    char* file_path_with_extenstion = argv[1];

    Parser parser = {(Program){NULL, new_scope(NULL)}, NULL};
    compile_file_to_scope(&parser, file_path_with_extenstion);

    // ----------- Generate IR ------------
    printf("\033[94mGenerating IR:\033[0m\n");
    IRList ir = ast_to_tac(&parser.program);
    // print_ir_list(ir);


    printf("\033[94mBlockification:\033[0m\n");
    FunctionBlocks* bb_l = NULL;
    for (int i = 0; arrlen(ir) > i; i++) {
        FunctionBlocks b = NULL;
        blockification(&b, ir[i]);
        arrpush(bb_l, b);
    }

    free_stmt_list_not_scope(parser.program);

    // ---------- Convert to ASM ----------
    printf("\033[94mGenerating ASM:\033[0m\n");

    MachineInstrList machine_code = NULL;
    generate_x64(&machine_code, &bb_l);

    arrfree(bb_l);

    for (int i = 0; arrlen(ir) > i; i++) {
        IR tac = ir[i];
        arrfree(tac);
    }
    arrfree(ir);
    deinit_scope(parser.program.scope);


    NOT_IMPLEMENTED;
    // ---------- Convert to ASM ----------
    printf("\033[94mAssembling:\033[0m\n");

    char* asm_file_command = (char*) malloc(sizeof(char) * 256);

    char file_path[strlen(file_path_with_extenstion) - 2];
    strncpy(
        file_path,
        file_path_with_extenstion,
        strlen(file_path_with_extenstion) - 2
    );
    file_path[strlen(file_path_with_extenstion) - 2] = '\0';

    sprintf(asm_file_command, "gcc tmp.s -o %s.exe", file_path);
    system(asm_file_command);

    printf("%s\n", asm_file_command);
    free(asm_file_command);

    // ---------- Cleanup ----------

    // test_idk();

    printf("\033[92mDone\033[0m\n");
    return 0;
}
