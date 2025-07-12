#include <stdio.h>
#include <stdlib.h>
#include "forth.h"

int main(int argc, char* argv[]) {
    printf("KISForth v0.0.1 - PC Development Version\n");
    printf("Memory size: %d bytes\n", FORTH_MEMORY_SIZE);
    printf("Initial HERE: %u\n", here);

    // Test basic memory allocation
    printf("\nTesting memory allocation:\n");

    forth_addr_t addr1 = forth_allot(sizeof(word_t));
    printf("Allocated word_t at address: %u, HERE now: %u\n", addr1, here);

    forth_align();
    printf("After alignment, HERE: %u\n", here);

    forth_addr_t addr2 = forth_allot(10);  // Some arbitrary bytes
    printf("Allocated 10 bytes at address: %u, HERE now: %u\n", addr2, here);

    forth_align();
    printf("After alignment, HERE: %u\n", here);

    // Test address conversion
    word_t* word_ptr = addr_to_word(addr1);
    forth_addr_t converted_back = word_to_addr(word_ptr);
    printf("Address conversion test: %u -> %p -> %u\n", addr1, (void*)word_ptr, converted_back);

    if (addr1 == converted_back) {
        printf("✓ Address conversion working correctly\n");
    } else {
        printf("✗ Address conversion failed\n");
        return 1;
    }

    printf("\nCore memory management tests passed!\n");
  return 0;
}
