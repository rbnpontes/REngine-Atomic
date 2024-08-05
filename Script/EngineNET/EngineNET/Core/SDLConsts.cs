#region License
/* SDL2# - C# Wrapper for SDL2
 *
 * Copyright (c) 2013-2015 Ethan Lee.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software in a
 * product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * Ethan "flibitijibibo" Lee <flibitijibibo@flibitijibibo.com>
 *
 */
#endregion

namespace AtomicEngine
{

    public static class SDL
    {


        public const int SDL_BUTTON_LEFT =    1;
        public const int SDL_BUTTON_MIDDLE =    2;
        public const int SDL_BUTTON_RIGHT =    3;
        public const int SDL_BUTTON_X1 =    4;
        public const int SDL_BUTTON_X2 =    5;
        public const int SDL_BUTTON_LMASK =    (1 << ((int)SDL_BUTTON_LEFT - 1));
        public const int SDL_BUTTON_MMASK =    (1 << ((int)SDL_BUTTON_MIDDLE - 1));
        public const int SDL_BUTTON_RMASK =    (1 << ((int)SDL_BUTTON_RIGHT - 1));
        public const int SDL_BUTTON_X1MASK = (1 << ((int)SDL_BUTTON_X1 - 1));
        public const int SDL_BUTTON_X2MASK = (1 << ((int)SDL_BUTTON_X2 - 1));

        public const byte SDL_HAT_CENTERED =    0x00;
        public const byte SDL_HAT_UP =        0x01;
        public const byte SDL_HAT_RIGHT =    0x02;
        public const byte SDL_HAT_DOWN =    0x04;
        public const byte SDL_HAT_LEFT =    0x08;
        public const byte SDL_HAT_RIGHTUP =    SDL_HAT_RIGHT | SDL_HAT_UP;
        public const byte SDL_HAT_RIGHTDOWN =    SDL_HAT_RIGHT | SDL_HAT_DOWN;
        public const byte SDL_HAT_LEFTUP =    SDL_HAT_LEFT | SDL_HAT_UP;
        public const byte SDL_HAT_LEFTDOWN =    SDL_HAT_LEFT | SDL_HAT_DOWN;

        public enum SDL_GameControllerButton
        {
            SDL_CONTROLLER_BUTTON_INVALID = -1,
            SDL_CONTROLLER_BUTTON_A,
            SDL_CONTROLLER_BUTTON_B,
            SDL_CONTROLLER_BUTTON_X,
            SDL_CONTROLLER_BUTTON_Y,
            SDL_CONTROLLER_BUTTON_BACK,
            SDL_CONTROLLER_BUTTON_GUIDE,
            SDL_CONTROLLER_BUTTON_START,
            SDL_CONTROLLER_BUTTON_LEFTSTICK,
            SDL_CONTROLLER_BUTTON_RIGHTSTICK,
            SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
            SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
            SDL_CONTROLLER_BUTTON_DPAD_UP,
            SDL_CONTROLLER_BUTTON_DPAD_DOWN,
            SDL_CONTROLLER_BUTTON_DPAD_LEFT,
            SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
            SDL_CONTROLLER_BUTTON_MAX,
        }

        public enum SDL_GameControllerAxis
        {
            SDL_CONTROLLER_AXIS_INVALID = -1,
            SDL_CONTROLLER_AXIS_LEFTX,
            SDL_CONTROLLER_AXIS_LEFTY,
            SDL_CONTROLLER_AXIS_RIGHTX,
            SDL_CONTROLLER_AXIS_RIGHTY,
            SDL_CONTROLLER_AXIS_TRIGGERLEFT,
            SDL_CONTROLLER_AXIS_TRIGGERRIGHT,
            SDL_CONTROLLER_AXIS_MAX
        }


        /* Scancodes based off USB keyboard page (0x07) */
        public enum SDL_Scancode
        {
            SDL_SCANCODE_UNKNOWN = 0,

            SDL_SCANCODE_A = 4,
            SDL_SCANCODE_B = 5,
            SDL_SCANCODE_C = 6,
            SDL_SCANCODE_D = 7,
            SDL_SCANCODE_E = 8,
            SDL_SCANCODE_F = 9,
            SDL_SCANCODE_G = 10,
            SDL_SCANCODE_H = 11,
            SDL_SCANCODE_I = 12,
            SDL_SCANCODE_J = 13,
            SDL_SCANCODE_K = 14,
            SDL_SCANCODE_L = 15,
            SDL_SCANCODE_M = 16,
            SDL_SCANCODE_N = 17,
            SDL_SCANCODE_O = 18,
            SDL_SCANCODE_P = 19,
            SDL_SCANCODE_Q = 20,
            SDL_SCANCODE_R = 21,
            SDL_SCANCODE_S = 22,
            SDL_SCANCODE_T = 23,
            SDL_SCANCODE_U = 24,
            SDL_SCANCODE_V = 25,
            SDL_SCANCODE_W = 26,
            SDL_SCANCODE_X = 27,
            SDL_SCANCODE_Y = 28,
            SDL_SCANCODE_Z = 29,

            SDL_SCANCODE_1 = 30,
            SDL_SCANCODE_2 = 31,
            SDL_SCANCODE_3 = 32,
            SDL_SCANCODE_4 = 33,
            SDL_SCANCODE_5 = 34,
            SDL_SCANCODE_6 = 35,
            SDL_SCANCODE_7 = 36,
            SDL_SCANCODE_8 = 37,
            SDL_SCANCODE_9 = 38,
            SDL_SCANCODE_0 = 39,

            SDL_SCANCODE_RETURN = 40,
            SDL_SCANCODE_ESCAPE = 41,
            SDL_SCANCODE_BACKSPACE = 42,
            SDL_SCANCODE_TAB = 43,
            SDL_SCANCODE_SPACE = 44,

