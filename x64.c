#include "slow_c.h"

HashSetVariable* hash_set_union(HashSetVariable* dest, HashSetVariable* b) {
    for (int i = 0; hmlen(b) > i; i++) {
        HashSetVariable a = b[i];
        hmputs(dest, a);
    }
    hmfree(b);
    return dest;
}

void address_assign_definition(Address* a) {
    if (a->kind != ADDR_VARIABLE) return;
    Liveness* liveness = &a->variable->liveness;
    liveness->is_live = false;
    liveness->next_use = NULL;
}
void address_assign_use(Address* a, TAC* tac) {
    if (a->kind != ADDR_VARIABLE) return;
    Liveness* liveness = &a->variable->liveness;
    liveness->is_live = true;
    liveness->next_use = tac;
}

MachineInstrList* tac_assign_liveness(
    MachineInstrList* result, 
    TAC* tac ,
    HashSetVariable** live_variables ) {

    switch (tac->op) {
        case TAC_LABEL:
            break;
        case TAC_RETURN:
        case TAC_CALL:
        case TAC_PARAM:
            // TODO: get rid of the fucking param thing and make it part of this TAC
            NOT_IMPLEMENTED;
            break;
        case TAC_ASSIGN:
        case TAC_IF_JMP:
        case TAC_IF_FALSE_JMP:
            NOT_IMPLEMENTED;
            break;
        case TAC_ADD:
        case TAC_SUB:
        case TAC_MUL:
        case TAC_DIV:
        case TAC_LE:
        case TAC_LT:
        case TAC_ME:
        case TAC_MT:
        case TAC_NE:
        case TAC_EQ:
            tac->liveness[0] = tac->result.variable->liveness;
            tac->liveness[1] = tac->arg1.variable->liveness;
            tac->liveness[2] = tac->arg2.variable->liveness;

            address_assign_definition(&tac->result);
            address_assign_use(&tac->arg1, tac);
            address_assign_use(&tac->arg2, tac);
            break;
        default:
            printf("tac_assign_liveness not implemented for TAC: %d\n", tac->op);
            my_exit(69);
    }

    if (!hmdel(*live_variables, tac->result.variable)) my_exit(69);

    HashSetVariable* current_vars = NULL;
    hmputs(current_vars, (HashSetVariable) {tac->result.variable});
    hmputs(current_vars, (HashSetVariable) {tac->result.variable});
    *live_variables = hash_set_union(*live_variables, current_vars);

    for (int i = 0; hmlen(live_variables) > i; i++) {
        Variable* v = (*live_variables)[i].key;

        v->buddies = hash_set_union(v->buddies, *live_variables);
    }

    // TAC
    print_tac_liveness(tac->liveness);
    print_tac(tac);


    return result;
}
MachineInstrList* basic_block_assign_liveness(
    MachineInstrList* result, 
    BasicBlock* bb,
    HashSetVariable** live_variables
) {
    // reverse traversal of basic block
    for (TAC* tac = bb->end; tac >= bb->leader; tac--) {
        result = tac_assign_liveness(result, tac, live_variables); 
    }
    return result;
}
MachineInstrList* function_block_assign_liveness(
    MachineInstrList* result, 
    FunctionBlocks* function
) {
    // we start from the last block
    BasicBlock* work_queue = NULL;
    arrput(work_queue, (*function)[arrlen(function) - 1]);

    // define variables that are live on entry
    HashSetVariable* live_variables = NULL;

    while (arrlen(work_queue) > 0) {
        BasicBlock bb = work_queue[ arrlen(work_queue) - 1 ];

        // remove the current block
        // NOTE: the error fixed at stb_ds.h:596
        arrpop(work_queue);

        // append the blocks predecesors to the queue
        int j = arraddnindex(work_queue, arrlen(work_queue));
        for (int i = 0; arrlen(work_queue) > i; i++) {
            work_queue[j] = work_queue[i];
            j++;
        }

        // generate the machine code for the current block
        result = basic_block_assign_liveness(result, &bb, &live_variables);
    }

    return result;
}

void generate_x64(MachineInstrList* result, FunctionBlocksList* program_p) {
    // Program
    FunctionBlocks* program = *program_p;

    for (int i = 0; arrlen(program) > i; i++) {
        // Function
        FunctionBlocks function = program[i];
        print_basic_block_list(&function);
        puts("");

        result = function_block_assign_liveness(result, &function);

        /*
        // reversed traversal of the functions basic blocks
        for (int i = arrlen(function) - 1; i >= 0; i--) {
            // Basic Block
            BasicBlock b = (function)[i];

            for (int i = 0; hmlen(b.variables) > i; i++) {
                Variable* v = b.variables[i].key;
                v->liveness.is_live = true;
            }

            // reversed traversal of basic block
            for (TAC* tac = b.end; tac >= b.leader; tac--) {
                memset(tac->liveness, 0, sizeof(tac->liveness));
                if (tac->op == TAC_CALL) NOT_IMPLEMENTED;
                if (tac->op != TAC_LABEL) {
                    if (tac->result.kind == ADDR_VARIABLE) {
                        Variable* result_v = tac->result.variable;
                        result_v->liveness = (Liveness) {
                            false, 
                            NULL
                        };
                        tac->liveness[0] = result_v->liveness;
                    }
                    if (tac->arg1.kind == ADDR_VARIABLE) {
                        Variable* arg1_v = tac->arg1.variable;
                        arg1_v->liveness = (Liveness) {
                            true, 
                            tac
                        };
                        tac->liveness[1] = arg1_v->liveness;
                    }
                    if (tac->arg2.kind == ADDR_VARIABLE) {
                        Variable* arg2_v = tac->arg2.variable;
                        arg2_v->liveness = (Liveness) {
                            true, 
                            tac
                        };
                        tac->liveness[2] = arg2_v->liveness;
                    }
                }

                // TAC
                print_tac_liveness(tac->liveness);
                print_tac(tac);

            }
            printf("B%d\n", i);
        }
        */
        puts("===============");
    }
}
