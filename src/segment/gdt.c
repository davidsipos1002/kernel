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

void segment_fill_gdt(void *gdt)
{
    segment_descriptor *desc = (segment_descriptor *) gdt;
    segment_fill_null(desc++);
    segment_fill_kernel_code(desc++);
    segment_fill_kernel_data(desc++);
    segment_fill_user_code(desc++);
    segment_fill_user_data(desc++);
}

void segment_set_gdt(void *gdt)
{
    __set_gdt(gdt, 55);
    __set_segment_regs();
}