            SDL_SCANCODE_MINUS = 45,
            SDL_SCANCODE_EQUALS = 46,
            SDL_SCANCODE_LEFTBRACKET = 47,
            SDL_SCANCODE_RIGHTBRACKET = 48,
            SDL_SCANCODE_BACKSLASH = 49,
            SDL_SCANCODE_NONUSHASH = 50,
            SDL_SCANCODE_SEMICOLON = 51,
            SDL_SCANCODE_APOSTROPHE = 52,
            SDL_SCANCODE_GRAVE = 53,
            SDL_SCANCODE_COMMA = 54,
            SDL_SCANCODE_PERIOD = 55,
            SDL_SCANCODE_SLASH = 56,

            SDL_SCANCODE_CAPSLOCK = 57,

            SDL_SCANCODE_F1 = 58,
            SDL_SCANCODE_F2 = 59,
            SDL_SCANCODE_F3 = 60,
            SDL_SCANCODE_F4 = 61,
            SDL_SCANCODE_F5 = 62,
            SDL_SCANCODE_F6 = 63,
            SDL_SCANCODE_F7 = 64,
            SDL_SCANCODE_F8 = 65,
            SDL_SCANCODE_F9 = 66,
            SDL_SCANCODE_F10 = 67,
            SDL_SCANCODE_F11 = 68,
            SDL_SCANCODE_F12 = 69,

            SDL_SCANCODE_PRINTSCREEN = 70,
            SDL_SCANCODE_SCROLLLOCK = 71,
            SDL_SCANCODE_PAUSE = 72,
            SDL_SCANCODE_INSERT = 73,
            SDL_SCANCODE_HOME = 74,
            SDL_SCANCODE_PAGEUP = 75,
            SDL_SCANCODE_DELETE = 76,
            SDL_SCANCODE_END = 77,
            SDL_SCANCODE_PAGEDOWN = 78,
            SDL_SCANCODE_RIGHT = 79,
            SDL_SCANCODE_LEFT = 80,
            SDL_SCANCODE_DOWN = 81,
            SDL_SCANCODE_UP = 82,

            SDL_SCANCODE_NUMLOCKCLEAR = 83,
            SDL_SCANCODE_KP_DIVIDE = 84,
            SDL_SCANCODE_KP_MULTIPLY = 85,
            SDL_SCANCODE_KP_MINUS = 86,
            SDL_SCANCODE_KP_PLUS = 87,
            SDL_SCANCODE_KP_ENTER = 88,
            SDL_SCANCODE_KP_1 = 89,
            SDL_SCANCODE_KP_2 = 90,
            SDL_SCANCODE_KP_3 = 91,
            SDL_SCANCODE_KP_4 = 92,
            SDL_SCANCODE_KP_5 = 93,
            SDL_SCANCODE_KP_6 = 94,
            SDL_SCANCODE_KP_7 = 95,
            SDL_SCANCODE_KP_8 = 96,
            SDL_SCANCODE_KP_9 = 97,
            SDL_SCANCODE_KP_0 = 98,
            SDL_SCANCODE_KP_PERIOD = 99,

            SDL_SCANCODE_NONUSBACKSLASH = 100,
            SDL_SCANCODE_APPLICATION = 101,
            SDL_SCANCODE_POWER = 102,
            SDL_SCANCODE_KP_EQUALS = 103,
            SDL_SCANCODE_F13 = 104,
            SDL_SCANCODE_F14 = 105,
            SDL_SCANCODE_F15 = 106,
            SDL_SCANCODE_F16 = 107,
            SDL_SCANCODE_F17 = 108,
            SDL_SCANCODE_F18 = 109,
            SDL_SCANCODE_F19 = 110,
            SDL_SCANCODE_F20 = 111,
            SDL_SCANCODE_F21 = 112,
            SDL_SCANCODE_F22 = 113,
            SDL_SCANCODE_F23 = 114,
            SDL_SCANCODE_F24 = 115,
            SDL_SCANCODE_EXECUTE = 116,
            SDL_SCANCODE_HELP = 117,
            SDL_SCANCODE_MENU = 118,
            SDL_SCANCODE_SELECT = 119,
            SDL_SCANCODE_STOP = 120,
            SDL_SCANCODE_AGAIN = 121,
            SDL_SCANCODE_UNDO = 122,
            SDL_SCANCODE_CUT = 123,
            SDL_SCANCODE_COPY = 124,
            SDL_SCANCODE_PASTE = 125,
            SDL_SCANCODE_FIND = 126,
            SDL_SCANCODE_MUTE = 127,
            SDL_SCANCODE_VOLUMEUP = 128,
            SDL_SCANCODE_VOLUMEDOWN = 129,
            /* not sure whether there's a reason to enable these */
            /*    SDL_SCANCODE_LOCKINGCAPSLOCK = 130,  */
            /*    SDL_SCANCODE_LOCKINGNUMLOCK = 131, */
            /*    SDL_SCANCODE_LOCKINGSCROLLLOCK = 132, */
            SDL_SCANCODE_KP_COMMA = 133,
            SDL_SCANCODE_KP_EQUALSAS400 = 134,

            SDL_SCANCODE_INTERNATIONAL1 = 135,
            SDL_SCANCODE_INTERNATIONAL2 = 136,
            SDL_SCANCODE_INTERNATIONAL3 = 137,
            SDL_SCANCODE_INTERNATIONAL4 = 138,
            SDL_SCANCODE_INTERNATIONAL5 = 139,
            SDL_SCANCODE_INTERNATIONAL6 = 140,
            SDL_SCANCODE_INTERNATIONAL7 = 141,
            SDL_SCANCODE_INTERNATIONAL8 = 142,
            SDL_SCANCODE_INTERNATIONAL9 = 143,
            SDL_SCANCODE_LANG1 = 144,
            SDL_SCANCODE_LANG2 = 145,
            SDL_SCANCODE_LANG3 = 146,
            SDL_SCANCODE_LANG4 = 147,
            SDL_SCANCODE_LANG5 = 148,
            SDL_SCANCODE_LANG6 = 149,
            SDL_SCANCODE_LANG7 = 150,
            SDL_SCANCODE_LANG8 = 151,
            SDL_SCANCODE_LANG9 = 152,

