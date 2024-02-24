#include <segment/gdt.h>

#include <gcc/utils.h>
#include <memory/manipulate.h>

extern void __set_gdt(void *gdt, uint16_t limit);
extern void __set_segment_regs(void);

static inline void ALWAYS_INLINE segment_fill_null(segment_descriptor *desc)
{
    desc->limit_0 = 0;
    desc->base_0 = 0;
    desc->base_1 = 0;
    desc->type = 0;
    desc->s = 0;
    desc->dpl = 0;
    desc->p = 0;
    desc->limit_1 = 0;
    desc->avl = 0;
    desc->l = 0;
    desc->db = 0;
    desc->g = 0;
    desc->base_2 = 0;
}

static inline void ALWAYS_INLINE segment_fill_kernel_code(segment_descriptor *desc)
{
    desc->limit_0 = 0xFFFF;
    desc->base_0 = 0;
    desc->base_1 = 0;
    desc->type = 0xA;
    desc->s = 1;
    desc->dpl = 0;
    desc->p = 1;
    desc->limit_1 = 0xF;
    desc->avl = 0;
    desc->l = 1;
    desc->db = 0;
    desc->g = 1;
    desc->base_2 = 0;
}

static inline void ALWAYS_INLINE segment_fill_kernel_data(segment_descriptor *desc)
{
    desc->limit_0 = 0xFFFF;
    desc->base_0 = 0;
    desc->base_1 = 0;
    desc->type = 0x2;
    desc->s = 1;
    desc->dpl = 0;
    desc->p = 1;
    desc->limit_1 = 0xF;
    desc->avl = 0;
    desc->l = 0;
    desc->db = 1;
    desc->g = 1;
    desc->base_2 = 0;
}

static inline void ALWAYS_INLINE segment_fill_user_code(segment_descriptor *desc)
{
    desc->limit_0 = 0xFFFF;
    desc->base_0 = 0;
    desc->base_1 = 0;
    desc->type = 0xA;
    desc->s = 1;
    desc->dpl = 3;
    desc->p = 1;
    desc->limit_1 = 0xF;
    desc->avl = 0;
    desc->l = 1;
    desc->db = 0;
    desc->g = 1;
    desc->base_2 = 0;
}

static inline void ALWAYS_INLINE segment_fill_user_data(segment_descriptor *desc)
{
    desc->limit_0 = 0xFFFF;
    desc->base_0 = 0;
    desc->base_1 = 0;
    desc->type = 0x2;
    desc->s = 1;
    desc->dpl = 3;
    desc->p = 1;
    desc->limit_1 = 0xF;
    desc->avl = 0;
    desc->l = 0;
    desc->db = 1;
    desc->g = 1;
    desc->base_2 = 0;
}

void segment_fill_gdt(cpu_state *state)
{
    segment_descriptor *desc = (segment_descriptor *) state->gdt;
    segment_fill_null(desc++);
    segment_fill_kernel_code(desc++);
    segment_fill_kernel_data(desc++);
    segment_fill_user_code(desc++);
    segment_fill_user_data(desc++);
}

void segment_set_gdt(cpu_state *state)
{
    __set_gdt(state->gdt, 55);
    __set_segment_regs();
}

void segment_fill_tss(cpu_state *state)
{
    uint8_t *p = (uint8_t *) state->gdt;
    p += 5 * sizeof(segment_descriptor);
    ldt64_descriptor *desc = (ldt64_descriptor *) p;
    uint64_t base = (uint64_t) state->tss;
    desc->limit_0 = 104;
    desc->base_0 = base & 0xFFFF;
    base >>= 16;
    desc->base_1 = base & 0xFF;
    base >>= 8;
    desc->type = 9;
    desc->s = 0;
    desc->dpl = 0;
    desc->p = 1;
    desc->limit_1 = 0;
    desc->avl = 0;
    desc->l = 0;
    desc->db = 0;
    desc->g = 0;
    desc->base_2 = base & 0xFF;
    base >>= 8;
    desc->base_3 = base;
    desc->reserved = 0;
}