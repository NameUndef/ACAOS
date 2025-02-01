#include <stdio.h>
#include <stdlib.h>

/* Страничная модель памяти
 * В регистре CR3 (PDBR) указывается адрес директории страниц PageDirectory
 */

struct LinearAddress {
    size_t dir_entry:   10;     // table number
    size_t table_entry: 10;     // page number
    size_t page_offset: 12;     // byte number
};

struct page_memory_info {
    void* mem;
    size_t mem_size;
    size_t page_size;
    size_t pages_per_table;
    size_t system_mem_size;
};

int init_page_memory(void* mem, size_t mem_size, size_t page_size)
{
    size_t entry_size = sizeof(size_t);

    if (!mem || !mem_size || !page_size || mem_size % page_size || page_size % entry_size)
        return 1;

    size_t phys_page_info_size_bits = 1;    // flags: on RAM
    size_t phys_pages_info_pages_num = ((mem_size / page_size) * phys_page_info_size_bits) / 8 / page_size + 1;

    size_t pages_per_table = page_size / entry_size;
    size_t system_mem_size = (2 + phys_pages_info_pages_num + pages_per_table) * page_size;  // page directory, page tables, phys pages info, info

    if (system_mem_size >= mem_size)
        return 2;

    void** page_table = (void**) mem + pages_per_table;
    for (void** page_dir_entry = mem; page_dir_entry < (void**) mem + pages_per_table; page_dir_entry++) {

        for (size_t i = 0; i < pages_per_table; i++)
            page_table[i] = NULL;

        *page_dir_entry = page_table;
        page_table += pages_per_table;
    }

    char* phys_pages_info = (char*) mem + system_mem_size - (phys_pages_info_pages_num * page_size);
    for (size_t i = 0; i < (mem_size / page_size) * phys_page_info_size_bits; i++) {

        size_t div = i / 8;
        size_t mod = i % 8;
        phys_pages_info[div] = (!!mod) * phys_pages_info[div] | 1 << (mod);
    }

    size_t* info = (size_t*)((char*) mem + system_mem_size - (phys_pages_info_pages_num + 1) * page_size);
    info[0] = mem_size;
    info[1] = page_size;
    info[2] = pages_per_table;
    info[3] = system_mem_size;
    info[4] = 0;
    info[5] = 0;
    info[6] = 0;
    info[7] = 0;

    ((size_t***)mem)[0][0] = info;
    for (size_t i = 0; i < phys_pages_info_pages_num; i++)
        ((size_t***)mem)[0][1 + i] = (size_t*)phys_pages_info + i * pages_per_table;

    return 0;
}

int main()
{
    char mem[1024];
    int res = init_page_memory(mem, sizeof mem / sizeof mem[0], 64);

    if (res) 
        printf("page memory init error: %d\n", res);

    
    return 0;
}