            SDL_SCANCODE_ALTERASE = 153,
            SDL_SCANCODE_SYSREQ = 154,
            SDL_SCANCODE_CANCEL = 155,
            SDL_SCANCODE_CLEAR = 156,
            SDL_SCANCODE_PRIOR = 157,
            SDL_SCANCODE_RETURN2 = 158,
            SDL_SCANCODE_SEPARATOR = 159,
            SDL_SCANCODE_OUT = 160,
            SDL_SCANCODE_OPER = 161,
            SDL_SCANCODE_CLEARAGAIN = 162,
            SDL_SCANCODE_CRSEL = 163,
            SDL_SCANCODE_EXSEL = 164,

            SDL_SCANCODE_KP_00 = 176,
            SDL_SCANCODE_KP_000 = 177,
            SDL_SCANCODE_THOUSANDSSEPARATOR = 178,
            SDL_SCANCODE_DECIMALSEPARATOR = 179,
            SDL_SCANCODE_CURRENCYUNIT = 180,
            SDL_SCANCODE_CURRENCYSUBUNIT = 181,
            SDL_SCANCODE_KP_LEFTPAREN = 182,
            SDL_SCANCODE_KP_RIGHTPAREN = 183,
            SDL_SCANCODE_KP_LEFTBRACE = 184,
            SDL_SCANCODE_KP_RIGHTBRACE = 185,
            SDL_SCANCODE_KP_TAB = 186,
            SDL_SCANCODE_KP_BACKSPACE = 187,
            SDL_SCANCODE_KP_A = 188,
            SDL_SCANCODE_KP_B = 189,
            SDL_SCANCODE_KP_C = 190,
            SDL_SCANCODE_KP_D = 191,
            SDL_SCANCODE_KP_E = 192,
            SDL_SCANCODE_KP_F = 193,
            SDL_SCANCODE_KP_XOR = 194,
            SDL_SCANCODE_KP_POWER = 195,
            SDL_SCANCODE_KP_PERCENT = 196,
            SDL_SCANCODE_KP_LESS = 197,
            SDL_SCANCODE_KP_GREATER = 198,
            SDL_SCANCODE_KP_AMPERSAND = 199,
            SDL_SCANCODE_KP_DBLAMPERSAND = 200,
            SDL_SCANCODE_KP_VERTICALBAR = 201,
            SDL_SCANCODE_KP_DBLVERTICALBAR = 202,
            SDL_SCANCODE_KP_COLON = 203,
            SDL_SCANCODE_KP_HASH = 204,
            SDL_SCANCODE_KP_SPACE = 205,
            SDL_SCANCODE_KP_AT = 206,
            SDL_SCANCODE_KP_EXCLAM = 207,
            SDL_SCANCODE_KP_MEMSTORE = 208,
            SDL_SCANCODE_KP_MEMRECALL = 209,
            SDL_SCANCODE_KP_MEMCLEAR = 210,
            SDL_SCANCODE_KP_MEMADD = 211,
            SDL_SCANCODE_KP_MEMSUBTRACT = 212,
            SDL_SCANCODE_KP_MEMMULTIPLY = 213,
            SDL_SCANCODE_KP_MEMDIVIDE = 214,
            SDL_SCANCODE_KP_PLUSMINUS = 215,
            SDL_SCANCODE_KP_CLEAR = 216,
            SDL_SCANCODE_KP_CLEARENTRY = 217,
            SDL_SCANCODE_KP_BINARY = 218,
            SDL_SCANCODE_KP_OCTAL = 219,
            SDL_SCANCODE_KP_DECIMAL = 220,
            SDL_SCANCODE_KP_HEXADECIMAL = 221,

            SDL_SCANCODE_LCTRL = 224,
            SDL_SCANCODE_LSHIFT = 225,
            SDL_SCANCODE_LALT = 226,
            SDL_SCANCODE_LGUI = 227,
            SDL_SCANCODE_RCTRL = 228,
            SDL_SCANCODE_RSHIFT = 229,
            SDL_SCANCODE_RALT = 230,
            SDL_SCANCODE_RGUI = 231,

            SDL_SCANCODE_MODE = 257,

            /* These come from the USB consumer page (0x0C) */
            SDL_SCANCODE_AUDIONEXT = 258,
            SDL_SCANCODE_AUDIOPREV = 259,
            SDL_SCANCODE_AUDIOSTOP = 260,
            SDL_SCANCODE_AUDIOPLAY = 261,
            SDL_SCANCODE_AUDIOMUTE = 262,
            SDL_SCANCODE_MEDIASELECT = 263,
            SDL_SCANCODE_WWW = 264,
            SDL_SCANCODE_MAIL = 265,
            SDL_SCANCODE_CALCULATOR = 266,
            SDL_SCANCODE_COMPUTER = 267,
            SDL_SCANCODE_AC_SEARCH = 268,
            SDL_SCANCODE_AC_HOME = 269,
            SDL_SCANCODE_AC_BACK = 270,
            SDL_SCANCODE_AC_FORWARD = 271,
            SDL_SCANCODE_AC_STOP = 272,
            SDL_SCANCODE_AC_REFRESH = 273,
            SDL_SCANCODE_AC_BOOKMARKS = 274,

