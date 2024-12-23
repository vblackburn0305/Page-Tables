#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "config.h"
#include <string.h>

#define VPN_BIT_SIZE (POBITS - 3)

size_t ptbr = 0;



void *allocate_page() {
    void *page;
    size_t alignment = 1 << POBITS;
    if (posix_memalign(&page, alignment, 1 << POBITS) != 0) {
        perror("posix_memalign failed");
        exit(1);
    }
    memset(page, 0, 1 << POBITS);
    return page;
}

size_t find_vpn(size_t va, int level) {
    int vpn_level = LEVELS - 1 - level; // calculate VPN level
    int shift = POBITS + vpn_level * VPN_BIT_SIZE; // calculate shift
    size_t vpn_shift = (va >> shift); // shift the virtual address
    size_t vpn_mask = (1 << VPN_BIT_SIZE) - 1; // create mask
    return vpn_shift & vpn_mask; // mask and return VPN
}

void page_allocate(size_t va) {
    if (ptbr == 0) {
        ptbr = (size_t)allocate_page(); // Allocate page if doesn't exist
    }

    size_t *current_ptbr = (size_t *)ptbr;

    for (int level = 0; level < LEVELS; ++level) {
        size_t vpn = find_vpn(va, level);

        if ((current_ptbr[vpn] & 1) == 0) {
            if (level == LEVELS - 1) {
                size_t physical_page = (size_t)allocate_page();
                current_ptbr[vpn] = physical_page | 1;
            } else {
                size_t next_table = (size_t)allocate_page();
                current_ptbr[vpn] = next_table | 1; 
                current_ptbr = (size_t *)next_table; 
            }
        } else {
            size_t mask = ~((1 << POBITS) - 1);
            current_ptbr = (size_t *)(current_ptbr[vpn] & mask);
        }
    }
}

size_t translate(size_t va) {
    if (ptbr == 0) {
        return ~(size_t)0; 
    }

    size_t *current_ptbr = (size_t *)ptbr; 
    size_t page_offset = va & ((1 << POBITS) - 1); 

    for (int level = 0; level < LEVELS; ++level) {
        size_t vpn = find_vpn(va, level);

        size_t entry = current_ptbr[vpn];

        if ((entry & 1) == 0) {
            return ~(size_t)0; 
        }

        if (level == LEVELS - 1) {
            size_t physical_page_number = entry >> POBITS;
            return (physical_page_number << POBITS) | page_offset;
        }

        size_t mask = ~((1 << POBITS) - 1);
        current_ptbr = (size_t *)(current_ptbr[vpn] & mask);
    }

    return ~(size_t)0; 
}
