#include "cpu/instr.h"

// reference: http://www.felixcloutier.com/x86/

static uint32_t f2u(float val)
{
	uint32_t *temp = (uint32_t *)&val;
	return *temp;
}

static float u2f(uint32_t val)
{
	float *temp = (float *)&val;
	return *temp;
}

static float i2f(uint32_t val) {
    return (float)((int)val);
}

static float l2f(uint64_t val) {
    return (float)((int64_t)val);
}

static uint32_t f2i(float val) {
    return (uint32_t)((int)val);
}

static uint64_t f2l(float val) {
    return (uint64_t)((int64_t)val);
}

typedef union {
	double dval;
	uint32_t uval[2];
} DOUBLE;

static uint32_t mem2d(OPERAND *opr) {
    DOUBLE temp;
	operand_read(opr);
	temp.uval[0] = opr->val;
	opr->addr += 4;
	operand_read(opr);
	temp.uval[1] = opr->val;
    return f2u((float)(temp.dval));
}

static void d2mem(OPERAND *opr, uint32_t val) {
    DOUBLE temp;
    temp.dval = (double)u2f(val);
    opr->val = temp.uval[0];
    operand_write(opr);
    opr->addr += 4;
    opr->val = temp.uval[1];
    operand_write(opr);
}

make_instr_func(x87_d8) {
    uint32_t op_code = instr_fetch(eip + 1, 1);
    uint32_t idx = op_code & 7;
    int len = 2;
    switch (op_code & ~7) {
        case 0xc0: fpu_add_idx(idx, 0); return len;
        case 0xc8: fpu_mul_idx(idx, 0); return len;
        case 0xd0: fpu_cmp(idx); return len;
        case 0xd8: fpu_cmp(idx); fpu_store(); return len;
        case 0xe0: fpu_sub_idx(idx, 0); return len;
        case 0xe8: fpu_subr_idx(idx, 0); return len;
        case 0xf0: fpu_div_idx(idx, 0); return len;
        case 0xf8: fpu_divr_idx(idx, 0); return len;
    }
    uint8_t op_code_;
    ssrc = 32;
    len = 1 + modrm_opcode_rm(eip + 1, &op_code_, psrc);
    operand_read(psrc);
    switch (op_code_) {
        case 0: fpu_add(vsrc); break;
        case 1: fpu_mul(vsrc); break;
        case 2: fpu_cmp_mem(vsrc); break;
        case 3: fpu_cmp_mem(vsrc); fpu_store(); break;
        case 4: fpu_sub(vsrc); break;
        case 5: fpu_subr(vsrc); break;
        case 6: fpu_div(vsrc); break;
        case 7: fpu_divr(vsrc); break;
        default: inv(eip, opcode);
    }
    return len;
}

make_instr_func(x87_d9) {
    uint32_t op_code = instr_fetch(eip + 1, 1);
    uint32_t idx = op_code & 7;
    int len = 2;
    switch (op_code & ~7) {
        case 0xc0: fpu_load_idx(idx); return len;
        case 0xc8: fpu_xch(idx); return len;
        case 0xe0: switch (idx) {case 0: fpu_chs(); return len;
                                 case 1: fpu_abs(); return len;}
        case 0xe8: switch (idx) {case 0: fpu_load(f2u(1.0f)); return len;
                                 case 6: fpu_load(f2u(0.0f)); return len;}
        case 0xf0: switch (idx) {case 6: fpu_dectop(); return len;
                                 case 7: fpu_store(); return len;}
    }
    uint8_t op_code_;
    ssrc = 32;
    len = 1 + modrm_opcode_rm(eip + 1, &op_code_, psrc);
    switch (op_code_) {
        case 0: operand_read(psrc); fpu_load(vsrc); break;
        case 2: vsrc = fpu_peek(); operand_write(psrc); break;
        case 3: vsrc = fpu_store(); operand_write(psrc); break;
        case 5: operand_read(psrc); fpu.control.val = (uint16_t)vsrc; break;
        case 7: vsrc = (uint32_t)fpu.control.val; operand_write(psrc); break;
        default: inv(eip, opcode);
    }
    return len;
}

