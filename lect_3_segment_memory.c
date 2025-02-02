#include <stdio.h>
#include <stdlib.h>

struct SegmentDescriptor {
    void* base;
    size_t limit;
    
    // struct Access {
    //     char present: 1;
    //     char privelege_level: 2;
    //     char descriptor_type: 1;
    //     // type: 4 bits
    //     char accessed: 1;
    //     char writable_or_readable: 1;
    //     char expand_down_or_conforming: 1;
    //     char reserved_or_executable: 1;
    // } access_rights;
};

struct LogicalAddress {
    size_t segment;
    size_t offset;
};

typedef void* PhysicalAddress;  // linear address

PhysicalAddress get_pysical_address(
    const struct SegmentDescriptor* const segment_descriptor, 
    size_t segment_descriptor_size, 
    const struct LogicalAddress* const logical_address)
{
    if (logical_address->segment >= segment_descriptor_size || 
        logical_address->offset >= segment_descriptor[logical_address->segment].limit)
        return NULL;    // segfault

    return segment_descriptor[logical_address->segment].base
        + logical_address->offset;
}

void print_physical_address(const struct SegmentDescriptor* const segment_descriptors, 
    size_t segment_descriptors_size, 
    const struct LogicalAddress* const logical_addresses,
    size_t logical_addresses_size) 
{
    for (size_t i = 0; i < logical_addresses_size; i++) {

        PhysicalAddress result = get_pysical_address(
            segment_descriptors,
            segment_descriptors_size,
            &logical_addresses[i]);
            
        printf("%zu: segment: %zu, offset: %zu, physical address: 0x%p%s\n",
            i,
            logical_addresses[i].segment, 
            logical_addresses[i].offset,
            result,
            (result == NULL) ? " (segfault)" : "");
    }
}

int main()
{
    char mem[2048] = {0};

    struct SegmentDescriptor segment_descriptors[] = {
        {(void*)(mem), 0x100},
        {(void*)(mem + 0x100), 0x100},
        {(void*)(mem + 0x100 * 2), 0x300},
        {(void*)(mem + 0x100 * 5), 0x100},
    };
    struct LogicalAddress logical_addresses[] = {
        {0, 0x0},
        {0, 0x100},
        {1, 0x80},
        {2, 0xFF},
        {3, 0x200}
    };

    print_physical_address(
        segment_descriptors, 
        sizeof(segment_descriptors) / sizeof(segment_descriptors[0]), 
        logical_addresses, 
        sizeof(logical_addresses) / sizeof(logical_addresses[0]));
    
    return 0;
}
