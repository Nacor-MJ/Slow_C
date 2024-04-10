# ifndef IR_C
#define IR_C

#include "slow_c.h"

void print_ir_val(IR_val* val) {
    if (val->type == IR_VAL_INT) {
        printf("%d", val->data.i);
    } else if (val->type == IR_VAL_PTR) {
        printf("v%d", val->data.p->index);
    }
}

void print_ir(IR* ir) {
    printf(
        "v%d = v%d %s v%d", 
        ir->index, 
        ir->l->index, 
        instructiontype_to_char(ir->op), 
        ir->r->index
    );
}

// returns the index of the result of the node
IR* node_to_ir(Node* nd, IRList* ir_list){
    NodeVar variant = nd->var;
    
    IRList child;
    vec_init(&child);
    
    IR* ir_instr;
    ir_instr = (IR*) malloc(sizeof(IR));
    if (ir_instr == NULL) exit(69);

    ir_instr.index = ir_list->length;

    if (variant == VAL) {
        ir_instr.l = nd->val.val;
        ir_instr.r = -1;
        ir_instr.op = MOV;
    } else if (variant == BIN_EXPR){
        BinExpr* be = nd->val.bin_expr;

        IR l = node_to_ir(be->l, ir_list);

        IR r = node_to_ir(be->r, ir_list);
    
        ir_instr.l = l.l;
        ir_instr.r = r.l;
        ir_instr.op = be->op;
    } else if (variant == PROGRAM) {
        NodeList ndlist = nd->val.program;

        int i; Node* nd;
        vec_foreach(&ndlist, nd, i) {
            I
        }
    } else if (variant == VARIABLE) {
        VariableAssignment* va = nd->val.variable_assignment;
        target_reg = node_to_ir(va->val, parent, target_reg);
    } else if (variant == FUNCTION_CALL) {
        FunctionCall fc = nd->val.function_call;
        NodeList ndlist = fc.args;

        int count = 0; Node* nd;
        vec_foreach(&ndlist, nd, count) {
            target_reg = node_to_ir(nd, parent, target_reg);
            count++;
        }

        if (count > 4) printf("Implement pushing args to stack\n");

        if (target_reg == RAX) {
            function_call_instruction(fc.name, R15);
        } else {
            push_instr(RAX);

            // reserving all registers
            Reg current = target_reg;
            for (int i = 0; i < count; i++) {
                push_instr(current);
                current = increase_reg(current);
            }
            
            Reg arg_stor = target_reg;
            Reg arg_target = RCX;
            for (int i = 0; i < count; i++) {
                mov_instr_r(arg_target, arg_stor);
                // TODO reserver used registers??
                arg_stor = increase_reg(arg_stor);
                arg_target = increase_reg(arg_target);
            }

            function_call_instruction(fc.name, target_reg);

            for (int i = 0; i < count; i++) {
                push_instr(current);
                current = decrease_reg(current);
            }
            pop_instr(RAX);
            target_reg = increase_reg(target_reg);
        }
    } else {
        printf("Node_to_ir not implemented for %d\n", variant);
    }

    vec_push(&ir_list, ir_instr);
    return ir_instr;
}

#endif
