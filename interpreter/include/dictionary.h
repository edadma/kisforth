#ifndef DICTIONARY_H
#define DICTIONARY_H

#include "types.h"

// Dictionary head - points to most recently defined word
extern word_t* dictionary_head;

// Core dictionary management functions (currently in dictionary.c)
void dictionary_init(void);
void link_word(word_t* word);
word_t* find_word(const char* name);
word_t* search_word(const char* name);
void show_dictionary(void);  // Debug helper

// Word creation and execution utilities
word_t* create_primitive_word(const char* name, void (*cfunc)(word_t* self));
cell_t* create_variable_word(const char* name, cell_t initial_value);
void create_area_word(const char* name);
word_t* create_immediate_primitive_word(const char* name, void (*cfunc)(word_t* self));
word_t* defining_word(void (*cfunc)(struct word* self));
void execute_word(word_t* word);
void execute_colon(word_t* self);
forth_addr_t store_counted_string(const char* str, int length);

// Word execution semantics
void f_address(word_t* self);       // Variable execution ( -- addr )
void f_param_field(word_t* self);   // CREATE execution ( -- addr )

#endif // DICTIONARY_H