make_instr_func(x87_da) {
    uint32_t op_code = instr_fetch(eip + 1, 1);
    uint32_t idx = op_code & 7;
    int len = 2;
    switch (op_code & ~7) {
        case 0xe8: fpu_cmp(idx); fpu_store(); fpu_store(); return len;
    }
    uint8_t op_code_;
    ssrc = 32;
    len = 1 + modrm_opcode_rm(eip + 1, &op_code_, psrc);
    operand_read(psrc);
    uint32_t val = f2u(i2f(vsrc));
    switch (op_code_) {
        case 0: fpu_add(val); break;
        case 1: fpu_mul(val); break;
        case 2: fpu_cmp_mem(val); break;
        case 3: fpu_cmp_mem(val); fpu_store(); break;
        case 4: fpu_sub(val); break;
        case 5: fpu_subr(val); break;
        case 6: fpu_div(val); break;
        case 7: fpu_divr(val); break;
        default: inv(eip, opcode);
    }
    return len;
}

make_instr_func(x87_db) {
    uint32_t op_code = instr_fetch(eip + 1, 1);
    uint32_t idx = op_code & 7;
    int len = 2;
    switch (op_code & ~7) {
        case 0xe8:
        case 0xf0: fpu_cmpi(idx); return len;
    }
    uint8_t op_code_;
    ssrc = 32;
    len = 1 + modrm_opcode_rm(eip + 1, &op_code_, psrc);
    switch (op_code_) {
        case 0: operand_read(psrc); fpu_load(f2u(i2f(vsrc))); break;
        case 2: vsrc = f2i(u2f(fpu_peek())); operand_write(psrc); break;
        case 1:
        case 3: vsrc = f2i(u2f(fpu_store())); operand_write(psrc); break;
        default: inv(eip, opcode);
    }
    return len;
}

make_instr_func(x87_dc) {
    uint32_t op_code = instr_fetch(eip + 1, 1);
    uint32_t idx = op_code & 7;
    int len = 2;
    switch (op_code & ~7) {
        case 0xc0: fpu_add_idx(idx, idx); return len;
        case 0xc8: fpu_mul_idx(idx, idx); return len;
        case 0xe0: fpu_sub_idx(idx, idx); return len;
        case 0xe8: fpu_subr_idx(idx, idx); return len;
        case 0xf0: fpu_div_idx(idx, idx); return len;
        case 0xf8: fpu_divr_idx(idx, idx); return len;
    }
    uint8_t op_code_;
    ssrc = 32;
    len = 1 + modrm_opcode_rm(eip + 1, &op_code_, psrc);
    uint32_t val = mem2d(psrc);
    switch (op_code_) {
        case 0: fpu_add(val); break;
        case 1: fpu_mul(val); break;
        case 2: fpu_cmp_mem(val); break;
        case 3: fpu_cmp_mem(val); fpu_store(); break;
        case 4: fpu_sub(val); break;
        case 5: fpu_subr(val); break;
        case 6: fpu_div(val); break;
        case 7: fpu_divr(val); break;
        default: inv(eip, opcode);
    }
    return len;
}

make_instr_func(x87_dd) {
    uint32_t op_code = instr_fetch(eip + 1, 1);
    uint32_t idx = op_code & 7;
    int len = 2;
    switch (op_code & ~7) {
        case 0xd0: fpu_copy(idx); return len;
        case 0xd8: fpu_copy(idx); fpu_store(); return len;
        case 0xe0: fpu_cmp(idx); return len;
        case 0xe8: fpu_cmp(idx); fpu_store(); return len;
    }
    uint8_t op_code_;
    ssrc = 32;
    len = 1 + modrm_opcode_rm(eip + 1, &op_code_, psrc);
    uint64_t val;
    switch (op_code_) {
        case 0: fpu_load(mem2d(psrc)); break;
        case 1: val = f2l(u2f(fpu_store())); vsrc = (uint32_t)val; operand_write(psrc);
                asrc += 4; vsrc = (uint32_t)(val >> 32); operand_write(psrc); break;
        case 2: d2mem(psrc, fpu_peek()); break;
        case 3: d2mem(psrc, fpu_store()); break;
        default: inv(eip, opcode);
    }
    return len;
}