            /* These come from other sources, and are mostly mac related */
            SDL_SCANCODE_BRIGHTNESSDOWN = 275,
            SDL_SCANCODE_BRIGHTNESSUP = 276,
            SDL_SCANCODE_DISPLAYSWITCH = 277,
            SDL_SCANCODE_KBDILLUMTOGGLE = 278,
            SDL_SCANCODE_KBDILLUMDOWN = 279,
            SDL_SCANCODE_KBDILLUMUP = 280,
            SDL_SCANCODE_EJECT = 281,
            SDL_SCANCODE_SLEEP = 282,

            SDL_SCANCODE_APP1 = 283,
            SDL_SCANCODE_APP2 = 284,

            /* This is not a key, simply marks the number of scancodes
             * so that you know how big to make your arrays. */
            SDL_NUM_SCANCODES = 512
        }

        public const int SDLK_SCANCODE_MASK = (1 << 30);

        public enum SDL_Keycode
        {
            SDLK_UNKNOWN = 0,

            SDLK_RETURN = '\r',
            SDLK_ESCAPE = 27,
            // '\033'
            SDLK_BACKSPACE = '\b',
            SDLK_TAB = '\t',
            SDLK_SPACE = ' ',
            SDLK_EXCLAIM = '!',
            SDLK_QUOTEDBL = '"',
            SDLK_HASH = '#',
            SDLK_PERCENT = '%',
            SDLK_DOLLAR = '$',
            SDLK_AMPERSAND = '&',
            SDLK_QUOTE = '\'',
            SDLK_LEFTPAREN = '(',
            SDLK_RIGHTPAREN = ')',
            SDLK_ASTERISK = '*',
            SDLK_PLUS = '+',
            SDLK_COMMA = ',',
            SDLK_MINUS = '-',
            SDLK_PERIOD = '.',
            SDLK_SLASH = '/',
            SDLK_0 = '0',
            SDLK_1 = '1',
            SDLK_2 = '2',
            SDLK_3 = '3',
            SDLK_4 = '4',
            SDLK_5 = '5',
            SDLK_6 = '6',
            SDLK_7 = '7',
            SDLK_8 = '8',
            SDLK_9 = '9',
            SDLK_COLON = ':',
            SDLK_SEMICOLON = ';',
            SDLK_LESS = '<',
            SDLK_EQUALS = '=',
            SDLK_GREATER = '>',
            SDLK_QUESTION = '?',
            SDLK_AT = '@',
            /*
            Skip uppercase letters
            */
            SDLK_LEFTBRACKET = '[',
            SDLK_BACKSLASH = '\\',
            SDLK_RIGHTBRACKET = ']',
            SDLK_CARET = '^',
            SDLK_UNDERSCORE = '_',
            SDLK_BACKQUOTE = '`',
            SDLK_a = 'a',
            SDLK_b = 'b',
            SDLK_c = 'c',
            SDLK_d = 'd',
            SDLK_e = 'e',
            SDLK_f = 'f',
            SDLK_g = 'g',
            SDLK_h = 'h',
            SDLK_i = 'i',
            SDLK_j = 'j',
            SDLK_k = 'k',
            SDLK_l = 'l',
            SDLK_m = 'm',
            SDLK_n = 'n',
            SDLK_o = 'o',
            SDLK_p = 'p',
            SDLK_q = 'q',
            SDLK_r = 'r',
            SDLK_s = 's',
            SDLK_t = 't',
            SDLK_u = 'u',
            SDLK_v = 'v',
            SDLK_w = 'w',
            SDLK_x = 'x',
            SDLK_y = 'y',
            SDLK_z = 'z',

            SDLK_CAPSLOCK = (int)SDL_Scancode.SDL_SCANCODE_CAPSLOCK | SDLK_SCANCODE_MASK,

            SDLK_F1 = (int)SDL_Scancode.SDL_SCANCODE_F1 | SDLK_SCANCODE_MASK,
            SDLK_F2 = (int)SDL_Scancode.SDL_SCANCODE_F2 | SDLK_SCANCODE_MASK,
            SDLK_F3 = (int)SDL_Scancode.SDL_SCANCODE_F3 | SDLK_SCANCODE_MASK,
            SDLK_F4 = (int)SDL_Scancode.SDL_SCANCODE_F4 | SDLK_SCANCODE_MASK,
            SDLK_F5 = (int)SDL_Scancode.SDL_SCANCODE_F5 | SDLK_SCANCODE_MASK,
            SDLK_F6 = (int)SDL_Scancode.SDL_SCANCODE_F6 | SDLK_SCANCODE_MASK,
            SDLK_F7 = (int)SDL_Scancode.SDL_SCANCODE_F7 | SDLK_SCANCODE_MASK,
            SDLK_F8 = (int)SDL_Scancode.SDL_SCANCODE_F8 | SDLK_SCANCODE_MASK,
            SDLK_F9 = (int)SDL_Scancode.SDL_SCANCODE_F9 | SDLK_SCANCODE_MASK,
            SDLK_F10 = (int)SDL_Scancode.SDL_SCANCODE_F10 | SDLK_SCANCODE_MASK,
            SDLK_F11 = (int)SDL_Scancode.SDL_SCANCODE_F11 | SDLK_SCANCODE_MASK,
            SDLK_F12 = (int)SDL_Scancode.SDL_SCANCODE_F12 | SDLK_SCANCODE_MASK,

