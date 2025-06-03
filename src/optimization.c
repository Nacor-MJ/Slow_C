#include "../include/slow_c.h"

Variable *hash_set_union(Variable *dest, Variable *b)
{
    for (int i = 0; hmlen(b) > i; i++)
    {
        Variable a = b[i];
        hmputs(dest, a);
    }
    hmfree(b);
    return dest;
}

typedef struct
{
    int key; // tac_index
    int value;
} IndexLabelHash;

void label_bb_switch(
    IndexLabelHash **hash,
    IR *program,
    int label,
    BasicBlock *bb)
{
    TAC *label_tac = *program + label;
    if (label_tac->op != TAC_LABEL)
    {
        puts("This should be a label:");
        print_tac(label_tac);
        my_exit(69)
    };
    hmput(*hash, label, bb->index);
}

void jmp_label_to_bb(
    IndexLabelHash *hash,
    TAC *tac)
{
    int bb_index = hmgeti(hash, tac->result.label_index);
    if (bb_index == -1)
    {
        printf("This fucking tac: \n");
        print_tac(tac);
        my_exit(69);
    }
    tac->result = (Address){.bb_index = hash[bb_index].value, ADDR_BASIC_BLOCK};
}

void blockification(FunctionBlocks *result, IR program)
{
    IndexLabelHash *hash = NULL;

    int index_counter = 0;
    BasicBlock block = (BasicBlock){NULL, NULL, index_counter++};

    block.leader = program + 0;

    for (int i = 0; i < arrlen(program); i++)
    {
        TAC *tac = program + i;

        if (i > 0)
        {
            Address addr_arr[3] = {
                tac->result,
                tac->arg1,
                tac->arg2};
            for (int j = 0; j < 3; j++)
            {
                Address address = addr_arr[j];
                if (address.kind == ADDR_VARIABLE)
                {
                    HashSetVariable var = {address.variable};
                    hmputs(block.variables, var);
                }
            }
        }

        if (tac->op == TAC_LABEL ||
            tac->op == TAC_CALL)
        {

            if (tac->op == TAC_CALL)
            {
                NOT_IMPLEMENTED;
            }

            block.end = program + i - 1;

            if (block.leader < block.end)
            {
                arrpush(*result, block);

                block = (BasicBlock){NULL, NULL, index_counter};
                block.leader = program + i;
                index_counter++;
            }
            // remove jump labels but not function labels
            if (block.leader->op != TAC_LABEL ||
                block.leader->result.kind != ADDR_VARIABLE)
            {
                block.leader += 1;
            }

            label_bb_switch(&hash, &program, i, &block);
        }
        else if (tac->op == TAC_JMP ||
                 tac->op == TAC_IF_JMP ||
                 tac->op == TAC_IF_FALSE_JMP)
        {
            block.end = program + i;
            if (block.leader < block.end)
            {
                arrpush(*result, block);

                block = (BasicBlock){NULL, NULL, index_counter};
                block.leader = program + i + 1;

                index_counter++;
            }
        }
    }

    block.end = program + arrlen(program) - 1;
    arrpush(*result, block);

    for (int i = 0; i < arrlen(program); i++)
    {
        TAC *tac = program + i;
        if (
            tac->op == TAC_JMP ||
            tac->op == TAC_IF_JMP ||
            tac->op == TAC_IF_FALSE_JMP)
        {
            jmp_label_to_bb(hash, tac);
        };
    }
};
