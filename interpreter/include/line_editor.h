#ifndef LINE_EDITOR_H
#define LINE_EDITOR_H

#include <stddef.h>

#include "forth.h"
#include "text.h"

typedef enum {
  KEY_NORMAL,
  KEY_LEFT,
  KEY_RIGHT,
  KEY_HOME,
  KEY_END,
  KEY_BACKSPACE,
  KEY_DELETE,
  KEY_ENTER
} key_type_t;

typedef struct {
  key_type_t type;
  char character;  // Only valid when type == KEY_NORMAL
} key_event_t;

typedef struct {
  char buffer[INPUT_BUFFER_SIZE];
  size_t length;
  size_t cursor_pos;
} line_buffer_t;

// Platform-specific function - each target must implement
key_event_t parse_key_sequence(void);

// Platform-independent line editing
void enhanced_get_line(char* buffer, size_t max_len);

// Terminal control functions (platform-specific)
void terminal_raw_mode_enter(void);
void terminal_raw_mode_exit(void);
void terminal_clear_eol(void);
void terminal_cursor_left(void);
void terminal_cursor_right(void);
void terminal_show_cursor(void);
void terminal_hide_cursor(void);

#endif  // LINE_EDITOR_H