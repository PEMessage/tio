/*
 * tio - a serial device I/O tool
 *
 * Copyright (c) 2014-2022  Martin Lund
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "print.h"
#include "options.h"

bool print_tainted = false;
char ansi_format[30];

// Color escape codes
#define COLOR_RESET_CODE   "\033[0m"
#define COLOR_RED_CODE     "\033[31m"
#define COLOR_GREEN_CODE   "\033[32m"
#define COLOR_YELLOW_CODE  "\033[33m"
#define COLOR_BLUE_CODE    "\033[34m"
#define COLOR_WHITE_CODE   "\033[37m"

// Character type constants
// Character type constants
#define SPACE_CHAR    ' '
#define TAB_CHAR      '\t'
#define NEWLINE_CHAR  '\n'
#define CARRIAGE_RETURN_CHAR '\r'
#define DELETE_CHAR   '\b'  // DEL character (still hex as it has no literal)
#define NULL_CHAR     '\0'
#define MAX_CHAR      0xFF  // Still keeping as hex for max byte value

typedef enum {
    COLOR_NONE = 0,
    COLOR_WHITE,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_RED
} ColorCode;


void print_hex(char current_char)
{
    static char previous_char = 0;
    static ColorCode previous_color = COLOR_NONE;

    const bool is_printable_expect_space = (current_char > 32 && current_char < 127);
    const bool is_printable = (current_char > 31 && current_char < 127);
    const bool was_printable = (previous_char > 31 && previous_char < 127);
    ColorCode current_color = COLOR_NONE;

    print_tainted = true;

    /* Determine current color */
    if (option.color > 0) {
        if (current_char == SPACE_CHAR) {
            current_color = COLOR_WHITE;
        } else if (is_printable_expect_space) {
            current_color = COLOR_GREEN;
        } else if (current_char == TAB_CHAR ||
                   current_char == NEWLINE_CHAR ||
                   current_char == CARRIAGE_RETURN_CHAR) {
            current_color = COLOR_YELLOW;
        } else if (current_char == 0) {
            current_color = COLOR_WHITE;
        } else if (current_char == (char)MAX_CHAR) {
            current_color = COLOR_BLUE;
        } else {
            current_color = COLOR_RED;
        }

        /* Only change color if different from previous */
        if (current_color != previous_color) {
            if (previous_color != COLOR_NONE) {
                printf(COLOR_RESET_CODE);
            }
            switch (current_color) {
                case COLOR_WHITE: printf(COLOR_WHITE_CODE); break;
                case COLOR_GREEN: printf(COLOR_GREEN_CODE); break;
                case COLOR_YELLOW: printf(COLOR_YELLOW_CODE); break;
                case COLOR_BLUE: printf(COLOR_BLUE_CODE); break;
                case COLOR_RED: printf(COLOR_RED_CODE); break;
                default: break;
            }
            previous_color = current_color;
        }
    }

    /* Hex mode printing */
    switch (option.hex_mode) {
        case HEX_MODE_MIX:
            if (was_printable && !is_printable) {
                printf(" ");  // Backspace for consecutive printable chars
            }

            if (current_char == SPACE_CHAR) {
                printf("_");
            } else if (previous_char == CARRIAGE_RETURN_CHAR && current_char == NEWLINE_CHAR) {
                printf("%02x \r\n", (unsigned char)current_char);
            } else if (is_printable_expect_space) {
                printf("%c", (unsigned char)current_char);
            } else {
                printf("%02x ", (unsigned char)current_char);
            }
            break;

        case HEX_MODE_MIX2:
            if (current_char == SPACE_CHAR) {
                printf("__ ");
            } else if (is_printable_expect_space) {
                printf("%c  ", (unsigned char)current_char);
            } else {
                printf("%02x ", (unsigned char)current_char);
            }
            break;

        default:  // Regular hex mode
            printf("%02x ", (unsigned char)current_char);
            break;
    }

    previous_char = current_char;
}

void print_normal(char c)
{
    print_tainted = true;
    putchar(c);
}

void print_init_ansi_formatting()
{
    if (option.color == 256)
    {
        // Set bold text with no color changes
        sprintf(ansi_format, "\e[1m");
    }
    else
    {
        // Set bold text with user defined ANSI color
        sprintf(ansi_format, "\e[1;38;5;%dm", option.color);
    }
}

void tio_printf_array(const char *array)
{
    int i = 0, j = 0;

    tio_printf("");

    while (array[i])
    {
        if (array[i] == '\n')
        {
            const char *line = &array[j];
            char *line_copy = strndup(line, i-j);
            tio_printf_raw("%s\r", line_copy);
            free(line_copy);
            j = i;
        }
        i++;
    }
    tio_printf("");
}

void print_tainted_set()
{
    print_tainted = true;
}

void print(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    vprintf(format, args);
    fflush(stdout);
    va_end(args);

    print_tainted = true;
}

void print_padded(char *string, size_t length, char pad_char)
{
    size_t padding = 0;
    size_t string_length = 0;
    size_t i;

    string_length = strlen(string);

    if (string_length < length)
    {
        padding += length - string_length;
        printf("%s", string);
        for (i=0; i<padding; i++)
        {
            putchar(pad_char);
        }
    }
    else
    {
        printf("%s", string);
    }
}
