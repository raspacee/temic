#ifndef EDITOR_TERMINAL_INCLUDE
#define EDITOR_TERMINAL_INCLUDE

void die(const char *s);
void enableRawMode(void);
void disableRawMode(void);
int editorReadKey(void);
int getWindowSize(int *rows, int *cols);

#endif