            SDLK_PRINTSCREEN = (int)SDL_Scancode.SDL_SCANCODE_PRINTSCREEN | SDLK_SCANCODE_MASK,
            SDLK_SCROLLLOCK = (int)SDL_Scancode.SDL_SCANCODE_SCROLLLOCK | SDLK_SCANCODE_MASK,
            SDLK_PAUSE = (int)SDL_Scancode.SDL_SCANCODE_PAUSE | SDLK_SCANCODE_MASK,
            SDLK_INSERT = (int)SDL_Scancode.SDL_SCANCODE_INSERT | SDLK_SCANCODE_MASK,
            SDLK_HOME = (int)SDL_Scancode.SDL_SCANCODE_HOME | SDLK_SCANCODE_MASK,
            SDLK_PAGEUP = (int)SDL_Scancode.SDL_SCANCODE_PAGEUP | SDLK_SCANCODE_MASK,
            SDLK_DELETE = 127,
            SDLK_END = (int)SDL_Scancode.SDL_SCANCODE_END | SDLK_SCANCODE_MASK,
            SDLK_PAGEDOWN = (int)SDL_Scancode.SDL_SCANCODE_PAGEDOWN | SDLK_SCANCODE_MASK,
            SDLK_RIGHT = (int)SDL_Scancode.SDL_SCANCODE_RIGHT | SDLK_SCANCODE_MASK,
            SDLK_LEFT = (int)SDL_Scancode.SDL_SCANCODE_LEFT | SDLK_SCANCODE_MASK,
            SDLK_DOWN = (int)SDL_Scancode.SDL_SCANCODE_DOWN | SDLK_SCANCODE_MASK,
            SDLK_UP = (int)SDL_Scancode.SDL_SCANCODE_UP | SDLK_SCANCODE_MASK,

            SDLK_NUMLOCKCLEAR = (int)SDL_Scancode.SDL_SCANCODE_NUMLOCKCLEAR | SDLK_SCANCODE_MASK,
            SDLK_KP_DIVIDE = (int)SDL_Scancode.SDL_SCANCODE_KP_DIVIDE | SDLK_SCANCODE_MASK,
            SDLK_KP_MULTIPLY = (int)SDL_Scancode.SDL_SCANCODE_KP_MULTIPLY | SDLK_SCANCODE_MASK,
            SDLK_KP_MINUS = (int)SDL_Scancode.SDL_SCANCODE_KP_MINUS | SDLK_SCANCODE_MASK,
            SDLK_KP_PLUS = (int)SDL_Scancode.SDL_SCANCODE_KP_PLUS | SDLK_SCANCODE_MASK,
            SDLK_KP_ENTER = (int)SDL_Scancode.SDL_SCANCODE_KP_ENTER | SDLK_SCANCODE_MASK,
            SDLK_KP_1 = (int)SDL_Scancode.SDL_SCANCODE_KP_1 | SDLK_SCANCODE_MASK,
            SDLK_KP_2 = (int)SDL_Scancode.SDL_SCANCODE_KP_2 | SDLK_SCANCODE_MASK,
            SDLK_KP_3 = (int)SDL_Scancode.SDL_SCANCODE_KP_3 | SDLK_SCANCODE_MASK,
            SDLK_KP_4 = (int)SDL_Scancode.SDL_SCANCODE_KP_4 | SDLK_SCANCODE_MASK,
            SDLK_KP_5 = (int)SDL_Scancode.SDL_SCANCODE_KP_5 | SDLK_SCANCODE_MASK,
            SDLK_KP_6 = (int)SDL_Scancode.SDL_SCANCODE_KP_6 | SDLK_SCANCODE_MASK,
            SDLK_KP_7 = (int)SDL_Scancode.SDL_SCANCODE_KP_7 | SDLK_SCANCODE_MASK,
            SDLK_KP_8 = (int)SDL_Scancode.SDL_SCANCODE_KP_8 | SDLK_SCANCODE_MASK,
            SDLK_KP_9 = (int)SDL_Scancode.SDL_SCANCODE_KP_9 | SDLK_SCANCODE_MASK,
            SDLK_KP_0 = (int)SDL_Scancode.SDL_SCANCODE_KP_0 | SDLK_SCANCODE_MASK,
            SDLK_KP_PERIOD = (int)SDL_Scancode.SDL_SCANCODE_KP_PERIOD | SDLK_SCANCODE_MASK,

