
#include <stdbool.h>
#include <time.h>

/* scheduler:
 *  work with threads and processes
 *  1. get process/thread context from processor and save
 *  2. determine which process/thread will be launched next
 *  3. get the saved context of another process/thread
 *  4. write context to processor
 *  5. give it control to thread
 */

/* process:
 *  container of resources:
 *  1. address space (memory)
 *  2. threads
 *  3. system objects (opened files, semaphores)
 */

/* thread:
 * thread of execution of
 * instructions in process memory.
 * unit of scheduling
 * states:
 * 1. active (on processor)
 * 2. ready
 * 3. waiting
 */

/* quantum:
 * time slice of process
 */

/* user space, kernel space (safe mode) 
 * different address space
 * different command sets (user space have reduced command set)
 * commication between user and OS is system calls
 */

#include <stdio.h>
#include <time.h>

enum ThreadStates {
    ACTIVE,
    READY,
    WAITING
};

struct Registers {
    int a;
    int b;
    int c;
};
typedef int software_interrupt;
typedef int hardware_interrupt;

software_interrupt thread(struct Registers* regs)
{
    regs->a++;
    
    if (regs->a % regs->b) {
        regs->c++;
        return 1;
    }
    
    return 0;
}

void thread_switch(
    size_t* cur_thread, 
    const size_t THREADS_NUM, 
    int* thread_states, 
    struct Registers* thread_contexts, 
    struct Registers* regs)
{
    size_t next_ready_thread = *cur_thread;
    do {
        next_ready_thread++;
        if (next_ready_thread > THREADS_NUM)
            next_ready_thread = 0;
        if (next_ready_thread < THREADS_NUM && thread_states[next_ready_thread] == READY)
            break;

    } while (next_ready_thread != *cur_thread);

    if (next_ready_thread == *cur_thread) {

        if (*cur_thread != THREADS_NUM && thread_states[*cur_thread] == WAITING)
            *cur_thread = THREADS_NUM;

        return;
    }

    if (*cur_thread != THREADS_NUM) {
        thread_contexts[*cur_thread] = *regs;
        if (thread_states[*cur_thread] == ACTIVE)
            thread_states[*cur_thread] = READY;
    }

    *regs = thread_contexts[next_ready_thread];
    thread_states[next_ready_thread] = ACTIVE;
    printf("%zu switch to thread %zu\n", *cur_thread, next_ready_thread);
    *cur_thread = next_ready_thread;
}

int main()
{    
    const size_t THREADS_NUM = 3;
    software_interrupt(*threads[])(struct Registers*) = {
        thread,
        thread,
        thread
    };
    
    int thread_states[] = {READY , READY, READY};

    struct Registers regs; 
    struct Registers thread_contexts[] = {{0, 10, 0}, {0, 20, 0}, {0, 200, 0}};
    
    size_t cur_thread = THREADS_NUM;
    
    clock_t quantum_target;
    const clock_t QUANTUM = 500;
    
    quantum_target = clock() + QUANTUM;

    const clock_t DEVICE_DELAY = 1000;
    const size_t DEVICE_BUFFER_DATA_CAPACITY = 1000;
    int device_buffer_data[DEVICE_BUFFER_DATA_CAPACITY];
    size_t device_buffer_threads[THREADS_NUM];
    clock_t device_delay_end = 0l;
    size_t device_buffer_data_size = 0;
    size_t device_buffer_threads_size = 0;

    software_interrupt si = 0;
    hardware_interrupt hi = 0;

    do {

        // CPU
        if (hi == 1) {    // hardware interrupt handler
            printf("hi: device\n");
            for (size_t i = 0; i < device_buffer_threads_size; i++)
                thread_states[device_buffer_threads[i]] = READY;
            hi = 0;

            if (device_buffer_threads_size == THREADS_NUM)
                thread_switch(&cur_thread, THREADS_NUM, thread_states, thread_contexts, &regs);
            device_buffer_data_size = 0;
            device_buffer_threads_size = 0;
        }

        if (clock() >= quantum_target) {    // also hardware interrupt handler
            printf("hi: timer\n");
            quantum_target = clock() + QUANTUM;
            thread_switch(&cur_thread, THREADS_NUM, thread_states, thread_contexts, &regs);
        }

        if (cur_thread != THREADS_NUM) {    // threads

            si = threads[cur_thread](&regs);
        }
        
        if (si == 1) {    // software interrput handler
            printf("si: driver\n");
            if (clock() >= device_delay_end && !device_buffer_data_size)    // driver
                device_delay_end = clock() + DEVICE_DELAY;
            device_buffer_data[device_buffer_data_size] = regs.c;
            device_buffer_data_size++;

            int need_add_thread = 1;
            for (size_t i = 0; i < device_buffer_threads_size; i++)
                if (device_buffer_threads[i] == cur_thread) {
                    need_add_thread = 0;
                    break;
                }

            if (need_add_thread) {
                device_buffer_threads[device_buffer_threads_size] = cur_thread;
                device_buffer_threads_size++;
                thread_states[cur_thread] = WAITING;
            }
            si = 0;
            thread_switch(&cur_thread, THREADS_NUM, thread_states, thread_contexts, &regs);
        }
        
        // Device
        if (device_buffer_data_size == DEVICE_BUFFER_DATA_CAPACITY || (device_buffer_data_size && clock() >= device_delay_end)) {
            printf("device\n");
            for (size_t i = 0; i < device_buffer_data_size; i++)
                printf("%d ", device_buffer_data[i]);
            printf("\n");
            hi = 1;
            
        }
        
    } while (1);
}
