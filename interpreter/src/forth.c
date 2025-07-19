#include "forth.h"

#include <stdbool.h>
#include <string.h>

#include "error.h"
#include "memory.h"

// Static mapping table (only accessible within this file)
typedef struct {
  forth_addr_t base_addr;
  size_t context_offset;
  size_t region_size;
} transient_mapping_t;

static const transient_mapping_t transient_mappings[] = {
    {FORTH_MEMORY_SIZE, offsetof(context_t, pad_buffer), PAD_SIZE},
    {FORTH_MEMORY_SIZE + PAD_SIZE, offsetof(context_t, word_buffer),
     WORD_BUFFER_SIZE},
    {FORTH_MEMORY_SIZE + PAD_SIZE + WORD_BUFFER_SIZE,
     offsetof(context_t, pictured_buffer), PICTURED_BUFFER_SIZE}};

// Address translation function
void* addr_to_ptr(context_t* ctx, forth_addr_t addr) {
  // Handle main Forth memory
  if (addr < FORTH_MEMORY_SIZE) {
    return &forth_memory[addr];
  }

  // Handle transient regions
  for (int i = 0; i < 3; i++) {
    const transient_mapping_t* mapping = &transient_mappings[i];
    if (addr >= mapping->base_addr &&
        addr < mapping->base_addr + mapping->region_size) {
      int buffer_offset = addr - mapping->base_addr;
      byte_t* buffer = (byte_t*)ctx + mapping->context_offset;
      return buffer + buffer_offset;
    }
  }

  error(ctx, "Invalid Forth address: %u", addr);
  return NULL;
}

// In forth.h or a new context.c file
void context_init(context_t* ctx, const char* name, bool is_interrupt_handler) {
  // Execution state
  ctx->ip = 0;

  // Initialize stacks (replaces old stack_init)
  ctx->data_stack_ptr = 0;
  ctx->return_stack_ptr = 0;

#ifdef FORTH_ENABLE_FLOATING
  ctx->float_stack_ptr = 0;
#endif

  // Clear transient buffers (optional but clean)
  memset(ctx->pad_buffer, 0, PAD_SIZE);
  memset(ctx->word_buffer, 0, WORD_BUFFER_SIZE);
  memset(ctx->pictured_buffer, 0, PICTURED_BUFFER_SIZE);

  // Input source state
  ctx->source_buffer = NULL;
  ctx->source_length = 0;
  ctx->source_index = 0;

  // Context identification
  ctx->name = name;
  ctx->is_interrupt_handler = is_interrupt_handler;
}