            SDLK_APPLICATION = (int)SDL_Scancode.SDL_SCANCODE_APPLICATION | SDLK_SCANCODE_MASK,
            SDLK_POWER = (int)SDL_Scancode.SDL_SCANCODE_POWER | SDLK_SCANCODE_MASK,
            SDLK_KP_EQUALS = (int)SDL_Scancode.SDL_SCANCODE_KP_EQUALS | SDLK_SCANCODE_MASK,
            SDLK_F13 = (int)SDL_Scancode.SDL_SCANCODE_F13 | SDLK_SCANCODE_MASK,
            SDLK_F14 = (int)SDL_Scancode.SDL_SCANCODE_F14 | SDLK_SCANCODE_MASK,
            SDLK_F15 = (int)SDL_Scancode.SDL_SCANCODE_F15 | SDLK_SCANCODE_MASK,
            SDLK_F16 = (int)SDL_Scancode.SDL_SCANCODE_F16 | SDLK_SCANCODE_MASK,
            SDLK_F17 = (int)SDL_Scancode.SDL_SCANCODE_F17 | SDLK_SCANCODE_MASK,
            SDLK_F18 = (int)SDL_Scancode.SDL_SCANCODE_F18 | SDLK_SCANCODE_MASK,
            SDLK_F19 = (int)SDL_Scancode.SDL_SCANCODE_F19 | SDLK_SCANCODE_MASK,
            SDLK_F20 = (int)SDL_Scancode.SDL_SCANCODE_F20 | SDLK_SCANCODE_MASK,
            SDLK_F21 = (int)SDL_Scancode.SDL_SCANCODE_F21 | SDLK_SCANCODE_MASK,
            SDLK_F22 = (int)SDL_Scancode.SDL_SCANCODE_F22 | SDLK_SCANCODE_MASK,
            SDLK_F23 = (int)SDL_Scancode.SDL_SCANCODE_F23 | SDLK_SCANCODE_MASK,
            SDLK_F24 = (int)SDL_Scancode.SDL_SCANCODE_F24 | SDLK_SCANCODE_MASK,
            SDLK_EXECUTE = (int)SDL_Scancode.SDL_SCANCODE_EXECUTE | SDLK_SCANCODE_MASK,
            SDLK_HELP = (int)SDL_Scancode.SDL_SCANCODE_HELP | SDLK_SCANCODE_MASK,
            SDLK_MENU = (int)SDL_Scancode.SDL_SCANCODE_MENU | SDLK_SCANCODE_MASK,
            SDLK_SELECT = (int)SDL_Scancode.SDL_SCANCODE_SELECT | SDLK_SCANCODE_MASK,
            SDLK_STOP = (int)SDL_Scancode.SDL_SCANCODE_STOP | SDLK_SCANCODE_MASK,
            SDLK_AGAIN = (int)SDL_Scancode.SDL_SCANCODE_AGAIN | SDLK_SCANCODE_MASK,
            SDLK_UNDO = (int)SDL_Scancode.SDL_SCANCODE_UNDO | SDLK_SCANCODE_MASK,
            SDLK_CUT = (int)SDL_Scancode.SDL_SCANCODE_CUT | SDLK_SCANCODE_MASK,
            SDLK_COPY = (int)SDL_Scancode.SDL_SCANCODE_COPY | SDLK_SCANCODE_MASK,
            SDLK_PASTE = (int)SDL_Scancode.SDL_SCANCODE_PASTE | SDLK_SCANCODE_MASK,
            SDLK_FIND = (int)SDL_Scancode.SDL_SCANCODE_FIND | SDLK_SCANCODE_MASK,
            SDLK_MUTE = (int)SDL_Scancode.SDL_SCANCODE_MUTE | SDLK_SCANCODE_MASK,
            SDLK_VOLUMEUP = (int)SDL_Scancode.SDL_SCANCODE_VOLUMEUP | SDLK_SCANCODE_MASK,
            SDLK_VOLUMEDOWN = (int)SDL_Scancode.SDL_SCANCODE_VOLUMEDOWN | SDLK_SCANCODE_MASK,
            SDLK_KP_COMMA = (int)SDL_Scancode.SDL_SCANCODE_KP_COMMA | SDLK_SCANCODE_MASK,
            SDLK_KP_EQUALSAS400 =
            (int)SDL_Scancode.SDL_SCANCODE_KP_EQUALSAS400 | SDLK_SCANCODE_MASK,

            SDLK_ALTERASE = (int)SDL_Scancode.SDL_SCANCODE_ALTERASE | SDLK_SCANCODE_MASK,
            SDLK_SYSREQ = (int)SDL_Scancode.SDL_SCANCODE_SYSREQ | SDLK_SCANCODE_MASK,
            SDLK_CANCEL = (int)SDL_Scancode.SDL_SCANCODE_CANCEL | SDLK_SCANCODE_MASK,
            SDLK_CLEAR = (int)SDL_Scancode.SDL_SCANCODE_CLEAR | SDLK_SCANCODE_MASK,
            SDLK_PRIOR = (int)SDL_Scancode.SDL_SCANCODE_PRIOR | SDLK_SCANCODE_MASK,
            SDLK_RETURN2 = (int)SDL_Scancode.SDL_SCANCODE_RETURN2 | SDLK_SCANCODE_MASK,
            SDLK_SEPARATOR = (int)SDL_Scancode.SDL_SCANCODE_SEPARATOR | SDLK_SCANCODE_MASK,
            SDLK_OUT = (int)SDL_Scancode.SDL_SCANCODE_OUT | SDLK_SCANCODE_MASK,
            SDLK_OPER = (int)SDL_Scancode.SDL_SCANCODE_OPER | SDLK_SCANCODE_MASK,
            SDLK_CLEARAGAIN = (int)SDL_Scancode.SDL_SCANCODE_CLEARAGAIN | SDLK_SCANCODE_MASK,
            SDLK_CRSEL = (int)SDL_Scancode.SDL_SCANCODE_CRSEL | SDLK_SCANCODE_MASK,
            SDLK_EXSEL = (int)SDL_Scancode.SDL_SCANCODE_EXSEL | SDLK_SCANCODE_MASK,

