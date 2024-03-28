#include <stdio.h>

#include "parser.c"

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
    }
    buffer[length] = '\0';

    return buffer;
}

int main(int argc, char *argv[]) {
    if (argc < 2){
        printf("Failed to provide program name\n");
        // exit(-1);
    } else {
        argv[1] = "tmp.txt";
    }

    char* file_path = argv[1];

    char* buff = load_file(file_path);;

    Node* nd = (Node*) malloc(sizeof(Node));

    *nd = parse(buff);


    Instruction* start = empty_instruction();

    start->next = NULL;

    node_to_ir(nd, start, RAX);

    char ir[256] = {'\0'};
    inst_seq_to_char(start, ir);

    FILE* fptr = fopen("out.asm", "w");
    fprintf(fptr, "global _start \n \tsection .text \n" );
    fprintf(fptr, ir);
    fprintf(fptr, "\tret\n");
    fclose(fptr);

    free_node_children(nd);
    free_instr_and_children(start);
    free(buff);
	return 0;
}
