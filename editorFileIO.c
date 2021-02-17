#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <ctype.h>

#include "data.h"
#include "editorFileIO.h"
#include "editorSyntaxHL.h"
#include "editorTerminal.h"
#include "editorOperations.h"
#include "editorInput.h"
#include "editorOutput.h"

struct editorConfig E;

void editorOpen(char *filename)
{
    E.filename = strdup(filename);

    editorSelectSyntaxHighlight();

    FILE *fp = fopen(filename, "r");
    if (!fp)
        die("fopen");

    char *line;
    size_t linecap = 0;
    ssize_t linelen;

    while ((linelen = getline(&line, &linecap, fp)) != -1) {
        // remove the \n or \r
        if (linelen > 0 && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r'))
                linelen--;

        editorInsertRow(E.numrows, line, linelen);
    }
    free(line);
    fclose(fp);

    // for line numbering
    E.widthlen = intLen(E.numrows);
    if (E.screencols > E.widthlen) {
        E.screencols -= (E.widthlen + 1);
        E.cx = E.widthlen + 1;
    }

    E.dirty = 0;
}

char *editorRowsToString(int *buflen)
{
    int totlen = 0;
    int j;
    for (j = 0; j < E.numrows; j++) {
        totlen += E.row[j].size + 1;
    }
    *buflen = totlen;

    char *buf = malloc(totlen);
    char *p = buf;
    for (j = 0; j < E.numrows; j++) {
        memcpy(p, E.row[j].chars, E.row[j].size);
        p += E.row[j].size;
        *p = '\n';
        p++;
    }
    return buf;
}

void editorSave()
{
    if (E.filename == NULL) {
        E.filename = editorPrompt("Save as: %s", NULL);
        if (E.filename == NULL) {
            editorSetStatusMessage("Save aborted");
            return;
        }
        editorSelectSyntaxHighlight();
    }

    int len;
    char *buf = editorRowsToString(&len);
    int fd = open(E.filename, O_RDWR | O_CREAT, 0644);
    if (fd != -1) {
        if (ftruncate(fd, len) != -1) {
            if (write(fd, buf, len) == len) {
                close(fd);
                free(buf);
                editorSetStatusMessage("%d bytes written to the disk", len);
                E.dirty = 0;
                return;
            }
        }
        close(fd);
    }
    free(buf);
    editorSetStatusMessage("Failed to write to disk, error: %s", strerror(errno));
}