make_instr_func(x87_de) {
    uint32_t op_code = instr_fetch(eip + 1, 1);
    uint32_t idx = op_code & 7;
    int len = 2;
    switch (op_code & ~7) {
        case 0xc0: fpu_add_idx(idx, idx); fpu_store(); return len;
        case 0xc8: fpu_mul_idx(idx, idx); fpu_store(); return len;
        case 0xd8: fpu_cmp(idx); fpu_store(); fpu_store(); return len;
        case 0xe0: fpu_sub_idx(idx, idx); fpu_store(); return len;
        case 0xe8: fpu_subr_idx(idx, idx); fpu_store(); return len;
        case 0xf0: fpu_div_idx(idx, idx); fpu_store(); return len;
        case 0xf8: fpu_divr_idx(idx, idx); fpu_store(); return len;
    }
    ssrc = 16;
    uint8_t op_code_;
    len = 1 + modrm_opcode_rm(eip + 1, &op_code_, psrc);
    operand_read(psrc);
    uint32_t val = f2u(i2f(vsrc));
    switch (op_code_) {
        case 0: fpu_add(val); break;
        case 1: fpu_mul(val); break;
        case 2: fpu_cmp_mem(val); break;
        case 3: fpu_cmp_mem(val); fpu_store(); break;
        case 4: fpu_sub(val); break;
        case 5: fpu_subr(val); break;
        case 6: fpu_div(val); break;
        case 7: fpu_divr(val); break;
        default: inv(eip, opcode);
    }
    return len;
}

make_instr_func(x87_df) {
    uint32_t op_code = instr_fetch(eip + 1, 1);
    uint32_t idx = op_code & 7;
    int len = 2;
    switch (op_code & ~7) {
        case 0xe0: reg_w(REG_AX) = fpu.status.val; return len;
        case 0xe8: fpu_cmpi(idx); fpu_store(); return len;
        case 0xf0: fpu_cmpi(idx); fpu_store(); return len;
    }
    uint8_t op_code_;
    ssrc = 32;
    len = 1 + modrm_opcode_rm(eip + 1, &op_code_, psrc);
    uint64_t val = 0;
    switch (op_code_) {
        case 0: ssrc = 16; operand_read(psrc); fpu_load(f2u(i2f(vsrc))); break;
        case 2: ssrc = 16; vsrc = f2i(u2f(fpu_peek())); operand_write(psrc); break;
        case 1:
        case 3: ssrc = 16; vsrc = f2i(u2f(fpu_store())); operand_write(psrc); break;
        case 5: operand_read(psrc); val = (uint64_t)vsrc; asrc += 4; 
                operand_read(psrc); val |= (((uint64_t)vsrc) << 32);
                fpu_load(f2u(l2f(vsrc))); break;
        case 7: val = f2l(u2f(fpu_store())); vsrc = (uint32_t)val; operand_write(psrc);
                asrc += 4; vsrc = (uint32_t)(val >> 32); operand_write(psrc); break;
        default: inv(eip, opcode);
    }
    return len;
}

// Under this line are deprecated functions.

make_instr_func(x87_fldl)
{
	int len = 1;
	DOUBLE temp;
	opr_src.data_size = 32;
	len += modrm_rm(eip + 1, &opr_src);
	operand_read(&opr_src);
	temp.uval[0] = opr_src.val;
	opr_src.addr += 4;
	operand_read(&opr_src);
	temp.uval[1] = opr_src.val;
	print_asm_0("fldl", "", len);
	fpu_load(f2u((float)temp.dval));
	return len;
}

make_instr_func(x87_flds)
{
	int len = 1;
	opr_src.data_size = 32;
	len += modrm_rm(eip + 1, &opr_src);
	operand_read(&opr_src);
	print_asm_0("flds", "", len);
	fpu_load(opr_src.val);
	return len;
}

make_instr_func(x87_fildl)
{
	int len = 1;
	opr_src.data_size = 32;
	len += modrm_rm(eip + 1, &opr_src);
	operand_read(&opr_src);
	fpu_load(f2u((float)((int)opr_src.val)));
	print_asm_0("fildl", "", len);
	return len;
}

make_instr_func(x87_fldx)
{
	int len = 2;
	fflush(stdout);
	switch (instr_fetch(eip + 1, 1))
	{
	case 0xE8:
		fpu_load(f2u(1.0));
		break;
	case 0xE9:
		assert(0);
		break;
	case 0xEA:
		assert(0);
		break;
	case 0xEB:
		assert(0);
		break;
	case 0xEC:
		assert(0);
		break;
	case 0xED:
		assert(0);
		break;
	case 0xEE:
		fpu_load(f2u(0.0));
		break;
	default: // fldcw
		len = 1;
		opr_dest.data_size = 16;
		len += modrm_rm(eip + 1, &opr_dest);
		operand_read(&opr_dest);
		fpu.control.val = opr_dest.val;
		print_asm_0("fldcw", "", len);
		break;
	}
	return len;
}

