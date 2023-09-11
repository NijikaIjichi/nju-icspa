#ifndef __INSTR_PUSH_H__
#define __INSTR_PUSH_H__
/*
Put the declarations of `push' instructions here.
*/

make_instr_decl_1op(push, rm, v)
make_instr_decl_1op(push, r, v)
make_instr_decl_1op(push, si, b)
make_instr_decl_1op(push, si, v)

make_instr_func(pusha);

#endif
