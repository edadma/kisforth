#include "line_editor.h"
#include <stdio.h>

// Pico doesn't need terminal mode changes - USB serial is already "raw"
void terminal_raw_mode_enter(void) {
    // No-op on Pico
}

void terminal_raw_mode_exit(void) {
    // No-op on Pico
}

// Parse Pico escape sequences
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