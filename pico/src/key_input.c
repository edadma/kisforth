#include "line_editor.h"
#include <stdio.h>

// Pico doesn't need terminal mode changes - USB serial is already "raw"
void terminal_raw_mode_enter(void) {
    // No-op on Pico
}

void terminal_raw_mode_exit(void) {
    // No-op on Pico
}

// Parse Pico escape sequences (based on your minicom output)
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

    // Check for escape sequences without ESC prefix
    // Based on your minicom output: [D, [C, [1~, OF
    if (c == '[') {
        c = getchar();
        switch (c) {
            case 'D':
                event.type = KEY_LEFT;
                break;
            case 'C':
                event.type = KEY_RIGHT;
                break;
            case '1':
                // Handle [1~ (Home key)
                c = getchar();
                if (c == '~') {
                    event.type = KEY_HOME;
                } else {
                    // Unknown sequence - treat as normal
                    event.type = KEY_NORMAL;
                    event.character = c;
                }
                break;
            default:
                // Unknown [ sequence - treat as normal
                event.type = KEY_NORMAL;
                event.character = c;
                break;
        }
        return event;
    }

    // Check for 'O' sequences (End key shows as 'OF')
    if (c == 'O') {
        c = getchar();
        if (c == 'F') {
            event.type = KEY_END;
        } else {
            // Unknown O sequence - treat as normal
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