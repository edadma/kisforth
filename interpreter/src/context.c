#include "context.h"

#include <stdbool.h>
#include <string.h>

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