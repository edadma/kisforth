#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include "line_editor.h"

static struct termios original_termios;
static bool raw_mode_active = false;

// Enter raw terminal mode for immediate key input
void terminal_raw_mode_enter(void) {
  if (raw_mode_active) return;

  // Save original terminal settings
  tcgetattr(STDIN_FILENO, &original_termios);

  struct termios raw = original_termios;

  // Disable line buffering and echo
  raw.c_lflag &= ~(ECHO | ICANON);

  // Set minimum characters and timeout for read
  raw.c_cc[VMIN] = 1;   // Read at least 1 character
  raw.c_cc[VTIME] = 0;  // No timeout

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
  raw_mode_active = true;
}

// Exit raw terminal mode
void terminal_raw_mode_exit(void) {
  if (!raw_mode_active) return;

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
  raw_mode_active = false;
}

// Parse Linux escape sequences
key_event_t parse_key_sequence(void) {
  key_event_t event = {0};
  int c = getchar();

  if (c == EOF) {
    event.type = KEY_ENTER;
    return event;
  }

  if (c == '\r' || c == '\n') {
    event.type = KEY_ENTER;
    return event;
  }

  if (c == '\b' || c == 127) {
    event.type = KEY_BACKSPACE;
    return event;
  }

  if (c == '\033') {  // ESC
    // Read '['
    c = getchar();
    if (c == '[') {
      // Read the command character
      c = getchar();
      switch (c) {
        case 'D':
          event.type = KEY_LEFT;
          break;
        case 'C':
          event.type = KEY_RIGHT;
          break;
        case 'H':
          event.type = KEY_HOME;
          break;
        case 'F':
          event.type = KEY_END;
          break;
        case '1':
          // Handle sequences like ESC[1~ (Home key variant)
          c = getchar();
          if (c == '~') {
            event.type = KEY_HOME;
          } else {
            // Unknown sequence - treat as normal
            event.type = KEY_NORMAL;
            event.character = c;
          }
          break;
        case '3':
          // Handle sequences like ESC[3~ (Delete key)
          c = getchar();
          if (c == '~') {
            event.type = KEY_DELETE;
          } else {
            // Unknown sequence - treat as normal
            event.type = KEY_NORMAL;
            event.character = c;
          }
          break;
        case '4':
          // Handle sequences like ESC[4~ (End key variant)
          c = getchar();
          if (c == '~') {
            event.type = KEY_END;
          } else {
            // Unknown sequence - treat as normal
            event.type = KEY_NORMAL;
            event.character = c;
          }
          break;
        default:
          // Unknown escape sequence - treat as normal
          event.type = KEY_NORMAL;
          event.character = c;
          break;
      }
    } else {
      // Not a [ sequence - treat as normal
      event.type = KEY_NORMAL;
      event.character = c;
    }
    return event;
  }

  // Normal character
  event.type = KEY_NORMAL;
  event.character = c;
  return event;
}