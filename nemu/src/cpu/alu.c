#include "cpu/cpu.h"

#define calc_uint(a, b, op, width1, width2) \
    ((concat3(uint, width2, _t)) \
    ((concat3(uint, width2, _t))(concat3(uint, width1, _t))(a) op \
     (concat3(uint, width2, _t))(concat3(uint, width1, _t))(b)))

#define calc_int(a, b, op, width1, width2) \
    ((concat3(int,  width2, _t)) \
    ((concat3(int,  width2, _t))(concat3(int,  width1, _t))(a) op \
     (concat3(int,  width2, _t))(concat3(int,  width1, _t))(b)))

#define calc_uint3(a, b, c, op, width1, width2) \
    ((concat3(uint, width2, _t)) \
    ((concat3(uint, width2, _t))(concat3(uint, width1, _t))(a) op \
     (concat3(uint, width2, _t))(concat3(uint, width1, _t))(b) op \
     (concat3(uint, width2, _t))(concat3(uint, width1, _t))(c)))

#define calc_int3(a, b, c, op, width1, width2) \
    ((concat3(int,  width2, _t)) \
    ((concat3(int,  width2, _t))(concat3(int,  width1, _t))(a) op \
     (concat3(int,  width2, _t))(concat3(int,  width1, _t))(b) op \
     (concat3(int,  width2, _t))(concat3(int,  width1, _t))(c)))

#define get_cf(a, b, op, width, d_width) \
    (calc_uint(a, b, op, width, width) != calc_uint(a, b, op, width, d_width))

#define get_of(a, b, op, width, d_width) \
    (calc_int (a, b, op, width, width) != calc_int (a, b, op, width, d_width))

#define get_cf3(a, b, c, op, width, d_width) \
    (calc_uint3(a, b, c, op, width, width) != calc_uint3(a, b, c, op, width, d_width))

#define get_of3(a, b, c, op, width, d_width) \
    (calc_int3 (a, b, c, op, width, width) != calc_int3 (a, b, c, op, width, d_width))

#define get_sf(r, width) ((concat3(int,  width, _t))(r) <  0)
#define get_zf(r, width) ((concat3(uint, width, _t))(r) == 0)

union uint8_m {
    uint8_t val;
    struct {
        uint8_t bit0: 1; uint8_t bit1: 1; uint8_t bit2: 1; uint8_t bit3: 1;
        uint8_t bit4: 1; uint8_t bit5: 1; uint8_t bit6: 1; uint8_t bit7: 1;
    };
};

static inline int get_pf(uint8_t r) {
    union uint8_m x;
    x.val = r;
    return !(x.bit0 ^ x.bit1 ^ x.bit2 ^ x.bit3 ^ x.bit4 ^ x.bit5 ^ x.bit6 ^ x.bit7);
}

#define set_sf_zf_pf(r, width) \
    do { \
        cpu.eflags.SF = get_sf(r, width); \
        cpu.eflags.ZF = get_zf(r, width); \
        cpu.eflags.PF = get_pf((uint8_t)r);} while (0)

#define set_cf_of(a, b, op, width, d_width) \
    do { \
        cpu.eflags.CF = get_cf(a, b, op, width, d_width); \
        cpu.eflags.OF = get_of(a, b, op, width, d_width);} while (0)

#define set_cf_of3(a, b, c, op, width, d_width) \
    do { \
        cpu.eflags.CF = get_cf3(a, b, c, op, width, d_width); \
        cpu.eflags.OF = get_of3(a, b, c, op, width, d_width);} while (0)

#define case_entry_2(a, b, r, op, width, d_width) \
    case width: (r) = (uint32_t)calc_uint(a, b, op, width, width); \
                set_cf_of(a, b, op, width, d_width); \
                set_sf_zf_pf(r, width); break;

#define case_entry_i(a, b, r, op, width, d_width) \
    case width: (r) = (uint32_t)(concat3(uint, width, _t)) \
                            calc_int(a, b, op, width, width); \
                set_cf_of(a, b, op, width, d_width); \
                set_sf_zf_pf(r, width); break;

#define case_entry_3(a, b, c, r, op, width, d_width) \
    case width: (r) = (uint32_t)calc_uint3(a, b, c, op, width, width); \
                set_cf_of3(a, b, c, op, width, d_width); \
                set_sf_zf_pf(r, width); break;

#define case_entry_umul(a, b, r, op, width, d_width) \
    case width: (r) = (uint64_t)calc_uint(a, b, op, d_width, d_width); \
                cpu.eflags.CF = cpu.eflags.OF = \
                    get_cf(a, b, op, width, d_width); break;

