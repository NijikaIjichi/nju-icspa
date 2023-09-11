#ifndef __INSTR_JMP_H__
#define __INSTR_JMP_H__

make_instr_decl_1op(jmp, si, b)
make_instr_decl_1op(jmp, si, v)
make_instr_decl_1op(jmp, rm, v)

make_instr_decl_2op(jmp, wi, i, v)
make_instr_decl_2op(jmp, wm, m, v)

#endif
