Instruction* pop_instr(Reg reg){
    Instruction* inst = empty_instruction();
    inst->type = POP;
    inst->args = REG;
    inst->first_arg.reg = reg;
    return inst;
}
Instruction* push_instr(Reg reg){
    Instruction* inst = empty_instruction();
    inst->type = PUSH;
    inst->args = REG;
    inst->first_arg.reg = reg;
    return inst;
}
Instruction* mov_instr_c(Reg reg, int val){
    Instruction* inst = empty_instruction();
    inst->type = MOV;
    inst->args = REG_CONST;
    inst->first_arg.reg = reg;
    inst->second_arg.constant = val;
    return inst;
}
Instruction* mov_instr_r(Reg reg, Reg reg2){
    Instruction* inst = empty_instruction();
    inst->type = MOV;
    inst->args = REG_REG;
    inst->first_arg.reg = reg;
    inst->second_arg.reg = reg2;
    return inst;
}
Instruction* mul_instr(Reg reg){
    Instruction* inst = empty_instruction();
    inst->type = MUL;
    inst->args = REG;
    inst->first_arg.reg = reg;
    return inst;
}
Instruction* div_instr(Reg reg){
    Instruction* inst = empty_instruction();
    inst->type = DIV;
    inst->args = REG;
    inst->first_arg.reg = reg;
    return inst;
}

Instruction* div_instr_seq(Reg reg, Reg reg2){
    Instruction* instr = empty_instruction();
    append_instruction(instr, push_instr(R15));
    append_instruction(instr, push_instr(RAX));
    append_instruction(instr, push_instr(RDX));
    append_instruction(instr, push_instr(RCX));

    append_instruction(instr, mov_instr_c(RDX, 0));
    if (reg != RAX) append_instruction(instr, mov_instr_r(RAX, reg));
    if (reg2 != RCX) append_instruction(instr, mov_instr_r(RCX, reg2));

    append_instruction(instr, div_instr(RCX));
    append_instruction(instr, mov_instr_r(R15, RAX));

    append_instruction(instr, pop_instr(RCX));
    append_instruction(instr, pop_instr(RDX));
    append_instruction(instr, pop_instr(RAX));

    append_instruction(instr, mov_instr_r(reg, R15));
    append_instruction(instr, pop_instr(R15));
    return instr;
}

Instruction* test_instr(Reg reg, Reg reg2){
    Instruction* inst = empty_instruction();
    inst->type = TEST;
    inst->args = REG_REG;
    inst->first_arg.reg = reg;
    inst->second_arg.reg = reg2;
    return inst;
}

Instruction* set_instr(Binop op, Reg reg){
    Instruction* inst = empty_instruction();
    inst->type = binop_to_instructiontype(op);
    inst->args = REG;
    inst->first_arg.reg = reg;
    return inst;
}

Instruction* function_call_instruction(char* name, Reg target_reg) {
    Instruction* inst = empty_instruction();

    if (target_reg != RAX) push_instr(target_reg);
    
    inst->type = CALL;
    inst->args = FUNCTION_NAME;
    inst->first_arg.function_name = name;

    if (target_reg != RAX) pop_instr(target_reg);

    return inst;
}
