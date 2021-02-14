#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifndef DATA_H_INCLUDE
#define DATA_H_INCLUDE

#include <ctype.h>
#include <termios.h>
#include <time.h>
#include <sys/types.h>

#define KILO_VERSION "0.0.1"
#define KILO_TAB_STOP 8
#define KILO_QUIT_TIMES 3

#define ABUF_INIT {NULL, 0};

#define HL_HIGHLIGHT_NUMBERS (1<<0)
#define HL_HIGHLIGHT_STRINGS (1<<1)

enum EDITOR_KEYS {
    CTRL_F = 6,
    CTRL_H = 8,
    CTRL_L = 12,
    CTRL_Q = 17,
    CTRL_S = 19,
    BACKSPACE = 127,
    ARROW_UP = 1000,
    ARROW_DOWN,
    ARROW_LEFT,
    ARROW_RIGHT,
    PAGE_UP,
    PAGE_DOWN,
    HOME_KEY,
    END_KEY,
    DEL_KEY
};

enum EDITOR_HIGHLIGHT {
    HL_NORMAL = 0,
    HL_KEYWORD1,
    HL_KEYWORD2,
    HL_COMMENT,
    HL_MLCOMMENT,
    HL_STRING,
    HL_NUMBER,
    HL_MATCH
};

// struct to represent a row of line
struct erow {
    int idx;
    char *chars;
    int size;
    int rsize;
    char *render;
    int hl_open_comment;
    unsigned char *hl;
};

// buffer to write
struct abuf {
    char *b;
    int len;
};

// for filletype detection
struct editorSyntax {
    char *filetype;
    char **filematch;
    char **keywords;
    char *singleline_comment_start;
    char *multiline_comment_start;
    char *multiline_comment_end;
    int flags;
};

// data
struct editorConfig {
    int cx, cy;
    int screenrows;
    int screencols;
    struct termios orig_termios;
    int numrows;
    struct erow *row;
    int rowoff;
    int coloff;
    int rx;
    char *filename;
    char statusmsg[80];
    time_t statusmsg_time;
    int dirty;
    struct editorSyntax *syntax;
};

#endif