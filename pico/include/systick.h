#ifndef SYSTICK_H
#define SYSTICK_H

#include "forth.h"

// Initialize SysTick system (call once at startup)
void systick_init(void);

// Register SysTick-related Forth words
void pico_register_systick_words(void);

#endif  // SYSTICK_H