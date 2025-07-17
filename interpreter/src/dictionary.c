#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stddef.h>
#include "dictionary.h"
#include "forth.h"
#include "memory.h"
#include "stack.h"
#include "error.h"
#include "debug.h"
#include "tools.h"
#include "floating.h"

// Dictionary head points to the most recently defined word
word_t* dictionary_head = NULL;

// Initialize empty dictionary
void dictionary_init(void) {
    dictionary_head = NULL;
    create_all_primitives();
    create_builtin_definitions();

    #ifdef FORTH_ENABLE_TOOLS
    create_tools_primitives();
    create_tools_definitions();
    #endif

    #ifdef FORTH_ENABLE_FLOATING
    float_stack_init();
    create_floating_primitives();
    create_floating_definitions();
    #endif
}

// Link a word into the dictionary (at the head of the linked list)
void link_word(word_t* word) {
    word->link = dictionary_head;  // Point to previous head
    dictionary_head = word;        // Make this word the new head
}

// Add this helper function to dictionary.c
static int case_insensitive_strcmp(const char* a, const char* b) {
    while (*a && *b) {
        char ca = tolower(*a);
        char cb = tolower(*b);
        if (ca != cb) return ca - cb;
        a++; b++;
    }
    return tolower(*a) - tolower(*b);
}

// Find a word in the dictionary by name (case-sensitive search)
word_t* find_word(const char* name) {
    word_t* word = search_word(name);

    if (!word) error("Word not found: %s", name);
    return word;
}

word_t* search_word(const char* name) {
    word_t* current = dictionary_head;

    while (current != NULL) {
        if (case_insensitive_strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->link;  // Move to next word in chain
    }

    return NULL;
}

// Debug helper - show all words in dictionary
void show_dictionary(void) {
    printf("Dictionary contents (newest first):\n");
    word_t* current = dictionary_head;
    int count = 0;

    while (current != NULL) {
        printf("  %d: %s (addr=%u)\n", count++, current->name, ptr_to_addr(current));
        current = current->link;
    }

    if (count == 0) {
        printf("  (empty)\n");
    }
}

// ============================================================================
// Word creation and execution utilities (moved from core.c)
// ============================================================================

// Create a primitive word in virtual memory
word_t* create_primitive_word(const char* name, void (*cfunc)(word_t* self)) {
    // Allocate space for the word structure
    forth_addr_t word_addr = forth_allot(sizeof(word_t));
    word_t* word = addr_to_ptr(word_addr);

    // Initialize the word structure
    word->link = NULL;  // Will be set by link_word()
    strncpy(word->name, name, sizeof(word->name) - 1);
    word->name[sizeof(word->name) - 1] = '\0';  // Ensure null termination
    word->flags = 0;
    word->cfunc = cfunc;
    word->param_field = here;
    link_word(word);

    return word;
}

// Create a variable word and return a C pointer to its value
cell_t* create_variable_word(const char* name, cell_t initial_value) {
    // Create the word header with f_address cfunc
    word_t* word = create_primitive_word(name, f_address);

    word->param_field = initial_value;  // Store value directly

    // Return C pointer to the parameter field for efficiency
    forth_addr_t word_addr = ptr_to_addr(word);
    forth_addr_t param_addr = word_addr + offsetof(word_t, param_field);
    return (cell_t*)&forth_memory[param_addr];
}

// Create an area word (calls create_primitive_word with f_address)
void create_area_word(const char* name) {
    create_primitive_word(name, f_address);
}

// Create an immediate primitive word
word_t* create_immediate_primitive_word(const char* name, void (*cfunc)(word_t* self)) {
    word_t* word = create_primitive_word(name, cfunc);
    word->flags |= WORD_FLAG_IMMEDIATE;
    return word;
}

// Helper function for creating new word definitions
word_t* defining_word(void (*cfunc)(struct word* self)) {
    char name_buffer[32];

    // Parse the name for the new definition
    char* name = parse_name(name_buffer, sizeof(name_buffer));

    if (!name) error("Missing name after ':'");

    debug("Creating word: %s", name);

    // Create word header but don't link it yet (hidden until ; is executed)
    forth_addr_t word_addr = forth_allot(sizeof(word_t));
    word_t* word = addr_to_ptr(word_addr);

    // Initialize word header
    strncpy(word->name, name, sizeof(word->name) - 1);
    word->name[sizeof(word->name) - 1] = '\0';
    word->flags = 0;
    word->cfunc = cfunc;
    word->param_field = here;  // Set parameter field to point to next free space
    link_word(word);

    return word;
}

// Execute a word by calling its cfunc
void execute_word(word_t* word) {
    require(word != NULL);
    require(word->cfunc != NULL);
    word->cfunc(word);
}

// ============================================================================
// Word execution semantics
// ============================================================================

// VARIABLE runtime behavior: Push address OF the param_field
// (param_field contains the variable's value directly)
void f_address(word_t* self) {
    // Parameter field is right after the word structure in memory
    forth_addr_t word_addr = ptr_to_addr(self);
    forth_addr_t param_addr = word_addr + offsetof(word_t, param_field);

    // Push the Forth address of the parameter field
    data_push(param_addr);
}

// CREATE runtime behavior: Push address IN the param_field
// (param_field contains a Forth address pointing to data space)
void f_param_field(word_t* self) {
    data_push(self->param_field);  // Push the value stored in param_field
}