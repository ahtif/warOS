#include <kernel/process.h>
#include <kernel/kheap.h>
#include <string.h>

process_t *process_create(char *name, struct regs *cpu_state, void *kernel_stack,
                          uint32_t kernel_stack_size, process_level_t level) {
    process_t* process = kmalloc(sizeof(process_t));
    process->name = strdup(name);
    process->cpu_state = cpu_state;
    process->kernel_stack = kernel_stack;
    process->kernel_stack_size = kernel_stack_size;
    process->level = level;
    process->state = READY;
    return process;
}

void process_free(process_t *process) {
    kfree(process->cpu_state);
    kfree(process->kernel_stack);
    kfree(process->name);
}
