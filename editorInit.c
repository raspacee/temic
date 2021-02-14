#include <termios.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include "data.h"
#include "editorInit.h"
#include "editorTerminal.h"

extern struct editorConfig E;

void initEditor(void)
{
    E.cx = 0;
    E.cy = 0;
    E.numrows = 0;
    E.row = NULL;
    E.rowoff = 0;
    E.coloff = 0;
    E.rx = 0;
    E.filename = NULL;
    E.statusmsg[0] = '\0';
    E.statusmsg_time = 0;
    E.dirty = 0;
    E.syntax = NULL;

    if (getWindowSize(&E.screenrows, &E.screencols) == -1) die("getWindowSize");

    E.screenrows -= 2;
}