            SDLK_KP_00 = (int)SDL_Scancode.SDL_SCANCODE_KP_00 | SDLK_SCANCODE_MASK,
            SDLK_KP_000 = (int)SDL_Scancode.SDL_SCANCODE_KP_000 | SDLK_SCANCODE_MASK,
            SDLK_THOUSANDSSEPARATOR =
            (int)SDL_Scancode.SDL_SCANCODE_THOUSANDSSEPARATOR | SDLK_SCANCODE_MASK,
            SDLK_DECIMALSEPARATOR =
            (int)SDL_Scancode.SDL_SCANCODE_DECIMALSEPARATOR | SDLK_SCANCODE_MASK,
            SDLK_CURRENCYUNIT = (int)SDL_Scancode.SDL_SCANCODE_CURRENCYUNIT | SDLK_SCANCODE_MASK,
            SDLK_CURRENCYSUBUNIT =
            (int)SDL_Scancode.SDL_SCANCODE_CURRENCYSUBUNIT | SDLK_SCANCODE_MASK,
            SDLK_KP_LEFTPAREN = (int)SDL_Scancode.SDL_SCANCODE_KP_LEFTPAREN | SDLK_SCANCODE_MASK,
            SDLK_KP_RIGHTPAREN = (int)SDL_Scancode.SDL_SCANCODE_KP_RIGHTPAREN | SDLK_SCANCODE_MASK,
            SDLK_KP_LEFTBRACE = (int)SDL_Scancode.SDL_SCANCODE_KP_LEFTBRACE | SDLK_SCANCODE_MASK,
            SDLK_KP_RIGHTBRACE = (int)SDL_Scancode.SDL_SCANCODE_KP_RIGHTBRACE | SDLK_SCANCODE_MASK,
            SDLK_KP_TAB = (int)SDL_Scancode.SDL_SCANCODE_KP_TAB | SDLK_SCANCODE_MASK,
            SDLK_KP_BACKSPACE = (int)SDL_Scancode.SDL_SCANCODE_KP_BACKSPACE | SDLK_SCANCODE_MASK,
            SDLK_KP_A = (int)SDL_Scancode.SDL_SCANCODE_KP_A | SDLK_SCANCODE_MASK,
            SDLK_KP_B = (int)SDL_Scancode.SDL_SCANCODE_KP_B | SDLK_SCANCODE_MASK,
            SDLK_KP_C = (int)SDL_Scancode.SDL_SCANCODE_KP_C | SDLK_SCANCODE_MASK,
            SDLK_KP_D = (int)SDL_Scancode.SDL_SCANCODE_KP_D | SDLK_SCANCODE_MASK,
            SDLK_KP_E = (int)SDL_Scancode.SDL_SCANCODE_KP_E | SDLK_SCANCODE_MASK,
            SDLK_KP_F = (int)SDL_Scancode.SDL_SCANCODE_KP_F | SDLK_SCANCODE_MASK,
            SDLK_KP_XOR = (int)SDL_Scancode.SDL_SCANCODE_KP_XOR | SDLK_SCANCODE_MASK,
            SDLK_KP_POWER = (int)SDL_Scancode.SDL_SCANCODE_KP_POWER | SDLK_SCANCODE_MASK,
            SDLK_KP_PERCENT = (int)SDL_Scancode.SDL_SCANCODE_KP_PERCENT | SDLK_SCANCODE_MASK,
            SDLK_KP_LESS = (int)SDL_Scancode.SDL_SCANCODE_KP_LESS | SDLK_SCANCODE_MASK,
            SDLK_KP_GREATER = (int)SDL_Scancode.SDL_SCANCODE_KP_GREATER | SDLK_SCANCODE_MASK,
            SDLK_KP_AMPERSAND = (int)SDL_Scancode.SDL_SCANCODE_KP_AMPERSAND | SDLK_SCANCODE_MASK,
            SDLK_KP_DBLAMPERSAND =
            (int)SDL_Scancode.SDL_SCANCODE_KP_DBLAMPERSAND | SDLK_SCANCODE_MASK,
            SDLK_KP_VERTICALBAR =
            (int)SDL_Scancode.SDL_SCANCODE_KP_VERTICALBAR | SDLK_SCANCODE_MASK,
            SDLK_KP_DBLVERTICALBAR =
            (int)SDL_Scancode.SDL_SCANCODE_KP_DBLVERTICALBAR | SDLK_SCANCODE_MASK,
            SDLK_KP_COLON = (int)SDL_Scancode.SDL_SCANCODE_KP_COLON | SDLK_SCANCODE_MASK,
            SDLK_KP_HASH = (int)SDL_Scancode.SDL_SCANCODE_KP_HASH | SDLK_SCANCODE_MASK,
            SDLK_KP_SPACE = (int)SDL_Scancode.SDL_SCANCODE_KP_SPACE | SDLK_SCANCODE_MASK,
            SDLK_KP_AT = (int)SDL_Scancode.SDL_SCANCODE_KP_AT | SDLK_SCANCODE_MASK,
            SDLK_KP_EXCLAM = (int)SDL_Scancode.SDL_SCANCODE_KP_EXCLAM | SDLK_SCANCODE_MASK,
            SDLK_KP_MEMSTORE = (int)SDL_Scancode.SDL_SCANCODE_KP_MEMSTORE | SDLK_SCANCODE_MASK,
            SDLK_KP_MEMRECALL = (int)SDL_Scancode.SDL_SCANCODE_KP_MEMRECALL | SDLK_SCANCODE_MASK,
            SDLK_KP_MEMCLEAR = (int)SDL_Scancode.SDL_SCANCODE_KP_MEMCLEAR | SDLK_SCANCODE_MASK,
            SDLK_KP_MEMADD = (int)SDL_Scancode.SDL_SCANCODE_KP_MEMADD | SDLK_SCANCODE_MASK,
            SDLK_KP_MEMSUBTRACT =
            (int)SDL_Scancode.SDL_SCANCODE_KP_MEMSUBTRACT | SDLK_SCANCODE_MASK,
            SDLK_KP_MEMMULTIPLY =
            (int)SDL_Scancode.SDL_SCANCODE_KP_MEMMULTIPLY | SDLK_SCANCODE_MASK,
            SDLK_KP_MEMDIVIDE = (int)SDL_Scancode.SDL_SCANCODE_KP_MEMDIVIDE | SDLK_SCANCODE_MASK,
            SDLK_KP_PLUSMINUS = (int)SDL_Scancode.SDL_SCANCODE_KP_PLUSMINUS | SDLK_SCANCODE_MASK,
            SDLK_KP_CLEAR = (int)SDL_Scancode.SDL_SCANCODE_KP_CLEAR | SDLK_SCANCODE_MASK,
            SDLK_KP_CLEARENTRY = (int)SDL_Scancode.SDL_SCANCODE_KP_CLEARENTRY | SDLK_SCANCODE_MASK,
            SDLK_KP_BINARY = (int)SDL_Scancode.SDL_SCANCODE_KP_BINARY | SDLK_SCANCODE_MASK,
            SDLK_KP_OCTAL = (int)SDL_Scancode.SDL_SCANCODE_KP_OCTAL | SDLK_SCANCODE_MASK,
            SDLK_KP_DECIMAL = (int)SDL_Scancode.SDL_SCANCODE_KP_DECIMAL | SDLK_SCANCODE_MASK,
            SDLK_KP_HEXADECIMAL =
            (int)SDL_Scancode.SDL_SCANCODE_KP_HEXADECIMAL | SDLK_SCANCODE_MASK,

