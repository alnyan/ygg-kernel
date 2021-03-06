#pragma once

static const char x86_ps2_scan[] = {
    0x00,

    [0x01] = '\027',

    [0x02] = '1',
             '2',
             '3',
             '4',
             '5',
             '6',
             '7',
             '8',
             '9',
             '0',
             '-',
             '=',
             '\b',
             '\t',

    [0x10] = 'q',
             'w',
             'e',
             'r',
             't',
             'y',
             'u',
             'i',
             'o',
             'p',
             '[',
             ']',
             '\n',

    [0x1e] = 'a',
             's',
             'd',
             'f',
             'g',
             'h',
             'j',
             'k',
             'l',
             ';',
             '\'',
             '`',
             0,
             '\\',

    [0x2c] = 'z',
             'x',
             'c',
             'v',
             'b',
             'n',
             'm',
             ',',
             '.',
             '/',

    [0x39] = ' ',

    [127] = 0
};

static const char x86_ps2_scan_alt[] = {
    0x00,

    [0x01] = '\027',

    [0x02] = '!',
             '@',
             '#',
             '$',
             '%',
             '^',
             '&',
             '*',
             '(',
             ')',
             '_',
             '=',
             '\b',
             '\t',

    [0x10] = 'Q',
             'W',
             'E',
             'R',
             'T',
             'Y',
             'U',
             'I',
             'O',
             'P',
             '{',
             '}',
             '\n',

    [0x1e] = 'A',
             'S',
             'D',
             'F',
             'G',
             'H',
             'J',
             'K',
             'L',
             ':',
             '"',
             '~',
             0,
             '|',

    [0x2c] = 'Z',
             'X',
             'C',
             'V',
             'B',
             'N',
             'M',
             '<',
             '>',
             '?',

    [0x39] = ' ',

    [127] = 0
};

