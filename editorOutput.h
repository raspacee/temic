#ifndef EDITOR_OUTPUT_INCLUDE
#define EDITOR_OUTPUT_INCLUDE

void editorRefreshScreen(void);
void editorDrawRows(struct abuf *ab);
void editorDrawStatusBar(struct abuf *ab);
void editorSetStatusMessage(const char *fmt, ...);
void editorDrawMessageBar(struct abuf *ab);
void editorScroll(void);

#endif