make_instr_func(x87_fadds)
{
	int len = 1;
	uint32_t idx;
	switch (instr_fetch(eip + 1, 1))
	{
	case 0xC0:
	case 0xC1:
	case 0xC2:
	case 0xC3:
	case 0xC4:
	case 0xC5:
	case 0xC6:
	case 0xC7:
		idx = instr_fetch(eip + 1, 1) & 0x7;
		fpu_add_idx(idx, 0);
		len++;
		break;
	default: //fadds m32fp
		opr_src.data_size = 32;
		len += modrm_rm(eip + 1, &opr_src);
		operand_read(&opr_src);
		fpu_add(opr_src.val);
	}
	print_asm_0("fadds", "", len);
	return len;
}

make_instr_func(x87_faddx)
{
	int len = 1;
	uint32_t idx;
	switch (instr_fetch(eip + 1, 1))
	{
	case 0xC0:
	case 0xC1:
	case 0xC2:
	case 0xC3:
	case 0xC4:
	case 0xC5:
	case 0xC6:
	case 0xC7:
		idx = instr_fetch(eip + 1, 1) & 0x7;
		fpu_add_idx(idx, idx);
		fpu_store();
		len++;
		break;
	default: //fiadd m16int
		opr_src.data_size = 16;
		len += modrm_rm(eip + 1, &opr_src);
		operand_read(&opr_src);
		fpu_add(f2u((float)((int)opr_src.val)));
	}
	print_asm_0("fadd", "", len);
	return len;
}

make_instr_func(x87_faddlx)
{
	int len = 1;
	uint32_t idx;
	DOUBLE temp;
	switch (instr_fetch(eip + 1, 1))
	{
	case 0xC0:
	case 0xC1:
	case 0xC2:
	case 0xC3:
	case 0xC4:
	case 0xC5:
	case 0xC6:
	case 0xC7:
		idx = instr_fetch(eip + 1, 1) & 0x7;
		fpu_add_idx(idx, idx);
		len++;
		break;
	default: //faddl
		opr_src.data_size = 32;
		len += modrm_rm(eip + 1, &opr_src);
		operand_read(&opr_src);
		temp.uval[0] = opr_src.val;
		opr_src.addr += 4;
		operand_read(&opr_src);
		temp.uval[1] = opr_src.val;
		fpu_add(f2u((float)(temp.dval)));
	}
	print_asm_0("faddl", "", len);
	return len;
}

make_instr_func(x87_fsubs)
{
	int len = 1;
	opr_src.data_size = 32;
	len += modrm_rm(eip + 1, &opr_src);
	operand_read(&opr_src);
	fpu_sub(opr_src.val);
	print_asm_0("fsubs", "", len);
	return len;
}

make_instr_func(x87_fmuls)
{
	int len = 1;
	opr_src.data_size = 32;
	len += modrm_rm(eip + 1, &opr_src);
	operand_read(&opr_src);
	fpu_mul(opr_src.val);
	print_asm_0("fmuls", "", len);
	return len;
}

make_instr_func(x87_fmullx)
{
	int len = 1;
	uint32_t idx;
	DOUBLE temp;
	switch (instr_fetch(eip + 1, 1))
	{
	case 0xC8:
	case 0xC9:
	case 0xCA:
	case 0xCB:
	case 0xCC:
	case 0xCD:
	case 0xCE:
	case 0xCF:
		idx = instr_fetch(eip + 1, 1) & 0x7;
		fpu_mul_idx(idx, idx);
		len++;
		break;
	default: //fmul m64fp
		opr_src.data_size = 32;
		len += modrm_rm(eip + 1, &opr_src);
		operand_read(&opr_src);
		temp.uval[0] = opr_src.val;
		opr_src.addr += 4;
		operand_read(&opr_src);
		temp.uval[1] = opr_src.val;
		fpu_mul(f2u((float)(temp.dval)));
	}
	print_asm_0("fmull", "", len);
	return len;
}

make_instr_func(x87_fdivs)
{
	int len = 1;
	opr_src.data_size = 32;
	len += modrm_rm(eip + 1, &opr_src);
	operand_read(&opr_src);
	fpu_div(opr_src.val);
	print_asm_0("fdivs", "", len);
	return len;
}

make_instr_func(x87_fstps)
{
	int len = 1;
	opr_dest.data_size = 32;
	len += modrm_rm(eip + 1, &opr_dest);
	opr_dest.val = fpu_store();
	operand_write(&opr_dest);
	print_asm_0("fstps", "", len);
	return len;
}

