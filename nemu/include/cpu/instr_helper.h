#ifndef __INSTR_HELPER_H__
#define __INSTR_HELPER_H__

// the type of an instruction entry
typedef int (*instr_func)(uint32_t eip, uint8_t opcode);

void print_asm_0(char *instr, char *suffix, uint8_t len);
void print_asm_1(char *instr, char *suffix, uint8_t len, OPERAND *opr_1);
void print_asm_2(char *instr, char *suffix, uint8_t len, OPERAND *opr_1, OPERAND *opr_2);
void print_asm_3(char *instr, char *suffix, uint8_t len, OPERAND *opr_1, OPERAND *opr_2, OPERAND *opr_3);

// macro for making an instruction entry
#define make_instr_func(name) int name(uint32_t eip, uint8_t opcode)

#define make_instr_decl_1op(inst_name, src_type, suffix) \
	make_instr_func(concat5(inst_name, _, src_type, _, suffix));

#define make_instr_decl_1op_cc(inst_name, src_type, suffix, cc) \
	make_instr_func(concat4(inst_name, cc, _, suffix));

#define make_instr_decl_2op(inst_name, src_type, dest_type, suffix) \
	make_instr_func(concat7(inst_name, _, src_type, 2, dest_type, _, suffix));

#define make_instr_decl_2op_cc(inst_name, src_type, dest_type, suffix, cc) \
	make_instr_func(concat7(concat(inst_name, cc), _, src_type, 2, dest_type, _, suffix));

