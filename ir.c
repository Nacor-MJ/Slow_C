typedef enum Reg{
    RAX,
    RCX,
    RDX,
    R8,
    R9,
    R10,
    R11,
    INVALID,
    AL,
    NA
} Reg;

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
        default:
            printf("Invalid register used\n");
            exit(-1);
    }
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

typedef enum InstructionType {
    // data transfer
    MOV,
    MOVZX,

    // arithmetic
    ADD,
    MUL,
    DIV,
    SUB,

    // binops
    SETE,
    SETNE,
    SETG,
    SETL,
    SETGE,
    SETLE,

    // cmp
    CMP,

    // stack manipulation
    PUSH,
    POP,

    // NOP
    NOP,
} InstructionType;

typedef union InstructionVal{
    Reg reg;
    int constant;
} InstructionVal;

typedef enum {
    REG_CONST,
    REG_REG,
    REG,
    CONST,
    NONE
} InstructionArgs;

typedef struct Instruction{
    InstructionType type;
    InstructionVal first_arg;
    InstructionVal second_arg;
    InstructionArgs args;
    Instruction* next;
} Instruction;

const char* parse_reg(InstructionVal instval){
    Reg reg = instval.reg;

    return reg_to_char(reg);
}

const char* instructiontype_to_char(InstructionType insttype){
    switch (insttype) {
        case MOV:
            return "mov";
        case MOVZX:
            return "movzx";
        case ADD:
            return "add";
        case SUB:
            return "sub";
        case DIV:
            return "div";
        case MUL:
            return "mul";
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
    char first_arg[12] = {'\0'}; // the longest int in C is 10 chars long
    char second_arg[12] = {'\0'};

    strcpy(instr, instructiontype_to_char(inst->type));

    if (inst->type == DIV){
        // TODO check if this works 
        strcat(dst, "\tpush rdx\n");
        strcat(dst, "\tpush rax\n");
        strcat(dst, "\tpush rcx\n");

        char tmp[30] = {'\0'};
        strcat(dst, "\tmov rdx, 0\n");

        sprintf(tmp, "\tmov rax, %s\n", reg_to_char(inst->first_arg.reg));
        strcat(dst, tmp);
        sprintf(tmp, "\tmov rcx, %s\n", reg_to_char(inst->second_arg.reg));
        strcat(dst, tmp);
        strcat(dst, "\tdiv rcx\n");

        strcat(dst, "\tpush rax\n");

        strcat(dst, "\tpop rcx\n");
        strcat(dst, "\tpop rax\n");
        strcat(dst, "\tpop rdx\n");

        sprintf(tmp, "\tpop %s\n", reg_to_char(inst->first_arg.reg));
        strcat(dst, tmp);
        return;
    } else if (inst->type == MUL) {
        // TODO fix this
        exit(13);
        strcat(dst, "\tpush rax\n");
        return;
    } 
    switch (inst->args) {
        case REG_CONST:
            strcpy(first_arg, reg_to_char(inst->first_arg.reg));
            sprintf(second_arg, "%d", inst->second_arg.constant);
            break;
        case REG_REG:
            strcpy(first_arg, reg_to_char(inst->first_arg.reg));
            strcpy(second_arg, reg_to_char(inst->second_arg.reg));
            break;
        case REG:
            strcpy(first_arg, reg_to_char(inst->first_arg.reg));
            break;
        case CONST:
            sprintf(first_arg, "%d", inst->first_arg.constant);
            break;
        case NONE:
            break;
        default:
            printf("Invalid instruction type\n");
            exit(-1);
    }
    

    if (strcmp(instr, "nop") == 0){
        sprintf(dst, "\t%s\n", instr);
        return;
    }
    sprintf(dst, "\t%s %s, %s\n", instr, first_arg, second_arg);
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
/*
    Instruction* curr = parent->next;

    while (curr != NULL){
        printf("curr: %s, %p, %p \n", instructiontype_to_char(curr->type), curr, curr->next);
        curr = curr->next;
    }
    curr->next = child;
*/
}
void free_instr_and_children(Instruction* instr){
    Instruction* curr = instr;
    while(curr){
        Instruction* next = curr->next;
        free(curr);
        curr = next;
    }
}
