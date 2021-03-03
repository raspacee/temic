#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "data.h"
#include "editorOperations.h"
#include "editorSyntaxHL.h"

extern struct editorConfig E;

void editorInsertChar(int c)
{
    if (E.cy == E.numrows)
        editorInsertRow(E.numrows, "", 0);

    editorRowInsertChar(&E.row[E.cy], E.cx - E.widthlen - 1, c);
    E.cx++;
}

void editorDelChar(void)
{
    if (E.cy == E.numrows)
        return;
    if (E.cx == E.widthlen + 1 && E.cy == 0)
        return;

    struct erow *row = &E.row[E.cy];
    if (E.cx > E.widthlen + 1) {
        editorRowDelChar(row, E.cx - E.widthlen - 2);
        E.cx--;
    } else {
        E.cx = E.row[E.cy - 1].size + E.widthlen + 1;
        editorRowAppendString(&E.row[E.cy - 1], row->chars, row->size);
        editorDelRow(E.cy);
        E.cy--;
    }
}

void editorRowAppendString(struct erow *row, char *s, size_t len)
{
    row->chars = realloc(row->chars, row->size + len + 1);
    memcpy(&row->chars[row->size], s, len);
    row->size += len;
    row->chars[row->size] = '\0';
    editorUpdateRow(row);
    E.dirty++;
}

void editorInsertNewline(int caller)
{
    if (E.cx == E.widthlen + 1) {
        editorInsertRow(E.cy, "", 0);
    } else {
        struct erow *row = &E.row[E.cy];

        editorInsertRow(E.cy + 1, &row->chars[E.cx - (E.widthlen + 1)], row->size - E.cx + (E.widthlen + 1));
        row = &E.row[E.cy];
        row->size = E.cx - (E.widthlen + 1);
        row->chars[row->size] = '\0';
        editorUpdateRow(row);
    }
    E.cy++;
    E.cx = E.widthlen + 1;
    E.coloff = 0;

    // for o and O functionality
    if (caller == EDITOR_PROCESS_KEYPRESS)
        E.cy--;

    if (E.syntax) {
        editorCalculateIndent();
        if (E.indent) {
            for (int i = 0; i < E.indent; i++) {
                editorInsertChar(TAB);
            }
        }
    }
}

// row operations
void editorInsertRow(int at, char *s, size_t len)
{
    if (at < 0 || at > E.numrows)
        return;

    E.row = realloc(E.row, sizeof(struct erow) * (E.numrows + 1));
    memmove(&E.row[at + 1], &E.row[at], sizeof(struct erow) * (E.numrows - at));
    for (int j = at + 1; j <= E.numrows; j++)
        E.row[j].idx++;

    E.row[at].idx = at;
    E.row[at].chars = malloc(len + 1);
    E.row[at].size = len;
    memcpy(E.row[at].chars, s, len);
    E.row[at].chars[len] = '\0';

    E.row[at].rsize = 0;
    E.row[at].render = NULL;
    E.row[at].hl = NULL;
    E.row[at].hl_open_comment = 0;

    editorUpdateRow(&E.row[at]);

    E.numrows++;
    E.dirty++;
}

void editorUpdateRow(struct erow *row)
{
    int tabs = 0;
    int j;

    for (j = 0; j < row->size; j++) {
        if (row->chars[j] == '\t')
            tabs++;
    }

    free(row->render);
    row->render = malloc(row->size + tabs * (TEMIC_TAB_STOP - 1) + 1);

    int idx = 0;

    for (j = 0; j < row->size; j++) {
        if (row->chars[j] == '\t') {
            row->render[idx++] = ' ';
            while (idx % TEMIC_TAB_STOP != 0)
                row->render[idx++] = ' ';
        } else {
            row->render[idx++] = row->chars[j];
        }
    }
    row->render[idx] = '\0';
    row->rsize = idx;

    editorUpdateSyntax(row);
}

int editorRowCxToRx(struct erow *row, int cx)
{
    int j;
    int rx = 0;

    for (j = 0; j < cx; j++) {
        if (row->chars[j] == '\t') {
            rx += (TEMIC_TAB_STOP - 1) - (rx % TEMIC_TAB_STOP);
        }
        rx++;
    }
    return rx;
}

void editorRowInsertChar(struct erow *row, int at, int c)
{
    if (at < 0 || at > row->size)
        at = row->size;
    row->chars = realloc(row->chars, row->size + 2);
    memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
    row->size++;
    row->chars[at] = c;

    editorUpdateRow(row);
    E.dirty++;
}

void editorRowDelChar(struct erow *row, int at)
{
    if (at < 0 || at > row->size)
        at = row->size;
    memmove(&row->chars[at], &row->chars[at + 1], row->size - at);
    row->size--;
    editorUpdateRow(row);
    E.dirty++;
}

void editorFreeRow(struct erow *row)
{
    free(row->chars);
    free(row->render);
    free(row->hl);
}

void editorDelRow(int at)
{
    if (at < 0 || at >= E.numrows)
        return;
    editorFreeRow(&E.row[at]);
    memmove(&E.row[at], &E.row[at + 1], sizeof(struct erow) * (E.numrows - at - 1));
    for (int j = at; j < E.numrows - 1; j++)
        E.row[j].idx--;

    E.numrows--;
    E.dirty++;
}

int editorRowRxToCx(struct erow *row, int rx)
{
    int cur_rx = 0;
    int cx;

    for (cx = 0; cx < row->size; cx++) {
        if (row->chars[cx] == '\t') {
            cur_rx += (TEMIC_TAB_STOP - 1);
        }
        cur_rx++;

        if (cur_rx > rx)
            return cx;
    }

    return cx;
}

void editorCalculateIndent(void)
{
    E.indent = 0;

    if (E.cy <= 1)
        return;

    int row, j;
    if (E.syntax->brace_end == 'N') {
        int in_indent = 0;

        for (row = 1; row < E.cy; row++) {
            if (E.indent && (E.row[E.cy - 1].size == 0 || !isspace(E.row[E.cy - 1].chars[0]))) {
                E.indent = 0;
                in_indent = 0;
            }

            for (j = 0; j < E.row[row].size; j++) {
                if (E.row[row].chars[j] == E.syntax->brace_start && (j + 1 == E.row[row].size) && !in_indent) {
                    E.indent++;
                    in_indent = 1;
                }
            }
        }
    } else {
        for (row = 0; row < E.cy; row++) {
            for (j = 0; j < E.row[row].size; j++) {
                if (E.row[row].chars[j] == E.syntax->brace_start) {
                    E.indent++;
                } else if (E.row[row].chars[j] == E.syntax->brace_end) {
                    if (E.indent > 0) {
                        E.indent--;
                    }
                }
            }
        }
    }
}