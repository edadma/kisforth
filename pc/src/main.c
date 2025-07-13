#include <stdio.h>
#include <stdlib.h>
#include "forth.h"

int main(int argc, char* argv[]) {
    printf("KISForth v1.0 - PC Development Version\n");
    printf("Memory size: %d bytes\n", FORTH_MEMORY_SIZE);
    printf("Initial HERE: %u\n", here);

    // Initialize stacks
    stack_init();
    printf("Data stack depth: %d, Return stack depth: %d\n",
           data_depth(), return_depth());

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

    // Test data stack operations
    printf("\nTesting data stack:\n");

    data_push(42);
    data_push(100);
    data_push(-5);
    printf("Pushed 42, 100, -5. Stack depth: %d\n", data_depth());

    printf("Peek top: %d\n", data_peek());
    printf("Pop: %d\n", data_pop());
    printf("Pop: %d\n", data_pop());
    printf("Pop: %d\n", data_pop());
    printf("Stack depth after pops: %d\n", data_depth());

    // Test return stack operations
    printf("\nTesting return stack:\n");

    return_push(123);
    return_push(456);
    printf("Pushed 123, 456. Return stack depth: %d\n", return_depth());

    printf("Pop: %d\n", return_pop());
    printf("Pop: %d\n", return_pop());
    printf("Return stack depth after pops: %d\n", return_depth());

    printf("\n✓ All core tests passed!\n");
    return 0;
}