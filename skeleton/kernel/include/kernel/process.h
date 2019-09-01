#ifndef _KERNEL_PROCESS_H
#define _KERNEL_PROCESS_H

#include <kernel/system.h>
#include <kernel/paging.h>

typedef enum process_level {
    USER, KERNEL
} process_level_t;

typedef enum process_state {
    RUNNING, READY, TERMINATED, SLEEPING, BLOCKED
} process_state_t;

typedef struct process {
    char* name;
    struct regs* cpu_state;
    page_directory_t* page_directory;
    void* kernel_stack;
    uint32_t kernel_stack_size;
    process_level_t level;
    process_state_t state;
} process_t;

process_t *process_create(char *name, struct regs *cpu_state, void *kernel_stack,
                          uint32_t kernel_stack_size, process_level_t level);

void process_free(process_t* process);

#endif //_KERNEL_PROCESS_H