            SDLK_LCTRL = (int)SDL_Scancode.SDL_SCANCODE_LCTRL | SDLK_SCANCODE_MASK,
            SDLK_LSHIFT = (int)SDL_Scancode.SDL_SCANCODE_LSHIFT | SDLK_SCANCODE_MASK,
            SDLK_LALT = (int)SDL_Scancode.SDL_SCANCODE_LALT | SDLK_SCANCODE_MASK,
            SDLK_LGUI = (int)SDL_Scancode.SDL_SCANCODE_LGUI | SDLK_SCANCODE_MASK,
            SDLK_RCTRL = (int)SDL_Scancode.SDL_SCANCODE_RCTRL | SDLK_SCANCODE_MASK,
            SDLK_RSHIFT = (int)SDL_Scancode.SDL_SCANCODE_RSHIFT | SDLK_SCANCODE_MASK,
            SDLK_RALT = (int)SDL_Scancode.SDL_SCANCODE_RALT | SDLK_SCANCODE_MASK,
            SDLK_RGUI = (int)SDL_Scancode.SDL_SCANCODE_RGUI | SDLK_SCANCODE_MASK,

            SDLK_MODE = (int)SDL_Scancode.SDL_SCANCODE_MODE | SDLK_SCANCODE_MASK,

            SDLK_AUDIONEXT = (int)SDL_Scancode.SDL_SCANCODE_AUDIONEXT | SDLK_SCANCODE_MASK,
            SDLK_AUDIOPREV = (int)SDL_Scancode.SDL_SCANCODE_AUDIOPREV | SDLK_SCANCODE_MASK,
            SDLK_AUDIOSTOP = (int)SDL_Scancode.SDL_SCANCODE_AUDIOSTOP | SDLK_SCANCODE_MASK,
            SDLK_AUDIOPLAY = (int)SDL_Scancode.SDL_SCANCODE_AUDIOPLAY | SDLK_SCANCODE_MASK,
            SDLK_AUDIOMUTE = (int)SDL_Scancode.SDL_SCANCODE_AUDIOMUTE | SDLK_SCANCODE_MASK,
            SDLK_MEDIASELECT = (int)SDL_Scancode.SDL_SCANCODE_MEDIASELECT | SDLK_SCANCODE_MASK,
            SDLK_WWW = (int)SDL_Scancode.SDL_SCANCODE_WWW | SDLK_SCANCODE_MASK,
            SDLK_MAIL = (int)SDL_Scancode.SDL_SCANCODE_MAIL | SDLK_SCANCODE_MASK,
            SDLK_CALCULATOR = (int)SDL_Scancode.SDL_SCANCODE_CALCULATOR | SDLK_SCANCODE_MASK,
            SDLK_COMPUTER = (int)SDL_Scancode.SDL_SCANCODE_COMPUTER | SDLK_SCANCODE_MASK,
            SDLK_AC_SEARCH = (int)SDL_Scancode.SDL_SCANCODE_AC_SEARCH | SDLK_SCANCODE_MASK,
            SDLK_AC_HOME = (int)SDL_Scancode.SDL_SCANCODE_AC_HOME | SDLK_SCANCODE_MASK,
            SDLK_AC_BACK = (int)SDL_Scancode.SDL_SCANCODE_AC_BACK | SDLK_SCANCODE_MASK,
            SDLK_AC_FORWARD = (int)SDL_Scancode.SDL_SCANCODE_AC_FORWARD | SDLK_SCANCODE_MASK,
            SDLK_AC_STOP = (int)SDL_Scancode.SDL_SCANCODE_AC_STOP | SDLK_SCANCODE_MASK,
            SDLK_AC_REFRESH = (int)SDL_Scancode.SDL_SCANCODE_AC_REFRESH | SDLK_SCANCODE_MASK,
            SDLK_AC_BOOKMARKS = (int)SDL_Scancode.SDL_SCANCODE_AC_BOOKMARKS | SDLK_SCANCODE_MASK,

            SDLK_BRIGHTNESSDOWN =
            (int)SDL_Scancode.SDL_SCANCODE_BRIGHTNESSDOWN | SDLK_SCANCODE_MASK,
            SDLK_BRIGHTNESSUP = (int)SDL_Scancode.SDL_SCANCODE_BRIGHTNESSUP | SDLK_SCANCODE_MASK,
            SDLK_DISPLAYSWITCH = (int)SDL_Scancode.SDL_SCANCODE_DISPLAYSWITCH | SDLK_SCANCODE_MASK,
            SDLK_KBDILLUMTOGGLE =
            (int)SDL_Scancode.SDL_SCANCODE_KBDILLUMTOGGLE | SDLK_SCANCODE_MASK,
            SDLK_KBDILLUMDOWN = (int)SDL_Scancode.SDL_SCANCODE_KBDILLUMDOWN | SDLK_SCANCODE_MASK,
            SDLK_KBDILLUMUP = (int)SDL_Scancode.SDL_SCANCODE_KBDILLUMUP | SDLK_SCANCODE_MASK,
            SDLK_EJECT = (int)SDL_Scancode.SDL_SCANCODE_EJECT | SDLK_SCANCODE_MASK,
            SDLK_SLEEP = (int)SDL_Scancode.SDL_SCANCODE_SLEEP | SDLK_SCANCODE_MASK
        }
    }

}
