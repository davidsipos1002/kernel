#include <segment/tss.h>

#include <memory/manipulate.h>

void tss_fill(cpu_state *state)
{
    task_state_segment *t = (task_state_segment *) state->tss;
    memset(t, 0, sizeof(task_state_segment)); 
    t->io_map_base = (uint32_t) state->io_map_base << 16;
    uint64_t j = 0;
    for (uint64_t i = 0; i < state->ist_count; i++)
    {
        t->ist[j++] = state->ist[i] & 0xFFFFFFFF;
        t->ist[j++] = (state->ist[i] >> 32) & 0xFFFFFFFF;
    }
}