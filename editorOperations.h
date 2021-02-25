#ifndef EDITOR_OPERATIONS_INCLUDE
#define EDITOR_OPERATIONS_INCLUDE

void editorInsertChar(int c);
void editorDelChar(void);

void editorInsertRow(int at, char *s, size_t len);
void editorUpdateRow(struct erow *row);
void editorFreeRow(struct erow *row);
void editorDelRow(int at);
void editorRowInsertChar(struct erow *row, int at, int c);
void editorRowDelChar(struct erow *row, int at);
void editorRowAppendString(struct erow *row, char *s, size_t len);
void editorInsertNewline(void);
void editorIndent(struct erow *row);
int editorRowRxToCx(struct erow *row, int rx);
int editorRowCxToRx(struct erow *row, int cx);

#endif