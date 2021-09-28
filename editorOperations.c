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

    editorRowInsertChar(&E.row[E.cy], E.cx, c);
    E.cx++;
}

void editorDelChar(void)
{
	if (E.cx == 0 && E.cy == 0)
		return;

    struct erow *row = &E.row[E.cy];
    if (E.cx > 0) {
        editorRowDelChar(row, E.cx - 1);
        E.cx--;
    } else {
        E.cx = E.row[E.cy - 1].size;
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

void editorInsertNewline(bool increment_cy)
{
    E.widthlen = intLen(E.numrows);
	
    if (E.cx == 0) {
        editorInsertRow(E.cy, "", 0);
    } else {
        struct erow *row = &E.row[E.cy];

        editorInsertRow(E.cy + 1, &row->chars[E.cx], row->size - E.cx);
        row = &E.row[E.cy];
        row->size = E.cx;
        row->chars[row->size] = '\0';
        editorUpdateRow(row);
    }

    // For 'o', 'O' and other functionality
    if (increment_cy)
        E.cy++;

    E.cx = 0;
    E.coloff = 0;

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

void editorModeToggle(void)
{
  	if (E.filemode == NORMAL_MODE) {
		E.filemode = INSERT_MODE;
		
		if (E.argc == 1 && E.numrows == 0)
			editorInsertNewline(false);
	} else {
		E.filemode = NORMAL_MODE;
	}
}

bool isEditorNormalMode(void)
{
	return E.filemode == NORMAL_MODE;
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

void editorNextWordIndex(int cx, int cy)
{
    if (cx >= E.row[E.cy].size && cy < E.numrows) {
        cx = 0;
        cy++;

        E.cx = cx;
        E.cy = cy;
        return;
    }

    for (int row = cy; row < E.numrows; row++) {
        for (int cur_index = cx; cur_index < E.row[row].size - 1; cur_index++) {
            char next_char = E.row[row].chars[cur_index + 1];
            if (isspace(E.row[row].chars[cur_index]) && isgraph(next_char)) {
                E.cx = cur_index + 1;
                E.cy = row;
                return;
            }
        }
        cx = 0;

        if (isgraph(E.row[row].chars[cx])) {
            E.cx = cx;
            E.cy = row + 1;
            return;
        }
    }

    return;
}

void editorPrevWordIndex(int cx, int cy)
{
    if (cx <= 0 && cy > 0) {
        cx = E.row[E.cy - 1].size;
        cy--;

        E.cx = cx;
        E.cy = cy;
        return;
    }

    for (int row = cy; row >= 0; row--){
        for (int cur_index = cx - 1; cur_index > 1; cur_index--) {
            char cur_char = E.row[row].chars[cur_index];
            if (isspace(E.row[row].chars[cur_index - 1]) && (isalnum(cur_char) || ispunct(cur_char) || isgraph(cur_char))) {
                E.cx = cur_index;
                E.cy = row;
                return;
            }
        }
        cx = E.row[row - 1].size;

        E.cx = 0;
        return;
    }

    return;
}


void editorCalculateIndent(void)
{
    E.indent = 0;

    if (E.cy <= 1)
        return;

    int row, j;
    if (E.syntax->brace_end == 'N') {
        // For languages like Python that don't have ending braces
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
        // For languages with ending braces
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
