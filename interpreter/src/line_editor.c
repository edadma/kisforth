#include "line_editor.h"
#include <stdio.h>
#include <string.h>
#include "text.h"

// Terminal control sequences
#define CURSOR_SAVE    "\033[s"
#define CURSOR_RESTORE "\033[u"
#define CURSOR_LEFT    "\033[D"
#define CURSOR_RIGHT   "\033[C"
#define CLEAR_EOL      "\033[K"

// Line buffer management functions
static void insert_char_at_cursor(line_buffer_t* line, char c) {
    if (line->length >= INPUT_BUFFER_SIZE - 1) {
        return; // Buffer full
    }

    // Shift characters right from cursor position
    for (size_t i = line->length; i > line->cursor_pos; i--) {
        line->buffer[i] = line->buffer[i - 1];
    }

    // Insert new character
    line->buffer[line->cursor_pos] = c;
    line->length++;
    line->cursor_pos++;
}

static void delete_char_at_cursor(line_buffer_t* line) {
    if (line->cursor_pos == 0) {
        return; // Nothing to delete
    }

    // Shift characters left from cursor position
    for (size_t i = line->cursor_pos - 1; i < line->length - 1; i++) {
        line->buffer[i] = line->buffer[i + 1];
    }

    line->length--;
    line->cursor_pos--;
}

static void redraw_line(line_buffer_t* line, const char* prompt) {
    // Move cursor to beginning of line
    printf("\r");

    // Clear line and redraw prompt + buffer
    printf("%s%.*s", prompt, (int)line->length, line->buffer);

    // Clear any remaining characters
    printf(CLEAR_EOL);

    // Position cursor correctly
    size_t chars_from_end = line->length - line->cursor_pos;
    for (size_t i = 0; i < chars_from_end; i++) {
        printf(CURSOR_LEFT);
    }

    fflush(stdout);
}

static void move_cursor_left(line_buffer_t* line) {
    if (line->cursor_pos > 0) {
        line->cursor_pos--;
        printf(CURSOR_LEFT);
        fflush(stdout);
    }
}

static void move_cursor_right(line_buffer_t* line) {
    if (line->cursor_pos < line->length) {
        line->cursor_pos++;
        printf(CURSOR_RIGHT);
        fflush(stdout);
    }
}

static void move_cursor_to_start(line_buffer_t* line) {
    while (line->cursor_pos > 0) {
        line->cursor_pos--;
        printf(CURSOR_LEFT);
    }
    fflush(stdout);
}

static void move_cursor_to_end(line_buffer_t* line) {
    while (line->cursor_pos < line->length) {
        line->cursor_pos++;
        printf(CURSOR_RIGHT);
    }
    fflush(stdout);
}

static void handle_backspace(line_buffer_t* line, const char* prompt) {
    if (line->cursor_pos > 0) {
        delete_char_at_cursor(line);
        redraw_line(line, prompt);
    }
}

static void handle_key_event(line_buffer_t* line, key_event_t event, const char* prompt) {
    switch (event.type) {
        case KEY_NORMAL:
            if (event.character >= 32 && event.character < 127) {
                insert_char_at_cursor(line, event.character);
                redraw_line(line, prompt);
            }
            break;

        case KEY_LEFT:
            move_cursor_left(line);
            break;

        case KEY_RIGHT:
            move_cursor_right(line);
            break;

        case KEY_HOME:
            move_cursor_to_start(line);
            break;

        case KEY_END:
            move_cursor_to_end(line);
            break;

        case KEY_BACKSPACE:
            handle_backspace(line, prompt);
            break;

        case KEY_ENTER:
            // Line complete - handled by caller
            break;
    }
}

// Main line editing function
void enhanced_get_line(char* buffer, size_t max_len) {
    line_buffer_t line = {0};

    // Use a simple prompt for testing
    const char* prompt = "";

    terminal_raw_mode_enter();

    while (1) {
        key_event_t event = parse_key_sequence();

        if (event.type == KEY_ENTER) {
            break;
        }

        handle_key_event(&line, event, prompt);
    }

    terminal_raw_mode_exit();

    // Copy result to output buffer
    size_t copy_len = (line.length < max_len - 1) ? line.length : max_len - 1;
    memcpy(buffer, line.buffer, copy_len);
    buffer[copy_len] = '\0';

    printf("\n");
    fflush(stdout);
}