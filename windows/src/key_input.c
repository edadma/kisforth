#include <conio.h>
#include <stdio.h>
#include <windows.h>

#include "line_editor.h"

// Windows doesn't need special terminal mode setup for _getch()
void terminal_raw_mode_enter(void) {
  // No-op on Windows with conio
}

void terminal_raw_mode_exit(void) {
  // No-op on Windows with conio
}

// Parse Windows key input using _getch()
key_event_t parse_key_sequence(void) {
  key_event_t event = {0};
  int c = _getch();

  if (c == '\r' || c == '\n') {
    event.type = KEY_ENTER;
    return event;
  }

  if (c == '\b' || c == 127) {
    event.type = KEY_BACKSPACE;
    return event;
  }

  if (c == 0 || c == 224) {  // Extended key prefix on Windows
    // Read the extended key code
    c = _getch();
    switch (c) {
      case 75:  // Left arrow
        event.type = KEY_LEFT;
        break;
      case 77:  // Right arrow
        event.type = KEY_RIGHT;
        break;
      case 71:  // Home
        event.type = KEY_HOME;
        break;
      case 79:  // End
        event.type = KEY_END;
        break;
      case 83:  // Delete
        event.type = KEY_DELETE;
        break;
      default:
        // Unknown extended key - treat as normal
        event.type = KEY_NORMAL;
        event.character = c;
        break;
    }
    return event;
  }

  // Normal character
  event.type = KEY_NORMAL;
  event.character = c;
  return event;
}