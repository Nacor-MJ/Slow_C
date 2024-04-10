#ifndef ASSEMBLY_C
#define ASSEMBLY_C

#include "slow_c.h"

const char* reg_to_char(Reg reg){
    switch (reg){
        case RAX:
            return "rax";
        case RCX:
            return "rcx";
        case RDX:
            return "rdx";
        case R8:
            return "r8";
        case R9:
            return "r9";
        case R10:
            return "r10";
        case R11:
            return "r11";
        case AL:
            return "al";
        case R15:
            return "r15";
        default:
            printf("Invalid register used\n");
            exit(-1);
    }
}

Reg decrease_reg(Reg reg){
    int tmp = (int) reg;
    if (reg == RAX) {
        printf("RAX can't be decreased\n");
        exit(-1);
    }
    tmp -= 1;
    Reg result = (Reg) tmp;
    return result;
}

Reg increase_reg(Reg reg){
    int tmp = (int) reg;
    tmp += 1;
    Reg result = (Reg) tmp;
    if (result == INVALID || result == NA){
        printf("Too many registers, fix this dude!!!");
        exit(-1);
    }
    return result;
}

const char* parse_reg(InstructionVal instval){
    Reg reg = instval.reg;

    return reg_to_char(reg);
}

const char* instructiontype_to_char(InstructionType insttype){
    switch (insttype) {
        case MOV:
            return "movq";
        case MOVZX:
            return "movzx";
        case ADD:
            return "add";
        case SUB:
            return "sub";
        case DIV:
            return "idiv";
        case MUL:
            return "imul";
        case SETE:
            return "sete";
        case SETNE:
            return "setne";
        case SETG:
            return "setg";
        case SETL:
            return "setl";
        case SETGE:
            return "setge";
        case SETLE:
            return "setle";
        case CMP:
            return "cmp";
        case TEST:
            return "test";
        case PUSH:
            return "push";
        case POP:
            return "pop";
        case NOP:
            return "nop";
        default:
            printf("Invalid instruction type\n");
            exit(-1);
    }
}

// TODO free the child instructions
Instruction* empty_instruction(){
    Instruction* instr = (Instruction*) malloc(sizeof(Instruction));

    instr->type = NOP;
    instr->args = NONE;
    InstructionVal val;
    val.reg = INVALID;
    instr->first_arg = val;
    instr->second_arg = val;
    instr->next = NULL;

    return instr;
}

void instruction_to_char(Instruction* inst, char* dst){
    char instr[6] = {'\0'};
    char dest_arg[12] = {'\0'}; // the longest int in C is 10 chars long
    char op_arg[12] = {'\0'};

    sprintf(instr, "%s", instructiontype_to_char(inst->type));

    switch (inst->args) {
        case REG_CONST:
            sprintf(dest_arg, "%s", reg_to_char(inst->first_arg.reg));
            sprintf(op_arg, ", %d", inst->second_arg.constant);
            break;
        case REG_REG:
            sprintf(dest_arg, "%s", reg_to_char(inst->first_arg.reg));
            sprintf(op_arg, ", %s",  reg_to_char(inst->second_arg.reg));
            break;
        case REG:
            sprintf(dest_arg, "%s",reg_to_char(inst->first_arg.reg));
            break;
        case CONST:
            sprintf(dest_arg, "%d", inst->first_arg.constant);
            break;
        case NONE:
            break;
        default:
            printf("Invalid instruction type\n");
            exit(-1);
    }

    if (strcmp(instr, "nop") == 0){
        return;
    }

    sprintf(dst, "\t%s %s%s\n", instr, dest_arg, op_arg);
}

void inst_seq_to_char(Instruction* instr, char* dst){
    Instruction* curr_instr = instr;
    while(curr_instr != NULL){
        char tmp[30] = {0};
        instruction_to_char(curr_instr, tmp);
        strcat(dst, tmp);
        curr_instr = curr_instr->next;
    }
}


void print_instr_seq(Instruction* instr){
    char tmp[640] = {0};
    inst_seq_to_char(instr, tmp);
    printf("%s", tmp);
}

void print_instr(Instruction* instr){
    char tmp[30] = {0};
    instruction_to_char(instr, tmp);
    printf("%s", tmp);
}

// appends to the instruction linked list
void append_instruction(Instruction* parent, Instruction* child){
    Instruction* next = parent->next;
    
    if (next == NULL){
        print_instr(child);
        parent->next = child;
    } else {
        append_instruction(next, child);
    }
}
void free_instr_and_children(Instruction* instr){
    Instruction* curr = instr;
    while(curr){
        Instruction* next = curr->next;
        free(curr);
        curr = next;
    }
}

#endif
