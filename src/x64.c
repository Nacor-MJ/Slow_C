#include "../include/slow_c.h"

void generate_x64(MachineInstrList *result, FunctionBlocksList *program_p)
{
    // Program
    FunctionBlocks *program = *program_p;

    for (int i = 0; arrlen(program) > i; i++)
    {
        // Function
        FunctionBlocks function = program[i];
        print_basic_block_list(&function);
        puts("");

        // reversed traversal of the functions basic blocks
        for (int i = arrlen(function) - 1; i >= 0; i--)
        {
            // Basic Block
            BasicBlock b = (function)[i];

            for (int i = 0; hmlen(b.variables) > i; i++)
            {
                Variable *v = b.variables[i].key;
                v->liveliness.is_live = true;
            }

            // reversed traversal of basic block
            for (TAC *tac = b.end; tac >= b.leader; tac--)
            {

                if (tac->op != TAC_LABEL)
                {
                    Address addr_arr[3] = {tac->result, tac->arg1, tac->arg2};
                    for (int j = 0; j < 3; j++)
                    {
                        Address address = addr_arr[j];
                        if (address.kind == ADDR_VARIABLE)
                        {
                        }
                    }
                }

                // TAC
                print_tac(tac);
            }
            printf("B%d\n", i);
        }
        puts("===============");
    }
}
