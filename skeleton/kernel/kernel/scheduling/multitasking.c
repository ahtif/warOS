#include <linkedlist.h>
#include <queue.h>
#include <kernel/multitasking.h>
#include <arch/i386/system.h>
#include <kernel/process.h>
#include <kernel/kheap.h>
#include <arch/i386/paging.h>

extern void switch_task(struct regs* current, struct regs* next);

queue_t* process_queue = NULL, * unblocked_queue = NULL, * terminated_queue = NULL;
linkedlist_t* sleeping_processes = NULL;
process_t* current_process = NULL;
uint32_t tick_counter = 0;
uint32_t milliseconds_counter = 0;

process_t* init_process = NULL, * cleaner_process = NULL;

void on_tick(struct regs* r) {
    tick_counter++;
    send_eoi(r->int_no);
    if(tick_counter >= MULTITASKING_TICKS_PER_SLICE) {
        multitasking_yield();
        tick_counter = 0;
    }
    milliseconds_counter += MULTITASKING_TICK_TIME;
}

int compare_sleeping_processes(void* p1, void* p2) {
    return ((sleeping_process_t*)p1)->wake_time - ((sleeping_process_t*)p2)->wake_time;
}

void cleaner() {
    while (true) {
        if(queue_size(terminated_queue) > 0) {
            process_t* proc = queue_dequeue(terminated_queue);
            process_free(proc);
            kfree(proc);
            multitasking_sleep(100);
        }
    }
}

void multitasking_exit_process() {
    current_process->state = TERMINATED;
    queue_enqueue(terminated_queue, current_process);
    multitasking_yield();
}

void multitasking_init(void* kernel_stack, uint32_t kernel_stack_size) {
    process_queue = queue_create();
    sleeping_processes = list_create();
    unblocked_queue = queue_create();
    terminated_queue = queue_create();

    // Create the init process
    init_process = current_process = process_create("init", kmalloc(sizeof(struct regs)), kernel_stack, kernel_stack_size, KERNEL);
    // The init process just carries on from when it was created so we don't set an entry point
    multitasking_init_process_state(current_process, 0);

    // Create the cleaner process
    cleaner_process = process_create("cleaner", kmalloc(sizeof(struct regs)), kmalloc(1024), 1024, KERNEL);
    multitasking_init_process_state(cleaner_process, cleaner);
    multitasking_schedule(cleaner_process);

    irq_install_handler(0, on_tick);
}

void multitasking_init_process_state(process_t *process, void (*entry_function)(void)) {
    struct regs* state = process->cpu_state;
    uint32_t* process_kernel_stack = (uint32_t*)process->kernel_stack;
    state->ebp = (uint32_t)process_kernel_stack;
    state->esp = (uint32_t)process_kernel_stack + process->kernel_stack_size - 4*5;
    state->edi = 0;
    state->ebx = 0;
    state->esi = 0;
    process_kernel_stack[process->kernel_stack_size/4 - 5] = state->ebp;
    process_kernel_stack[process->kernel_stack_size/4 - 4] = state->edi;
    process_kernel_stack[process->kernel_stack_size/4 - 3] = state->esi;
    process_kernel_stack[process->kernel_stack_size/4 - 2] = state->ebx;
    process_kernel_stack[process->kernel_stack_size/4 - 1] = (uint32_t)entry_function;
    process_kernel_stack[process->kernel_stack_size/4] = (uint32_t)multitasking_exit_process;
}

void multitasking_schedule(process_t *process) {
    if(process->state != TERMINATED) queue_enqueue(process_queue, process);
}

bool process_can_run(process_t* proc) {
    return proc->state != SLEEPING && proc->state != BLOCKED && proc->state != TERMINATED;
}

process_t *get_next_from_queue(queue_t *queue) {
    process_t* head = queue_dequeue(queue);
    // Cut off processes that aren't ready to run yet
    while (!process_can_run(head) && queue_size(queue) > 0) head = queue_dequeue(queue);
    return head;
}

void switch_to_next() {
    process_t* next_process = NULL;
    // Check the list of sleeping processes
    if(list_size(sleeping_processes) > 0) {
        //log_debug("More than 0 sleeping processes\n");
        sleeping_process_t* sp = list_get(sleeping_processes, 0);
        // If the first process in the list is ready to be woken up
        if(sp->wake_time <= milliseconds_counter) {
            //log_debug("Ready to wake first one\n");
            next_process = sp->process;
            next_process->state = READY;
            // Remove it from the list and free the wrapper struct
            list_remove(sleeping_processes, 0);
            kfree(sp);
        }
    }
    // Check the queue of unblocked processes
    if(!next_process && queue_size(unblocked_queue) > 0) next_process = get_next_from_queue(unblocked_queue);
    // Check the queue of ready processes
    if(!next_process && queue_size(process_queue) > 0) next_process = get_next_from_queue(process_queue);
    if(next_process && process_can_run(next_process)) {
        next_process->state = RUNNING;
        process_t *tmp = current_process;
        current_process = next_process;
        switch_task(tmp->cpu_state, current_process->cpu_state);
    }
}

void multitasking_yield() {
    multitasking_schedule(current_process);
    current_process->state = READY;
    switch_to_next();
}

void multitasking_sleep(uint32_t milliseconds) {
    sleeping_process_t* sp = kmalloc(sizeof(sleeping_process_t));
    if(sp) {
        sp->process = current_process;
        sp->wake_time = milliseconds_counter + milliseconds;
        sorted_list_add(sleeping_processes, sp, compare_sleeping_processes);
        // Setting it to sleeping state will get it cut from the process queue if it's ever at the front
        current_process->state = SLEEPING;
        switch_to_next();
    }
}

void multitasking_block(process_t *process) {
    process->state = BLOCKED;
    if(process->state == RUNNING || current_process == process) {
        switch_to_next();
    }
}

void multitasking_unblock(process_t *process) {
    if(process->state != TERMINATED) {
        process->state = READY;
        multitasking_schedule(process);
    }
}