#define case_entry_imul(a, b, r, op, width, d_width) \
    case width: (r) = (int64_t)calc_int(a, b, op, d_width, d_width); \
                cpu.eflags.CF = cpu.eflags.OF = \
                    get_of(a, b, op, width, d_width); break;

#define case_entry_udiv(a, b, r, op, width, d_width) \
    case width: (r) = (uint32_t)calc_uint(a, b, op, d_width, d_width); break;

#define case_entry_idiv(a, b, r, op, width, d_width) \
    case width: (r) = (int32_t)calc_int(a, b, op, d_width, d_width); break;

#define case_all(suffix, data_size, ...) \
    do { switch (data_size) { \
        concat(case_entry_, suffix)(__VA_ARGS__,  8, 16) \
        concat(case_entry_, suffix)(__VA_ARGS__, 16, 32) \
        concat(case_entry_, suffix)(__VA_ARGS__, 32, 64) \
        default: assert(0); \
    } } while (0)

uint32_t alu_add(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_add(src, dest, data_size);
#else
	uint32_t r;
    case_all(2, data_size, dest, src, r, +);
	return r;
#endif
}

uint32_t alu_adc(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_adc(src, dest, data_size);
#else
	uint32_t r, c = cpu.eflags.CF;
    case_all(3, data_size, dest, src, c, r, +);
	return r;
#endif
}

uint32_t alu_sub(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_sub(src, dest, data_size);
#else
	uint32_t r;
    case_all(2, data_size, dest, src, r, -);
	return r;
#endif
}

uint32_t alu_sbb(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_sbb(src, dest, data_size);
#else
	uint32_t r, c = cpu.eflags.CF;
    case_all(3, data_size, dest, src, c, r, -);
	return r;
#endif
}

uint64_t alu_mul(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_mul(src, dest, data_size);
#else
	uint64_t r;
    case_all(umul, data_size, dest, src, r, *);
	return r;
#endif
}

int64_t alu_imul(int32_t src, int32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_imul(src, dest, data_size);
#else
	int64_t r;
    case_all(imul, data_size, dest, src, r, *);
	return r;
#endif
}

// need to implement alu_mod before testing
uint32_t alu_div(uint64_t src, uint64_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_div(src, dest, data_size);
#else
	uint32_t r;
    case_all(udiv, data_size, dest, src, r, /);
	return r;
#endif
}

// need to implement alu_imod before testing
int32_t alu_idiv(int64_t src, int64_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_idiv(src, dest, data_size);
#else
	int32_t r;
    case_all(idiv, data_size, dest, src, r, /);
	return r;
#endif
}

uint32_t alu_mod(uint64_t src, uint64_t dest)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_mod(src, dest);
#else
	return (uint32_t)(dest % src);
#endif
}

int32_t alu_imod(int64_t src, int64_t dest)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_imod(src, dest);
#else
	return (int32_t)(dest % src);
#endif
}

uint32_t alu_and(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_and(src, dest, data_size);
#else
	uint32_t r;
    case_all(2, data_size, dest, src, r, &);
	return r;
#endif
}

uint32_t alu_xor(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_xor(src, dest, data_size);
#else
	uint32_t r;
    case_all(2, data_size, dest, src, r, ^);
	return r;
#endif
}

uint32_t alu_or(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_or(src, dest, data_size);
#else
	uint32_t r;
    case_all(2, data_size, dest, src, r, |);
	return r;
#endif
}

uint32_t alu_shl(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_shl(src, dest, data_size);
#else
	uint32_t r;
    case_all(2, data_size, dest, src & 0x1f, r, <<);
    cpu.eflags.CF = !!((dest) & (1 << (data_size - (src & 0x1f))));
	return r;
#endif
}

uint32_t alu_shr(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_shr(src, dest, data_size);
#else
	uint32_t r;
    case_all(2, data_size, dest, src & 0x1f, r, >>);
    cpu.eflags.CF = !!((dest) & (1 << ((src & 0x1f) - 1)));
	return r;
#endif
}

uint32_t alu_sar(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_sar(src, dest, data_size);
#else
	uint32_t r;
    case_all(i, data_size, dest, src & 0x1f, r, >>);
    cpu.eflags.CF = !!((dest) & (1 << ((src & 0x1f) - 1)));
	return r;
#endif
}

uint32_t alu_sal(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_sal(src, dest, data_size);
#else
	return alu_shl(src, dest, data_size);
#endif
}
