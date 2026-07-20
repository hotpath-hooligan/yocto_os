#include "keyboard.h"

#include "console.h"
#include "kernel.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

#define SCANCODE_EXTENDED 0xE0
#define SCANCODE_RELEASED 0x80
#define KEY_LEFT_SHIFT 0x2A
#define KEY_RIGHT_SHIFT 0x36
#define KEY_CAPS_LOCK 0x3A
#define KEY_ESCAPE 0x01
#define KEY_BACKSPACE 0x0E
#define KEY_TAB 0x0F
#define KEY_ENTER 0x1C

struct keyboard_state {
    int left_shift;
    int right_shift;
    int caps_lock;
    int extended;
};

static struct keyboard_state state;

static const char normal_map[128] = {
    [0x02] = '1', [0x03] = '2', [0x04] = '3', [0x05] = '4',
    [0x06] = '5', [0x07] = '6', [0x08] = '7', [0x09] = '8',
    [0x0A] = '9', [0x0B] = '0', [0x0C] = '-', [0x0D] = '=',
    [0x10] = 'q', [0x11] = 'w', [0x12] = 'e', [0x13] = 'r',
    [0x14] = 't', [0x15] = 'y', [0x16] = 'u', [0x17] = 'i',
    [0x18] = 'o', [0x19] = 'p', [0x1A] = '[', [0x1B] = ']',
    [0x1E] = 'a', [0x1F] = 's', [0x20] = 'd', [0x21] = 'f',
    [0x22] = 'g', [0x23] = 'h', [0x24] = 'j', [0x25] = 'k',
    [0x26] = 'l', [0x27] = ';', [0x28] = '\'', [0x29] = '`',
    [0x2B] = '\\', [0x2C] = 'z', [0x2D] = 'x', [0x2E] = 'c',
    [0x2F] = 'v', [0x30] = 'b', [0x31] = 'n', [0x32] = 'm',
    [0x33] = ',', [0x34] = '.', [0x35] = '/', [0x39] = ' ',
};

static const char shifted_map[128] = {
    [0x02] = '!', [0x03] = '@', [0x04] = '#', [0x05] = '$',
    [0x06] = '%', [0x07] = '^', [0x08] = '&', [0x09] = '*',
    [0x0A] = '(', [0x0B] = ')', [0x0C] = '_', [0x0D] = '+',
    [0x10] = 'Q', [0x11] = 'W', [0x12] = 'E', [0x13] = 'R',
    [0x14] = 'T', [0x15] = 'Y', [0x16] = 'U', [0x17] = 'I',
    [0x18] = 'O', [0x19] = 'P', [0x1A] = '{', [0x1B] = '}',
    [0x1E] = 'A', [0x1F] = 'S', [0x20] = 'D', [0x21] = 'F',
    [0x22] = 'G', [0x23] = 'H', [0x24] = 'J', [0x25] = 'K',
    [0x26] = 'L', [0x27] = ':', [0x28] = '"', [0x29] = '~',
    [0x2B] = '|', [0x2C] = 'Z', [0x2D] = 'X', [0x2E] = 'C',
    [0x2F] = 'V', [0x30] = 'B', [0x31] = 'N', [0x32] = 'M',
    [0x33] = '<', [0x34] = '>', [0x35] = '?', [0x39] = ' ',
};

static int is_letter(char character)
{
    return character >= 'a' && character <= 'z';
}

void keyboard_initialize(void)
{
    state.left_shift = 0;
    state.right_shift = 0;
    state.caps_lock = 0;
    state.extended = 0;

    while ((inb(KEYBOARD_STATUS_PORT) & 0x01) != 0) {
        (void)inb(KEYBOARD_DATA_PORT);
    }
}

void keyboard_handle_scancode(uint8_t scancode)
{
    if (scancode == SCANCODE_EXTENDED) {
        state.extended = 1;
        return;
    }

    int released = (scancode & SCANCODE_RELEASED) != 0;
    uint8_t code = (uint8_t)(scancode & ~SCANCODE_RELEASED);

    if (state.extended) {
        state.extended = 0;
        return;
    }

    if (code == KEY_LEFT_SHIFT) {
        state.left_shift = !released;
        return;
    }
    if (code == KEY_RIGHT_SHIFT) {
        state.right_shift = !released;
        return;
    }
    if (released) {
        return;
    }

    switch (code) {
    case KEY_CAPS_LOCK:
        state.caps_lock = !state.caps_lock;
        return;
    case KEY_ESCAPE:
        console_clear();
        return;
    case KEY_BACKSPACE:
        console_backspace();
        return;
    case KEY_TAB:
        console_input_character(' ');
        console_input_character(' ');
        console_input_character(' ');
        console_input_character(' ');
        return;
    case KEY_ENTER:
        console_submit();
        return;
    default:
        break;
    }

    char character = normal_map[code];
    int shifted = state.left_shift || state.right_shift;

    if (character == '\0') {
        return;
    }
    if (is_letter(character)) {
        if (shifted != state.caps_lock) {
            character = (char)(character - 'a' + 'A');
        }
    } else if (shifted) {
        character = shifted_map[code];
    }

    console_input_character(character);
}
