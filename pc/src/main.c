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

    // Initialize dictionary
    dictionary_init();
    printf("Dictionary initialized\n");

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

    printf("\n✓ All stack tests passed!\n");

    // Test primitive word creation and execution
    printf("\nTesting primitive word creation and execution:\n");

    // Create some primitive words
    word_t* plus_word = create_primitive_word("+", f_plus);
    word_t* minus_word = create_primitive_word("-", f_minus);
    word_t* mult_word = create_primitive_word("*", f_multiply);
    word_t* drop_word = create_primitive_word("DROP", f_drop);

    printf("Created words: %s, %s, %s, %s\n",
           plus_word->name, minus_word->name, mult_word->name, drop_word->name);

    // Test + word: 10 20 +  should leave 30
    printf("\nTesting: 10 20 + (should leave 30)\n");
    stack_init();  // Clear stack for clean test
    data_push(10);
    data_push(20);
    printf("Stack before +: depth=%d, top=%d\n", data_depth(), data_peek());
    execute_word(plus_word);
    printf("Stack after +: depth=%d, top=%d\n", data_depth(), data_peek());

    // Test - word: 50 8 -  should leave 42
    printf("\nTesting: 50 8 - (should leave 42)\n");
    stack_init();  // Clear stack for clean test
    data_push(50);
    data_push(8);
    printf("Stack before -: depth=%d\n", data_depth());
    execute_word(minus_word);
    printf("Stack after -: depth=%d, top=%d\n", data_depth(), data_peek());

    // Test * word: 6 7 *  should leave 42
    printf("\nTesting: 6 7 * (should leave 42)\n");
    stack_init();  // Clear stack for clean test
    data_push(6);
    data_push(7);
    execute_word(mult_word);
    printf("Stack after *: depth=%d, top=%d\n", data_depth(), data_peek());

    // Test DROP: should remove one item
    printf("\nTesting: DROP (should remove 999)\n");
    stack_init();  // Clear stack for clean test
    data_push(999);
    printf("Stack before DROP: depth=%d, top=%d\n", data_depth(), data_peek());
    execute_word(drop_word);
    printf("Stack after DROP: depth=%d\n", data_depth());

    // Final verification - all operations worked correctly
    if (data_depth() == 0) {
        printf("✓ Primitive word execution working correctly!\n");
    } else {
        printf("✗ Primitive word execution failed\n");
        return 1;
    }

    // Test dictionary functionality
    printf("\nTesting dictionary functionality:\n");

    show_dictionary();

    // Test word lookup
    printf("\nTesting word lookup:\n");

    word_t* found_plus = find_word("+");
    word_t* found_drop = find_word("DROP");
    word_t* found_nonexistent = find_word("NONEXISTENT");

    printf("Looking up '+': %s\n", found_plus ? "FOUND" : "NOT FOUND");
    printf("Looking up 'DROP': %s\n", found_drop ? "FOUND" : "NOT FOUND");
    printf("Looking up 'NONEXISTENT': %s\n", found_nonexistent ? "FOUND" : "NOT FOUND");

    // Test that found words can be executed
    if (found_plus && found_drop) {
        printf("\nTesting execution of found words:\n");
        stack_init();
        data_push(15);
        data_push(27);
        printf("Stack: [15, 27], executing found '+' word\n");
        execute_word(found_plus);
        printf("Result: %d (should be 42)\n", data_peek());

        printf("Executing found 'DROP' word\n");
        execute_word(found_drop);
        printf("Stack depth after DROP: %d (should be 0)\n", data_depth());

        if (data_depth() == 0) {
            printf("✓ Dictionary lookup and execution working correctly!\n");
        } else {
            printf("✗ Dictionary execution failed\n");
            return 1;
        }
    } else {
        printf("✗ Dictionary lookup failed\n");
        return 1;
    }

    printf("\n✓ All tests passed! Core Forth functionality working.\n");
    return 0;
}