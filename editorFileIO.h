#ifndef EDITOR_FILEIO_INCLUDE
#define EDITOR_FILEIO_INCLUDE

void editorOpen(char *filename);
char *editorRowsToString(int *buflen);
void editorSave(void);

#endif