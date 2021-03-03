#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include "data.h"
#include "editorInput.h"
#include "editorTerminal.h"
#include "editorOperations.h"
#include "editorFind.h"
#include "editorOutput.h"
#include "editorFileIO.h"

extern struct editorConfig E;

void editorMoveCursor(int key)
{
    struct erow *row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];

    switch(key) {
        case ARROW_LEFT:
            if (E.cx != E.widthlen + 1) {
                E.cx--;
            } else if (E.cy > 0) {
                E.cy--;
                E.cx = E.row[E.cy].size + E.widthlen + 1;
            }
            break;
        case ARROW_RIGHT:
            if (row && (E.cx - E.widthlen + 1) <= row->rsize) {
                E.cx++;
            } else if (row && E.cy < E.numrows - 1) {
                E.cy++;
                E.cx = E.widthlen + 1;
                E.coloff = 0;
            }
            break;
        case ARROW_UP:
            if (E.cy != 0) {
                E.cy--;
            }
            break;
        case ARROW_DOWN:
            if (E.cy < E.numrows) {
                E.cy++;
            }
            break;
    }

    // snap cursor to end of line
    row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
    int rowlen = row ? row->size : 0;
    if (E.cx > rowlen + E.widthlen + 1)
        E.cx = rowlen + E.widthlen + 1;
}

void editorProcessKeypress(void)
{
    static int quit_times = TEMIC_QUIT_TIMES;

    int c = editorReadKey();

    switch(c) {
        case '\r':
            editorInsertNewline(DEFAULT_CALLER);
            break;

        case CTRL_C:
            if (E.filemode == INSERT_MODE)
                E.filemode = NORMAL_MODE;
            break;

        case CTRL_F:
            editorFind();
            break;

        case CTRL_Q:
            if (E.dirty && quit_times > 0) {
                editorSetStatusMessage("Warning! File has unsaved changes. Press CTRL-Q %d more times to quit.", quit_times);
                quit_times--;
                return;
            }

            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);

            // freeing pointers
            free(E.filename);

            exit(0);
            break;

        case CTRL_S:
            editorSave();
            break;

        case BACKSPACE:
        case CTRL_H:
        case DEL_KEY:
            if (c == DEL_KEY)
                editorMoveCursor(ARROW_RIGHT);
            editorDelChar();
            break;

        case ARROW_UP:
        case ARROW_DOWN:
        case ARROW_LEFT:
        case ARROW_RIGHT:
            editorMoveCursor(c);
            break;

        case PAGE_UP:
        case PAGE_DOWN:
            {
                if (c == PAGE_UP) {
                    E.cy = 0;
                } else if (c == PAGE_DOWN) {
                    E.cy = E.numrows;
                }
            }
            break;

        case HOME_KEY:
            E.cx = E.widthlen + 1;
            E.coloff = 0;
            break;

        case END_KEY:
            E.cx = E.row[E.cy].rsize + E.widthlen + 1;

        case CTRL_L:
        case '\x1b':
            break;

        case 'i':
        case 'a':
        case 'A':
        case 'o':
        case 'O':
        {
            if (E.filemode == NORMAL_MODE) {
                E.filemode = INSERT_MODE;
                if (c == 'o') {
                    if (E.cy < E.numrows - 1) {
                            E.cx = E.widthlen + 1;
                            editorMoveCursor(ARROW_DOWN);
                            editorInsertNewline(EDITOR_PROCESS_KEYPRESS);
                    }
                } else if (c == 'O') {
                    if (E.cy > 1) {
                        E.cx = E.widthlen + 1;
                        editorInsertNewline(EDITOR_PROCESS_KEYPRESS);
                    }
                } else if (c == 'a') {
                    if (E.row[E.cy].chars[0] != ' ')
                        E.cx++;
                } else if (c == 'A') {
                    E.cx = E.row[E.cy].rsize + E.widthlen + 1;
                }
                break;
            }
        }


        case 'j':
        case 'k':
        case 'h':
        case 'l':
        {
            if (E.filemode == NORMAL_MODE) {
                if (c == 'j')
                    editorMoveCursor(ARROW_DOWN);
                else if (c == 'k')
                    editorMoveCursor(ARROW_UP);
                else if (c == 'h')
                    editorMoveCursor(ARROW_LEFT);
                else if (c == 'l')
                    editorMoveCursor(ARROW_RIGHT);
            }
        }

        default:
        {
            if (E.filemode == INSERT_MODE)
                editorInsertChar(c);
        }
            break;
    }

    quit_times = TEMIC_QUIT_TIMES;
}

char *editorPrompt(char *prompt, void (*callback)(char *, int))
{
    size_t bufsize = 128;
    char *buf = malloc(bufsize);
    size_t buflen = 0;
    buf[0] = '\0';

    while (1) {
        editorSetStatusMessage(prompt, buf);
        editorRefreshScreen();

        int c = editorReadKey();
        if (c == DEL_KEY || c == CTRL_H || c == BACKSPACE) {
            if (buflen != 0)
                buf[--buflen] = '\0';
        } else if (c == '\x1b') {
            editorSetStatusMessage("");
            if (callback)
                callback(buf, c);
            free(buf);
            return NULL;
        } else if (c == '\r') {
            if (buflen != 0) {
                editorSetStatusMessage("");
                if (callback)
                    callback(buf, c);
                return buf;
            }
        }
        else if (!iscntrl(c) && c < 128) {
            if (buflen == bufsize - 1) {
                bufsize *= 2;
                buf = realloc(buf, bufsize);
            }
            buf[buflen++] = c;
            buf[buflen] = '\0';
        }

        if (callback)
            callback(buf, c);
    }
}