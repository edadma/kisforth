#include "forth.h"
#include <string.h>
#include <stdio.h>

// Dictionary head points to the most recently defined word
word_t* dictionary_head = NULL;

// Initialize empty dictionary
void dictionary_init(void) {
    dictionary_head = NULL;
}

// Link a word into the dictionary (at the head of the linked list)
void link_word(word_t* word) {
    word->link = dictionary_head;  // Point to previous head
    dictionary_head = word;        // Make this word the new head
}

// Find a word in the dictionary by name (case-sensitive search)
word_t* find_word(const char* name) {
    word_t* current = dictionary_head;

    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;  // Found it!
        }
        current = current->link;  // Move to next word in chain
    }

    return NULL;  // Not found
}

// Debug helper - show all words in dictionary
void show_dictionary(void) {
    printf("Dictionary contents (newest first):\n");
    word_t* current = dictionary_head;
    int count = 0;

    while (current != NULL) {
        printf("  %d: %s (addr=%u)\n", count++, current->name, word_to_addr(current));
        current = current->link;
    }

    if (count == 0) {
        printf("  (empty)\n");
    }
}