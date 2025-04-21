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

bool print_tainted = false;
char ansi_format[30];

void print_hex(char c)
{
    print_tainted = true;
    if (option.color > 0) {
        if (c > 31 && c < 127) {
            printf("\033[32m"); // COLOR_GREEN
        } else if (c == 9 || c == 10 || c == 13) {
            printf("\033[33m"); // COLOR_YELLOW
        } else if (c == 0) {
            printf("\033[37m"); // COLOR_WHITE
        } else if (c == 255) {
            printf("\033[34m"); // COLOR_BLUE
        } else {
            printf("\033[31m"); // COLOR_RED
        }
    }
    printf("%02x ", (unsigned char) c);
    if (option.color > 0) {
        printf("\033[0m");
    }
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
