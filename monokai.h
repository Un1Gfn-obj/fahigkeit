// https://airtightinteractive.com/util/hex-to-glsl/

#pragma once

// Affected by xiccd?

// http://terminal.sexy/
// gcolor2
// https://en.wikipedia.org/wiki/ANSI_escape_code#3-bit_and_4-bit
// #define MK_CYAN    // ?
#define MK_BLACK   0x28,0x29,0x23 // Background
#define MK_BLUE    0xAC,0x80,0xFF // C type
#define MK_BROWN   0xFD,0x96,0x21 // Python self
#define MK_GRAY    0x74,0x70,0x5D // C comments
#define MK_GREEN   0xA6,0xE2,0x2C // C macro
#define MK_MAGENTA 0x67,0xD8,0xEF // C numeric literal
#define MK_RED     0xF9,0x24,0x72 // C preprocessor directives
#define MK_WHITE   0xF8,0xF8,0xF2 // C function param
#define MK_YELLOW  0xE7,0xDB,0x74 // C static string

#define STR0(a,b,c,d) #a","#b","#c","#d
#define STR(a,b,c,d) STR0(a,b,c,d)
#define HEX2GLSL0(R,G,B) STR(float(R)/255.0f,float(G)/255.0f,float(B)/255.0f,1.0f)
#define HEX2RGBA0(R,G,B)     (float)R/255.0f,(float)G/255.0f,(float)B/255.0f,1.0f
// https://stackoverflow.com/a/38338245
#define HEX2RGBA(RGB) HEX2RGBA0(RGB)
#define HEX2GLSL(RGB) HEX2GLSL0(RGB)