// macro for generating the implementation of an instruction with one operand
#define make_instr_impl_1op(inst_name, src_type, suffix) \
	make_instr_func(concat5(inst_name, _, src_type, _, suffix)) \
	{ \
		int len = 1; \
		concat(decode_data_size_, suffix) \
		concat3(decode_operand, _, src_type) \
		print_asm_1(#inst_name, opr_src.data_size == 8 ? "b" : (opr_src.data_size == 16 ? "w" : "l"), len, &opr_src); \
		instr_execute_1op(); \
		return len; \
	}

// macro for generating the implementation of an instruction with one operand and condition
// for jcc and setcc, the opcode type are always fixed so it will not appear in the function name
#define make_instr_impl_1op_cc(inst_name, src_type, suffix, cc) \
	make_instr_func(concat4(inst_name, cc, _, suffix))          \
	{                                                           \
		int len = 1;                                            \
		concat(decode_data_size_, suffix)                       \
		concat3(decode_operand, _, src_type)                \
		print_asm_1(#inst_name, #cc, len, &opr_src);    \
		if (concat(condition_, cc))                             \
			instr_execute_1op_cc_pass();                        \
		else                                                    \
			instr_execute_1op_cc_fail();                        \
		return len;                                             \
	}

// macro for generating the implementation of an instruction with two operands
#define make_instr_impl_2op(inst_name, src_type, dest_type, suffix) \
	make_instr_func(concat7(inst_name, _, src_type, 2, dest_type, _, suffix)) \
	{ \
		int len = 1; \
		concat(decode_data_size_, suffix) \
		concat3(decode_operand, _, concat3(src_type, 2, dest_type)) \
		print_asm_2(#inst_name, opr_dest.data_size == 8 ? "b" : (opr_dest.data_size == 16 ? "w" : "l"), len, &opr_src, &opr_dest); \
		instr_execute_2op(); \
		return len; \
	}

// macro for generating the implementation of an instruction with two operands and condition
#define make_instr_impl_2op_cc(inst_name, src_type, dest_type, suffix, cc) \
	make_instr_func(concat7(concat(inst_name, cc), _, src_type, 2, dest_type, _, suffix)) \
	{ \
		int len = 1; \
		concat(decode_data_size_, suffix) \
		concat3(decode_operand, _, concat3(src_type, 2, dest_type)) \
		print_asm_2(#inst_name, #cc, len, &opr_src, &opr_dest); \
		if (concat(condition_, cc)) \
			instr_execute_2op_cc_pass(); \
		else \
			instr_execute_2op_cc_fail(); \
		return len; \
	}

// determine the data size of operands
// possible sizes b, w, l, v, bv, short, near
#define decode_data_size_b opr_src.data_size = opr_dest.data_size = 8;
#define decode_data_size_w opr_src.data_size = opr_dest.data_size = 16;
#define decode_data_size_l opr_src.data_size = opr_dest.data_size = 32;
#define decode_data_size_v opr_src.data_size = opr_dest.data_size = data_size;
#define decode_data_size_bv \
	opr_src.data_size = 8;  \
	opr_dest.data_size = data_size;
#define decode_data_size_short_ opr_src.data_size = opr_dest.data_size = 8;
#define decode_data_size_near opr_src.data_size = opr_dest.data_size = 32;

// determine the type and address of operand(s)
// possible types: r, rm, i, m, a, c, o
// here c stands for CL, CX, or ECX

#define decode_operand_rm \
	len += modrm_rm(eip + 1, &opr_src);

#define decode_operand_r    \
	opr_src.type = OPR_REG; \
	opr_src.addr = opcode & 0x7;

// AL = AX = EAX
#define decode_operand_a    \
	opr_src.type = OPR_REG; \
	opr_src.addr = REG_AL;

#define decode_operand_i    \
	opr_src.type = OPR_IMM; \
	opr_src.sreg = SREG_CS; \
	opr_src.addr = eip + 1; \
	len += opr_src.data_size / 8;

#define decode_operand_si    \
	opr_src.type = OPR_SIMM; \
	opr_src.sreg = SREG_CS; \
	opr_src.addr = eip + 1; \
	len += opr_src.data_size / 8;

#define decode_operand_r2rm \
	len += modrm_r_rm(eip + 1, &opr_src, &opr_dest);

#define decode_operand_rm2r \
	len += modrm_r_rm(eip + 1, &opr_dest, &opr_src);

#define decode_operand_sr2rm \
	len += modrm_r_rm(eip + 1, &opr_src, &opr_dest); \
    opr_src.type = OPR_SREG;

#define decode_operand_rm2sr \
	len += modrm_r_rm(eip + 1, &opr_dest, &opr_src); \
    opr_dest.type = OPR_SREG;

#define decode_operand_cr2rm \
	len += modrm_r_rm(eip + 1, &opr_src, &opr_dest); \
    opr_src.type = OPR_CREG;

#define decode_operand_rm2cr \
	len += modrm_r_rm(eip + 1, &opr_dest, &opr_src); \
    opr_dest.type = OPR_CREG;

#define decode_operand_i2rm              \
	len += modrm_rm(eip + 1, &opr_dest); \
	opr_src.type = OPR_IMM;              \
	opr_src.sreg = SREG_CS;              \
	opr_src.addr = eip + len;            \
	len += opr_src.data_size / 8;

#define decode_operand_si2rm             \
	len += modrm_rm(eip + 1, &opr_dest); \
	opr_src.type = OPR_SIMM;             \
	opr_src.sreg = SREG_CS;              \
	opr_src.addr = eip + len;            \
	len += opr_src.data_size / 8;

#define decode_operand_i2r        \
	len += opr_src.data_size / 8; \
	opr_src.type = OPR_IMM;       \
	opr_src.sreg = SREG_CS;       \
	opr_src.addr = eip + 1;       \
	opr_dest.type = OPR_REG;      \
	opr_dest.addr = opcode & 0x7;

// REG_AL == REG_AX == REG_EAX == 0
#define decode_operand_i2a   \
	opr_src.type = OPR_IMM;  \
	opr_src.sreg = SREG_CS;  \
	opr_src.addr = eip + 1;  \
	opr_dest.type = OPR_REG; \
	opr_dest.addr = REG_AL;  \
	len += opr_src.data_size / 8;

#define decode_operand_wd2a \
    opr_src.type = OPR_REG; \
    opr_src.addr = REG_DX; \
    opr_src.data_size = 16; \
    opr_dest.type = OPR_REG; \
    opr_dest.addr = REG_EAX;

// REG_CL == REG_CX == REG_ECX == 1
#define decode_operand_c2rm              \
	len += modrm_rm(eip + 1, &opr_dest); \
	opr_src.type = OPR_REG;              \
	opr_src.addr = REG_CL;

#define decode_operand_o2a                    \
	opr_src.type = OPR_MEM;                   \
	opr_src.sreg = SREG_DS;                   \
	if (verbose)                              \
		clear_operand_mem_addr(&opr_src);     \
	opr_src.addr = instr_fetch(eip + 1, 4);   \
	if (verbose)                              \
		opr_src.mem_addr.disp = opr_src.addr; \
	opr_dest.type = OPR_REG;                  \
	opr_dest.addr = REG_AL;                   \
	len += 4;

#define decode_operand_a2o                      \
	opr_dest.type = OPR_MEM;                    \
	opr_dest.sreg = SREG_DS;                    \
	if (verbose)                                \
		clear_operand_mem_addr(&opr_dest);      \
	opr_dest.addr = instr_fetch(eip + 1, 4);    \
	if (verbose)                                \
		opr_dest.mem_addr.disp = opr_dest.addr; \
	opr_src.type = OPR_REG;                     \
	opr_src.addr = REG_AL;                      \
	len += 4;

#define decode_operand_wi2i \
    opr_dest.type = OPR_IMM; \
    opr_dest.sreg = SREG_CS;  \
	opr_dest.addr = eip + 1;  \
    len += opr_dest.data_size / 8; \
    opr_src.type = OPR_IMM; \
    opr_src.data_size = 16; \
    opr_src.sreg = SREG_CS;  \
	opr_src.addr = eip + len;  \
    len += 2;

#define decode_operand_wm2m \
    ssrc = 16; \
	len += modrm_rm(eip + 1, psrc); \
    modrm_rm(eip + 1, pdst); \
    adst += 2;

#define make_al_impl(name) \
    make_instr_impl_2op(name, i, a, b) \
    make_instr_impl_2op(name, i, a, v) \
    make_instr_impl_2op(name, i, rm, b) \
    make_instr_impl_2op(name, i, rm, v) \
    make_instr_impl_2op(name, si, rm, bv) \
    make_instr_impl_2op(name, r, rm, b) \
    make_instr_impl_2op(name, r, rm, v) \
    make_instr_impl_2op(name, rm, r, b) \
    make_instr_impl_2op(name, rm, r, v)

#define make_al_decl(name) \
    make_instr_decl_2op(name, i, a, b) \
    make_instr_decl_2op(name, i, a, v) \
    make_instr_decl_2op(name, i, rm, b) \
    make_instr_decl_2op(name, i, rm, v) \
    make_instr_decl_2op(name, si, rm, bv) \
    make_instr_decl_2op(name, r, rm, b) \
    make_instr_decl_2op(name, r, rm, v) \
    make_instr_decl_2op(name, rm, r, b) \
    make_instr_decl_2op(name, rm, r, v)


#define make_al_ex_nwb(name) \
    static void instr_execute_2op() { \
        operand_read(pdst); \
        operand_read(psrc); \
        concat(alu_, name)(vsrc, vdst, sdst); \
    }

#define make_al_ex_wb(name) \
    static void instr_execute_2op() { \
        operand_read(pdst); \
        operand_read(psrc); \
        vdst = concat(alu_, name)(vsrc, vdst, sdst); \
        operand_write(pdst); \
    }

// conditions
// possible condition: e, a, ae, b, be, o, p, s , ne, na, no, np, ns, g, ge, l, le, ecxz

static inline bool inv_cc();

#define condition_e \
	is_zf

#define condition_a \
	!is_cf && !is_zf

#define condition_ae \
	!is_cf

#define condition_b \
	is_cf

#define condition_be \
	is_cf || is_zf

#define condition_o \
	is_of

#define condition_p \
	is_pf

#define condition_s \
	is_sf

#define condition_ne \
	!is_zf

#define condition_na \
	is_cf || is_zf

#define condition_no \
	!is_of

#define condition_np \
	!is_pf

#define condition_ns \
	!is_sf

#define condition_g \
	!is_zf && of_eq_sf

#define condition_ge \
	of_eq_sf

#define condition_l \
	!of_eq_sf

#define condition_le \
	is_zf || !of_eq_sf

#define condition_ecxz \
	cpu.ecx == 0

#define condition_c \
	is_cf

static inline bool inv_cc()
{
	printf("Please implement cc condition in instr_helper.h\n");
	fflush(stdout);
	assert(0);
	return false;
}

#define all_cc(f, ...) \
    f(__VA_ARGS__, o) f(__VA_ARGS__, no) f(__VA_ARGS__, b)  f(__VA_ARGS__, ae) \
    f(__VA_ARGS__, e) f(__VA_ARGS__, ne) f(__VA_ARGS__, na) f(__VA_ARGS__, a) \
    f(__VA_ARGS__, s) f(__VA_ARGS__, ns) f(__VA_ARGS__, p)  f(__VA_ARGS__, np) \
    f(__VA_ARGS__, l) f(__VA_ARGS__, ge) f(__VA_ARGS__, le) f(__VA_ARGS__, g)

#endif
