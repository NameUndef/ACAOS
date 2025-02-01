#include <stdlib.h>
#include <stdio.h>

struct MemoryControlBlock {
    size_t offset;
    size_t size;
    struct MemoryControlBlock* next;
};

void* mem_alloc(void* mem, size_t mem_size, size_t size)
{
    struct MemoryControlBlock* const root 
    = (struct MemoryControlBlock*) mem;
    
    struct MemoryControlBlock* mcb_cur = root;
    
    struct MemoryControlBlock* mcb_new = NULL;
    struct MemoryControlBlock* mcb_last = root;
    
    while (mcb_cur->next != NULL) {
        // for each find nearest mcb at left and check capacity
        
        struct MemoryControlBlock* mcb_nearest 
            = root;
        struct MemoryControlBlock* mcb_comp 
            = root;
            
        do {
            if (mcb_comp->offset > mcb_cur->offset
                && (mcb_comp->offset < mcb_nearest->offset
                || mcb_nearest == root))
                mcb_nearest = mcb_comp;

            mcb_last = mcb_comp;                
            mcb_comp = mcb_comp->next;
        } while (mcb_comp != NULL);
        
        if (mcb_nearest != root // have nearest
            && mcb_nearest->offset 
                - mcb_cur->offset
                - sizeof(struct MemoryControlBlock)
                - mcb_cur->size
                >= size + sizeof(struct MemoryControlBlock)) {
            mcb_new 
                = mcb_cur 
                + sizeof(struct MemoryControlBlock)
                + mcb_cur->size;
            mcb_new->offset 
                = mcb_cur->offset
                + sizeof(struct MemoryControlBlock)
                + mcb_cur->size;
            break;
        }
        
        mcb_cur = mcb_cur->next;
    }
    
    if (mcb_new == NULL) {
        // find most far at root and check capacity
        
        mcb_cur = root;
        struct MemoryControlBlock* mcb_far_to_root = root;
            
        while (mcb_cur != NULL) {
            if (mcb_cur->offset > mcb_far_to_root->offset)
                mcb_far_to_root = mcb_cur;
            mcb_cur = mcb_cur->next;
        }
        
        if (size + sizeof(struct MemoryControlBlock) 
            <= mem_size 
                - mcb_far_to_root->offset
                - sizeof(struct MemoryControlBlock)
                - mcb_far_to_root->size) {
            mcb_new 
                = mcb_far_to_root
                + sizeof(struct MemoryControlBlock)
                + mcb_far_to_root->size;
            mcb_new->offset
                = mcb_far_to_root->offset
                + sizeof(struct MemoryControlBlock)
                + mcb_far_to_root->size;
        }
    }
    
    if (mcb_new != NULL) {
        mcb_new->size = size;
        mcb_new->next = NULL;
        mcb_last->next = mcb_new;
        return (void*)( 
            (char*)mcb_new + sizeof(struct MemoryControlBlock));
    } else {
        return NULL;
    }
}

void mem_free(void* mem, void* ptr)
{
     struct MemoryControlBlock* const root 
         = (struct MemoryControlBlock*) mem;
    
    struct MemoryControlBlock* mcb_free 
        = (struct MemoryControlBlock*)(
    (char*)ptr - sizeof(struct MemoryControlBlock));
    
    struct MemoryControlBlock* mcb_prev = root;
    struct MemoryControlBlock* mcb_cur = root;
    
    while (mcb_cur->next != NULL) {
        mcb_prev = mcb_cur;
        mcb_cur = mcb_cur->next;
        
        if (mcb_cur == mcb_free) {
            mcb_prev->next = mcb_cur->next;
            break;
        }
    }
}

void mem_init(void* mem)
{
    struct MemoryControlBlock* const root 
        = (struct MemoryControlBlock*) mem;
        
    root->offset = 0;
    root->size = 0;
    root->next = NULL;
}

void mem_clear(void* mem)
{
    struct MemoryControlBlock* const root 
        = (struct MemoryControlBlock*) mem;
        
    root->next = NULL;
}

void mem_log(void* mem)
{
     struct MemoryControlBlock* const root 
         = (struct MemoryControlBlock*) mem;
         
     struct MemoryControlBlock* mcb_cur = root;
     size_t i = 0;
     size_t alloc_size = 0;
     size_t mcb_size = 0;
     do {
         printf("%d, offset: %d, size: %d\n",
         i, mcb_cur->offset, mcb_cur->size);
         alloc_size += mcb_cur->size;
         mcb_size += sizeof(struct MemoryControlBlock);
         mcb_cur = mcb_cur->next;
         i++;
     } while (mcb_cur != NULL);
     
     printf("total count: "
     "%d, alloc size: %d, mcb size: %d, "
     "total size: %d\n", 
     i, 
     alloc_size,
     mcb_size,
     alloc_size + mcb_size);
     fflush(stdout);
}

int main()
{
    const size_t MEM_SIZE = 1024;
    
    void* mem = malloc(MEM_SIZE);
    if (mem == NULL)
        return -1;
        
    mem_init(mem);
    
    mem_alloc(mem, MEM_SIZE, 64);
    void* block = mem_alloc(mem, MEM_SIZE, 64);
    void* block_2 = mem_alloc(mem, MEM_SIZE, 64);
    mem_alloc(mem, MEM_SIZE, 64);
    mem_free(mem, block_2);
    mem_alloc(mem, MEM_SIZE, 64);
    mem_free(mem, block);
    mem_alloc(mem, MEM_SIZE, 64);
    mem_log(mem);
    
    mem_clear(mem);
    free(mem);
    return 0;
}
