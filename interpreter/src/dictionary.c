#include "forth.h"
#include "tools.h"
#include "floating.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

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