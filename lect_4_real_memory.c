#include <stdio.h>
#include <stdlib.h>

struct LogicalAddress {
    size_t segment;
    size_t offset;
};

typedef void* PhysicalAddress;

PhysicalAddress get_pysical_address(const struct LogicalAddress* const logical_address, size_t mem_size)
{
    if (logical_address->segment >= mem_size >> 4 || logical_address->offset >= 0x10000)
        return NULL;

    size_t result = (logical_address->segment << 4) + logical_address->offset;

    return (void*) result;
}

int main()
{
    struct LogicalAddress logical_addresses[] = {
        {0, 0x0},
        {0, 0x100},
        {1, 0x80},
        {2, 0xFF},
        {3, 0x200}
    };

    for (size_t i = 0; i < sizeof(logical_addresses) / sizeof(logical_addresses[0]); i++) {
        printf("%zu: segment: %zu, offset: %zu, physical address: 0x%p\n",
            i,
            logical_addresses[i].segment,
            logical_addresses[i].offset,
            get_pysical_address(&logical_addresses[i], 2048));
    }

    return 0;
}