make_instr_func(x87_fxch)
{
	uint32_t idx = instr_fetch(eip + 1, 1) & 0x7;
	fpu_xch(idx);
	print_asm_0("fxch", "", 2);
	return 2;
}

make_instr_func(x87_fucompp)
{
	fpu_cmp(1);
	fpu_store();
	fpu_store();
	print_asm_0("fucompp", "", 2);
	return 2;
}

make_instr_func(x87_fnstsw)
{
	cpu.gpr[0]._16 = fpu.status.val;
	print_asm_0("fnstsw", "", 2);
	return 2;
}

make_instr_func(x87_fstpx)
{
	int len = 1;
	uint32_t idx;
	DOUBLE temp;
	switch (instr_fetch(eip + 1, 1))
	{
	case 0xD8:
	case 0xD9:
	case 0xDA:
	case 0xDB:
	case 0xDC:
	case 0xDD:
	case 0xDE:
	case 0xDF:
		idx = instr_fetch(eip + 1, 1) & 0x7;
		fpu_copy(idx);
		fpu_store();
		len++;
		break;
	default: // fstpl
		opr_dest.data_size = 32;
		len += modrm_rm(eip + 1, &opr_dest);
		temp.dval = (double)u2f(fpu_store());
		opr_dest.val = temp.uval[0];
		operand_write(&opr_dest);
		opr_dest.addr += 4;
		opr_dest.val = temp.uval[1];
		operand_write(&opr_dest);
	}
	print_asm_0("fstp", "", len);
	return len;
}

make_instr_func(x87_fstx)
{
	int len = 1;
	uint32_t idx;
	DOUBLE temp;
	switch (instr_fetch(eip + 1, 1))
	{
	case 0xD0:
	case 0xD1:
	case 0xD2:
	case 0xD3:
	case 0xD4:
	case 0xD5:
	case 0xD6:
	case 0xD7:
		idx = instr_fetch(eip + 1, 1) & 0x7;
		fpu_copy(idx);
		len++;
		break;
	default: // fstl
		opr_dest.data_size = 32;
		len += modrm_rm(eip + 1, &opr_dest);
		temp.dval = (double)u2f(fpu_peek());
		opr_dest.val = temp.uval[0];
		operand_write(&opr_dest);
		opr_dest.addr += 4;
		opr_dest.val = temp.uval[1];
		operand_write(&opr_dest);
	}
	print_asm_0("fst", "", len);
	return len;
}

make_instr_func(x87_fsts)
{
	int len = 1;
	opr_dest.data_size = 32;
	len += modrm_rm(eip + 1, &opr_dest);
	opr_dest.val = fpu_peek();
	operand_write(&opr_dest);
	print_asm_0("fsts", "", len);
	return len;
}

make_instr_func(x87_fucom)
{
	uint32_t idx = instr_fetch(eip + 1, 1) & 0x7;
	fpu_cmp(idx);
	print_asm_0("fucom", "", 2);
	return 2;
}

make_instr_func(x87_fucomi)
{
	uint32_t idx = instr_fetch(eip + 1, 1) & 0x7;
	fpu_cmpi(idx);
	print_asm_0("fucomi", "", 2);
	return 2;
}

make_instr_func(x87_fcomi)
{
	return x87_fucomi(eip, opcode);
}

make_instr_func(x87_fucomip)
{
	uint32_t idx = instr_fetch(eip + 1, 1) & 0x7;
	fpu_cmpi(idx);
	fpu_store();
	print_asm_0("fucomip", "", 2);
	return 2;
}

make_instr_func(x87_fcomip)
{
	return x87_fucomip(eip, opcode);
}

make_instr_func(x87_fucomp)
{
	uint32_t idx = instr_fetch(eip + 1, 1) & 0x7;
	fpu_cmp(idx);
	fpu_store();
	print_asm_0("fucomp", "", 2);
	return 2;
}

make_instr_func(x87_fnstcw)
{
	int len = 1;
	opr_dest.data_size = 16;
	len += modrm_rm(eip + 1, &opr_dest);
	opr_dest.val = fpu.control.val;
	operand_write(&opr_dest);
	print_asm_0("fnstcw", "", len);
	return len;
}

make_instr_func(x87_fistpl)
{
	int len = 1;
	float f;
	uint32_t u;
	int i;
	opr_dest.data_size = 32;
	len += modrm_rm(eip + 1, &opr_dest);
	u = fpu_store();
	f = u2f(u);
	i = (int)f;
	opr_dest.val = i;
	operand_write(&opr_dest);
	print_asm_0("fistpl", "", len);
	return len;
}
